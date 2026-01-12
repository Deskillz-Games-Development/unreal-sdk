// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeskillzSocialTypes.h"
#include "DeskillzSocialEvents.generated.h"

// ============================================================================
// Additional Social Game Event Delegates
// ============================================================================

/** Called when session created */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialSessionCreated, const FString&, SessionId, const FString&, RoomCode);

/** Called when joining session */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialJoiningSession, const FString&, RoomId);

/** Called when session join fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialJoinFailed, const FString&, RoomId, const FString&, ErrorMessage);

/** Called when player reconnects */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialPlayerReconnected, const FString&, PlayerId);

/** Called when player status changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialPlayerStatusChanged, const FString&, PlayerId, EDeskillzSocialPlayerStatus, NewStatus);

/** Called when seat changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzSocialSeatChanged, const FString&, PlayerId, int32, OldSeat, int32, NewSeat);

/** Called when rebuy completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzSocialRebuyCompleted, bool, bSuccess, double, Amount, int32, NewBalance);

/** Called when low balance warning */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialLowBalance, int32, CurrentBalance, int32, MinRequired);

/** Called when action required (your turn) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialActionRequired, float, TimeLimit);

/** Called when action submitted */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialActionSubmitted, const FString&, ActionType, bool, bSuccess);

/** Called when other player acts */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeskillzSocialPlayerAction, const FString&, PlayerId, const FString&, ActionType, double, Amount);

/** Called when pause vote needed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialPauseVoteNeeded, const FString&, RequesterId, float, VoteTimeLimit);

/** Called when pause countdown starts */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialPauseCountdown, int32, SecondsRemaining);

/** Called when session about to end */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSocialSessionEnding, int32, RoundsRemaining);

/** Called when rake collected */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzSocialRakeCollected, double, Amount, int32, RoundNumber);

// ============================================================================
// Social Game Event Aggregator
// ============================================================================

/**
 * Deskillz Social Game Event Aggregator
 * 
 * Central hub for all social game events. Provides easy Blueprint access
 * to subscribe to any social game event in one place.
 * 
 * Usage in Blueprint:
 * - Get Deskillz Social Events node
 * - Bind to desired event (e.g., OnTurnStarted, OnRoundEnded)
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSocialEvents : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzSocialEvents();

    /** Get the singleton social events instance */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social|Events", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Social Events"))
    static UDeskillzSocialEvents* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Session Events
    // ========================================================================

    /** Session created */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Session")
    FOnDeskillzSocialSessionCreated OnSessionCreated;

    /** Joining session */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Session")
    FOnDeskillzSocialJoiningSession OnJoiningSession;

    /** Join failed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Session")
    FOnDeskillzSocialJoinFailed OnJoinFailed;

    /** Session state changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Session")
    FOnDeskillzSocialSessionStateChanged OnSessionStateChanged;

    /** Session ending soon */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Session")
    FOnDeskillzSocialSessionEnding OnSessionEnding;

    /** Session settlement */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Session")
    FOnDeskillzSocialSessionSettlement OnSessionSettlement;

    // ========================================================================
    // Player Events
    // ========================================================================

    /** Player joined */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Players")
    FOnDeskillzSocialPlayerJoined OnPlayerJoined;

    /** Player left */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Players")
    FOnDeskillzSocialPlayerLeft OnPlayerLeft;

    /** Player reconnected */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Players")
    FOnDeskillzSocialPlayerReconnected OnPlayerReconnected;

    /** Player status changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Players")
    FOnDeskillzSocialPlayerStatusChanged OnPlayerStatusChanged;

    /** Seat changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Players")
    FOnDeskillzSocialSeatChanged OnSeatChanged;

    // ========================================================================
    // Balance Events
    // ========================================================================

    /** Balance updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Balance")
    FOnDeskillzSocialBalanceUpdated OnBalanceUpdated;

    /** Buy-in completed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Balance")
    FOnDeskillzSocialBuyInCompleted OnBuyInCompleted;

    /** Rebuy completed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Balance")
    FOnDeskillzSocialRebuyCompleted OnRebuyCompleted;

    /** Cash out completed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Balance")
    FOnDeskillzSocialCashOutCompleted OnCashOutCompleted;

    /** Low balance warning */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Balance")
    FOnDeskillzSocialLowBalance OnLowBalance;

    // ========================================================================
    // Turn Events
    // ========================================================================

    /** Turn started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Turn")
    FOnDeskillzSocialTurnStarted OnTurnStarted;

    /** Turn timer tick */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Turn")
    FOnDeskillzSocialTurnTimerTick OnTurnTimerTick;

    /** Turn timer expired */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Turn")
    FOnDeskillzSocialTurnTimerExpired OnTurnTimerExpired;

    /** Action required (your turn) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Turn")
    FOnDeskillzSocialActionRequired OnActionRequired;

    /** Action submitted */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Turn")
    FOnDeskillzSocialActionSubmitted OnActionSubmitted;

    /** Player action (other players) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Turn")
    FOnDeskillzSocialPlayerAction OnPlayerAction;

    // ========================================================================
    // Round Events
    // ========================================================================

    /** Round started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Round")
    FOnDeskillzSocialRoundStarted OnRoundStarted;

    /** Round ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Round")
    FOnDeskillzSocialRoundEnded OnRoundEnded;

    /** Pot updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Round")
    FOnDeskillzSocialPotUpdated OnPotUpdated;

    /** Rake collected */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Round")
    FOnDeskillzSocialRakeCollected OnRakeCollected;

    // ========================================================================
    // Pause Events
    // ========================================================================

    /** Pause requested */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Pause")
    FOnDeskillzSocialPauseRequested OnPauseRequested;

    /** Pause vote updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Pause")
    FOnDeskillzSocialPauseVoteUpdated OnPauseVoteUpdated;

    /** Pause vote needed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Pause")
    FOnDeskillzSocialPauseVoteNeeded OnPauseVoteNeeded;

    /** Pause countdown */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Pause")
    FOnDeskillzSocialPauseCountdown OnPauseCountdown;

    /** Game paused */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Pause")
    FOnDeskillzSocialGamePaused OnGamePaused;

    /** Game resumed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events|Pause")
    FOnDeskillzSocialGameResumed OnGameResumed;

    // ========================================================================
    // Broadcast Methods (Internal)
    // ========================================================================

    void BroadcastSessionStateChanged(EDeskillzSocialSessionState NewState, EDeskillzSocialSessionState OldState);
    void BroadcastPlayerJoined(const FDeskillzSocialPlayer& Player);
    void BroadcastPlayerLeft(const FString& PlayerId, const FString& Reason);
    void BroadcastBalanceUpdated(const FString& PlayerId, int32 NewBalance, int32 Delta);
    void BroadcastTurnStarted(const FString& PlayerId, float TimeLimit);
    void BroadcastRoundStarted(int32 RoundNumber);
    void BroadcastRoundEnded(const FDeskillzSocialRoundResult& Result);
    void BroadcastGamePaused(float DurationSeconds);
    void BroadcastGameResumed();

private:
    static UDeskillzSocialEvents* Instance;
};

// ============================================================================
// Social Game Blueprint Function Library
// ============================================================================

/**
 * Blueprint function library for social game convenience functions
 */
UCLASS()
class DESKILLZ_API UDeskillzSocialEventLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Check if in a social game session
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static bool IsInSocialSession(const UObject* WorldContextObject);

    /**
     * Check if it's local player's turn
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static bool IsMyTurn(const UObject* WorldContextObject);

    /**
     * Get local player balance in points
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static int32 GetLocalBalance(const UObject* WorldContextObject);

    /**
     * Get local player balance in dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static double GetLocalBalanceDollars(const UObject* WorldContextObject);

    /**
     * Get current pot in dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static double GetCurrentPot(const UObject* WorldContextObject);

    /**
     * Get current round number
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static int32 GetCurrentRound(const UObject* WorldContextObject);

    /**
     * Check if game is paused
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static bool IsGamePaused(const UObject* WorldContextObject);

    /**
     * Get remaining turn time
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static float GetRemainingTurnTime(const UObject* WorldContextObject);

    /**
     * Check if can rebuy
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static bool CanRebuy(const UObject* WorldContextObject);

    /**
     * Get remaining rebuys
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static int32 GetRemainingRebuys(const UObject* WorldContextObject);

    /**
     * Get active player count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static int32 GetActivePlayerCount(const UObject* WorldContextObject);

    /**
     * Convert points to dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static double PointsToDollars(const UObject* WorldContextObject, int32 Points);

    /**
     * Convert dollars to points
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject"))
    static int32 DollarsToPoints(const UObject* WorldContextObject, double Dollars);
};