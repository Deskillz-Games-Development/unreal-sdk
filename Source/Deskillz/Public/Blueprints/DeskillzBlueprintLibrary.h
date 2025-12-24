// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzBlueprintLibrary.generated.h"

/**
 * Deskillz Blueprint Function Library
 * 
 * Static API for Deskillz SDK - Use these functions directly in Blueprints!
 * No setup required - just drag & drop these nodes into your Blueprint graphs.
 * 
 * MINIMAL INTEGRATION (2 nodes):
 * 1. When game ends: Call "Deskillz Submit Score"
 * 2. After submit: Call "Deskillz End Match"
 * 
 * That's it! The SDK handles everything else automatically.
 * 
 * @see UDeskillzSDK for the full C++ API
 */
UCLASS(meta = (ScriptName = "Deskillz"))
class DESKILLZ_API UDeskillzBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// ========================================================================
	// 🎯 QUICK START - Most Common Functions (Start Here!)
	// ========================================================================
	
	/**
	 * Submit the player's final score
	 * Call this when your game ends
	 * 
	 * @param Score The final score to submit
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Quick Start", meta = (DisplayName = "Deskillz Submit Score", Keywords = "deskillz score submit send final"))
	static void SubmitScore(int64 Score);
	
	/**
	 * End the current match
	 * Call this after submitting the score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Quick Start", meta = (DisplayName = "Deskillz End Match", Keywords = "deskillz match end finish complete"))
	static void EndMatch();
	
	/**
	 * Check if we're currently in a Deskillz tournament match
	 * Use this to know when to enable tournament features
	 * 
	 * @return True if in an active tournament match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Quick Start", meta = (DisplayName = "Is In Deskillz Match", Keywords = "deskillz match tournament playing active"))
	static bool IsInMatch();
	
	/**
	 * Get the current match info
	 * Contains opponent info, time limits, match type, etc.
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Quick Start", meta = (DisplayName = "Get Current Match Info", Keywords = "deskillz match info opponent"))
	static FDeskillzMatchInfo GetCurrentMatchInfo();
	
	/**
	 * Get remaining match time in seconds
	 * @return Seconds remaining, 0 if no time limit or match not active
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Quick Start", meta = (DisplayName = "Get Match Time Remaining", Keywords = "deskillz time remaining seconds left"))
	static float GetMatchTimeRemaining();
	
	// ========================================================================
	// 🎮 MATCH FLOW
	// ========================================================================
	
	/**
	 * Signal that the game is ready to start
	 * Call when your game scene is fully loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match", meta = (DisplayName = "Deskillz Match Ready", Keywords = "deskillz match start ready loaded"))
	static void MatchReady();
	
	/**
	 * Update the score during gameplay (for real-time display)
	 * @param CurrentScore The player's current score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match", meta = (DisplayName = "Update Score", Keywords = "deskillz score update current"))
	static void UpdateScore(int64 CurrentScore);
	
	/**
	 * Abort/forfeit the current match
	 * @param Reason Optional reason for aborting
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match", meta = (DisplayName = "Abort Match", Keywords = "deskillz match abort forfeit quit"))
	static void AbortMatch(const FString& Reason = TEXT(""));
	
	/**
	 * Get the random seed for this match (ensures fair play)
	 * Use this to seed your random number generator for deterministic gameplay
	 * @return Match random seed (same for both players in sync matches)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match", meta = (DisplayName = "Get Match Random Seed", Keywords = "deskillz random seed deterministic"))
	static int64 GetMatchRandomSeed();
	
	/**
	 * Is this a synchronous (real-time) match?
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match", meta = (DisplayName = "Is Sync Match", Keywords = "deskillz sync synchronous realtime multiplayer"))
	static bool IsSyncMatch();
	
	/**
	 * Get elapsed match time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match", meta = (DisplayName = "Get Match Elapsed Time", Keywords = "deskillz time elapsed played"))
	static float GetMatchElapsedTime();
	
	// ========================================================================
	// 🏆 TOURNAMENTS
	// ========================================================================
	
	/**
	 * Show the tournament selection UI
	 * Opens the built-in Deskillz tournament browser
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments", meta = (DisplayName = "Show Tournaments", Keywords = "deskillz tournaments list show browse"))
	static void ShowTournaments();
	
	/**
	 * Join a specific tournament
	 * @param TournamentId The tournament to join
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments", meta = (DisplayName = "Join Tournament", Keywords = "deskillz tournament join enter"))
	static void JoinTournament(const FString& TournamentId);
	
	/**
	 * Start matchmaking for a tournament
	 * @param TournamentId The tournament to find a match in
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments", meta = (DisplayName = "Find Match", Keywords = "deskillz matchmaking find opponent"))
	static void FindMatch(const FString& TournamentId);
	
	/**
	 * Cancel matchmaking
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments", meta = (DisplayName = "Cancel Matchmaking", Keywords = "deskillz matchmaking cancel stop"))
	static void CancelMatchmaking();
	
	/**
	 * Is currently searching for a match?
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Tournaments", meta = (DisplayName = "Is Matchmaking", Keywords = "deskillz matchmaking searching"))
	static bool IsMatchmaking();
	
	// ========================================================================
	// 🎯 PRACTICE MODE
	// ========================================================================
	
	/**
	 * Start a practice session (no entry fee, no prizes)
	 * Great for warmup or learning the game
	 * @param DurationSeconds Practice session length (default 3 minutes)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Practice", meta = (DisplayName = "Start Practice", Keywords = "deskillz practice start warmup free"))
	static void StartPractice(int32 DurationSeconds = 180);
	
	/**
	 * End the current practice session
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Practice", meta = (DisplayName = "End Practice", Keywords = "deskillz practice end stop"))
	static void EndPractice();
	
	/**
	 * Is currently in practice mode?
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Practice", meta = (DisplayName = "Is In Practice", Keywords = "deskillz practice mode"))
	static bool IsInPractice();
	
	// ========================================================================
	// 👤 PLAYER INFO
	// ========================================================================
	
	/**
	 * Get the current player's info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Player", meta = (DisplayName = "Get Current Player", Keywords = "deskillz player current user me"))
	static FDeskillzPlayer GetCurrentPlayer();
	
	/**
	 * Get the opponent's info (if in a match)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Player", meta = (DisplayName = "Get Opponent", Keywords = "deskillz opponent enemy other player"))
	static FDeskillzPlayer GetOpponent();
	
	/**
	 * Is the user logged in?
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Player", meta = (DisplayName = "Is Logged In", Keywords = "deskillz logged in authenticated"))
	static bool IsLoggedIn();
	
	/**
	 * Show login screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Player", meta = (DisplayName = "Show Login", Keywords = "deskillz login signin authenticate"))
	static void ShowLogin();
	
	/**
	 * Log out the current user
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Player", meta = (DisplayName = "Logout", Keywords = "deskillz logout signout"))
	static void Logout();
	
	// ========================================================================
	// 💰 WALLET
	// ========================================================================
	
	/**
	 * Get balance for a specific currency
	 * @param Currency The cryptocurrency to check
	 * @return Balance amount
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Wallet", meta = (DisplayName = "Get Balance", Keywords = "deskillz wallet balance money crypto"))
	static double GetBalance(EDeskillzCurrency Currency);
	
	/**
	 * Get formatted balance string (e.g., "1.234 ETH")
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Wallet", meta = (DisplayName = "Get Balance Text", Keywords = "deskillz wallet balance text formatted"))
	static FString GetBalanceFormatted(EDeskillzCurrency Currency);
	
	/**
	 * Check if user has enough funds for an entry fee
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Wallet", meta = (DisplayName = "Has Enough Funds", Keywords = "deskillz wallet funds enough entry fee"))
	static bool HasEnoughFunds(EDeskillzCurrency Currency, double Amount);
	
	/**
	 * Show the wallet UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Wallet", meta = (DisplayName = "Show Wallet", Keywords = "deskillz wallet show deposit withdraw"))
	static void ShowWallet();
	
	// ========================================================================
	// 🏅 LEADERBOARDS
	// ========================================================================
	
	/**
	 * Show the leaderboard UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Leaderboard", meta = (DisplayName = "Show Leaderboard", Keywords = "deskillz leaderboard show rankings"))
	static void ShowLeaderboard();
	
	/**
	 * Get the player's current rank (0 if not ranked)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Leaderboard", meta = (DisplayName = "Get Player Rank", Keywords = "deskillz rank position leaderboard"))
	static int32 GetPlayerRank();
	
	// ========================================================================
	// ⚙️ SDK STATE
	// ========================================================================
	
	/**
	 * Check if SDK is initialized and ready
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|SDK", meta = (DisplayName = "Is SDK Ready", Keywords = "deskillz sdk ready initialized"))
	static bool IsSDKReady();
	
	/**
	 * Get the current SDK state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|SDK", meta = (DisplayName = "Get SDK State", Keywords = "deskillz sdk state status"))
	static EDeskillzSDKState GetSDKState();
	
	/**
	 * Get SDK version string
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|SDK", meta = (DisplayName = "Get SDK Version", Keywords = "deskillz sdk version"))
	static FString GetSDKVersion();
	
	/**
	 * Get current environment (Production/Sandbox)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|SDK", meta = (DisplayName = "Get Environment", Keywords = "deskillz environment sandbox production"))
	static EDeskillzEnvironment GetEnvironment();
	
	/**
	 * Force re-initialization of the SDK
	 * Normally not needed - SDK initializes automatically
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|SDK", meta = (DisplayName = "Initialize SDK", Keywords = "deskillz sdk initialize setup"))
	static void InitializeSDK();
	
	// ========================================================================
	// 📱 UI HELPERS
	// ========================================================================
	
	/**
	 * Show the results screen after a match
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI", meta = (DisplayName = "Show Results", Keywords = "deskillz results show win lose"))
	static void ShowResults();
	
	/**
	 * Show a Deskillz popup message
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI", meta = (DisplayName = "Show Popup", Keywords = "deskillz popup message alert"))
	static void ShowPopup(const FString& Title, const FString& Message);
	
	/**
	 * Hide all Deskillz UI overlays
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI", meta = (DisplayName = "Hide All UI", Keywords = "deskillz ui hide close"))
	static void HideAllUI();
};
