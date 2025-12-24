// Copyright Deskillz Games. All Rights Reserved.
// DeskillzBridge.h - Bridge between game SDK and main Deskillz app
//
// This is a NEW file for the centralized lobby architecture.
// It handles communication between the game and the main Deskillz app.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzLobbyTypes.h"
#include "DeskillzBridge.generated.h"

/**
 * Bridge for Game <-> Main Deskillz App Communication
 * 
 * With the centralized lobby architecture, games are launched from the main
 * Deskillz app and need to communicate back when:
 * - Match is completed
 * - Player wants to return to lobby
 * - Results need to be shown
 * - Errors occur
 * 
 * This bridge provides:
 * 1. Deep link generation to return to main app
 * 2. Match result reporting
 * 3. Error/abort notification
 * 4. App-to-app communication handling
 * 
 * Usage in Game:
 * 
 *   // When match is complete
 *   UDeskillzBridge* Bridge = UDeskillzBridge::Get();
 *   
 *   FDeskillzMatchCompletionData Completion;
 *   Completion.MatchId = CurrentMatchId;
 *   Completion.FinalScore = PlayerScore;
 *   Completion.Result = EDeskillzMatchResult::Win;
 *   
 *   Bridge->CompleteMatch(Completion);
 *   Bridge->ReturnToMainApp(EDeskillzReturnDestination::Results);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzBridge : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzBridge();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Bridge instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge", meta = (DisplayName = "Get Deskillz Bridge"))
	static UDeskillzBridge* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize the bridge with launch data
	 * @param LaunchData The launch data received from deep link
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void Initialize(const FDeskillzMatchLaunchData& LaunchData);
	
	/**
	 * Check if bridge is initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	bool IsInitialized() const { return bIsInitialized; }
	
	/**
	 * Get current launch data
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	FDeskillzMatchLaunchData GetLaunchData() const { return CurrentLaunchData; }
	
	// ========================================================================
	// Match Lifecycle
	// ========================================================================
	
	/**
	 * Report match started
	 * Call when gameplay actually begins
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void ReportMatchStarted();
	
	/**
	 * Report score update during match
	 * @param CurrentScore The player's current score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void ReportScoreUpdate(int64 CurrentScore);
	
	/**
	 * Complete the match and submit score
	 * @param CompletionData Match completion data including final score
	 * @return true if score submission initiated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	bool CompleteMatch(const FDeskillzMatchCompletionData& CompletionData);
	
	/**
	 * Abort the match (forfeit)
	 * @param Reason Reason for aborting
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void AbortMatch(const FString& Reason = TEXT(""));
	
	// ========================================================================
	// Navigation
	// ========================================================================
	
	/**
	 * Return to the main Deskillz app
	 * @param Destination Where to navigate in the main app
	 * @param Data Optional data to pass (e.g., match ID for results)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void ReturnToMainApp(EDeskillzReturnDestination Destination = EDeskillzReturnDestination::Lobby, const FString& Data = TEXT(""));
	
	/**
	 * Generate return URL for main app
	 * @param Destination The destination in main app
	 * @param Data Optional data to include
	 * @return The deep link URL
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	FString GenerateReturnURL(EDeskillzReturnDestination Destination, const FString& Data = TEXT("")) const;
	
	/**
	 * Open URL (platform-specific)
	 * @param URL The URL to open
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void OpenURL(const FString& URL);
	
	/**
	 * Check if we can return to main app (has valid return path)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	bool CanReturnToMainApp() const;
	
	// ========================================================================
	// Match State
	// ========================================================================
	
	/**
	 * Get current match ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	FString GetMatchId() const { return CurrentLaunchData.MatchId; }
	
	/**
	 * Get player token for API calls
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	FString GetPlayerToken() const { return CurrentLaunchData.PlayerToken; }
	
	/**
	 * Get opponent info (if available)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	FDeskillzPlayer GetOpponent() const { return CurrentLaunchData.Opponent; }
	
	/**
	 * Check if match has opponent info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	bool HasOpponent() const { return CurrentLaunchData.HasOpponent(); }
	
	/**
	 * Get match duration in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	int32 GetMatchDuration() const { return CurrentLaunchData.DurationSeconds; }
	
	/**
	 * Get random seed for deterministic gameplay
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	int64 GetRandomSeed() const { return CurrentLaunchData.RandomSeed; }
	
	/**
	 * Is this a tournament match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	bool IsTournamentMatch() const { return CurrentLaunchData.IsTournamentMatch(); }
	
	/**
	 * Is this a synchronous match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	bool IsSynchronousMatch() const { return CurrentLaunchData.IsSynchronous(); }
	
	/**
	 * Is this a private room match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	bool IsPrivateRoom() const { return CurrentLaunchData.bIsPrivateRoom; }
	
	/**
	 * Get custom parameter
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Bridge")
	FString GetCustomParameter(const FString& Key, const FString& DefaultValue = TEXT("")) const;
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when returning to main app */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Bridge")
	FOnReturnToMainApp OnReturnToMainApp;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/**
	 * Set the main app URL scheme
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void SetMainAppScheme(const FString& Scheme) { MainAppScheme = Scheme; }
	
	/**
	 * Set the main app bundle ID (for iOS)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void SetMainAppBundleId(const FString& BundleId) { MainAppBundleId = BundleId; }
	
	/**
	 * Set the main app package name (for Android)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Bridge")
	void SetMainAppPackageName(const FString& PackageName) { MainAppPackageName = PackageName; }
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Is bridge initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** Current launch data */
	UPROPERTY()
	FDeskillzMatchLaunchData CurrentLaunchData;
	
	/** Match started time */
	UPROPERTY()
	FDateTime MatchStartTime;
	
	/** Has match started */
	UPROPERTY()
	bool bMatchStarted = false;
	
	/** Has match completed */
	UPROPERTY()
	bool bMatchCompleted = false;
	
	/** Main app URL scheme */
	UPROPERTY()
	FString MainAppScheme = TEXT("deskillz");
	
	/** Main app bundle ID (iOS) */
	UPROPERTY()
	FString MainAppBundleId = TEXT("com.deskillz.app");
	
	/** Main app package name (Android) */
	UPROPERTY()
	FString MainAppPackageName = TEXT("com.deskillz.app");
	
	/** Last reported score */
	UPROPERTY()
	int64 LastReportedScore = 0;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Submit score to backend */
	void SubmitScoreToBackend(const FDeskillzMatchCompletionData& CompletionData);
	
	/** Get destination path for return URL */
	FString GetDestinationPath(EDeskillzReturnDestination Destination) const;
	
	/** Check if main app is installed */
	bool IsMainAppInstalled() const;
	
	/** Open main app directly (if installed) */
	void OpenMainAppDirect(const FString& URL);
	
	/** Fall back to web URL */
	void OpenWebFallback(EDeskillzReturnDestination Destination, const FString& Data);
	
private:
	/** Singleton instance */
	static UDeskillzBridge* Instance;
};