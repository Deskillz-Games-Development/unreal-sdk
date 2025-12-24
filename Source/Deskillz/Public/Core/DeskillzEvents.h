// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeskillzTypes.h"
#include "DeskillzEvents.generated.h"

// ============================================================================
// Additional Event Delegates
// ============================================================================

/** Called when user authentication state changes */
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
	// Authentication Events
	// ========================================================================
	
	/** User auth state changed (login/logout) */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events|Auth")
	FOnDeskillzAuthStateChanged OnAuthStateChanged;
	
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
	// Internal - Event Broadcasting
	// ========================================================================
	
	/** Broadcast SDK initialized */
	void BroadcastSDKInitialized(bool bSuccess, const FDeskillzError& Error);
	
	/** Broadcast error */
	void BroadcastError(const FDeskillzError& Error);
	
	/** Broadcast auth state changed */
	void BroadcastAuthStateChanged(bool bIsAuthenticated, const FDeskillzPlayer& Player);
	
	/** Broadcast match started */
	void BroadcastMatchStarted(const FDeskillzMatchInfo& MatchInfo, const FDeskillzError& Error);
	
	/** Broadcast match completed */
	void BroadcastMatchCompleted(const FDeskillzMatchResult& Result, const FDeskillzError& Error);
	
	/** Broadcast score submitted */
	void BroadcastScoreSubmitted(bool bSuccess, const FDeskillzError& Error);
	
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
};
