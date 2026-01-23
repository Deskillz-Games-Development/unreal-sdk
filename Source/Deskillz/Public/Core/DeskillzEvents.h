// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeskillzTypes.h"
#include "DeskillzEvents.generated.h"

// ============================================================================
// Additional Event Delegates
// ============================================================================

/** Called when user authentication state changes (LEGACY - use FOnDeskillzAuthStateChangedNew) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzAuthStateChanged, bool, bIsAuthenticated, const FDeskillzPlayer&, Player);

/** Called when matchmaking status changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzMatchmakingStatus, bool, bIsSearching, float, WaitTime);

/** Called when opponent found during matchmaking */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzOpponentFound, const FDeskillzPlayer&, Opponent);

/** Called when match time updates (every second during gameplay) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzMatchTimeUpdate, float, RemainingTime, float, ElapsedTime);

/** Called when score updates (real-time for synchronous matches) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzScoreUpdate, int64, PlayerScore, int64, OpponentScore);

/** Called when tournament joined successfully */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzTournamentJoined, const FDeskillzTournament&, Tournament);

/** Called when tournament state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzTournamentStateChanged, const FString&, TournamentId, EDeskillzTournamentStatus, NewStatus);

/** Called when connection state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzConnectionStateChanged, bool, bIsConnected);

/** Called when practice mode starts */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzPracticeStarted, int32, DurationSeconds);

/** Called when practice mode ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzPracticeEnded, int64, FinalScore);

// ============================================================================
// Authentication Delegates (Self-Sufficient Architecture)
// ============================================================================

/** Called when user successfully logs in (email/password or social) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzAuthLoginSuccess, const FDeskillzAuthUser&, User);

/** Called when user successfully signs up */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzAuthSignUpSuccess, const FDeskillzAuthUser&, User);

/** Called when user logs out */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzAuthLogout);

/** Called when authentication fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzAuthErrorEvent, const FString&, ErrorMessage);

/** Called when auth state changes (new version with EDeskillzAuthState) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzAuthStateChangedNew, EDeskillzAuthState, NewState, const FDeskillzAuthUser&, User);

/** Called when wallet is linked to account */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzWalletLinked, const FString&, WalletAddress);

/** Called when wallet is disconnected from account */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzWalletUnlinked);

/** Called when password reset email is sent */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzPasswordResetSent);

/** Called when auth flow is complete (ready for lobby) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzAuthFlowComplete, const FDeskillzAuthUser&, User);

/** Called when scene/level changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSceneChanged, const FString&, SceneName);

/** Called when wallet is required for an action */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzWalletRequired, const FString&, Reason);

// ============================================================================
// Event Aggregator
// ============================================================================

/**
 * Deskillz Event Aggregator
 * 
 * Central hub for all Deskillz SDK events. Provides easy Blueprint access
 * to subscribe to any SDK event in one place.
 * 
 * Usage in Blueprint:
 * - Get Deskillz Events node
 * - Bind to desired event (e.g., OnMatchStarted, OnScoreSubmitted)
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzEvents : public UObject
{
    GENERATED_BODY()
    
public:
    UDeskillzEvents();
    
    /** Get the singleton events instance */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Deskillz Events"))
    static UDeskillzEvents* Get(const UObject* WorldContextObject);
    
    // ========================================================================
    // Core Events
    // ========================================================================
    
    /** SDK initialized (or failed to initialize) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Core")
    FOnDeskillzInitialized OnSDKInitialized;
    
    /** Error occurred */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Core")
    FOnDeskillzError OnError;
    
    /** Connection state changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Core")
    FOnDeskillzConnectionStateChanged OnConnectionStateChanged;
    
    // ========================================================================
    // Authentication Events (Legacy)
    // ========================================================================
    
    /** User auth state changed - LEGACY (use OnAuthStateChangedNew for Self-Sufficient Architecture) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzAuthStateChanged OnAuthStateChangedLegacy;
    
    // ========================================================================
    // Authentication Events (Self-Sufficient Architecture)
    // ========================================================================

    /** Called when user successfully logs in */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzAuthLoginSuccess OnAuthLoginSuccess;

    /** Called when user successfully signs up */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzAuthSignUpSuccess OnAuthSignUpSuccess;

    /** Called when user logs out */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzAuthLogout OnAuthLogout;

    /** Called when authentication fails */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzAuthErrorEvent OnAuthError;

    /** Called when auth state changes (new - with EDeskillzAuthState enum) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzAuthStateChangedNew OnAuthStateChangedNew;

    /** Called when wallet is linked to account */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzWalletLinked OnWalletLinked;

    /** Called when wallet is disconnected from account */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzWalletUnlinked OnWalletUnlinked;

    /** Called when password reset email is sent */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzPasswordResetSent OnPasswordResetSent;

    /** Called when auth flow is complete */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzAuthFlowComplete OnAuthFlowComplete;

    /** Called when wallet is required for an action */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
    FOnDeskillzWalletRequired OnWalletRequired;

    // ========================================================================
    // Navigation Events
    // ========================================================================

    /** Called when scene/level changes */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Navigation")
    FOnDeskillzSceneChanged OnSceneChanged;
    
    // ========================================================================
    // Tournament Events
    // ========================================================================
    
    /** Tournament list received */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Tournaments")
    FOnDeskillzTournamentsReceived OnTournamentsReceived;
    
    /** Successfully joined tournament */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Tournaments")
    FOnDeskillzTournamentJoined OnTournamentJoined;
    
    /** Tournament status changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Tournaments")
    FOnDeskillzTournamentStateChanged OnTournamentStateChanged;
    
    // ========================================================================
    // Matchmaking Events
    // ========================================================================
    
    /** Matchmaking status update */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Matchmaking")
    FOnDeskillzMatchmakingStatus OnMatchmakingStatus;
    
    /** Opponent found */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Matchmaking")
    FOnDeskillzOpponentFound OnOpponentFound;
    
    // ========================================================================
    // Match Events
    // ========================================================================
    
    /** Match started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Match")
    FOnDeskillzMatchStarted OnMatchStarted;
    
    /** Match completed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Match")
    FOnDeskillzMatchCompleted OnMatchCompleted;
    
    /** Match time update (fires every second) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Match")
    FOnDeskillzMatchTimeUpdate OnMatchTimeUpdate;
    
    /** Score update (real-time for sync matches) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Match")
    FOnDeskillzScoreUpdate OnScoreUpdate;
    
    /** Score submitted */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Match")
    FOnDeskillzScoreSubmitted OnScoreSubmitted;
    
    // ========================================================================
    // Practice Events
    // ========================================================================
    
    /** Practice session started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Practice")
    FOnDeskillzPracticeStarted OnPracticeStarted;
    
    /** Practice session ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Practice")
    FOnDeskillzPracticeEnded OnPracticeEnded;
    
    // ========================================================================
    // Wallet Events
    // ========================================================================
    
    /** Wallet balances updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Wallet")
    FOnDeskillzWalletUpdated OnWalletUpdated;
    
    // ========================================================================
    // Leaderboard Events
    // ========================================================================
    
    /** Leaderboard data received */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Leaderboard")
    FOnDeskillzLeaderboardReceived OnLeaderboardReceived;

    // ========================================================================
    // Internal - Event Broadcasting (Legacy)
    // ========================================================================
    
    /** Broadcast SDK initialized */
    void BroadcastSDKInitialized(bool bSuccess, const FDeskillzError& Error);
    
    /** Broadcast error */
    void BroadcastError(const FDeskillzError& Error);
    
    /** Broadcast auth state changed (legacy) */
    void BroadcastAuthStateChanged(bool bIsAuthenticated, const FDeskillzPlayer& Player);
    
    /** Broadcast match started */
    void BroadcastMatchStarted(const FDeskillzMatchInfo& MatchInfo, const FDeskillzError& Error);
    
    /** Broadcast match completed */
    void BroadcastMatchCompleted(const FDeskillzMatchResult& Result, const FDeskillzError& Error);
    
    /** Broadcast score submitted */
    void BroadcastScoreSubmitted(bool bSuccess, const FDeskillzError& Error);

    // ========================================================================
    // Internal - Event Broadcasting (Self-Sufficient Architecture)
    // ========================================================================

    /** Broadcast login success */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastAuthLoginSuccess(const FDeskillzAuthUser& User)
    {
        OnAuthLoginSuccess.Broadcast(User);
    }

    /** Broadcast sign up success */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastAuthSignUpSuccess(const FDeskillzAuthUser& User)
    {
        OnAuthSignUpSuccess.Broadcast(User);
    }

    /** Broadcast logout */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastAuthLogout()
    {
        OnAuthLogout.Broadcast();
    }

    /** Broadcast auth error */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastAuthError(const FString& ErrorMessage)
    {
        OnAuthError.Broadcast(ErrorMessage);
    }

    /** Broadcast auth state changed (new) */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastAuthStateChangedNew(EDeskillzAuthState NewState, const FDeskillzAuthUser& User)
    {
        OnAuthStateChangedNew.Broadcast(NewState, User);
    }

    /** Broadcast wallet linked */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastWalletLinked(const FString& WalletAddress)
    {
        OnWalletLinked.Broadcast(WalletAddress);
    }

    /** Broadcast wallet unlinked */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastWalletUnlinked()
    {
        OnWalletUnlinked.Broadcast();
    }

    /** Broadcast password reset sent */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastPasswordResetSent()
    {
        OnPasswordResetSent.Broadcast();
    }

    /** Broadcast auth flow complete */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastAuthFlowComplete(const FDeskillzAuthUser& User)
    {
        OnAuthFlowComplete.Broadcast(User);
    }

    /** Broadcast scene changed */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Navigation")
    void BroadcastSceneChanged(const FString& SceneName)
    {
        OnSceneChanged.Broadcast(SceneName);
    }

    /** Broadcast wallet required */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Events|Auth")
    void BroadcastWalletRequired(const FString& Reason)
    {
        OnWalletRequired.Broadcast(Reason);
    }
    
private:
    /** Singleton instance */
    static UDeskillzEvents* Instance;
};

// ============================================================================
// Blueprint Function Library for Events
// ============================================================================

/**
 * Blueprint function library for quick event binding
 */
UCLASS()
class DESKILLZ_API UDeskillzEventLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    /**
     * Check if SDK is ready for use
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject"))
    static bool IsSDKReady(const UObject* WorldContextObject);
    
    /**
     * Get current match remaining time
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject"))
    static float GetMatchRemainingTime(const UObject* WorldContextObject);
    
    /**
     * Get current player score
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject"))
    static int64 GetCurrentScore(const UObject* WorldContextObject);
    
    /**
     * Check if in active match
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject"))
    static bool IsInMatch(const UObject* WorldContextObject);
    
    /**
     * Check if in practice mode
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject"))
    static bool IsInPractice(const UObject* WorldContextObject);

    /**
     * Check if user is authenticated
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject"))
    static bool IsAuthenticated(const UObject* WorldContextObject);

    /**
     * Check if user has wallet connected
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (WorldContext = "WorldContextObject"))
    static bool HasWalletConnected(const UObject* WorldContextObject);
};