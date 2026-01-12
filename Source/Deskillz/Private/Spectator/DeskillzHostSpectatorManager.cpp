// =============================================================================
// Deskillz SDK for Unreal Engine - Host Spectator Manager Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Only room hosts can spectate their own private social rooms.
// General public spectating is NOT available.
// Hosts can see game board and scores but NOT player hands (anti-cheat).
// =============================================================================

#include "Host/DeskillzHostSpectatorManager.h"
#include "Core/DeskillzSDK.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// ============================================================================
// Error Code Definitions
// ============================================================================

const FString FDeskillzHostSpectatorError::CODE_NOT_AUTHENTICATED = TEXT("NOT_AUTHENTICATED");
const FString FDeskillzHostSpectatorError::CODE_NOT_AUTHORIZED = TEXT("NOT_AUTHORIZED");
const FString FDeskillzHostSpectatorError::CODE_ROOM_NOT_FOUND = TEXT("ROOM_NOT_FOUND");
const FString FDeskillzHostSpectatorError::CODE_NOT_YOUR_ROOM = TEXT("NOT_YOUR_ROOM");
const FString FDeskillzHostSpectatorError::CODE_NOT_SOCIAL_ROOM = TEXT("NOT_SOCIAL_ROOM");
const FString FDeskillzHostSpectatorError::CODE_ALREADY_CONNECTING = TEXT("ALREADY_CONNECTING");
const FString FDeskillzHostSpectatorError::CODE_MAX_ROOMS_REACHED = TEXT("MAX_ROOMS_REACHED");
const FString FDeskillzHostSpectatorError::CODE_NETWORK_ERROR = TEXT("NETWORK_ERROR");
const FString FDeskillzHostSpectatorError::CODE_SERVER_ERROR = TEXT("SERVER_ERROR");

// ============================================================================
// Constructor
// ============================================================================

UDeskillzHostSpectatorManager::UDeskillzHostSpectatorManager()
    : bIsInitialized(false)
    , bIsSpectating(false)
    , ConnectionState(EDeskillzHostSpectatorState::Disconnected)
    , ReconnectAttempts(0)
{
}

// ============================================================================
// Subsystem Lifecycle
// ============================================================================

void UDeskillzHostSpectatorManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Subsystem initialized"));
}

void UDeskillzHostSpectatorManager::Deinitialize()
{
    if (bIsInitialized)
    {
        ShutdownHostSpectator();
    }
    
    Super::Deinitialize();
}

bool UDeskillzHostSpectatorManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

// ============================================================================
// Static Accessor
// ============================================================================

UDeskillzHostSpectatorManager* UDeskillzHostSpectatorManager::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UDeskillzHostSpectatorManager>();
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzHostSpectatorManager::InitializeHostSpectator()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HostSpectatorManager] Already initialized"));
        return;
    }

    // Get SDK reference
    SDK = UDeskillzSDK::Get(this);
    if (!SDK)
    {
        UE_LOG(LogTemp, Error, TEXT("[HostSpectatorManager] SDK not available"));
        OnError.Broadcast(FDeskillzHostSpectatorError(
            FDeskillzHostSpectatorError::CODE_NOT_AUTHENTICATED,
            TEXT("Deskillz SDK not initialized")
        ));
        return;
    }

    // Verify host is authenticated (would check HostManager in real implementation)
    // For now, we trust the SDK is properly initialized

    bIsInitialized = true;
    ConnectionState = EDeskillzHostSpectatorState::Disconnected;
    WatchedRooms.Empty();
    CachedRooms.Empty();

    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Initialized for host-only spectating"));
}

void UDeskillzHostSpectatorManager::ShutdownHostSpectator()
{
    if (!bIsInitialized)
    {
        return;
    }

    // Stop all spectating
    StopSpectatingAll();

    // Clear state
    CachedRooms.Empty();
    SDK = nullptr;
    bIsInitialized = false;

    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Shutdown complete"));
}

// ============================================================================
// Room Discovery
// ============================================================================

void UDeskillzHostSpectatorManager::FetchHostRooms(const FDeskillzHostRoomFilter& Filter)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("[HostSpectatorManager] Not initialized"));
        OnRoomsFetched.Broadcast(TArray<FDeskillzHostRoom>());
        return;
    }

    // Build API request
    // In real implementation, this would call the API
    // For now, we simulate the response

    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Fetching host rooms (GameCategory=%s, Active=%s)"),
        Filter.GameCategory == EDeskillzGameCategory::Social ? TEXT("Social") : TEXT("Esports"),
        Filter.bIsActive ? TEXT("true") : TEXT("false"));

    // TODO: Implement actual API call to /api/v1/host/spectator/my-rooms
    // The API will only return rooms that YOU created

    // Simulate empty response for now
    CachedRooms.Empty();
    OnRoomsFetched.Broadcast(CachedRooms);
}

// ============================================================================
// Spectating
// ============================================================================

void UDeskillzHostSpectatorManager::SpectateRoom(const FString& RoomId)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("[HostSpectatorManager] Not initialized"));
        OnError.Broadcast(FDeskillzHostSpectatorError(
            FDeskillzHostSpectatorError::CODE_NOT_AUTHENTICATED,
            TEXT("Host Spectator Manager not initialized")
        ));
        return;
    }

    if (ConnectionState == EDeskillzHostSpectatorState::Connecting)
    {
        OnError.Broadcast(FDeskillzHostSpectatorError(
            FDeskillzHostSpectatorError::CODE_ALREADY_CONNECTING,
            TEXT("Already connecting to a room")
        ));
        return;
    }

    // Check if already watching this room
    for (const FDeskillzHostSpectatorSession& Session : WatchedRooms)
    {
        if (Session.RoomId == RoomId)
        {
            // Switch to this room as primary
            CurrentSession = Session;
            OnRoomSwitched.Broadcast(Session);
            UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Switched to already-watched room: %s"), *RoomId);
            return;
        }
    }

    // Check max rooms
    if (WatchedRooms.Num() >= MAX_WATCHED_ROOMS)
    {
        OnError.Broadcast(FDeskillzHostSpectatorError(
            FDeskillzHostSpectatorError::CODE_MAX_ROOMS_REACHED,
            FString::Printf(TEXT("Maximum %d rooms can be watched simultaneously"), MAX_WATCHED_ROOMS)
        ));
        return;
    }

    SetConnectionState(EDeskillzHostSpectatorState::Connecting);

    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Spectating room: %s"), *RoomId);

    // TODO: Implement actual API call to /api/v1/host/spectator/spectate
    // The API will verify you are the host of this room and it's a social room

    // For now, simulate successful connection
    FDeskillzHostSpectatorSession NewSession;
    NewSession.SessionId = FGuid::NewGuid().ToString();
    NewSession.RoomId = RoomId;
    NewSession.RoomName = TEXT("Your Room");
    NewSession.RoomCode = TEXT("DSKZ-HOST");
    NewSession.StartedAt = FDateTime::Now();

    CurrentSession = NewSession;
    WatchedRooms.Add(NewSession);
    bIsSpectating = true;
    ReconnectAttempts = 0;

    SetConnectionState(EDeskillzHostSpectatorState::Connected);

    // Connect WebSocket for real-time updates
    ConnectWebSocket(RoomId);

    OnConnected.Broadcast(NewSession);
}

void UDeskillzHostSpectatorManager::StopSpectating()
{
    if (!bIsSpectating || CurrentSession.RoomId.IsEmpty())
    {
        return;
    }

    FString RoomId = CurrentSession.RoomId;
    StopSpectatingRoom(RoomId);
}

void UDeskillzHostSpectatorManager::StopSpectatingRoom(const FString& RoomId)
{
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Stopping spectate for room: %s"), *RoomId);

    // Remove from watched rooms
    RemoveWatchedRoom(RoomId);

    // If this was the current session, switch to another or disconnect
    if (CurrentSession.RoomId == RoomId)
    {
        if (WatchedRooms.Num() > 0)
        {
            CurrentSession = WatchedRooms[0];
            OnRoomSwitched.Broadcast(CurrentSession);
        }
        else
        {
            CurrentSession = FDeskillzHostSpectatorSession();
            bIsSpectating = false;
            SetConnectionState(EDeskillzHostSpectatorState::Disconnected);
        }
    }

    OnDisconnected.Broadcast(RoomId);
}

void UDeskillzHostSpectatorManager::StopSpectatingAll()
{
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Stopping all spectating"));

    // Disconnect WebSocket
    DisconnectWebSocket();

    // Clear all watched rooms
    TArray<FString> RoomIds;
    for (const FDeskillzHostSpectatorSession& Session : WatchedRooms)
    {
        RoomIds.Add(Session.RoomId);
    }

    WatchedRooms.Empty();
    CurrentSession = FDeskillzHostSpectatorSession();
    bIsSpectating = false;
    SetConnectionState(EDeskillzHostSpectatorState::Disconnected);

    // Broadcast disconnect for each room
    for (const FString& RoomId : RoomIds)
    {
        OnDisconnected.Broadcast(RoomId);
    }
}

void UDeskillzHostSpectatorManager::SwitchRoom(const FString& RoomId)
{
    // Check if room is in watched list
    for (const FDeskillzHostSpectatorSession& Session : WatchedRooms)
    {
        if (Session.RoomId == RoomId)
        {
            CurrentSession = Session;
            OnRoomSwitched.Broadcast(Session);
            UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Switched to room: %s"), *Session.RoomName);
            return;
        }
    }

    // Room not in watched list, try to spectate it
    SpectateRoom(RoomId);
}

// ============================================================================
// State Management
// ============================================================================

void UDeskillzHostSpectatorManager::RefreshState()
{
    if (!bIsSpectating || CurrentSession.RoomId.IsEmpty())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Refreshing state for room: %s"), *CurrentSession.RoomId);

    // TODO: Implement actual API call to refresh game state
}

void UDeskillzHostSpectatorManager::SetConnectionState(EDeskillzHostSpectatorState NewState)
{
    if (ConnectionState != NewState)
    {
        ConnectionState = NewState;
        OnStateChanged.Broadcast(NewState);
    }
}

void UDeskillzHostSpectatorManager::RemoveWatchedRoom(const FString& RoomId)
{
    WatchedRooms.RemoveAll([&RoomId](const FDeskillzHostSpectatorSession& Session) {
        return Session.RoomId == RoomId;
    });
}

// ============================================================================
// WebSocket
// ============================================================================

void UDeskillzHostSpectatorManager::ConnectWebSocket(const FString& RoomId)
{
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] WebSocket connecting for room: %s"), *RoomId);

    // TODO: Implement actual WebSocket connection to receive real-time updates
    // WebSocket endpoint: wss://api.deskillz.games/api/v1/host/spectator/ws/{roomId}
    // Events: game_state, score_update, turn_change, round_start, round_end, chat, pause, resume, game_end
}

void UDeskillzHostSpectatorManager::DisconnectWebSocket()
{
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] WebSocket disconnected"));

    // TODO: Implement actual WebSocket disconnection
}

void UDeskillzHostSpectatorManager::HandleWebSocketMessage(const FString& Message)
{
    // Parse JSON message and dispatch appropriate events
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[HostSpectatorManager] Failed to parse WebSocket message"));
        return;
    }

    FString EventType;
    if (!JsonObject->TryGetStringField(TEXT("type"), EventType))
    {
        return;
    }

    if (EventType == TEXT("game_state"))
    {
        // Parse and broadcast game state update
        // NOTE: Server ensures player hands are NOT included
        FDeskillzHostGameState State;
        // ... parse state from JSON ...
        OnGameStateUpdated.Broadcast(State);
    }
    else if (EventType == TEXT("score_update"))
    {
        FDeskillzHostScoreUpdate Update;
        // ... parse update from JSON ...
        OnScoreUpdated.Broadcast(Update);
    }
    else if (EventType == TEXT("turn_change"))
    {
        FString PlayerId;
        if (JsonObject->TryGetStringField(TEXT("playerId"), PlayerId))
        {
            OnTurnChanged.Broadcast(PlayerId);
        }
    }
    else if (EventType == TEXT("round_start"))
    {
        int32 RoundNumber = JsonObject->GetIntegerField(TEXT("roundNumber"));
        OnRoundStarted.Broadcast(RoundNumber);
    }
    else if (EventType == TEXT("round_end"))
    {
        FDeskillzHostRoundResult Result;
        // ... parse result from JSON ...
        OnRoundEnded.Broadcast(Result);
    }
    else if (EventType == TEXT("chat"))
    {
        FDeskillzHostChatMessage ChatMessage;
        // ... parse message from JSON ...
        OnChatReceived.Broadcast(ChatMessage);
    }
    else if (EventType == TEXT("pause"))
    {
        float Duration = JsonObject->GetNumberField(TEXT("duration"));
        OnGamePaused.Broadcast(Duration);
    }
    else if (EventType == TEXT("resume"))
    {
        OnGameResumed.Broadcast();
    }
    else if (EventType == TEXT("game_end"))
    {
        FDeskillzHostGameEndResult Result;
        // ... parse result from JSON ...
        OnGameEnded.Broadcast(Result);
    }
}

void UDeskillzHostSpectatorManager::AttemptReconnect()
{
    if (ReconnectAttempts >= RECONNECT_MAX_ATTEMPTS)
    {
        UE_LOG(LogTemp, Error, TEXT("[HostSpectatorManager] Max reconnect attempts reached"));
        SetConnectionState(EDeskillzHostSpectatorState::Error);
        OnError.Broadcast(FDeskillzHostSpectatorError(
            FDeskillzHostSpectatorError::CODE_NETWORK_ERROR,
            TEXT("Failed to reconnect after multiple attempts")
        ));
        return;
    }

    ReconnectAttempts++;
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorManager] Reconnect attempt %d/%d"), 
        ReconnectAttempts, RECONNECT_MAX_ATTEMPTS);

    // TODO: Implement actual reconnection logic with delay
    SetConnectionState(EDeskillzHostSpectatorState::Connecting);
}

// ============================================================================
// Event Component Implementation
// ============================================================================

UDeskillzHostSpectatorEventComponent::UDeskillzHostSpectatorEventComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDeskillzHostSpectatorEventComponent::BeginPlay()
{
    Super::BeginPlay();
    SubscribeToEvents();
}

void UDeskillzHostSpectatorEventComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnsubscribeFromEvents();
    Super::EndPlay(EndPlayReason);
}

void UDeskillzHostSpectatorEventComponent::SubscribeToEvents()
{
    UDeskillzHostSpectatorManager* Manager = UDeskillzHostSpectatorManager::Get(this);
    if (!Manager)
    {
        return;
    }

    Manager->OnConnected.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleConnected);
    Manager->OnDisconnected.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleDisconnected);
    Manager->OnStateChanged.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleStateChanged);
    Manager->OnRoomsFetched.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoomsFetched);
    Manager->OnRoomSwitched.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoomSwitched);
    Manager->OnGameStateUpdated.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGameStateUpdated);
    Manager->OnScoreUpdated.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleScoreUpdated);
    Manager->OnTurnChanged.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleTurnChanged);
    Manager->OnRoundStarted.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoundStarted);
    Manager->OnRoundEnded.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoundEnded);
    Manager->OnChatReceived.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleChatReceived);
    Manager->OnGamePaused.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGamePaused);
    Manager->OnGameResumed.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGameResumed);
    Manager->OnGameEnded.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGameEnded);
    Manager->OnError.AddDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleError);
}

void UDeskillzHostSpectatorEventComponent::UnsubscribeFromEvents()
{
    UDeskillzHostSpectatorManager* Manager = UDeskillzHostSpectatorManager::Get(this);
    if (!Manager)
    {
        return;
    }

    Manager->OnConnected.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleConnected);
    Manager->OnDisconnected.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleDisconnected);
    Manager->OnStateChanged.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleStateChanged);
    Manager->OnRoomsFetched.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoomsFetched);
    Manager->OnRoomSwitched.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoomSwitched);
    Manager->OnGameStateUpdated.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGameStateUpdated);
    Manager->OnScoreUpdated.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleScoreUpdated);
    Manager->OnTurnChanged.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleTurnChanged);
    Manager->OnRoundStarted.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoundStarted);
    Manager->OnRoundEnded.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleRoundEnded);
    Manager->OnChatReceived.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleChatReceived);
    Manager->OnGamePaused.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGamePaused);
    Manager->OnGameResumed.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGameResumed);
    Manager->OnGameEnded.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleGameEnded);
    Manager->OnError.RemoveDynamic(this, &UDeskillzHostSpectatorEventComponent::HandleError);
}

// Event handler implementations - forward to component delegates
void UDeskillzHostSpectatorEventComponent::HandleConnected(const FDeskillzHostSpectatorSession& Session) { OnConnected.Broadcast(Session); }
void UDeskillzHostSpectatorEventComponent::HandleDisconnected(const FString& RoomId) { OnDisconnected.Broadcast(RoomId); }
void UDeskillzHostSpectatorEventComponent::HandleStateChanged(EDeskillzHostSpectatorState NewState) { OnStateChanged.Broadcast(NewState); }
void UDeskillzHostSpectatorEventComponent::HandleRoomsFetched(const TArray<FDeskillzHostRoom>& Rooms) { OnRoomsFetched.Broadcast(Rooms); }
void UDeskillzHostSpectatorEventComponent::HandleRoomSwitched(const FDeskillzHostSpectatorSession& NewSession) { OnRoomSwitched.Broadcast(NewSession); }
void UDeskillzHostSpectatorEventComponent::HandleGameStateUpdated(const FDeskillzHostGameState& State) { OnGameStateUpdated.Broadcast(State); }
void UDeskillzHostSpectatorEventComponent::HandleScoreUpdated(const FDeskillzHostScoreUpdate& Update) { OnScoreUpdated.Broadcast(Update); }
void UDeskillzHostSpectatorEventComponent::HandleTurnChanged(const FString& PlayerId) { OnTurnChanged.Broadcast(PlayerId); }
void UDeskillzHostSpectatorEventComponent::HandleRoundStarted(int32 RoundNumber) { OnRoundStarted.Broadcast(RoundNumber); }
void UDeskillzHostSpectatorEventComponent::HandleRoundEnded(const FDeskillzHostRoundResult& Result) { OnRoundEnded.Broadcast(Result); }
void UDeskillzHostSpectatorEventComponent::HandleChatReceived(const FDeskillzHostChatMessage& Message) { OnChatReceived.Broadcast(Message); }
void UDeskillzHostSpectatorEventComponent::HandleGamePaused(float Duration) { OnGamePaused.Broadcast(Duration); }
void UDeskillzHostSpectatorEventComponent::HandleGameResumed() { OnGameResumed.Broadcast(); }
void UDeskillzHostSpectatorEventComponent::HandleGameEnded(const FDeskillzHostGameEndResult& Result) { OnGameEnded.Broadcast(Result); }
void UDeskillzHostSpectatorEventComponent::HandleError(const FDeskillzHostSpectatorError& Error) { OnError.Broadcast(Error); }