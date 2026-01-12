// =============================================================================
// Deskillz SDK for Unreal Engine - Spectator Manager Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "Spectator/DeskillzSpectatorManager.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

UDeskillzSpectatorManager* UDeskillzSpectatorManager::Instance = nullptr;

UDeskillzSpectatorManager::UDeskillzSpectatorManager()
    : bIsInitialized(false)
    , bIsSpectating(false)
    , CurrentViewMode(EDeskillzSpectatorViewMode::TableView)
    , PlaybackSpeed(1.0f)
    , bIsPlaying(true)
{
}

UDeskillzSpectatorManager* UDeskillzSpectatorManager::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzSpectatorManager>();
        Instance->AddToRoot();
    }
    return Instance;
}

void UDeskillzSpectatorManager::Initialize()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DeskillzSpectatorManager already initialized"));
        return;
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("DeskillzSpectatorManager initialized"));
}

void UDeskillzSpectatorManager::Shutdown()
{
    if (!bIsInitialized) return;

    if (bIsSpectating)
    {
        LeaveSpectatorMode();
    }

    AvailableRooms.Empty();
    bIsInitialized = false;
    
    UE_LOG(LogTemp, Log, TEXT("DeskillzSpectatorManager shutdown"));
}

// =============================================================================
// Room Discovery
// =============================================================================

void UDeskillzSpectatorManager::FetchSpectatorRooms(const FDeskillzSpectatorRoomFilter& Filter)
{
    if (!bIsInitialized)
    {
        OnRoomsFetched.Broadcast(TArray<FDeskillzSpectatorRoomInfo>());
        return;
    }

    // Build query parameters
    TSharedPtr<FJsonObject> QueryParams = MakeShareable(new FJsonObject);
    
    if (!Filter.GameId.IsEmpty())
        QueryParams->SetStringField(TEXT("gameId"), Filter.GameId);
    if (Filter.MinPlayers > 0)
        QueryParams->SetNumberField(TEXT("minPlayers"), Filter.MinPlayers);
    if (Filter.MaxPlayers > 0)
        QueryParams->SetNumberField(TEXT("maxPlayers"), Filter.MaxPlayers);
    if (Filter.MinStakes > 0)
        QueryParams->SetNumberField(TEXT("minStakes"), Filter.MinStakes);
    if (Filter.bActiveOnly)
        QueryParams->SetBoolField(TEXT("activeOnly"), true);

    // Send API request
    SendSpectatorApiRequest(TEXT("spectator/rooms"), TEXT("GET"), QueryParams,
        [this](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess && Response.IsValid())
            {
                TArray<FDeskillzSpectatorRoomInfo> Rooms;
                ParseRoomList(Response, Rooms);
                
                AvailableRooms.Empty();
                for (const FDeskillzSpectatorRoomInfo& Room : Rooms)
                {
                    AvailableRooms.Add(Room.RoomId, Room);
                }
                
                OnRoomsFetched.Broadcast(Rooms);
            }
            else
            {
                OnRoomsFetched.Broadcast(TArray<FDeskillzSpectatorRoomInfo>());
            }
        });
}

void UDeskillzSpectatorManager::SearchRooms(const FString& SearchQuery)
{
    TArray<FDeskillzSpectatorRoomInfo> Results;
    
    for (const auto& Pair : AvailableRooms)
    {
        const FDeskillzSpectatorRoomInfo& Room = Pair.Value;
        
        if (Room.RoomName.Contains(SearchQuery, ESearchCase::IgnoreCase) ||
            Room.HostName.Contains(SearchQuery, ESearchCase::IgnoreCase) ||
            Room.GameName.Contains(SearchQuery, ESearchCase::IgnoreCase))
        {
            Results.Add(Room);
        }
    }
    
    OnRoomSearchResults.Broadcast(Results);
}

TArray<FDeskillzSpectatorRoomInfo> UDeskillzSpectatorManager::GetAvailableRooms() const
{
    TArray<FDeskillzSpectatorRoomInfo> Result;
    AvailableRooms.GenerateValueArray(Result);
    return Result;
}

FDeskillzSpectatorRoomInfo UDeskillzSpectatorManager::GetRoomInfo(const FString& RoomId) const
{
    if (const FDeskillzSpectatorRoomInfo* Room = AvailableRooms.Find(RoomId))
    {
        return *Room;
    }
    return FDeskillzSpectatorRoomInfo();
}

// =============================================================================
// Spectator Mode
// =============================================================================

void UDeskillzSpectatorManager::JoinAsSpectator(const FString& RoomId)
{
    if (!bIsInitialized)
    {
        OnSpectatorError.Broadcast(TEXT("Manager not initialized"));
        return;
    }

    if (bIsSpectating)
    {
        // Leave current room first
        LeaveSpectatorMode();
    }

    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetStringField(TEXT("roomId"), RoomId);

    SendSpectatorApiRequest(TEXT("spectator/join"), TEXT("POST"), JsonRequest,
        [this, RoomId](bool bSuccess, TSharedPtr<FJsonObject> Response)
        {
            if (bSuccess && Response.IsValid())
            {
                CurrentRoomId = RoomId;
                bIsSpectating = true;
                
                // Parse initial room state
                ParseRoomState(Response);
                
                // Connect to WebSocket for real-time updates
                ConnectToRoomStream(RoomId);
                
                OnJoinedAsSpectator.Broadcast(RoomId);
                UE_LOG(LogTemp, Log, TEXT("Joined room %s as spectator"), *RoomId);
            }
            else
            {
                FString ErrorMsg = Response.IsValid() ? 
                    Response->GetStringField(TEXT("error")) : TEXT("Failed to join as spectator");
                OnSpectatorError.Broadcast(ErrorMsg);
            }
        });
}

void UDeskillzSpectatorManager::LeaveSpectatorMode()
{
    if (!bIsSpectating)
    {
        return;
    }

    // Disconnect from WebSocket
    DisconnectFromRoomStream();

    // Notify server
    TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
    JsonRequest->SetStringField(TEXT("roomId"), CurrentRoomId);

    SendSpectatorApiRequest(TEXT("spectator/leave"), TEXT("POST"), JsonRequest, nullptr);

    FString LeftRoomId = CurrentRoomId;
    
    // Reset state
    bIsSpectating = false;
    CurrentRoomId = TEXT("");
    CurrentRoomState = FDeskillzSpectatorRoomState();
    FollowedPlayerId = TEXT("");
    CurrentViewMode = EDeskillzSpectatorViewMode::TableView;

    OnLeftSpectatorMode.Broadcast(LeftRoomId);
    UE_LOG(LogTemp, Log, TEXT("Left spectator mode for room %s"), *LeftRoomId);
}

void UDeskillzSpectatorManager::SwitchRoom(const FString& NewRoomId)
{
    if (NewRoomId == CurrentRoomId)
    {
        return;
    }

    FString OldRoomId = CurrentRoomId;
    
    // Leave current room and join new one
    LeaveSpectatorMode();
    JoinAsSpectator(NewRoomId);

    OnRoomSwitched.Broadcast(OldRoomId, NewRoomId);
}

bool UDeskillzSpectatorManager::IsSpectating() const
{
    return bIsSpectating;
}

FString UDeskillzSpectatorManager::GetCurrentRoomId() const
{
    return CurrentRoomId;
}

// =============================================================================
// View Control
// =============================================================================

void UDeskillzSpectatorManager::SetViewMode(EDeskillzSpectatorViewMode Mode)
{
    if (CurrentViewMode != Mode)
    {
        EDeskillzSpectatorViewMode OldMode = CurrentViewMode;
        CurrentViewMode = Mode;
        
        if (Mode != EDeskillzSpectatorViewMode::FollowPlayer)
        {
            FollowedPlayerId = TEXT("");
        }
        
        OnViewModeChanged.Broadcast(OldMode, Mode);
    }
}

EDeskillzSpectatorViewMode UDeskillzSpectatorManager::GetViewMode() const
{
    return CurrentViewMode;
}

void UDeskillzSpectatorManager::FollowPlayer(const FString& PlayerId)
{
    if (!bIsSpectating)
    {
        return;
    }

    // Verify player is in the room
    bool bPlayerFound = false;
    for (const FDeskillzSpectatorPlayerState& Player : CurrentRoomState.Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            bPlayerFound = true;
            break;
        }
    }

    if (!bPlayerFound)
    {
        OnSpectatorError.Broadcast(TEXT("Player not found in room"));
        return;
    }

    FollowedPlayerId = PlayerId;
    SetViewMode(EDeskillzSpectatorViewMode::FollowPlayer);
    OnPlayerFollowed.Broadcast(PlayerId);
}

void UDeskillzSpectatorManager::StopFollowingPlayer()
{
    FollowedPlayerId = TEXT("");
    SetViewMode(EDeskillzSpectatorViewMode::TableView);
}

FString UDeskillzSpectatorManager::GetFollowedPlayerId() const
{
    return FollowedPlayerId;
}

void UDeskillzSpectatorManager::CycleToNextPlayer()
{
    if (!bIsSpectating || CurrentRoomState.Players.Num() == 0)
    {
        return;
    }

    int32 CurrentIndex = -1;
    for (int32 i = 0; i < CurrentRoomState.Players.Num(); ++i)
    {
        if (CurrentRoomState.Players[i].PlayerId == FollowedPlayerId)
        {
            CurrentIndex = i;
            break;
        }
    }

    int32 NextIndex = (CurrentIndex + 1) % CurrentRoomState.Players.Num();
    FollowPlayer(CurrentRoomState.Players[NextIndex].PlayerId);
}

void UDeskillzSpectatorManager::CycleToPreviousPlayer()
{
    if (!bIsSpectating || CurrentRoomState.Players.Num() == 0)
    {
        return;
    }

    int32 CurrentIndex = 0;
    for (int32 i = 0; i < CurrentRoomState.Players.Num(); ++i)
    {
        if (CurrentRoomState.Players[i].PlayerId == FollowedPlayerId)
        {
            CurrentIndex = i;
            break;
        }
    }

    int32 PrevIndex = (CurrentIndex - 1 + CurrentRoomState.Players.Num()) % CurrentRoomState.Players.Num();
    FollowPlayer(CurrentRoomState.Players[PrevIndex].PlayerId);
}

// =============================================================================
// Playback Control
// =============================================================================

void UDeskillzSpectatorManager::SetPlaybackSpeed(float Speed)
{
    PlaybackSpeed = FMath::Clamp(Speed, MinPlaybackSpeed, MaxPlaybackSpeed);
    OnPlaybackSpeedChanged.Broadcast(PlaybackSpeed);
}

float UDeskillzSpectatorManager::GetPlaybackSpeed() const
{
    return PlaybackSpeed;
}

void UDeskillzSpectatorManager::PausePlayback()
{
    if (bIsPlaying)
    {
        bIsPlaying = false;
        OnPlaybackPaused.Broadcast();
    }
}

void UDeskillzSpectatorManager::ResumePlayback()
{
    if (!bIsPlaying)
    {
        bIsPlaying = true;
        OnPlaybackResumed.Broadcast();
    }
}

void UDeskillzSpectatorManager::TogglePlayback()
{
    if (bIsPlaying)
    {
        PausePlayback();
    }
    else
    {
        ResumePlayback();
    }
}

bool UDeskillzSpectatorManager::IsPlaying() const
{
    return bIsPlaying;
}

// =============================================================================
// Game State
// =============================================================================

FDeskillzSpectatorRoomState UDeskillzSpectatorManager::GetCurrentRoomState() const
{
    return CurrentRoomState;
}

TArray<FDeskillzSpectatorPlayerState> UDeskillzSpectatorManager::GetPlayerStates() const
{
    return CurrentRoomState.Players;
}

FDeskillzSpectatorPlayerState UDeskillzSpectatorManager::GetPlayerState(const FString& PlayerId) const
{
    for (const FDeskillzSpectatorPlayerState& Player : CurrentRoomState.Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            return Player;
        }
    }
    return FDeskillzSpectatorPlayerState();
}

void UDeskillzSpectatorManager::ProcessGameEvent(const FDeskillzSpectatorGameEvent& Event)
{
    // Apply event to current state
    switch (Event.EventType)
    {
        case EDeskillzSpectatorEventType::RoundStart:
            CurrentRoomState.CurrentRound = Event.RoundNumber;
            CurrentRoomState.CurrentPhase = TEXT("Started");
            OnRoundStarted.Broadcast(Event.RoundNumber);
            break;

        case EDeskillzSpectatorEventType::RoundEnd:
            OnRoundEnded.Broadcast(Event.RoundNumber, Event.WinnerId);
            break;

        case EDeskillzSpectatorEventType::PlayerAction:
            OnPlayerAction.Broadcast(Event.PlayerId, Event.ActionType, Event.ActionValue);
            UpdatePlayerState(Event.PlayerId, Event.ActionType, Event.ActionValue);
            break;

        case EDeskillzSpectatorEventType::PotUpdate:
            CurrentRoomState.CurrentPot = Event.PotAmount;
            OnPotUpdated.Broadcast(Event.PotAmount);
            break;

        case EDeskillzSpectatorEventType::PlayerJoined:
            AddPlayerToState(Event.PlayerId, Event.PlayerName);
            OnPlayerJoined.Broadcast(Event.PlayerId);
            break;

        case EDeskillzSpectatorEventType::PlayerLeft:
            RemovePlayerFromState(Event.PlayerId);
            OnPlayerLeft.Broadcast(Event.PlayerId);
            break;

        case EDeskillzSpectatorEventType::GamePaused:
            CurrentRoomState.bIsPaused = true;
            OnGamePaused.Broadcast();
            break;

        case EDeskillzSpectatorEventType::GameResumed:
            CurrentRoomState.bIsPaused = false;
            OnGameResumed.Broadcast();
            break;
    }

    // Add to event history
    EventHistory.Add(Event);
    while (EventHistory.Num() > MaxEventHistorySize)
    {
        EventHistory.RemoveAt(0);
    }
}

// =============================================================================
// Statistics
// =============================================================================

FDeskillzSpectatorStats UDeskillzSpectatorManager::GetSpectatorStats() const
{
    FDeskillzSpectatorStats Stats;
    
    if (const FDeskillzSpectatorRoomInfo* RoomInfo = AvailableRooms.Find(CurrentRoomId))
    {
        Stats.SpectatorCount = RoomInfo->SpectatorCount;
    }
    
    Stats.CurrentRound = CurrentRoomState.CurrentRound;
    Stats.TotalPot = CurrentRoomState.CurrentPot;
    Stats.PlayerCount = CurrentRoomState.Players.Num();
    Stats.SessionDuration = FDateTime::UtcNow() - SpectatingStartTime;
    
    return Stats;
}

int32 UDeskillzSpectatorManager::GetSpectatorCount() const
{
    if (const FDeskillzSpectatorRoomInfo* RoomInfo = AvailableRooms.Find(CurrentRoomId))
    {
        return RoomInfo->SpectatorCount;
    }
    return 0;
}

// =============================================================================
// Private Methods
// =============================================================================

void UDeskillzSpectatorManager::ConnectToRoomStream(const FString& RoomId)
{
    // WebSocket connection for real-time updates
    // Implementation would use WebSocket module
    
    SpectatingStartTime = FDateTime::UtcNow();
    UE_LOG(LogTemp, Log, TEXT("Connected to room stream: %s"), *RoomId);
}

void UDeskillzSpectatorManager::DisconnectFromRoomStream()
{
    // Disconnect WebSocket
    UE_LOG(LogTemp, Log, TEXT("Disconnected from room stream"));
}

void UDeskillzSpectatorManager::ParseRoomList(TSharedPtr<FJsonObject> JsonData, TArray<FDeskillzSpectatorRoomInfo>& OutRooms)
{
    if (!JsonData.IsValid()) return;

    const TArray<TSharedPtr<FJsonValue>>* RoomsArray;
    if (JsonData->TryGetArrayField(TEXT("rooms"), RoomsArray))
    {
        for (const TSharedPtr<FJsonValue>& RoomValue : *RoomsArray)
        {
            TSharedPtr<FJsonObject> RoomObj = RoomValue->AsObject();
            if (RoomObj.IsValid())
            {
                FDeskillzSpectatorRoomInfo Room;
                Room.RoomId = RoomObj->GetStringField(TEXT("roomId"));
                Room.RoomName = RoomObj->GetStringField(TEXT("roomName"));
                Room.GameId = RoomObj->GetStringField(TEXT("gameId"));
                Room.GameName = RoomObj->GetStringField(TEXT("gameName"));
                Room.HostId = RoomObj->GetStringField(TEXT("hostId"));
                Room.HostName = RoomObj->GetStringField(TEXT("hostName"));
                Room.CurrentPlayers = RoomObj->GetIntegerField(TEXT("currentPlayers"));
                Room.MaxPlayers = RoomObj->GetIntegerField(TEXT("maxPlayers"));
                Room.SpectatorCount = RoomObj->GetIntegerField(TEXT("spectatorCount"));
                Room.CurrentPot = RoomObj->GetNumberField(TEXT("currentPot"));
                Room.MinBuyIn = RoomObj->GetNumberField(TEXT("minBuyIn"));
                Room.bIsActive = RoomObj->GetBoolField(TEXT("isActive"));
                Room.bCanJoinAsPlayer = RoomObj->GetBoolField(TEXT("canJoinAsPlayer"));
                
                OutRooms.Add(Room);
            }
        }
    }
}

void UDeskillzSpectatorManager::ParseRoomState(TSharedPtr<FJsonObject> JsonData)
{
    if (!JsonData.IsValid()) return;

    CurrentRoomState.RoomId = JsonData->GetStringField(TEXT("roomId"));
    CurrentRoomState.CurrentRound = JsonData->GetIntegerField(TEXT("currentRound"));
    CurrentRoomState.CurrentPhase = JsonData->GetStringField(TEXT("currentPhase"));
    CurrentRoomState.CurrentPot = JsonData->GetNumberField(TEXT("currentPot"));
    CurrentRoomState.CurrentTurnPlayerId = JsonData->GetStringField(TEXT("currentTurnPlayerId"));
    CurrentRoomState.bIsPaused = JsonData->GetBoolField(TEXT("isPaused"));

    // Parse players
    CurrentRoomState.Players.Empty();
    const TArray<TSharedPtr<FJsonValue>>* PlayersArray;
    if (JsonData->TryGetArrayField(TEXT("players"), PlayersArray))
    {
        for (const TSharedPtr<FJsonValue>& PlayerValue : *PlayersArray)
        {
            TSharedPtr<FJsonObject> PlayerObj = PlayerValue->AsObject();
            if (PlayerObj.IsValid())
            {
                FDeskillzSpectatorPlayerState Player;
                Player.PlayerId = PlayerObj->GetStringField(TEXT("playerId"));
                Player.DisplayName = PlayerObj->GetStringField(TEXT("displayName"));
                Player.ChipCount = PlayerObj->GetNumberField(TEXT("chipCount"));
                Player.CurrentBet = PlayerObj->GetNumberField(TEXT("currentBet"));
                Player.bIsActive = PlayerObj->GetBoolField(TEXT("isActive"));
                Player.bIsTurn = PlayerObj->GetBoolField(TEXT("isTurn"));
                Player.SeatPosition = PlayerObj->GetIntegerField(TEXT("seatPosition"));
                
                CurrentRoomState.Players.Add(Player);
            }
        }
    }
}

void UDeskillzSpectatorManager::UpdatePlayerState(const FString& PlayerId, const FString& ActionType, float Value)
{
    for (FDeskillzSpectatorPlayerState& Player : CurrentRoomState.Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            if (ActionType == TEXT("bet") || ActionType == TEXT("raise"))
            {
                Player.CurrentBet += Value;
                Player.ChipCount -= Value;
            }
            else if (ActionType == TEXT("fold"))
            {
                Player.bIsActive = false;
            }
            else if (ActionType == TEXT("win"))
            {
                Player.ChipCount += Value;
            }
            break;
        }
    }
}

void UDeskillzSpectatorManager::AddPlayerToState(const FString& PlayerId, const FString& PlayerName)
{
    FDeskillzSpectatorPlayerState NewPlayer;
    NewPlayer.PlayerId = PlayerId;
    NewPlayer.DisplayName = PlayerName;
    NewPlayer.ChipCount = 0.0f;
    NewPlayer.CurrentBet = 0.0f;
    NewPlayer.bIsActive = true;
    NewPlayer.bIsTurn = false;
    
    CurrentRoomState.Players.Add(NewPlayer);
}

void UDeskillzSpectatorManager::RemovePlayerFromState(const FString& PlayerId)
{
    CurrentRoomState.Players.RemoveAll([PlayerId](const FDeskillzSpectatorPlayerState& Player)
    {
        return Player.PlayerId == PlayerId;
    });
}

void UDeskillzSpectatorManager::SendSpectatorApiRequest(const FString& Endpoint, const FString& Method,
    TSharedPtr<FJsonObject> Body, TFunction<void(bool, TSharedPtr<FJsonObject>)> Callback)
{
    // Implementation would use HTTP module
    FString Url = FString::Printf(TEXT("%s/api/v1/%s"), *ApiBaseUrl, *Endpoint);
    
    UE_LOG(LogTemp, Log, TEXT("Spectator API Request: %s %s"), *Method, *Url);
    
    // In production, this would make actual HTTP requests
    // For now, callback with simulated success if provided
    if (Callback)
    {
        TSharedPtr<FJsonObject> EmptyResponse = MakeShareable(new FJsonObject);
        Callback(true, EmptyResponse);
    }
}