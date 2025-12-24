// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeskillzTypes.h"
#include "DeskillzConfig.h"
#include "Interfaces/IHttpRequest.h"
#include "DeskillzSDK.generated.h"

class IWebSocket;
class FJsonObject;

/**
 * Deskillz SDK - Main Entry Point
 * 
 * The primary interface for integrating your Unreal game with the Deskillz
 * competitive gaming platform. Provides access to tournaments, matchmaking,
 * cryptocurrency prizes, and real-time multiplayer.
 * 
 * Usage:
 * 1. Configure API Key and Game ID in Project Settings > Plugins > Deskillz SDK
 * 2. Call Initialize() at game startup
 * 3. Use GetTournaments(), JoinTournament(), SubmitScore(), etc.
 * 
 * Blueprint Access:
 * - Get via "Get Game Instance Subsystem" > "Deskillz SDK"
 * - Or use the static BlueprintCallable functions
 * 
 * @see UDeskillzConfig for configuration options
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSDK : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UDeskillzSDK();
	
	// ========================================================================
	// Subsystem Lifecycle
	// ========================================================================
	
	/** Called when the subsystem is created */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/** Called when the subsystem is destroyed */
	virtual void Deinitialize() override;
	
	/** Determine if this subsystem should be created */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	// ========================================================================
	// Static Accessors
	// ========================================================================
	
	/**
	 * Get the Deskillz SDK instance
	 * @param WorldContextObject Any UObject to get world context from
	 * @return The SDK instance or nullptr if not available
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Deskillz SDK"))
	static UDeskillzSDK* Get(const UObject* WorldContextObject);
	
	/**
	 * Check if the SDK is initialized and ready
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz")
	static bool IsInitialized(const UObject* WorldContextObject);
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize the SDK with configured settings
	 * Uses API Key and Game ID from Project Settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Core")
	void InitializeSDK();
	
	/**
	 * Initialize the SDK with explicit credentials
	 * @param InAPIKey Your Deskillz API key
	 * @param InGameId Your Game ID
	 * @param InEnvironment Target environment (Sandbox/Production)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Core")
	void InitializeWithCredentials(const FString& InAPIKey, const FString& InGameId, EDeskillzEnvironment InEnvironment);
	
	/**
	 * Shutdown the SDK and cleanup resources
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Core")
	void Shutdown();
	
	/**
	 * Get current SDK state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Core")
	EDeskillzSDKState GetSDKState() const { return SDKState; }
	
	/**
	 * Check if SDK is ready for use
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Core")
	bool IsReady() const { return SDKState == EDeskillzSDKState::Initialized; }
	
	// ========================================================================
	// Authentication
	// ========================================================================
	
	/**
	 * Authenticate user with their wallet
	 * @param WalletAddress User's cryptocurrency wallet address
	 * @param Signature Signed message proving wallet ownership
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
	void AuthenticateWithWallet(const FString& WalletAddress, const FString& Signature);
	
	/**
	 * Authenticate with username/password (alternative to wallet)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
	void AuthenticateWithCredentials(const FString& Username, const FString& Password);
	
	/**
	 * Logout current user
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
	void Logout();
	
	/**
	 * Check if a user is authenticated
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
	bool IsAuthenticated() const { return bIsAuthenticated; }
	
	/**
	 * Get the current authenticated player
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
	FDeskillzPlayer GetCurrentPlayer() const { return CurrentPlayer; }
	
	// ========================================================================
	// Tournaments
	// ========================================================================
	
	/**
	 * Fetch available tournaments for this game
	 * Results returned via OnTournamentsReceived delegate
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void GetTournaments();
	
	/**
	 * Get tournaments with filters
	 * @param Status Filter by tournament status
	 * @param MatchType Filter by match type
	 * @param MinEntryFee Minimum entry fee filter
	 * @param MaxEntryFee Maximum entry fee filter
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void GetTournamentsFiltered(EDeskillzTournamentStatus Status, EDeskillzMatchType MatchType, float MinEntryFee = 0.0f, float MaxEntryFee = 10000.0f);
	
	/**
	 * Get details for a specific tournament
	 * @param TournamentId The tournament ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void GetTournamentDetails(const FString& TournamentId);
	
	/**
	 * Join a tournament
	 * @param TournamentId The tournament to join
	 * @param Currency Currency to pay entry fee with
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void JoinTournament(const FString& TournamentId, EDeskillzCurrency Currency = EDeskillzCurrency::USDT);
	
	/**
	 * Leave a tournament (before it starts)
	 * @param TournamentId The tournament to leave
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Tournaments")
	void LeaveTournament(const FString& TournamentId);
	
	// ========================================================================
	// Matchmaking
	// ========================================================================
	
	/**
	 * Start matchmaking for a tournament
	 * @param TournamentId The tournament to find a match in
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void StartMatchmaking(const FString& TournamentId);
	
	/**
	 * Cancel matchmaking
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void CancelMatchmaking();
	
	/**
	 * Check if currently in matchmaking
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	bool IsInMatchmaking() const { return bIsMatchmaking; }
	
	/**
	 * Get current match info (if in a match)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	FDeskillzMatchInfo GetCurrentMatch() const { return CurrentMatch; }
	
	/**
	 * Check if currently in a match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	bool IsInMatch() const { return CurrentMatch.IsInProgress(); }
	
	// ========================================================================
	// Match Lifecycle
	// ========================================================================
	
	/**
	 * Signal that the match has started (gameplay beginning)
	 * Call this when your game scene is loaded and ready
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void StartMatch();
	
	/**
	 * Update the player's score during gameplay
	 * @param Score Current score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void UpdateScore(int64 Score);
	
	/**
	 * Submit final score and end the match
	 * @param FinalScore The player's final score
	 * @param bForceSubmit Force submission even if validation fails
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void SubmitScore(int64 FinalScore, bool bForceSubmit = false);
	
	/**
	 * Abort the current match (forfeit)
	 * @param Reason Reason for aborting
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void AbortMatch(const FString& Reason = TEXT(""));
	
	/**
	 * Get the current score
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	int64 GetCurrentScore() const { return CurrentScore; }
	
	/**
	 * Get remaining match time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	float GetRemainingTime() const;
	
	/**
	 * Get elapsed match time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	float GetElapsedTime() const;
	
	// ========================================================================
	// Practice Mode
	// ========================================================================
	
	/**
	 * Start a practice session (no entry fee)
	 * @param DurationSeconds How long the practice session lasts
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Practice")
	void StartPractice(int32 DurationSeconds = 180);
	
	/**
	 * End practice session
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Practice")
	void EndPractice();
	
	/**
	 * Check if in practice mode
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Practice")
	bool IsInPractice() const { return bIsInPractice; }
	
	// ========================================================================
	// Wallet & Currency
	// ========================================================================
	
	/**
	 * Fetch wallet balances for all supported currencies
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Wallet")
	void GetWalletBalances();
	
	/**
	 * Get cached wallet balance for a specific currency
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Wallet")
	FDeskillzWalletBalance GetBalance(EDeskillzCurrency Currency) const;
	
	/**
	 * Check if user has sufficient funds for entry fee
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Wallet")
	bool HasSufficientFunds(const FDeskillzEntryFee& EntryFee) const;
	
	// ========================================================================
	// Leaderboards
	// ========================================================================
	
	/**
	 * Get leaderboard for a tournament
	 * @param TournamentId Tournament ID
	 * @param Period Time period for leaderboard
	 * @param Offset Starting position (for pagination)
	 * @param Limit Number of entries to fetch
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Leaderboard")
	void GetLeaderboard(const FString& TournamentId, EDeskillzLeaderboardPeriod Period = EDeskillzLeaderboardPeriod::AllTime, int32 Offset = 0, int32 Limit = 100);
	
	/**
	 * Get global game leaderboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Leaderboard")
	void GetGlobalLeaderboard(EDeskillzLeaderboardPeriod Period = EDeskillzLeaderboardPeriod::AllTime, int32 Offset = 0, int32 Limit = 100);
	
	// ========================================================================
	// Events / Delegates
	// ========================================================================
	
	/** Called when SDK initialization completes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzInitialized OnInitialized;
	
	/** Called when tournament list is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzTournamentsReceived OnTournamentsReceived;
	
	/** Called when a match starts */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzMatchStarted OnMatchStarted;
	
	/** Called when a match completes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzMatchCompleted OnMatchCompleted;
	
	/** Called when a score is submitted */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzScoreSubmitted OnScoreSubmitted;
	
	/** Called when wallet balances are updated */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzWalletUpdated OnWalletUpdated;
	
	/** Called when leaderboard data is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzLeaderboardReceived OnLeaderboardReceived;
	
	/** Called when an error occurs */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnDeskillzError OnError;
	
	// ========================================================================
	// Utility
	// ========================================================================
	
	/**
	 * Get the SDK version string
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Utility")
	static FString GetSDKVersion();
	
	/**
	 * Get the current environment
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Utility")
	EDeskillzEnvironment GetEnvironment() const { return ActiveEnvironment; }
	
	/**
	 * Generate a unique device ID for this device
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Utility")
	static FString GetDeviceId();
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Current SDK state */
	UPROPERTY()
	EDeskillzSDKState SDKState = EDeskillzSDKState::Uninitialized;
	
	/** Active environment */
	UPROPERTY()
	EDeskillzEnvironment ActiveEnvironment = EDeskillzEnvironment::Sandbox;
	
	/** API Key */
	FString APIKey;
	
	/** Game ID */
	FString GameId;
	
	/** Authentication token */
	FString AuthToken;
	
	/** Is user authenticated */
	bool bIsAuthenticated = false;
	
	/** Current player data */
	UPROPERTY()
	FDeskillzPlayer CurrentPlayer;
	
	/** Is currently matchmaking */
	bool bIsMatchmaking = false;
	
	/** Current match data */
	UPROPERTY()
	FDeskillzMatchInfo CurrentMatch;
	
	/** Current score */
	int64 CurrentScore = 0;
	
	/** Match start time */
	FDateTime MatchStartTime;
	
	/** Is in practice mode */
	bool bIsInPractice = false;
	
	/** Cached wallet balances */
	UPROPERTY()
	TMap<EDeskillzCurrency, FDeskillzWalletBalance> WalletBalances;
	
	/** Active endpoints */
	FDeskillzEndpoints ActiveEndpoints;
	
	/** WebSocket connection */
	TSharedPtr<IWebSocket> WebSocket;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Make an authenticated API request */
	void MakeAPIRequest(const FString& Endpoint, const FString& Method, const TSharedPtr<FJsonObject>& Body, TFunction<void(TSharedPtr<FJsonObject>, FDeskillzError)> Callback);
	
	/** Handle HTTP response */
	void HandleHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, TFunction<void(TSharedPtr<FJsonObject>, FDeskillzError)> Callback);
	
	/** Parse JSON response */
	TSharedPtr<FJsonObject> ParseJsonResponse(const FString& Content);
	
	/** Connect WebSocket for real-time features */
	void ConnectWebSocket();
	
	/** Disconnect WebSocket */
	void DisconnectWebSocket();
	
	/** Handle WebSocket message */
	void OnWebSocketMessage(const FString& Message);
	
	/** Handle WebSocket connection */
	void OnWebSocketConnected();
	
	/** Handle WebSocket disconnection */
	void OnWebSocketDisconnected(int32 StatusCode, const FString& Reason, bool bWasClean);
	
	/** Handle WebSocket error */
	void OnWebSocketError(const FString& Error);
	
	/** Broadcast error to delegates */
	void BroadcastError(const FDeskillzError& Error);
	
	/** Timer handle for match timing */
	FTimerHandle MatchTimerHandle;
	
	/** WebSocket reconnect timer */
	FTimerHandle WebSocketReconnectHandle;
	
	/** Number of WebSocket reconnect attempts */
	int32 WebSocketReconnectAttempts = 0;
};
