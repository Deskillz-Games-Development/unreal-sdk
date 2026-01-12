// =============================================================================
// Deskillz SDK for Unreal Engine - Social Game Manager Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "Social/DeskillzSocialGameManager.h"
#include "Host/DeskillzHostManager.h"
#include "Social/DeskillzRakeCalculator.h"
#include "Social/DeskillzBuyInManager.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

UDeskillzSocialGameManager* UDeskillzSocialGameManager::Instance = nullptr;

UDeskillzSocialGameManager::UDeskillzSocialGameManager()
    : bIsInitialized(false)
    , bIsInSession(false)
    , CurrentSessionState(EDeskillzSocialSessionState::Idle)
{
}

UDeskillzSocialGameManager* UDeskillzSocialGameManager::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzSocialGameManager>();
        Instance->AddToRoot();
    }
    return Instance;
}

void UDeskillzSocialGameManager::Initialize()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DeskillzSocialGameManager already initialized"));
        return;
    }

    // Get references to other managers
    RakeCalculator = UDeskillzRakeCalculator::Get();
    BuyInManager = UDeskillzBuyInManager::Get();

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("DeskillzSocialGameManager initialized"));
}

void UDeskillzSocialGameManager::Shutdown()
{
    if (!bIsInitialized) return;

    if (bIsInSession)
    {
        EndSession(TEXT("Manager shutdown"));
    }

    bIsInitialized = false;
    UE_LOG(LogTemp, Log, TEXT("DeskillzSocialGameManager shutdown"));
}

// =============================================================================
// Session Management
// =============================================================================

void UDeskillzSocialGameManager::StartSession(const FDeskillzSocialSessionConfig& Config)
{
    if (!bIsInitialized)
    {
        OnSessionError.Broadcast(TEXT("Manager not initialized"));
        return;
    }

    if (bIsInSession)
    {
        OnSessionError.Broadcast(TEXT("Session already in progress"));
        return;
    }

    // Validate config
    if (!ValidateSessionConfig(Config))
    {
        return;
    }

    // Initialize session
    CurrentSession = FDeskillzSocialSession();
    CurrentSession.SessionId = GenerateSessionId();
    CurrentSession.RoomId = Config.RoomId;
    CurrentSession.GameId = Config.GameId;
    CurrentSession.HostId = Config.HostId;
    CurrentSession.PointValue = Config.PointValue;
    CurrentSession.RakePercent = Config.RakePercent;
    CurrentSession.RakeCap = Config.RakeCap;
    CurrentSession.MinBuyIn = Config.MinBuyIn;
    CurrentSession.MaxBuyIn = Config.MaxBuyIn;
    CurrentSession.StartTime = FDateTime::UtcNow();
    CurrentSession.CurrentRound = 0;
    CurrentSession.TotalPotCollected = 0.0f;
    CurrentSession.TotalRakeCollected = 0.0f;

    // Configure rake calculator
    if (RakeCalculator)
    {
        RakeCalculator->SetRakeConfig(Config.RakePercent, Config.RakeCap);
    }

    // Configure buy-in manager
    if (BuyInManager)
    {
        FDeskillzBuyInConfig BuyInConfig;
        BuyInConfig.MinBuyIn = Config.MinBuyIn;
        BuyInConfig.MaxBuyIn = Config.MaxBuyIn;
        BuyInConfig.PointValue = Config.PointValue;
        BuyInConfig.bAllowRebuy = Config.bAllowRebuy;
        BuyInConfig.RebuyPeriodRounds = Config.RebuyPeriodRounds;
        BuyInManager->SetBuyInConfig(BuyInConfig);
    }

    bIsInSession = true;
    SetSessionState(EDeskillzSocialSessionState::WaitingForPlayers);

    OnSessionStarted.Broadcast(CurrentSession.SessionId);
    UE_LOG(LogTemp, Log, TEXT("Social session started: %s"), *CurrentSession.SessionId);
}

void UDeskillzSocialGameManager::EndSession(const FString& Reason)
{
    if (!bIsInSession)
    {
        return;
    }

    // Calculate final settlements
    SettleAllPlayers();

    // Record session end
    CurrentSession.EndTime = FDateTime::UtcNow();

    // Send session report to server
    SendSessionReport();

    FString SessionId = CurrentSession.SessionId;
    
    // Reset state
    bIsInSession = false;
    SetSessionState(EDeskillzSocialSessionState::Idle);
    CurrentSession = FDeskillzSocialSession();
    PlayerSessions.Empty();

    OnSessionEnded.Broadcast(SessionId, Reason);
    UE_LOG(LogTemp, Log, TEXT("Social session ended: %s - %s"), *SessionId, *Reason);
}

void UDeskillzSocialGameManager::PauseSession(const FString& RequesterId)
{
    if (!bIsInSession || CurrentSessionState == EDeskillzSocialSessionState::Paused)
    {
        return;
    }

    FDeskillzPlayerSession* Requester = PlayerSessions.Find(RequesterId);
    if (!Requester)
    {
        OnSessionError.Broadcast(TEXT("Invalid pause requester"));
        return;
    }

    // Check pause limits
    if (Requester->PausesUsed >= MaxPausesPerPlayer)
    {
        OnSessionError.Broadcast(TEXT("Maximum pauses reached"));
        return;
    }

    Requester->PausesUsed++;
    PauseStartTime = FDateTime::UtcNow();
    
    SetSessionState(EDeskillzSocialSessionState::Paused);
    OnSessionPaused.Broadcast(RequesterId);
}

void UDeskillzSocialGameManager::ResumeSession()
{
    if (CurrentSessionState != EDeskillzSocialSessionState::Paused)
    {
        return;
    }

    SetSessionState(EDeskillzSocialSessionState::InProgress);
    OnSessionResumed.Broadcast();
}

FDeskillzSocialSession UDeskillzSocialGameManager::GetCurrentSession() const
{
    return CurrentSession;
}

EDeskillzSocialSessionState UDeskillzSocialGameManager::GetSessionState() const
{
    return CurrentSessionState;
}

// =============================================================================
// Player Management
// =============================================================================

void UDeskillzSocialGameManager::AddPlayer(const FString& PlayerId, float BuyInAmount)
{
    if (!bIsInSession)
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("No active session"));
        return;
    }

    if (PlayerSessions.Contains(PlayerId))
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("Player already in session"));
        return;
    }

    // Validate buy-in
    if (BuyInManager && !BuyInManager->ValidateBuyIn(PlayerId, BuyInAmount))
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("Invalid buy-in amount"));
        return;
    }

    // Create player session
    FDeskillzPlayerSession PlayerSession;
    PlayerSession.PlayerId = PlayerId;
    PlayerSession.SessionId = CurrentSession.SessionId;
    PlayerSession.TotalBuyIn = BuyInAmount;
    PlayerSession.CurrentBalance = BuyInAmount;
    PlayerSession.InitialBalance = BuyInAmount;
    PlayerSession.RoundsPlayed = 0;
    PlayerSession.RoundsWon = 0;
    PlayerSession.PausesUsed = 0;
    PlayerSession.JoinTime = FDateTime::UtcNow();
    PlayerSession.bIsActive = true;

    PlayerSessions.Add(PlayerId, PlayerSession);

    // Process buy-in
    if (BuyInManager)
    {
        BuyInManager->ProcessBuyIn(PlayerId, BuyInAmount);
    }

    OnPlayerJoined.Broadcast(PlayerId, BuyInAmount);
    
    // Check if we can start the game
    CheckSessionReadyToStart();
}

void UDeskillzSocialGameManager::RemovePlayer(const FString& PlayerId, const FString& Reason)
{
    FDeskillzPlayerSession* PlayerSession = PlayerSessions.Find(PlayerId);
    if (!PlayerSession)
    {
        return;
    }

    // Settle player's remaining balance
    SettlePlayer(PlayerId);

    PlayerSession->bIsActive = false;
    PlayerSession->LeaveTime = FDateTime::UtcNow();

    OnPlayerLeft.Broadcast(PlayerId, Reason);

    // Check if session should end due to insufficient players
    CheckMinimumPlayers();
}

void UDeskillzSocialGameManager::ProcessRebuy(const FString& PlayerId, float RebuyAmount)
{
    if (!bIsInSession)
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("No active session"));
        return;
    }

    FDeskillzPlayerSession* PlayerSession = PlayerSessions.Find(PlayerId);
    if (!PlayerSession)
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("Player not in session"));
        return;
    }

    // Validate rebuy
    if (BuyInManager && !BuyInManager->ValidateRebuy(PlayerId, RebuyAmount, CurrentSession.CurrentRound))
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("Rebuy not allowed"));
        return;
    }

    // Process rebuy
    PlayerSession->TotalBuyIn += RebuyAmount;
    PlayerSession->CurrentBalance += RebuyAmount;
    PlayerSession->RebuyCount++;

    if (BuyInManager)
    {
        BuyInManager->ProcessRebuy(PlayerId, RebuyAmount);
    }

    OnPlayerRebuy.Broadcast(PlayerId, RebuyAmount);
}

void UDeskillzSocialGameManager::ProcessCashOut(const FString& PlayerId)
{
    FDeskillzPlayerSession* PlayerSession = PlayerSessions.Find(PlayerId);
    if (!PlayerSession || !PlayerSession->bIsActive)
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("Invalid player"));
        return;
    }

    float CashOutAmount = PlayerSession->CurrentBalance;
    
    // Calculate net result
    float NetResult = CashOutAmount - PlayerSession->TotalBuyIn;
    
    // Process settlement
    SettlePlayer(PlayerId);

    OnPlayerCashOut.Broadcast(PlayerId, CashOutAmount, NetResult);
}

TArray<FDeskillzPlayerSession> UDeskillzSocialGameManager::GetActivePlayers() const
{
    TArray<FDeskillzPlayerSession> ActivePlayers;
    
    for (const auto& Pair : PlayerSessions)
    {
        if (Pair.Value.bIsActive)
        {
            ActivePlayers.Add(Pair.Value);
        }
    }
    
    return ActivePlayers;
}

FDeskillzPlayerSession UDeskillzSocialGameManager::GetPlayerSession(const FString& PlayerId) const
{
    if (const FDeskillzPlayerSession* Session = PlayerSessions.Find(PlayerId))
    {
        return *Session;
    }
    return FDeskillzPlayerSession();
}

// =============================================================================
// Round Management
// =============================================================================

void UDeskillzSocialGameManager::StartRound()
{
    if (!bIsInSession || CurrentSessionState != EDeskillzSocialSessionState::InProgress)
    {
        OnSessionError.Broadcast(TEXT("Cannot start round in current state"));
        return;
    }

    CurrentSession.CurrentRound++;
    CurrentRoundPot = 0.0f;
    
    // Reset round-specific player data
    for (auto& Pair : PlayerSessions)
    {
        if (Pair.Value.bIsActive)
        {
            Pair.Value.CurrentRoundBet = 0.0f;
        }
    }

    OnRoundStarted.Broadcast(CurrentSession.CurrentRound);
}

void UDeskillzSocialGameManager::EndRound(const FString& WinnerId, float PotAmount)
{
    if (!bIsInSession)
    {
        return;
    }

    // Calculate rake
    float RakeAmount = 0.0f;
    if (RakeCalculator)
    {
        RakeAmount = RakeCalculator->CalculateRake(PotAmount);
    }

    float WinnerPayout = PotAmount - RakeAmount;

    // Update session totals
    CurrentSession.TotalPotCollected += PotAmount;
    CurrentSession.TotalRakeCollected += RakeAmount;

    // Award winner
    FDeskillzPlayerSession* Winner = PlayerSessions.Find(WinnerId);
    if (Winner && Winner->bIsActive)
    {
        Winner->CurrentBalance += WinnerPayout;
        Winner->TotalWinnings += WinnerPayout;
        Winner->RoundsWon++;
    }

    // Update all players' rounds played
    for (auto& Pair : PlayerSessions)
    {
        if (Pair.Value.bIsActive)
        {
            Pair.Value.RoundsPlayed++;
        }
    }

    // Notify rake calculator for tracking
    if (RakeCalculator)
    {
        RakeCalculator->RecordRake(CurrentSession.SessionId, RakeAmount);
    }

    OnRoundEnded.Broadcast(CurrentSession.CurrentRound, WinnerId, WinnerPayout, RakeAmount);
}

void UDeskillzSocialGameManager::RecordBet(const FString& PlayerId, float BetAmount)
{
    FDeskillzPlayerSession* PlayerSession = PlayerSessions.Find(PlayerId);
    if (!PlayerSession || !PlayerSession->bIsActive)
    {
        return;
    }

    if (BetAmount > PlayerSession->CurrentBalance)
    {
        OnPlayerError.Broadcast(PlayerId, TEXT("Insufficient balance"));
        return;
    }

    PlayerSession->CurrentBalance -= BetAmount;
    PlayerSession->CurrentRoundBet += BetAmount;
    CurrentRoundPot += BetAmount;

    OnBetRecorded.Broadcast(PlayerId, BetAmount, CurrentRoundPot);
}

void UDeskillzSocialGameManager::RecordFold(const FString& PlayerId)
{
    FDeskillzPlayerSession* PlayerSession = PlayerSessions.Find(PlayerId);
    if (!PlayerSession || !PlayerSession->bIsActive)
    {
        return;
    }

    // Player forfeits their current round bet (already added to pot)
    OnPlayerFolded.Broadcast(PlayerId);
}

// =============================================================================
// Private Methods
// =============================================================================

bool UDeskillzSocialGameManager::ValidateSessionConfig(const FDeskillzSocialSessionConfig& Config)
{
    if (Config.RoomId.IsEmpty())
    {
        OnSessionError.Broadcast(TEXT("Room ID is required"));
        return false;
    }

    if (Config.GameId.IsEmpty())
    {
        OnSessionError.Broadcast(TEXT("Game ID is required"));
        return false;
    }

    if (Config.PointValue <= 0)
    {
        OnSessionError.Broadcast(TEXT("Invalid point value"));
        return false;
    }

    if (Config.MinBuyIn <= 0 || Config.MaxBuyIn < Config.MinBuyIn)
    {
        OnSessionError.Broadcast(TEXT("Invalid buy-in range"));
        return false;
    }

    if (Config.RakePercent < 0 || Config.RakePercent > 100)
    {
        OnSessionError.Broadcast(TEXT("Invalid rake percentage"));
        return false;
    }

    return true;
}

void UDeskillzSocialGameManager::SetSessionState(EDeskillzSocialSessionState NewState)
{
    if (CurrentSessionState != NewState)
    {
        EDeskillzSocialSessionState OldState = CurrentSessionState;
        CurrentSessionState = NewState;
        OnSessionStateChanged.Broadcast(OldState, NewState);
    }
}

void UDeskillzSocialGameManager::CheckSessionReadyToStart()
{
    if (CurrentSessionState != EDeskillzSocialSessionState::WaitingForPlayers)
    {
        return;
    }

    int32 ActiveCount = 0;
    for (const auto& Pair : PlayerSessions)
    {
        if (Pair.Value.bIsActive)
        {
            ActiveCount++;
        }
    }

    if (ActiveCount >= MinPlayersToStart)
    {
        SetSessionState(EDeskillzSocialSessionState::InProgress);
    }
}

void UDeskillzSocialGameManager::CheckMinimumPlayers()
{
    int32 ActiveCount = 0;
    for (const auto& Pair : PlayerSessions)
    {
        if (Pair.Value.bIsActive)
        {
            ActiveCount++;
        }
    }

    if (ActiveCount < MinPlayersToStart && CurrentSessionState == EDeskillzSocialSessionState::InProgress)
    {
        EndSession(TEXT("Insufficient players"));
    }
}

void UDeskillzSocialGameManager::SettlePlayer(const FString& PlayerId)
{
    FDeskillzPlayerSession* PlayerSession = PlayerSessions.Find(PlayerId);
    if (!PlayerSession)
    {
        return;
    }

    float SettlementAmount = PlayerSession->CurrentBalance;
    
    // Transfer balance back to player's wallet
    // This would integrate with wallet system
    
    PlayerSession->CurrentBalance = 0.0f;
    PlayerSession->bIsActive = false;

    OnPlayerSettled.Broadcast(PlayerId, SettlementAmount);
}

void UDeskillzSocialGameManager::SettleAllPlayers()
{
    for (auto& Pair : PlayerSessions)
    {
        if (Pair.Value.bIsActive)
        {
            SettlePlayer(Pair.Key);
        }
    }
}

void UDeskillzSocialGameManager::SendSessionReport()
{
    TSharedPtr<FJsonObject> Report = MakeShareable(new FJsonObject);
    Report->SetStringField(TEXT("sessionId"), CurrentSession.SessionId);
    Report->SetStringField(TEXT("roomId"), CurrentSession.RoomId);
    Report->SetStringField(TEXT("gameId"), CurrentSession.GameId);
    Report->SetStringField(TEXT("hostId"), CurrentSession.HostId);
    Report->SetNumberField(TEXT("totalRounds"), CurrentSession.CurrentRound);
    Report->SetNumberField(TEXT("totalPot"), CurrentSession.TotalPotCollected);
    Report->SetNumberField(TEXT("totalRake"), CurrentSession.TotalRakeCollected);
    Report->SetNumberField(TEXT("playerCount"), PlayerSessions.Num());

    // Send to API
    // Implementation would use HTTP module
    
    UE_LOG(LogTemp, Log, TEXT("Session report sent for: %s"), *CurrentSession.SessionId);
}

FString UDeskillzSocialGameManager::GenerateSessionId() const
{
    return FGuid::NewGuid().ToString();
}