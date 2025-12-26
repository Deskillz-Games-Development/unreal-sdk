// Copyright Deskillz Games. All Rights Reserved.
// DeskillzDeepLinkHandler.h - Dedicated deep link handler for centralized lobby architecture
// 
// This file handles deep links from the main Deskillz app that launch game matches
// AND navigation deep links from the website (tournaments, wallet, profile, etc.)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzLobbyTypes.h"
#include "DeskillzDeepLinkHandler.generated.h"

// =============================================================================
// NAVIGATION ACTION ENUM
// =============================================================================

/**
 * Navigation actions from deep links (when app is opened from website).
 * These are non-match deep links that navigate to specific screens.
 */
UENUM(BlueprintType)
enum class EDeskillzNavigationAction : uint8
{
	None			UMETA(DisplayName = "None"),
	Tournaments		UMETA(DisplayName = "Tournaments"),
	Wallet			UMETA(DisplayName = "Wallet"),
	Profile			UMETA(DisplayName = "Profile"),
	Game			UMETA(DisplayName = "Game"),
	Settings		UMETA(DisplayName = "Settings")
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================

/** Delegate for navigation deep links (non-match) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNavigationReceived, EDeskillzNavigationAction, Action, const FString&, TargetId);

/**
 * Deep Link Handler for Centralized Lobby
 * 
 * With the centralized lobby architecture, players browse tournaments and
 * matchmake through the main Deskillz website/app. When a match is found,
 * the game app is launched via deep link with match parameters.
 * 
 * This handler:
 * 1. Receives deep links from the main Deskillz app/website
 * 2. Detects if it's a navigation link (tournaments, wallet, etc.) or match launch
 * 3. For navigation: Triggers OnNavigationReceived
 * 4. For matches: Parses match parameters and triggers OnMatchReady
 * 
 * Deep Link Formats:
 *   Navigation:
 *     deskillz://tournaments
 *     deskillz://wallet
 *     deskillz://profile
 *     deskillz://game?id=xxx
 *     deskillz://settings
 *   
 *   Match Launch:
 *     deskillz://launch?matchId=xxx&token=yyy&...
 *     deskillz://match/start?matchId=xxx&token=yyy&...
 *     https://deskillz.games/game/mygame/launch?matchId=xxx&...
 * 
 * Usage in Game:
 * 
 *   // In your GameMode or Manager
 *   UDeskillzDeepLinkHandler* Handler = UDeskillzDeepLinkHandler::Get();
 *   Handler->OnNavigationReceived.AddDynamic(this, &AMyGameMode::OnNavigation);
 *   Handler->OnMatchReady.AddDynamic(this, &AMyGameMode::OnMatchReady);
 *   
 *   void AMyGameMode::OnNavigation(EDeskillzNavigationAction Action, const FString& TargetId)
 *   {
 *       switch (Action)
 *       {
 *           case EDeskillzNavigationAction::Tournaments:
 *               ShowTournamentsUI();
 *               break;
 *           case EDeskillzNavigationAction::Wallet:
 *               ShowWalletUI();
 *               break;
 *           // ... etc
 *       }
 *   }
 *   
 *   void AMyGameMode::OnMatchReady(const FDeskillzMatchLaunchData& LaunchData)
 *   {
 *       StartMatch(LaunchData.MatchId, LaunchData.DurationSeconds, LaunchData.RandomSeed);
 *   }
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzDeepLinkHandler : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzDeepLinkHandler();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Deep Link Handler instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby", meta = (DisplayName = "Get Deskillz Deep Link Handler"))
	static UDeskillzDeepLinkHandler* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize the deep link handler
	 * Call this early in game startup (e.g., GameInstance::Init)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void Initialize();
	
	/**
	 * Shutdown the handler
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void Shutdown();
	
	/**
	 * Check if handler is initialized
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	bool IsInitialized() const { return bIsInitialized; }
	
	// ========================================================================
	// Deep Link Handling
	// ========================================================================
	
	/**
	 * Handle incoming deep link URL
	 * Called automatically by platform handler, or manually for testing
	 * 
	 * @param URL The deep link URL
	 * @return true if URL was a valid Deskillz deep link (navigation or launch)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	bool HandleDeepLink(const FString& URL);
	
	/**
	 * Parse a deep link URL into launch data
	 * Does not trigger events, just parses
	 * 
	 * @param URL The deep link URL to parse
	 * @return Parsed launch data (check bIsValid)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	FDeskillzMatchLaunchData ParseLaunchURL(const FString& URL) const;
	
	/**
	 * Validate launch data before starting match
	 * Checks token, match ID, expiration, etc.
	 * 
	 * @param LaunchData The launch data to validate
	 * @param OutError Error message if validation fails
	 * @return true if valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	bool ValidateLaunchData(const FDeskillzMatchLaunchData& LaunchData, FString& OutError) const;
	
	// ========================================================================
	// Pending Launch Data
	// ========================================================================
	
	/**
	 * Check if there's pending launch data (received before initialization)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	bool HasPendingLaunch() const { return PendingLaunchData.bIsValid; }
	
	/**
	 * Get pending launch data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	bool GetPendingLaunch(FDeskillzMatchLaunchData& OutLaunchData);
	
	/**
	 * Clear pending launch data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void ClearPendingLaunch();
	
	/**
	 * Process pending launch (if any)
	 * Call this after your game is ready to receive matches
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void ProcessPendingLaunch();
	
	// ========================================================================
	// Current Match Data
	// ========================================================================
	
	/**
	 * Get the current/last launch data
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	FDeskillzMatchLaunchData GetCurrentLaunchData() const { return CurrentLaunchData; }
	
	/**
	 * Check if we have valid current launch data
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	bool HasValidLaunchData() const { return CurrentLaunchData.bIsValid; }
	
	/**
	 * Get the player token for API calls
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	FString GetPlayerToken() const { return CurrentLaunchData.PlayerToken; }
	
	/**
	 * Get the current match ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lobby")
	FString GetCurrentMatchId() const { return CurrentLaunchData.MatchId; }
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when a navigation deep link is received (not a match launch) */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby")
	FOnNavigationReceived OnNavigationReceived;
	
	/** Called when a match launch is received from deep link */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby")
	FOnMatchLaunchReceived OnMatchLaunchReceived;
	
	/** Called when match is validated and ready to start */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby")
	FOnMatchReady OnMatchReady;
	
	/** Called when launch validation fails */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lobby")
	FOnLaunchValidationFailed OnValidationFailed;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/**
	 * Set custom URL scheme (default: "deskillz")
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void SetURLScheme(const FString& Scheme) { URLScheme = Scheme; }
	
	/**
	 * Set whether to auto-validate launch data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void SetAutoValidate(bool bAutoValidate) { bAutoValidateLaunch = bAutoValidate; }
	
	/**
	 * Set token expiration time in seconds (default: 300)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lobby")
	void SetTokenExpirationSeconds(int32 Seconds) { TokenExpirationSeconds = Seconds; }
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Is handler initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** URL scheme for custom URLs */
	UPROPERTY()
	FString URLScheme = TEXT("deskillz");
	
	/** Auto-validate launch data */
	UPROPERTY()
	bool bAutoValidateLaunch = true;
	
	/** Token expiration time */
	UPROPERTY()
	int32 TokenExpirationSeconds = 300;
	
	/** Pending launch data (received before ready) */
	UPROPERTY()
	FDeskillzMatchLaunchData PendingLaunchData;
	
	/** Current/last launch data */
	UPROPERTY()
	FDeskillzMatchLaunchData CurrentLaunchData;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Check if URL is a navigation deep link (tournaments, wallet, etc.) */
	bool IsNavigationDeepLink(const FString& URL) const;
	
	/** Parse navigation deep link and extract action/target */
	bool ParseNavigationLink(const FString& URL, EDeskillzNavigationAction& OutAction, FString& OutTargetId) const;
	
	/** Check if URL is a launch deep link (match start) */
	bool IsLaunchDeepLink(const FString& URL) const;
	
	/** Parse query parameters from URL */
	TMap<FString, FString> ParseQueryParameters(const FString& URL) const;
	
	/** URL decode a string */
	FString URLDecode(const FString& EncodedString) const;
	
	/** Process validated launch data */
	void ProcessLaunchData(const FDeskillzMatchLaunchData& LaunchData);
	
	/** Register for platform deep link events */
	void RegisterPlatformHandler();
	
	/** Unregister platform handler */
	void UnregisterPlatformHandler();
	
	/** Handle platform callback */
	void OnPlatformDeepLink(const FString& URL);
	
private:
	/** Singleton instance */
	static UDeskillzDeepLinkHandler* Instance;
};