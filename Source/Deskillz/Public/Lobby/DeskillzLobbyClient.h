// Copyright Deskillz Games. All Rights Reserved.
// DeskillzLobbyClient.h - Lobby API client for centralized lobby architecture
//
// This is a NEW file for the centralized lobby architecture.
// It provides API methods for interacting with the lobby backend.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzLobbyTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "DeskillzLobbyClient.generated.h"

class FJsonObject;

// ============================================================================
// Response Delegates
// ============================================================================

/** Score submission response */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreSubmitResponse, bool, bSuccess, const FString&, Message, const FDeskillzMatchResult&, Result);

/** Match status response */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchStatusResponse, bool, bSuccess, EDeskillzMatchStatus, Status);

/** Generic error response */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyApiError, int32, ErrorCode, const FString&, ErrorMessage);

/**
 * Lobby API Client
 * 
 * Provides API methods for games to interact with the Deskillz backend
 * in the centralized lobby architecture.
 * 
 * Main functions:
 * - Submit final scores
 * - Report match status
 * - Fetch match results
 * - Handle reconnection for sync matches
 * 
 * Usage:
 * 
 *   UDeskillzLobbyClient* Client = UDeskillzLobbyClient::Get();
 *   Client->Initialize(PlayerToken, MatchId);
 *   
 *   // When match is complete
 *   Client->SubmitScore(FinalScore, OnScoreSubmitted);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzLobbyClient : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzLobbyClient();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Lobby Client instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby", meta = (DisplayName = "Get Deskillz Lobby Client"))
	static UDeskillzLobbyClient* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize the client with authentication token
	 * @param InPlayerToken Player's authentication token
	 * @param InMatchId Current match ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void Initialize(const FString& InPlayerToken, const FString& InMatchId);
	
	/**
	 * Initialize from launch data
	 * @param LaunchData The launch data from deep link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void InitializeFromLaunchData(const FDeskillzMatchLaunchData& LaunchData);
	
	/**
	 * Check if client is initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	bool IsInitialized() const { return bIsInitialized; }
	
	/**
	 * Set API base URL
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void SetApiBaseUrl(const FString& BaseUrl) { ApiBaseUrl = BaseUrl; }
	
	// ========================================================================
	// Score Submission
	// ========================================================================
	
	/**
	 * Submit final score for the match
	 * @param FinalScore The player's final score
	 * @param Stats Optional gameplay stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void SubmitScore(int64 FinalScore, const TMap<FString, FString>& Stats);
	
	/**
	 * Submit score with completion data
	 * @param CompletionData Full completion data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void SubmitScoreWithData(const FDeskillzMatchCompletionData& CompletionData);
	
	// ========================================================================
	// Match Status
	// ========================================================================
	
	/**
	 * Report match started
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void ReportMatchStarted();
	
	/**
	 * Report match aborted
	 * @param Reason Reason for abort
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void ReportMatchAborted(const FString& Reason = TEXT(""));
	
	/**
	 * Get current match status from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void GetMatchStatus();
	
	/**
	 * Get match result after completion
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void GetMatchResult();
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when score submission completes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby")
	FOnScoreSubmitResponse OnScoreSubmitted;
	
	/** Called when match status is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby")
	FOnMatchStatusResponse OnMatchStatusReceived;
	
	/** Called when API error occurs */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby")
	FOnLobbyApiError OnApiError;
	
	// ========================================================================
	// Score Signing (Anti-Cheat)
	// ========================================================================
	
	/**
	 * Generate HMAC signature for score
	 * @param Score The score to sign
	 * @param Timestamp Unix timestamp
	 * @return HMAC-SHA256 signature
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	FString GenerateScoreSignature(int64 Score, int64 Timestamp) const;
	
	/**
	 * Set HMAC secret key (from SDK config)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void SetHmacSecret(const FString& Secret) { HmacSecret = Secret; }
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Is client initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** Player authentication token */
	UPROPERTY()
	FString PlayerToken;
	
	/** Current match ID */
	UPROPERTY()
	FString MatchId;
	
	/** API base URL */
	UPROPERTY()
	FString ApiBaseUrl = TEXT("https://api.deskillz.games/v1");
	
	/** HMAC secret for score signing */
	UPROPERTY()
	FString HmacSecret;
	
	/** Game ID */
	UPROPERTY()
	FString GameId;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Make authenticated API request */
	void MakeApiRequest(const FString& Endpoint, const FString& Method, const TSharedPtr<FJsonObject>& Body,
		TFunction<void(bool, TSharedPtr<FJsonObject>)> Callback);
	
	/** Handle HTTP response */
	void HandleHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess,
		TFunction<void(bool, TSharedPtr<FJsonObject>)> Callback);
	
	/** Parse JSON response */
	TSharedPtr<FJsonObject> ParseJsonResponse(const FString& Content);
	
	/** Compute HMAC-SHA256 */
	FString ComputeHmacSha256(const FString& Message, const FString& Key) const;
	
	/** Handle score submit response */
	void HandleScoreSubmitResponse(bool bSuccess, TSharedPtr<FJsonObject> Response);
	
	/** Handle match status response */
	void HandleMatchStatusResponse(bool bSuccess, TSharedPtr<FJsonObject> Response);
	
private:
	/** Singleton instance */
	static UDeskillzLobbyClient* Instance;
};