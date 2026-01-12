// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeskillzSpectatorTypes.h"
#include "DeskillzSpectatorEvents.generated.h"

// ============================================================================
// Additional Spectator Event Delegates
// ============================================================================

/** Called when joining spectator mode */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorJoining, const FString&, RoomId);

/** Called when spectator join fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorJoinFailed, const FString&, RoomId, const FString&, ErrorMessage);

/** Called when reconnecting */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorReconnecting, int32, AttemptNumber);

/** Called when watched room added */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorRoomAdded, const FDeskillzWatchedRoom&, Room);

/** Called when watched room removed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorRoomRemoved, const FString&, RoomId);

/** Called when switching rooms */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorRoomSwitched, const FString&, OldRoomId, const FString&, NewRoomId);

/** Called when player joined game being watched */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorPlayerJoined, const FDeskillzSpectatorPlayerInfo&, Player);

/** Called when player left game being watched */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorPlayerLeft, const FString&, PlayerId, const FString&, Reason);

/** Called when turn changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorTurnChanged, const FString&, OldPlayerId, const FString&, NewPlayerId);

/** Called when turn timer updates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorTurnTimerUpdate, float, RemainingSeconds);

/** Called when pot updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorPotUpdated, double, NewPot, double, Delta);

/** Called when game phase changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSpectatorPhaseChanged, const FString&, NewPhase, const FString&, OldPhase);

/** Called when settings updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorSettingsUpdated, const FDeskillzSpectatorSettings&, Settings);

/** Called when action feed updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSpectatorActionFeedUpdated, int32, ActionCount);

// ============================================================================
// Spectator Event Aggregator
// ============================================================================

/**
 * Deskillz Spectator Event Aggregator
 * 
 * Central hub for all spectator mode events. Provides easy Blueprint access
 * to subscribe to any spectator event in one place.
 * 
 * Usage in Blueprint:
 * - Get Deskillz Spectator Events node
 * - Bind to desired event (e.g., OnGameStateUpdated, OnGameAction)
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSpectatorEvents : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzSpectatorEvents();

    /** Get the singleton spectator events instance */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator|Events", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Spectator Events"))
    static UDeskillzSpectatorEvents* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Connection Events
    // ========================================================================

    /** Joining spectator mode */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Connection")
    FOnDeskillzSpectatorJoining OnJoining;

    /** Connected to spectator session */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Connection")
    FOnDeskillzSpectatorConnected OnConnected;

    /** Disconnected from session */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Connection")
    FOnDeskillzSpectatorDisconnected OnDisconnected;

    /** Join failed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Connection")
    FOnDeskillzSpectatorJoinFailed OnJoinFailed;

    /** Connection state changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Connection")
    FOnDeskillzSpectatorConnectionStateChanged OnConnectionStateChanged;

    /** Reconnecting */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Connection")
    FOnDeskillzSpectatorReconnecting OnReconnecting;

    // ========================================================================
    // Multi-Room Events
    // ========================================================================

    /** Room added */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Rooms")
    FOnDeskillzSpectatorRoomAdded OnRoomAdded;

    /** Room removed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Rooms")
    FOnDeskillzSpectatorRoomRemoved OnRoomRemoved;

    /** Room switched */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Rooms")
    FOnDeskillzSpectatorRoomSwitched OnRoomSwitched;

    // ========================================================================
    // Game State Events
    // ========================================================================

    /** Game state updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|State")
    FOnDeskillzSpectatorGameStateUpdated OnGameStateUpdated;

    /** Phase changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|State")
    FOnDeskillzSpectatorPhaseChanged OnPhaseChanged;

    /** Pot updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|State")
    FOnDeskillzSpectatorPotUpdated OnPotUpdated;

    /** Spectator count changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|State")
    FOnDeskillzSpectatorCountChanged OnSpectatorCountChanged;

    // ========================================================================
    // Player Events
    // ========================================================================

    /** Player updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Players")
    FOnDeskillzSpectatorPlayerUpdated OnPlayerUpdated;

    /** Player joined */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Players")
    FOnDeskillzSpectatorPlayerJoined OnPlayerJoined;

    /** Player left */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Players")
    FOnDeskillzSpectatorPlayerLeft OnPlayerLeft;

    /** Score updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Players")
    FOnDeskillzSpectatorScoreUpdated OnScoreUpdated;

    // ========================================================================
    // Turn Events
    // ========================================================================

    /** Turn changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Turn")
    FOnDeskillzSpectatorTurnChanged OnTurnChanged;

    /** Turn timer update */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Turn")
    FOnDeskillzSpectatorTurnTimerUpdate OnTurnTimerUpdate;

    // ========================================================================
    // Round Events
    // ========================================================================

    /** Round started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Round")
    FOnDeskillzSpectatorRoundStarted OnRoundStarted;

    /** Round ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Round")
    FOnDeskillzSpectatorRoundEnded OnRoundEnded;

    // ========================================================================
    // Pause Events
    // ========================================================================

    /** Game paused */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Pause")
    FOnDeskillzSpectatorGamePaused OnGamePaused;

    /** Game resumed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Pause")
    FOnDeskillzSpectatorGameResumed OnGameResumed;

    // ========================================================================
    // Action Feed Events
    // ========================================================================

    /** Game action occurred */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Actions")
    FOnDeskillzSpectatorGameAction OnGameAction;

    /** Action feed updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|Actions")
    FOnDeskillzSpectatorActionFeedUpdated OnActionFeedUpdated;

    // ========================================================================
    // View Events
    // ========================================================================

    /** View mode changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|View")
    FOnDeskillzSpectatorViewModeChanged OnViewModeChanged;

    /** Settings updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events|View")
    FOnDeskillzSpectatorSettingsUpdated OnSettingsUpdated;

    // ========================================================================
    // Broadcast Methods (Internal)
    // ========================================================================

    void BroadcastConnected(const FDeskillzSpectatorSession& Session);
    void BroadcastDisconnected(const FString& Reason, bool bWasClean);
    void BroadcastConnectionStateChanged(EDeskillzSpectatorConnectionState NewState);
    void BroadcastGameStateUpdated(const FDeskillzGameStateSnapshot& State);
    void BroadcastScoreUpdated(const FDeskillzScoreUpdate& Update);
    void BroadcastGameAction(const FDeskillzGameAction& Action);
    void BroadcastRoundStarted(int32 RoundNumber);
    void BroadcastRoundEnded(const FDeskillzSpectatorRoundEnd& Result);
    void BroadcastViewModeChanged(EDeskillzSpectatorViewMode NewMode, const FString& FollowedPlayerId);

private:
    static UDeskillzSpectatorEvents* Instance;
};

// ============================================================================
// Spectator Blueprint Function Library
// ============================================================================

/**
 * Blueprint function library for spectator convenience functions
 */
UCLASS()
class DESKILLZ_API UDeskillzSpectatorEventLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Check if currently spectating
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static bool IsSpectating(const UObject* WorldContextObject);

    /**
     * Get spectator count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static int32 GetSpectatorCount(const UObject* WorldContextObject);

    /**
     * Get current round
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static int32 GetCurrentRound(const UObject* WorldContextObject);

    /**
     * Get current pot
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static double GetCurrentPot(const UObject* WorldContextObject);

    /**
     * Get player count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static int32 GetPlayerCount(const UObject* WorldContextObject);

    /**
     * Get watched room count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static int32 GetWatchedRoomCount(const UObject* WorldContextObject);

    /**
     * Check if game is paused
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static bool IsGamePaused(const UObject* WorldContextObject);

    /**
     * Get current turn player name
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static FString GetCurrentTurnPlayerName(const UObject* WorldContextObject);

    /**
     * Get remaining turn time
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static float GetRemainingTurnTime(const UObject* WorldContextObject);

    /**
     * Get current view mode
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static EDeskillzSpectatorViewMode GetViewMode(const UObject* WorldContextObject);

    /**
     * Check if can add more rooms
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject"))
    static bool CanAddMoreRooms(const UObject* WorldContextObject);

    /**
     * Get action type display name
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    static FString GetActionTypeName(EDeskillzGameActionType ActionType);

    /**
     * Get connection state display name
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    static FString GetConnectionStateName(EDeskillzSpectatorConnectionState State);

    /**
     * Get view mode display name
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    static FString GetViewModeName(EDeskillzSpectatorViewMode Mode);
};