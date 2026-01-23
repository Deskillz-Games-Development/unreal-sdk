// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzTypes.h"
#include "DeskillzAuthController.generated.h"

/**
 * Scene/Level flow controller for authentication.
 * Manages navigation between auth, lobby, and game levels.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzAuthController : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzAuthController();

    // ========================================================================
    // Singleton Access
    // ========================================================================

    /**
     * Get the auth controller instance
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth", meta = (WorldContext = "WorldContextObject"))
    static UDeskillzAuthController* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Initialization
    // ========================================================================

    /**
     * Initialize the auth controller
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Initialize();

    /**
     * Shutdown and cleanup
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Shutdown();

    /**
     * Check if initialized
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    bool IsInitialized() const { return bIsInitialized; }

    // ========================================================================
    // Navigation
    // ========================================================================

    /**
     * Navigate to authentication level
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Navigation")
    void GoToAuth();

    /**
     * Navigate to lobby level
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Navigation")
    void GoToLobby();

    /**
     * Navigate to game level
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Navigation")
    void GoToGame();

    /**
     * Launch a match with given data
     * @param MatchData The match launch data from deep link
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Navigation")
    void LaunchMatch(const FDeskillzMatchLaunchData& MatchData);

    /**
     * Return to lobby after match
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Navigation")
    void ReturnToLobby();

    /**
     * Logout and return to auth
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Navigation")
    void LogoutAndGoToAuth();

    /**
     * Determine initial navigation based on auth state
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Navigation")
    void DetermineInitialNavigation();

    // ========================================================================
    // State
    // ========================================================================

    /**
     * Check if user is authenticated
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    bool IsAuthenticated() const;

    /**
     * Check if currently transitioning between levels
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    bool IsTransitioning() const { return bIsTransitioning; }

    /**
     * Get current level name
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    FString GetCurrentLevelName() const;

    /**
     * Check if there's a pending match
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    bool HasPendingMatch() const { return bHasPendingMatch; }

    /**
     * Get pending match data
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    FDeskillzMatchLaunchData GetPendingMatch() const { return PendingMatchData; }

    // ========================================================================
    // Configuration
    // ========================================================================

    /**
     * Set level names
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth|Config")
    void SetLevelNames(const FString& AuthLevel, const FString& LobbyLevel, 
                       const FString& GameLevel, const FString& LoadingLevel);

    /**
     * Get level names
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth|Config")
    void GetLevelNames(FString& OutAuthLevel, FString& OutLobbyLevel, 
                       FString& OutGameLevel, FString& OutLoadingLevel) const;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when navigating to a new level */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanging, const FString&, LevelName);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnLevelChanging OnLevelChanging;

    /** Called when level change is complete */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, const FString&, LevelName);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnLevelChanged OnLevelChanged;

    /** Called when auth flow is complete */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthFlowCompleteController, const FDeskillzAuthUser&, User);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnAuthFlowCompleteController OnAuthFlowComplete;

    /** Called when logout is complete */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLogoutCompleteController);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnLogoutCompleteController OnLogoutComplete;

protected:
    // ========================================================================
    // Internal State
    // ========================================================================

    /** Whether controller is initialized */
    bool bIsInitialized = false;

    /** Whether currently transitioning between levels */
    bool bIsTransitioning = false;

    /** Pending level to navigate to */
    FString PendingDestination;

    /** Whether there's a pending match */
    bool bHasPendingMatch = false;

    /** Pending match data */
    UPROPERTY()
    FDeskillzMatchLaunchData PendingMatchData;

    // ========================================================================
    // Level Names
    // ========================================================================

    /** Auth level name */
    UPROPERTY()
    FString AuthLevelName = TEXT("/Game/Deskillz/Maps/DeskillzAuth");

    /** Lobby level name */
    UPROPERTY()
    FString LobbyLevelName = TEXT("/Game/Deskillz/Maps/DeskillzLobby");

    /** Game level name */
    UPROPERTY()
    FString GameLevelName = TEXT("/Game/Maps/Game");

    /** Loading level name */
    UPROPERTY()
    FString LoadingLevelName = TEXT("/Game/Deskillz/Maps/Loading");

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Navigate to a level */
    void NavigateToLevel(const FString& LevelName);

    /** Handle level loaded */
    void OnLevelLoaded();

    /** Handle login success */
    UFUNCTION()
    void HandleLoginSuccess(const FDeskillzAuthUser& User);

    /** Handle logout */
    UFUNCTION()
    void HandleLogout();

    /** Handle auth error */
    UFUNCTION()
    void HandleAuthError(const FString& Error);

    /** Handle match launch from deep link */
    void HandleMatchLaunchReceived(const FDeskillzMatchLaunchData& MatchData);

    /** Handle normal launch */
    void HandleNormalLaunch();

    /** Process pending navigation */
    void ProcessPendingNavigation();

    /** World context for level operations */
    UPROPERTY()
    TWeakObjectPtr<UWorld> WorldContext;

private:
    /** Singleton instance */
    static UDeskillzAuthController* Instance;
};

// ============================================================================
// Match Launch Data Struct
// ============================================================================

/**
 * Data received from deep link for match launch
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchLaunchData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    FString MatchId;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    FString TournamentId;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    FString Token;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    FString GameId;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    EDeskillzMatchType MatchType = EDeskillzMatchType::Asynchronous;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    double EntryFee = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    double PrizePool = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    EDeskillzCurrency Currency = EDeskillzCurrency::Free;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    int32 TimeLimitSeconds = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    int32 MaxPlayers = 2;

    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
    TMap<FString, FString> CustomParams;

    FDeskillzMatchLaunchData() = default;

    bool IsValid() const { return !MatchId.IsEmpty() && !Token.IsEmpty(); }
};