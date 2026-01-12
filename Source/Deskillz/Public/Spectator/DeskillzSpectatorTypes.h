// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "DeskillzSpectatorTypes.generated.h"

// ============================================================================
// Spectator Enums
// ============================================================================

/**
 * Spectator view mode
 */
UENUM(BlueprintType)
enum class EDeskillzSpectatorViewMode : uint8
{
    Overview        UMETA(DisplayName = "Overview"),
    FollowPlayer    UMETA(DisplayName = "Follow Player"),
    FreeCamera      UMETA(DisplayName = "Free Camera"),
    Broadcast       UMETA(DisplayName = "Broadcast Mode")
};

/**
 * Spectator connection state
 */
UENUM(BlueprintType)
enum class EDeskillzSpectatorConnectionState : uint8
{
    Disconnected    UMETA(DisplayName = "Disconnected"),
    Connecting      UMETA(DisplayName = "Connecting"),
    Connected       UMETA(DisplayName = "Connected"),
    Reconnecting    UMETA(DisplayName = "Reconnecting"),
    Error           UMETA(DisplayName = "Error")
};

/**
 * Game action type for action feed
 */
UENUM(BlueprintType)
enum class EDeskillzGameActionType : uint8
{
    Generic         UMETA(DisplayName = "Generic"),
    Bet             UMETA(DisplayName = "Bet"),
    Fold            UMETA(DisplayName = "Fold"),
    Check           UMETA(DisplayName = "Check"),
    Raise           UMETA(DisplayName = "Raise"),
    AllIn           UMETA(DisplayName = "All-In"),
    Win             UMETA(DisplayName = "Win"),
    Bust            UMETA(DisplayName = "Bust"),
    Join            UMETA(DisplayName = "Join"),
    Leave           UMETA(DisplayName = "Leave"),
    RoundStart      UMETA(DisplayName = "Round Start"),
    RoundEnd        UMETA(DisplayName = "Round End"),
    Pause           UMETA(DisplayName = "Pause"),
    Resume          UMETA(DisplayName = "Resume")
};

// ============================================================================
// Spectator Player Info
// ============================================================================

/**
 * Player information for spectator view
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSpectatorPlayerInfo
{
    GENERATED_BODY()

    /** Player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString PlayerId;

    /** Username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString Username;

    /** Avatar URL */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString AvatarUrl;

    /** Current score/balance */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 Score = 0;

    /** Whether it's this player's turn */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    bool bIsCurrentTurn = false;

    /** Whether player is connected */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    bool bIsConnected = true;

    /** Whether player is sitting out */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    bool bIsSittingOut = false;

    /** Seat position */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 SeatPosition = -1;

    /** Last action description */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString LastAction;

    /** Custom data (game-specific) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    TMap<FString, FString> CustomData;

    FDeskillzSpectatorPlayerInfo() = default;
};

// ============================================================================
// Game State Snapshot
// ============================================================================

/**
 * Game state snapshot for spectators
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzGameStateSnapshot
{
    GENERATED_BODY()

    /** Current round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 RoundNumber = 0;

    /** Current pot size */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    double CurrentPot = 0.0;

    /** Current phase/stage */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString Phase;

    /** Whether game is paused */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    bool bIsPaused = false;

    /** Current turn player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString CurrentTurnPlayerId;

    /** Turn time remaining */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    float TurnTimeRemaining = 0.0f;

    /** Game-specific state data (JSON) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString GameStateJson;

    /** Timestamp of snapshot */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FDateTime Timestamp;

    FDeskillzGameStateSnapshot() = default;
};

// ============================================================================
// Game Actions
// ============================================================================

/**
 * Game action for action feed
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzGameAction
{
    GENERATED_BODY()

    /** Action ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString ActionId;

    /** Player who performed action */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString PlayerId;

    /** Player username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString Username;

    /** Action type */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    EDeskillzGameActionType ActionType = EDeskillzGameActionType::Generic;

    /** Action description */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString Description;

    /** Amount involved (if applicable) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    double Amount = 0.0;

    /** When action occurred */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FDateTime Timestamp;

    /** Additional data */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    TMap<FString, FString> Metadata;

    FDeskillzGameAction() = default;
};

/**
 * Score update event
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzScoreUpdate
{
    GENERATED_BODY()

    /** Player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString PlayerId;

    /** Previous score */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 PreviousScore = 0;

    /** New score */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 NewScore = 0;

    /** Delta (change) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 Delta = 0;

    /** Reason for change */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString Reason;

    FDeskillzScoreUpdate() = default;

    /** Calculate delta */
    void CalculateDelta() { Delta = NewScore - PreviousScore; }
};

// ============================================================================
// Round Events
// ============================================================================

/**
 * Round end event for spectators
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSpectatorRoundEnd
{
    GENERATED_BODY()

    /** Round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 RoundNumber = 0;

    /** Winner player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString WinnerId;

    /** Winner username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString WinnerUsername;

    /** Pot won */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    double PotWon = 0.0;

    /** Final player scores */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    TMap<FString, int32> FinalScores;

    /** Round duration in seconds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    float DurationSeconds = 0.0f;

    FDeskillzSpectatorRoundEnd() = default;
};

// ============================================================================
// Spectator Session
// ============================================================================

/**
 * Spectator session information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSpectatorSession
{
    GENERATED_BODY()

    /** Session ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString SessionId;

    /** Room ID being spectated */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString RoomId;

    /** Room code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString RoomCode;

    /** Room name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString RoomName;

    /** Game ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString GameId;

    /** Game name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString GameName;

    /** Players in the game */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    TArray<FDeskillzSpectatorPlayerInfo> Players;

    /** Current game state */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FDeskillzGameStateSnapshot CurrentState;

    /** Number of spectators */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 SpectatorCount = 0;

    /** When joined as spectator */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FDateTime JoinedAt;

    /** Connection state */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    EDeskillzSpectatorConnectionState ConnectionState = EDeskillzSpectatorConnectionState::Disconnected;

    FDeskillzSpectatorSession() = default;

    /** Get player by ID */
    const FDeskillzSpectatorPlayerInfo* GetPlayer(const FString& PlayerId) const
    {
        for (const auto& Player : Players)
        {
            if (Player.PlayerId == PlayerId) return &Player;
        }
        return nullptr;
    }

    /** Get current turn player */
    const FDeskillzSpectatorPlayerInfo* GetCurrentTurnPlayer() const
    {
        return GetPlayer(CurrentState.CurrentTurnPlayerId);
    }
};

// ============================================================================
// Multi-Room Spectating
// ============================================================================

/**
 * Watched room info for multi-room spectating
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzWatchedRoom
{
    GENERATED_BODY()

    /** Room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString RoomId;

    /** Room code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString RoomCode;

    /** Room name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString RoomName;

    /** Game name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    FString GameName;

    /** Player count */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 PlayerCount = 0;

    /** Current round */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 CurrentRound = 0;

    /** Whether paused */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    bool bIsPaused = false;

    /** Spectator count */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    int32 SpectatorCount = 0;

    /** Whether connected */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator")
    bool bIsConnected = true;

    FDeskillzWatchedRoom() = default;
};

// ============================================================================
// Spectator Settings
// ============================================================================

/**
 * Spectator view settings
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSpectatorSettings
{
    GENERATED_BODY()

    /** Current view mode */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    EDeskillzSpectatorViewMode ViewMode = EDeskillzSpectatorViewMode::Overview;

    /** Player to follow (if FollowPlayer mode) */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    FString FollowedPlayerId;

    /** Show action feed */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    bool bShowActionFeed = true;

    /** Show player scores */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    bool bShowScores = true;

    /** Show spectator count */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    bool bShowSpectatorCount = true;

    /** Auto-follow current turn */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    bool bAutoFollowTurn = false;

    /** Play sounds */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    bool bPlaySounds = true;

    /** Show chat (if available) */
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Spectator")
    bool bShowChat = false;

    FDeskillzSpectatorSettings() = default;
};

// ============================================================================
// Callback Delegates
// ============================================================================

/** Connected to spectator session */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorConnected, const FDeskillzSpectatorSession&, Session);

/** Disconnected from spectator session */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorDisconnected, const FString&, Reason, bool, bWasClean);

/** Connection state changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorConnectionStateChanged, EDeskillzSpectatorConnectionState, NewState);

/** Game state updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorGameStateUpdated, const FDeskillzGameStateSnapshot&, State);

/** Score updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorScoreUpdated, const FDeskillzScoreUpdate&, Update);

/** Game action occurred */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorGameAction, const FDeskillzGameAction&, Action);

/** Player updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorPlayerUpdated, const FDeskillzSpectatorPlayerInfo&, Player);

/** Spectator count changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorCountChanged, int32, Count);

/** Game paused */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorGamePaused, float, DurationSeconds);

/** Game resumed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzSpectatorGameResumed);

/** Round started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorRoundStarted, int32, RoundNumber);

/** Round ended */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorRoundEnded, const FDeskillzSpectatorRoundEnd&, Result);

/** View mode changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorViewModeChanged, EDeskillzSpectatorViewMode, NewMode, const FString&, FollowedPlayerId);