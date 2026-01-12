// =============================================================================
// Deskillz SDK for Unreal Engine - Host Spectator Types
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Type definitions for host spectator mode.
// Only room hosts can spectate their own private social rooms.
// Hosts can see board/scores but NOT player hands (anti-cheat).
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "DeskillzHostSpectatorTypes.generated.h"

// ============================================================================
// Enums
// ============================================================================

/**
 * Host spectator connection state
 */
UENUM(BlueprintType)
enum class EDeskillzHostSpectatorState : uint8
{
    /** Not spectating any room */
    Disconnected UMETA(DisplayName = "Disconnected"),

    /** Connecting to room */
    Connecting UMETA(DisplayName = "Connecting"),

    /** Connected and receiving updates */
    Connected UMETA(DisplayName = "Connected"),

    /** Connection error occurred */
    Error UMETA(DisplayName = "Error")
};

/**
 * Game category for host rooms.
 * Only SOCIAL rooms support host spectating.
 */
UENUM(BlueprintType)
enum class EDeskillzGameCategory : uint8
{
    /** Social games with rake (host spectating ALLOWED) */
    Social UMETA(DisplayName = "Social"),

    /** Esports tournaments (host spectating NOT allowed) */
    Esports UMETA(DisplayName = "Esports")
};

// ============================================================================
// Filter Structures
// ============================================================================

/**
 * Filter for fetching host's own rooms.
 * Only returns rooms YOU created.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostRoomFilter
{
    GENERATED_BODY()

    /** Filter by game category (Social only for spectating) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator")
    EDeskillzGameCategory GameCategory = EDeskillzGameCategory::Social;

    /** Only return active rooms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator")
    bool bIsActive = true;

    /** Optional game ID filter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator")
    FString GameId;

    FDeskillzHostRoomFilter() = default;
};

// ============================================================================
// Room Structures
// ============================================================================

/**
 * Host's room information for spectating.
 * Only YOUR rooms are returned by the API.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostRoom
{
    GENERATED_BODY()

    /** Room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Id;

    /** Room name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Name;

    /** Room code for sharing */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString RoomCode;

    /** Game ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString GameId;

    /** Game name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString GameName;

    /** Current player count */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 CurrentPlayers = 0;

    /** Maximum players */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 MaxPlayers = 0;

    /** Current round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 CurrentRound = 0;

    /** Total rounds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 TotalRounds = 0;

    /** Whether game is currently active */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    bool bIsActive = false;

    /** Current pot amount */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    double CurrentPot = 0.0;

    /** Currency code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Currency;

    /** Room creation timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FDateTime CreatedAt;

    FDeskillzHostRoom() = default;
};

// ============================================================================
// Player Structures (Public Info Only - NO Hands)
// ============================================================================

/**
 * Player information visible to host spectator.
 * Does NOT include private info like cards/hands (anti-cheat).
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostPlayerInfo
{
    GENERATED_BODY()

    /** Player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString PlayerId;

    /** Player username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Username;

    /** Player avatar URL */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString AvatarUrl;

    /** Current score/points */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 Score = 0;

    /** Current chip stack (for poker-style games) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    double ChipStack = 0.0;

    /** Whether it's this player's turn */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    bool bIsCurrentTurn = false;

    /** Whether player is still active (not folded/busted) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    bool bIsActive = true;

    /** Player's seat position */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 SeatPosition = 0;

    // NOTE: No Hand, Cards, or private info fields - ANTI-CHEAT PROTECTION

    FDeskillzHostPlayerInfo() = default;
};

/**
 * Player score information for host spectator.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostPlayerScore
{
    GENERATED_BODY()

    /** Player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString PlayerId;

    /** Player username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Username;

    /** Current score */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 Score = 0;

    /** Rounds won */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 RoundsWon = 0;

    /** Current chip stack */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    double ChipStack = 0.0;

    /** Position/rank */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 Position = 0;

    FDeskillzHostPlayerScore() = default;
};

// ============================================================================
// Game State (Board/Scores Only - NO Hands)
// ============================================================================

/**
 * Game state snapshot for host spectator.
 * Contains board state and scores but NOT player hands.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostGameState
{
    GENERATED_BODY()

    /** Current round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 CurrentRound = 0;

    /** Total rounds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 TotalRounds = 0;

    /** Current game phase */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Phase;

    /** Current pot amount */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    double CurrentPot = 0.0;

    /** Player whose turn it is */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString CurrentTurnPlayerId;

    /** Turn time remaining in seconds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    float TurnTimeRemaining = 0.0f;

    /** Whether game is paused */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    bool bIsPaused = false;

    /**
     * Board state (game-specific, serialized JSON).
     * NOTE: Does NOT include player hands or hidden cards.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString BoardState;

    /** Player scores (public information) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostPlayerScore> Scores;

    /** Timestamp of this snapshot */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FDateTime Timestamp;

    FDeskillzHostGameState() = default;
};

// ============================================================================
// Session Structure
// ============================================================================

/**
 * Host spectator session data.
 * Represents an active connection to YOUR room.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostSpectatorSession
{
    GENERATED_BODY()

    /** Session ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString SessionId;

    /** Room ID being monitored */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString RoomId;

    /** Room name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString RoomName;

    /** Room code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString RoomCode;

    /** Game ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString GameId;

    /** Game name */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString GameName;

    /** Current game state (board/scores, NOT hands) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FDeskillzHostGameState CurrentState;

    /** Players in the room (public info only) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostPlayerInfo> Players;

    /** Session start time */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FDateTime StartedAt;

    FDeskillzHostSpectatorSession() = default;
};

// ============================================================================
// Event Structures
// ============================================================================

/**
 * Score update event for host spectator.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostScoreUpdate
{
    GENERATED_BODY()

    /** Player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString PlayerId;

    /** Previous score */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 PreviousScore = 0;

    /** New score */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 NewScore = 0;

    /** Score change delta */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 Delta = 0;

    /** Reason for change */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Reason;

    FDeskillzHostScoreUpdate() = default;

    /** Calculate delta */
    void CalculateDelta() { Delta = NewScore - PreviousScore; }
};

/**
 * Round result for host spectator.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostRoundResult
{
    GENERATED_BODY()

    /** Round number */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    int32 RoundNumber = 0;

    /** Winner player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString WinnerId;

    /** Winner username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString WinnerUsername;

    /** Pot won */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    double PotWon = 0.0;

    /** Final scores for this round */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostPlayerScore> FinalScores;

    /** Round duration in seconds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    float DurationSeconds = 0.0f;

    FDeskillzHostRoundResult() = default;
};

/**
 * Game end result for host spectator.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostGameEndResult
{
    GENERATED_BODY()

    /** Winner player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString WinnerId;

    /** Winner username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString WinnerUsername;

    /** Final standings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostPlayerScore> FinalStandings;

    /** Total pot distributed */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    double TotalPot = 0.0;

    /** Your rake earnings from this game */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    double RakeEarnings = 0.0;

    /** Game duration in seconds */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    float DurationSeconds = 0.0f;

    FDeskillzHostGameEndResult() = default;
};

/**
 * Chat message for host spectator.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostChatMessage
{
    GENERATED_BODY()

    /** Message ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString MessageId;

    /** Sender player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString SenderId;

    /** Sender username */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString SenderUsername;

    /** Message content */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Content;

    /** Timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FDateTime Timestamp;

    FDeskillzHostChatMessage() = default;
};

// ============================================================================
// Error Structure
// ============================================================================

/**
 * Error information for host spectator operations.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostSpectatorError
{
    GENERATED_BODY()

    /** Error code */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Code;

    /** Error message */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator")
    FString Message;

    FDeskillzHostSpectatorError() = default;

    FDeskillzHostSpectatorError(const FString& InCode, const FString& InMessage)
        : Code(InCode), Message(InMessage) {}

    /** Known error codes */
    static const FString CODE_NOT_AUTHENTICATED;
    static const FString CODE_NOT_AUTHORIZED;
    static const FString CODE_ROOM_NOT_FOUND;
    static const FString CODE_NOT_YOUR_ROOM;
    static const FString CODE_NOT_SOCIAL_ROOM;
    static const FString CODE_ALREADY_CONNECTING;
    static const FString CODE_MAX_ROOMS_REACHED;
    static const FString CODE_NETWORK_ERROR;
    static const FString CODE_SERVER_ERROR;
};