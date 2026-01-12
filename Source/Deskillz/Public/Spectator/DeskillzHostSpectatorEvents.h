// =============================================================================
// Deskillz SDK for Unreal Engine - Host Spectator Events
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Event delegates for host spectator mode.
// Only room hosts can spectate their own private social rooms.
// Hosts can see board/scores but NOT player hands (anti-cheat).
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "DeskillzHostSpectatorTypes.h"
#include "DeskillzHostSpectatorEvents.generated.h"

// ============================================================================
// Connection Events
// ============================================================================

/** Connected to your room as host spectator */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostSpectatorConnected,
    const FDeskillzHostSpectatorSession&, Session
);

/** Disconnected from session */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostSpectatorDisconnected,
    const FString&, RoomId
);

/** Connection state changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostSpectatorStateChanged,
    EDeskillzHostSpectatorState, NewState
);

// ============================================================================
// Room Events
// ============================================================================

/** Your rooms fetched */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostRoomsFetched,
    const TArray<FDeskillzHostRoom>&, Rooms
);

/** Room switched (multi-room hosting) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostRoomSwitched,
    const FDeskillzHostSpectatorSession&, NewSession
);

// ============================================================================
// Game State Events (Board/Scores Only - NO Hands)
// ============================================================================

/** Game state updated (board/scores, NOT player hands) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostGameStateUpdated,
    const FDeskillzHostGameState&, State
);

/** Score updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostScoreUpdated,
    const FDeskillzHostScoreUpdate&, Update
);

/** Turn changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostTurnChanged,
    const FString&, PlayerId
);

// ============================================================================
// Round Events
// ============================================================================

/** Round started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostRoundStarted,
    int32, RoundNumber
);

/** Round ended */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostRoundEnded,
    const FDeskillzHostRoundResult&, Result
);

// ============================================================================
// Chat Events
// ============================================================================

/** Chat message received */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostChatReceived,
    const FDeskillzHostChatMessage&, Message
);

// ============================================================================
// Pause Events
// ============================================================================

/** Game paused */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostGamePaused,
    float, Duration
);

/** Game resumed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
    FOnDeskillzHostGameResumed
);

// ============================================================================
// Game End Events
// ============================================================================

/** Game ended */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostGameEnded,
    const FDeskillzHostGameEndResult&, Result
);

// ============================================================================
// Error Events
// ============================================================================

/** Error occurred */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDeskillzHostSpectatorError,
    const FDeskillzHostSpectatorError&, Error
);

// ============================================================================
// Event Hub Component (Optional)
// ============================================================================

/**
 * Optional component to centralize host spectator event subscriptions.
 * Attach to an Actor to easily subscribe to events in Blueprints.
 * 
 * IMPORTANT: This is HOST-ONLY.
 * - Only the room creator can receive these events
 * - Game state events show board/scores but NOT player hands
 */
UCLASS(ClassGroup=(Deskillz), meta=(BlueprintSpawnableComponent))
class DESKILLZ_API UDeskillzHostSpectatorEventComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDeskillzHostSpectatorEventComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ========================================================================
    // Connection Events
    // ========================================================================

    /** Connected to your room */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostSpectatorConnected OnConnected;

    /** Disconnected from session */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostSpectatorDisconnected OnDisconnected;

    /** Connection state changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostSpectatorStateChanged OnStateChanged;

    // ========================================================================
    // Room Events
    // ========================================================================

    /** Your rooms fetched */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Rooms")
    FOnDeskillzHostRoomsFetched OnRoomsFetched;

    /** Room switched */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Rooms")
    FOnDeskillzHostRoomSwitched OnRoomSwitched;

    // ========================================================================
    // Game State Events (Board/Scores Only)
    // ========================================================================

    /** Game state updated (NO player hands - anti-cheat) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|State")
    FOnDeskillzHostGameStateUpdated OnGameStateUpdated;

    /** Score updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|State")
    FOnDeskillzHostScoreUpdated OnScoreUpdated;

    /** Turn changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|State")
    FOnDeskillzHostTurnChanged OnTurnChanged;

    // ========================================================================
    // Round Events
    // ========================================================================

    /** Round started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Round")
    FOnDeskillzHostRoundStarted OnRoundStarted;

    /** Round ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Round")
    FOnDeskillzHostRoundEnded OnRoundEnded;

    // ========================================================================
    // Chat Events
    // ========================================================================

    /** Chat received */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Chat")
    FOnDeskillzHostChatReceived OnChatReceived;

    // ========================================================================
    // Pause Events
    // ========================================================================

    /** Game paused */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Pause")
    FOnDeskillzHostGamePaused OnGamePaused;

    /** Game resumed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Pause")
    FOnDeskillzHostGameResumed OnGameResumed;

    // ========================================================================
    // Game End Events
    // ========================================================================

    /** Game ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|End")
    FOnDeskillzHostGameEnded OnGameEnded;

    // ========================================================================
    // Error Events
    // ========================================================================

    /** Error occurred */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events|Error")
    FOnDeskillzHostSpectatorError OnError;

protected:
    /** Subscribe to manager events */
    void SubscribeToEvents();

    /** Unsubscribe from manager events */
    void UnsubscribeFromEvents();

    // Event handlers
    UFUNCTION()
    void HandleConnected(const FDeskillzHostSpectatorSession& Session);

    UFUNCTION()
    void HandleDisconnected(const FString& RoomId);

    UFUNCTION()
    void HandleStateChanged(EDeskillzHostSpectatorState NewState);

    UFUNCTION()
    void HandleRoomsFetched(const TArray<FDeskillzHostRoom>& Rooms);

    UFUNCTION()
    void HandleRoomSwitched(const FDeskillzHostSpectatorSession& NewSession);

    UFUNCTION()
    void HandleGameStateUpdated(const FDeskillzHostGameState& State);

    UFUNCTION()
    void HandleScoreUpdated(const FDeskillzHostScoreUpdate& Update);

    UFUNCTION()
    void HandleTurnChanged(const FString& PlayerId);

    UFUNCTION()
    void HandleRoundStarted(int32 RoundNumber);

    UFUNCTION()
    void HandleRoundEnded(const FDeskillzHostRoundResult& Result);

    UFUNCTION()
    void HandleChatReceived(const FDeskillzHostChatMessage& Message);

    UFUNCTION()
    void HandleGamePaused(float Duration);

    UFUNCTION()
    void HandleGameResumed();

    UFUNCTION()
    void HandleGameEnded(const FDeskillzHostGameEndResult& Result);

    UFUNCTION()
    void HandleError(const FDeskillzHostSpectatorError& Error);
};