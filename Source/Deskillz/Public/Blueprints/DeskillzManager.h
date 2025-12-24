// Copyright Deskillz Games. All Rights Reserved.
// DeskillzManager_Updated.h - UPDATED for centralized lobby architecture
//
// CHANGES:
// - Added new delegates for lobby events
// - Added methods for lobby integration
// - Added storage for launch data

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/DeskillzTypes.h"
#include "Lobby/DeskillzLobbyTypes.h"  // NEW: Include lobby types
#include "DeskillzManager.generated.h"

class UDeskillzSDK;

// ============================================================================
// NEW: Lobby Event Delegates
// ============================================================================

/** Called when a deep link is received (before validation) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeepLinkReceived, const FDeskillzMatchLaunchData&, LaunchData);

/** Called when lobby match is ready to start */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyMatchReady, const FDeskillzMatchLaunchData&, LaunchData);

/** Called when lobby error occurs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyError, const FString&, ErrorMessage);

// ============================================================================
// Original Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManagerSDKInitialized, bool, bSuccess, const FDeskillzError&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManagerMatchReady, const FDeskillzMatchInfo&, MatchInfo, const FDeskillzError&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManagerGameplayStart, const FDeskillzMatchInfo&, MatchInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManagerMatchCompleted, const FDeskillzMatchResult&, Result, const FDeskillzError&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManagerError, const FDeskillzError&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManagerMatchTimeUpdate, float, RemainingTime, float, ElapsedTime);

/**
 * Deskillz Manager Actor
 * 
 * High-level manager for game developers to interact with the Deskillz SDK.
 * Provides Blueprint-friendly events and simplified match lifecycle.
 * 
 * UPDATED for Centralized Lobby Architecture:
 * - Matches are now launched via deep links from the main Deskillz app
 * - Tournament browsing and matchmaking happen in the main app
 * - This manager handles deep link reception and match lifecycle
 * 
 * Place one in your level or let it auto-spawn.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API ADeskillzManager : public AActor
{
	GENERATED_BODY()
	
public:
	ADeskillzManager();
	
	// ========================================================================
	// Static Access
	// ========================================================================
	
	/**
	 * Get the Deskillz Manager instance
	 * Auto-spawns if none exists
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz", meta = (WorldContext = "WorldContextObject"))
	static ADeskillzManager* Get(const UObject* WorldContextObject);
	
	/**
	 * Check if manager is available
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz")
	static bool IsManagerAvailable();
	
	// ========================================================================
	// Lifecycle
	// ========================================================================
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize the SDK
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Core")
	void Initialize();
	
	/**
	 * Check if SDK is ready
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Core")
	bool IsReady() const;
	
	/**
	 * Check if currently in a match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Core")
	bool IsInMatch() const;
	
	/**
	 * Get current match info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	FDeskillzMatchInfo GetMatchInfo() const;
	
	// ========================================================================
	// NEW: Lobby Methods
	// ========================================================================
	
	/**
	 * Start the match (call after OnLobbyMatchReady)
	 * This reports match started to the backend
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void StartMatchFromLobby();
	
	/**
	 * Complete the match and submit score
	 * @param FinalScore The player's final score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void CompleteMatchFromLobby(int64 FinalScore);
	
	/**
	 * Return to the main Deskillz app
	 * @param bShowResults If true, navigates to results screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void ReturnToLobby(bool bShowResults = true);
	
	/**
	 * Get the current launch data (from deep link)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	FDeskillzMatchLaunchData GetCurrentLaunchData() const;
	
	/**
	 * Check if we have valid launch data
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	bool HasLaunchData() const;
	
	/**
	 * Process any pending deep link
	 * Call this after your game is ready to receive matches
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void ProcessPendingDeepLink();
	
	// ========================================================================
	// Events - Original
	// ========================================================================
	
	/** Called when SDK initialization completes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnManagerSDKInitialized OnSDKInitialized;
	
	/** Called when a match is found and ready */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnManagerMatchReady OnMatchReady;
	
	/** Called when gameplay should start */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnManagerGameplayStart OnGameplayStart;
	
	/** Called when match is completed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnManagerMatchCompleted OnMatchCompleted;
	
	/** Called when an error occurs */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnManagerError OnError;
	
	/** Called every second with match time update */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Events")
	FOnManagerMatchTimeUpdate OnMatchTimeUpdate;
	
	// ========================================================================
	// NEW: Events - Lobby
	// ========================================================================
	
	/** Called when a deep link is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby|Events")
	FOnDeepLinkReceived OnDeepLinkReceived;
	
	/** Called when lobby match is validated and ready */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby|Events")
	FOnLobbyMatchReady OnLobbyMatchReady;
	
	/** Called when lobby error occurs */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby|Events")
	FOnLobbyError OnLobbyError;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Auto-initialize SDK on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Config")
	bool bAutoInitialize = true;
	
	/** Use built-in Deskillz UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Config")
	bool bUseBuiltInUI = true;
	
	/** Persist manager across level loads */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Config")
	bool bPersistAcrossLevels = true;
	
	/** Show debug info on screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Config")
	bool bShowDebugInfo = false;
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Singleton instance */
	static ADeskillzManager* Instance;
	
	/** Reference to SDK subsystem */
	UPROPERTY()
	UDeskillzSDK* SDK = nullptr;
	
	/** Is initialized */
	bool bIsInitialized = false;
	
	/** Timer for match time updates */
	float MatchTimeUpdateTimer = 0.0f;
	
	/** NEW: Current launch data from deep link */
	UPROPERTY()
	FDeskillzMatchLaunchData CurrentLaunchData;
	
	// ========================================================================
	// Internal Methods - Original
	// ========================================================================
	
	/** Bind to SDK events */
	void BindSDKEvents();
	
	/** Handle SDK initialized */
	UFUNCTION()
	void HandleSDKInitialized(bool bSuccess, const FDeskillzError& Error);
	
	/** Handle match started */
	UFUNCTION()
	void HandleMatchStarted(const FDeskillzMatchInfo& MatchInfo, const FDeskillzError& Error);
	
	/** Handle match completed */
	UFUNCTION()
	void HandleMatchCompleted(const FDeskillzMatchResult& Result, const FDeskillzError& Error);
	
	/** Handle error */
	UFUNCTION()
	void HandleError(const FDeskillzError& Error);
	
	// ========================================================================
	// NEW: Internal Methods - Lobby
	// ========================================================================
	
	/** Initialize lobby components */
	void InitializeLobbyComponents();
	
	/** Cleanup lobby components */
	void CleanupLobbyComponents();
	
	/** Handle deep link match launch received */
	UFUNCTION()
	void HandleMatchLaunchReceived(const FDeskillzMatchLaunchData& LaunchData);
	
	/** Handle match ready from deep link */
	UFUNCTION()
	void HandleMatchReady(const FDeskillzMatchLaunchData& LaunchData);
	
	/** Handle launch validation failed */
	UFUNCTION()
	void HandleLaunchValidationFailed(const FString& Reason, const FDeskillzMatchLaunchData& LaunchData);
};