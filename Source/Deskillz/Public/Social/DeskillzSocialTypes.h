// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "DeskillzSocialTypes.generated.h"

// ============================================================================
// Social Game Enums
// ============================================================================

/**
 * Social game session state
 */
UENUM(BlueprintType)
enum class EDeskillzSocialSessionState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    WaitingForPlayers UMETA(DisplayName = "Waiting for Players"),
    Starting        UMETA(DisplayName = "Starting"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Paused          UMETA(DisplayName = "Paused"),
    RoundEnd        UMETA(DisplayName = "Round End"),
    Settlement      UMETA(DisplayName = "Settlement"),
    Completed       UMETA(DisplayName = "Completed"),
    Cancelled       UMETA(DisplayName = "Cancelled")
};

/**
 * Social player status
 */
UENUM(BlueprintType)
enum class EDeskillzSocialPlayerStatus : uint8
{
    Waiting         UMETA(DisplayName = "Waiting"),
    Ready           UMETA(DisplayName = "Ready"),
    Playing         UMETA(DisplayName = "Playing"),
    SittingOut      UMETA(DisplayName = "Sitting Out"),
    Busted          UMETA(DisplayName = "Busted"),
    CashedOut       UMETA(DisplayName = "Cashed Out"),
    Disconnected    UMETA(DisplayName = "Disconnected")
};

/**
 * Pause request status
 */
UENUM(BlueprintType)
enum class EDeskillzPauseStatus : uint8
{
    None            UMETA(DisplayName = "None"),
    Requested       UMETA(DisplayName = "Requested"),
    Voting          UMETA(DisplayName = "Voting"),
    Approved        UMETA(DisplayName = "Approved"),
    Denied          UMETA(DisplayName = "Denied"),
    Active          UMETA(DisplayName = "Active"),
    Resuming        UMETA(DisplayName = "Resuming")
};

/**
 * Settlement trigger type
 */
UENUM(BlueprintType)
enum class EDeskillzSettlementTrigger : uint8
{
    RoundComplete   UMETA(DisplayName = "Round Complete"),
    SessionEnd      UMETA(DisplayName = "Session End"),
    PlayerCashOut   UMETA(DisplayName = "Player Cash Out"),
    RakeCap         UMETA(DisplayName = "Rake Cap Reached"),
    Manual          UMETA(DisplayName = "Manual")
};

// ============================================================================
// Social Game Settings
// ============================================================================

/**
 * Social game room settings
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSocialGameSettings
{
    GENERATED_BODY()

    /** Point value in dollars */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    double PointValue = 1.0;

    /** Rake percentage (1-10%) */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    float RakePercent = 5.0f;

    /** Maximum rake per round */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    double RakeCap = 50.0;

    /** Minimum buy-in (in points) */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    int32 MinBuyInPoints = 50;

    /** Maximum buy-in (in points) */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    int32 MaxBuyInPoints = 500;

    /** Enable turn timer */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    bool bTurnTimerEnabled = true;

    /** Turn timer seconds */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    int32 TurnTimerSeconds = 30;

    /** Allow pauses */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    bool bPauseEnabled = true;

    /** Max pauses per player per session */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    int32 MaxPausesPerPlayer = 3;

    /** Max pause duration in minutes */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    int32 MaxPauseDurationMinutes = 5;

    /** Pause cooldown in minutes */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    int32 PauseCooldownMinutes = 15;

    /** Allow rebuys */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    bool bAllowRebuys = true;

    /** Max rebuys per session */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Social")
    int32 MaxRebuysPerSession = 3;

    FDeskillzSocialGameSettings() = default;

    /** Get minimum buy-in in dollars */
    double GetMinBuyInDollars() const { return PointValue * MinBuyInPoints; }

    /** Get maximum buy-in in dollars */
    double GetMaxBuyInDollars() const { return PointValue * MaxBuyInPoints; }
};

// ============================================================================
// Social Player
// ============================================================================

/**
 * Social game player information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSocialPlayer
{
    GENERATED_BODY()

    /** Player ID (odid) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString PlayerId;

    /** Username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString Username;

    /** Avatar URL */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString AvatarUrl;

    /** Current status */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    EDeskillzSocialPlayerStatus Status = EDeskillzSocialPlayerStatus::Waiting;

    /** Current balance (in points) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 Balance = 0;

    /** Total buy-in this session */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double TotalBuyIn = 0.0;

    /** Rebuy count this session */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 RebuyCount = 0;

    /** Pauses used this session */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 PausesUsed = 0;

    /** Whether it's this player's turn */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    bool bIsCurrentTurn = false;

    /** Whether connected */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    bool bIsConnected = true;

    /** Seat position (0-based) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 SeatPosition = -1;

    /** When joined */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime JoinedAt;

    FDeskillzSocialPlayer() = default;

    /** Get balance in dollars */
    double GetBalanceDollars(double PointValue) const { return Balance * PointValue; }

    /** Check if player is active */
    bool IsActive() const
    {
        return Status == EDeskillzSocialPlayerStatus::Playing ||
               Status == EDeskillzSocialPlayerStatus::Ready ||
               Status == EDeskillzSocialPlayerStatus::Waiting;
    }
};

// ============================================================================
// Round & Pot
// ============================================================================

/**
 * Round pot information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSocialPot
{
    GENERATED_BODY()

    /** Current pot size (in points) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 Total = 0;

    /** Rake collected this round */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double RakeCollected = 0.0;

    /** Players in pot */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    TArray<FString> ContributingPlayers;

    FDeskillzSocialPot() = default;

    /** Get pot in dollars */
    double GetTotalDollars(double PointValue) const { return Total * PointValue; }
};

/**
 * Round result
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSocialRoundResult
{
    GENERATED_BODY()

    /** Round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 RoundNumber = 0;

    /** Winner player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString WinnerId;

    /** Winner username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString WinnerUsername;

    /** Pot won (in points) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 PotWon = 0;

    /** Rake collected */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double RakeCollected = 0.0;

    /** Per-player balance changes */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    TMap<FString, int32> BalanceChanges;

    FDeskillzSocialRoundResult() = default;
};

// ============================================================================
// Pause System
// ============================================================================

/**
 * Pause request
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzPauseRequest
{
    GENERATED_BODY()

    /** Request ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString RequestId;

    /** Requester player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString RequesterId;

    /** Requester username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString RequesterUsername;

    /** Current status */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    EDeskillzPauseStatus Status = EDeskillzPauseStatus::None;

    /** Votes received (PlayerId -> Vote) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    TMap<FString, bool> Votes;

    /** Votes required for approval */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 VotesRequired = 0;

    /** When requested */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime RequestedAt;

    /** Voting deadline */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime VotingDeadline;

    FDeskillzPauseRequest() = default;

    /** Get yes vote count */
    int32 GetYesVotes() const
    {
        int32 Count = 0;
        for (const auto& Pair : Votes)
        {
            if (Pair.Value) Count++;
        }
        return Count;
    }

    /** Check if approved */
    bool IsApproved() const { return GetYesVotes() >= VotesRequired; }
};

/**
 * Active pause state
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzActivePause
{
    GENERATED_BODY()

    /** Pause ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString PauseId;

    /** Who initiated */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString InitiatorId;

    /** When started */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime StartedAt;

    /** When expires */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime ExpiresAt;

    /** Reason */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString Reason;

    FDeskillzActivePause() = default;

    /** Get remaining time in seconds */
    float GetRemainingSeconds() const
    {
        FTimespan Remaining = ExpiresAt - FDateTime::UtcNow();
        return FMath::Max(0.0f, static_cast<float>(Remaining.GetTotalSeconds()));
    }
};

// ============================================================================
// Session & Settlement
// ============================================================================

/**
 * Social game session
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSocialSession
{
    GENERATED_BODY()

    /** Session ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString SessionId;

    /** Room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString RoomId;

    /** Room code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString RoomCode;

    /** Game ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString GameId;

    /** Host player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString HostId;

    /** Current state */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    EDeskillzSocialSessionState State = EDeskillzSocialSessionState::Idle;

    /** Game settings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDeskillzSocialGameSettings Settings;

    /** Players in session */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    TArray<FDeskillzSocialPlayer> Players;

    /** Current round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 CurrentRound = 0;

    /** Current pot */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDeskillzSocialPot CurrentPot;

    /** Total rake collected this session */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double TotalRakeCollected = 0.0;

    /** Active pause (if any) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDeskillzActivePause ActivePause;

    /** Whether paused */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    bool bIsPaused = false;

    /** Current player turn (ID) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString CurrentTurnPlayerId;

    /** Turn deadline */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime TurnDeadline;

    /** When session started */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime StartedAt;

    FDeskillzSocialSession() = default;

    /** Get player by ID */
    const FDeskillzSocialPlayer* GetPlayer(const FString& PlayerId) const
    {
        for (const auto& Player : Players)
        {
            if (Player.PlayerId == PlayerId) return &Player;
        }
        return nullptr;
    }

    /** Get active player count */
    int32 GetActivePlayerCount() const
    {
        int32 Count = 0;
        for (const auto& Player : Players)
        {
            if (Player.IsActive()) Count++;
        }
        return Count;
    }

    /** Check if local player is host */
    bool IsHost(const FString& LocalPlayerId) const { return HostId == LocalPlayerId; }
};

/**
 * Session settlement summary
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSessionSettlement
{
    GENERATED_BODY()

    /** Session ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString SessionId;

    /** Total rounds played */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    int32 TotalRounds = 0;

    /** Total rake collected */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double TotalRake = 0.0;

    /** Host earnings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double HostEarnings = 0.0;

    /** Platform earnings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double PlatformEarnings = 0.0;

    /** Settlement trigger */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    EDeskillzSettlementTrigger Trigger = EDeskillzSettlementTrigger::SessionEnd;

    /** Per-player final balances */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    TMap<FString, double> PlayerBalances;

    /** Per-player net profit/loss */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    TMap<FString, double> PlayerNetResults;

    /** When settled */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FDateTime SettledAt;

    FDeskillzSessionSettlement() = default;
};

// ============================================================================
// Buy-In Options
// ============================================================================

/**
 * Buy-in options for a room
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzBuyInOptions
{
    GENERATED_BODY()

    /** Point value */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double PointValue = 1.0;

    /** Minimum buy-in */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double MinBuyIn = 50.0;

    /** Maximum buy-in */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double MaxBuyIn = 500.0;

    /** Default/recommended buy-in */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double DefaultBuyIn = 100.0;

    /** Suggested amounts */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    TArray<double> SuggestedAmounts;

    FDeskillzBuyInOptions() = default;
};

/**
 * Buy-in validation result
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzBuyInValidation
{
    GENERATED_BODY()

    /** Whether valid */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    bool bIsValid = false;

    /** Error message (if invalid) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    FString ErrorMessage;

    /** Minimum allowed */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double MinAllowed = 0.0;

    /** Maximum allowed */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Social")
    double MaxAllowed = 0.0;

    FDeskillzBuyInValidation() = default;

    static FDeskillzBuyInValidation Valid()
    {
        FDeskillzBuyInValidation Result;
        Result.bIsValid = true;
        return Result;
    }

    static FDeskillzBuyInValidation Invalid(const FString& Message)
    {
        FDeskillzBuyInValidation Result;
        Result.bIsValid = false;
        Result.ErrorMessage = Message;
        return Result;
    }
};

// ============================================================================
// Callback Delegates
// ============================================================================

/** Session state changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialSessionStateChanged, EDeskillzSocialSessionState, NewState, EDeskillzSocialSessionState, OldState);

/** Player joined session */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialPlayerJoined, const FDeskillzSocialPlayer&, Player);

/** Player left session */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialPlayerLeft, const FString&, PlayerId, const FString&, Reason);

/** Player balance updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzSocialBalanceUpdated, const FString&, PlayerId, int32, NewBalance, int32, Delta);

/** Turn started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialTurnStarted, const FString&, PlayerId, float, TimeLimit);

/** Turn timer tick */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialTurnTimerTick, float, RemainingSeconds);

/** Turn timer expired */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzSocialTurnTimerExpired);

/** Round started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialRoundStarted, int32, RoundNumber);

/** Round ended */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialRoundEnded, const FDeskillzSocialRoundResult&, Result);

/** Pot updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialPotUpdated, const FDeskillzSocialPot&, Pot);

/** Pause requested */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialPauseRequested, const FDeskillzPauseRequest&, Request);

/** Pause vote updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialPauseVoteUpdated, const FString&, PlayerId, bool, bVotedYes);

/** Game paused */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialGamePaused, float, DurationSeconds);

/** Game resumed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzSocialGameResumed);

/** Buy-in completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzSocialBuyInCompleted, bool, bSuccess, double, Amount, const FString&, ErrorMessage);

/** Cash out completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialCashOutCompleted, bool, bSuccess, double, Amount);

/** Session settlement */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialSessionSettlement, const FDeskillzSessionSettlement&, Settlement);