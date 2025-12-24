// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Network/DeskillzHttpClient.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzApiService.generated.h"

class UDeskillzHttpClient;

// ============================================================================
// API Response Delegates
// ============================================================================

/** Generic success/fail delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzApiResult, bool /*bSuccess*/, const FString& /*Error*/);

/** User profile delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzUserLoaded, bool /*bSuccess*/, const FDeskillzPlayerInfo& /*User*/);

/** Tournament list delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzTournamentsLoaded, bool /*bSuccess*/, const TArray<FDeskillzTournament>& /*Tournaments*/);

/** Single tournament delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzTournamentLoaded, bool /*bSuccess*/, const FDeskillzTournament& /*Tournament*/);

/** Match delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzMatchLoaded, bool /*bSuccess*/, const FDeskillzMatch& /*Match*/);

/** Match result delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzMatchResultLoaded, bool /*bSuccess*/, const FDeskillzMatchResult& /*Result*/);

/** Wallet balances delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzBalancesLoaded, bool /*bSuccess*/, const TMap<FString, double>& /*Balances*/);

/** Leaderboard delegate */
DECLARE_DELEGATE_TwoParams(FOnDeskillzLeaderboardLoaded, bool /*bSuccess*/, const TArray<FDeskillzLeaderboardEntry>& /*Entries*/);

/** Auth delegate */
DECLARE_DELEGATE_ThreeParams(FOnDeskillzAuthResult, bool /*bSuccess*/, const FString& /*Token*/, const FString& /*Error*/);

/**
 * Leaderboard entry for API
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzLeaderboardEntry
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int32 Rank = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	FString PlayerId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	FString Username;
	
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int64 Score = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	int32 Wins = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	float WinRate = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
	bool bIsCurrentPlayer = false;
};

/**
 * Deskillz API Service
 * 
 * High-level API wrapper providing typed methods for all endpoints:
 * - Authentication
 * - User management
 * - Tournament operations
 * - Match lifecycle
 * - Wallet operations
 * - Leaderboards
 * 
 * All methods handle JSON parsing and return typed structures.
 * 
 * Usage:
 *   UDeskillzApiService* Api = UDeskillzApiService::Get();
 *   Api->GetTournaments(FOnDeskillzTournamentsLoaded::CreateLambda([](bool bSuccess, const TArray<FDeskillzTournament>& Tournaments) {
 *       if (bSuccess) { ... }
 *   }));
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzApiService : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzApiService();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the API Service instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|API", meta = (DisplayName = "Get Deskillz API"))
	static UDeskillzApiService* Get();
	
	// ========================================================================
	// Authentication
	// ========================================================================
	
	/**
	 * Login with email and password
	 */
	void Login(const FString& Email, const FString& Password, const FOnDeskillzAuthResult& OnComplete);
	
	/**
	 * Register new account
	 */
	void Register(const FString& Email, const FString& Password, const FString& Username, 
		const FOnDeskillzAuthResult& OnComplete);
	
	/**
	 * Logout current session
	 */
	void Logout(const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Connect crypto wallet
	 */
	void ConnectWallet(const FString& WalletAddress, const FString& Signature, 
		const FOnDeskillzAuthResult& OnComplete);
	
	// ========================================================================
	// User
	// ========================================================================
	
	/**
	 * Get current user profile
	 */
	void GetCurrentUser(const FOnDeskillzUserLoaded& OnComplete);
	
	/**
	 * Get user by ID
	 */
	void GetUser(const FString& UserId, const FOnDeskillzUserLoaded& OnComplete);
	
	/**
	 * Update current user profile
	 */
	void UpdateProfile(const FString& Username, const FString& AvatarUrl, const FOnDeskillzApiResult& OnComplete);
	
	// ========================================================================
	// Tournaments
	// ========================================================================
	
	/**
	 * Get list of tournaments
	 */
	void GetTournaments(const FOnDeskillzTournamentsLoaded& OnComplete, 
		const FString& GameId = TEXT(""), bool bFeaturedOnly = false, int32 Limit = 50);
	
	/**
	 * Get tournament by ID
	 */
	void GetTournament(const FString& TournamentId, const FOnDeskillzTournamentLoaded& OnComplete);
	
	/**
	 * Enter tournament
	 */
	void EnterTournament(const FString& TournamentId, const FString& Currency, 
		const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Leave tournament
	 */
	void LeaveTournament(const FString& TournamentId, const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Get my active tournaments
	 */
	void GetMyTournaments(const FOnDeskillzTournamentsLoaded& OnComplete);
	
	// ========================================================================
	// Matches
	// ========================================================================
	
	/**
	 * Find match (start matchmaking)
	 */
	void FindMatch(const FString& TournamentId, const FOnDeskillzMatchLoaded& OnComplete);
	
	/**
	 * Cancel matchmaking
	 */
	void CancelMatchmaking(const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Get match by ID
	 */
	void GetMatch(const FString& MatchId, const FOnDeskillzMatchLoaded& OnComplete);
	
	/**
	 * Start match
	 */
	void StartMatch(const FString& MatchId, const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Submit score
	 */
	void SubmitScore(const FString& MatchId, int64 Score, const FString& ScoreProof, 
		const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Complete match
	 */
	void CompleteMatch(const FString& MatchId, const FOnDeskillzMatchResultLoaded& OnComplete);
	
	/**
	 * Abort match
	 */
	void AbortMatch(const FString& MatchId, const FString& Reason, const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Get match result
	 */
	void GetMatchResult(const FString& MatchId, const FOnDeskillzMatchResultLoaded& OnComplete);
	
	// ========================================================================
	// Wallet
	// ========================================================================
	
	/**
	 * Get all wallet balances
	 */
	void GetWalletBalances(const FOnDeskillzBalancesLoaded& OnComplete);
	
	/**
	 * Get deposit address for currency
	 */
	void GetDepositAddress(const FString& Currency, 
		TFunction<void(bool, const FString&)> OnComplete);
	
	/**
	 * Request withdrawal
	 */
	void RequestWithdrawal(const FString& Currency, double Amount, const FString& ToAddress,
		const FOnDeskillzApiResult& OnComplete);
	
	/**
	 * Get transaction history
	 */
	void GetTransactions(int32 Page, int32 Limit,
		TFunction<void(bool, const TArray<TSharedPtr<FJsonValue>>&)> OnComplete);
	
	// ========================================================================
	// Leaderboard
	// ========================================================================
	
	/**
	 * Get global leaderboard
	 */
	void GetGlobalLeaderboard(int32 Limit, const FOnDeskillzLeaderboardLoaded& OnComplete);
	
	/**
	 * Get tournament leaderboard
	 */
	void GetTournamentLeaderboard(const FString& TournamentId, int32 Limit, 
		const FOnDeskillzLeaderboardLoaded& OnComplete);
	
	/**
	 * Get nearby ranks (around current user)
	 */
	void GetNearbyRanks(int32 Range, const FOnDeskillzLeaderboardLoaded& OnComplete);
	
	// ========================================================================
	// Games
	// ========================================================================
	
	/**
	 * Get game config
	 */
	void GetGameConfig(const FString& GameId,
		TFunction<void(bool, TSharedPtr<FJsonObject>)> OnComplete);
	
protected:
	/** HTTP Client reference */
	UPROPERTY()
	UDeskillzHttpClient* Http;
	
	// ========================================================================
	// Parsing Helpers
	// ========================================================================
	
	/** Parse user from JSON */
	static FDeskillzPlayerInfo ParseUser(const TSharedPtr<FJsonObject>& Json);
	
	/** Parse tournament from JSON */
	static FDeskillzTournament ParseTournament(const TSharedPtr<FJsonObject>& Json);
	
	/** Parse match from JSON */
	static FDeskillzMatch ParseMatch(const TSharedPtr<FJsonObject>& Json);
	
	/** Parse match result from JSON */
	static FDeskillzMatchResult ParseMatchResult(const TSharedPtr<FJsonObject>& Json);
	
	/** Parse leaderboard entry from JSON */
	static FDeskillzLeaderboardEntry ParseLeaderboardEntry(const TSharedPtr<FJsonObject>& Json);
	
	/** Parse balances from JSON */
	static TMap<FString, double> ParseBalances(const TSharedPtr<FJsonObject>& Json);
};
