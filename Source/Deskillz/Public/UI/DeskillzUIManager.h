// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Blueprint/UserWidget.h"
#include "Core/DeskillzConfig.h"
#include "DeskillzUIManager.generated.h"

class UDeskillzTournamentListWidget;
class UDeskillzMatchmakingWidget;
class UDeskillzResultsWidget;
class UDeskillzWalletWidget;
class UDeskillzHUDWidget;
class UDeskillzPopupWidget;
class UDeskillzLeaderboardWidget;

/**
 * UI Layer for widget stacking
 */
UENUM(BlueprintType)
enum class EDeskillzUILayer : uint8
{
	/** Background layer */
	Background,
	
	/** Main content layer */
	Content,
	
	/** Overlay layer (HUD) */
	Overlay,
	
	/** Popup/Modal layer */
	Popup,
	
	/** Toast/Notification layer */
	Toast
};

// NOTE: FDeskillzUITheme struct is defined in DeskillzConfig.h

/** Delegate for UI events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzUIEvent, const FString&, EventName);

/**
 * Deskillz UI Manager
 * 
 * Central manager for all Deskillz UI widgets:
 * - Widget lifecycle management
 * - Layer-based widget stacking
 * - Theme configuration
 * - Animation coordination
 * - Event routing
 * 
 * Usage:
 *   UDeskillzUIManager* UI = UDeskillzUIManager::Get(this);
 *   UI->ShowTournamentList();
 *   UI->ShowMatchmaking(TournamentId);
 *   UI->ShowResults(MatchResult);
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzUIManager : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzUIManager();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the UI Manager instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Deskillz UI"))
	static UDeskillzUIManager* Get(const UObject* WorldContextObject);
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Current UI theme */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI")
	FDeskillzUITheme Theme;
	
	/** Use built-in widgets (set false for fully custom UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI")
	bool bUseBuiltInWidgets = true;
	
	/** Enable UI animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI")
	bool bEnableAnimations = true;
	
	/** Enable UI sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI")
	bool bEnableSounds = true;
	
	// ========================================================================
	// Widget Class Overrides (for custom widgets)
	// ========================================================================
	
	/** Custom tournament list widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI|Overrides")
	TSubclassOf<UDeskillzTournamentListWidget> TournamentListWidgetClass;
	
	/** Custom matchmaking widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI|Overrides")
	TSubclassOf<UDeskillzMatchmakingWidget> MatchmakingWidgetClass;
	
	/** Custom results widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI|Overrides")
	TSubclassOf<UDeskillzResultsWidget> ResultsWidgetClass;
	
	/** Custom wallet widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI|Overrides")
	TSubclassOf<UDeskillzWalletWidget> WalletWidgetClass;
	
	/** Custom HUD widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|UI|Overrides")
	TSubclassOf<UDeskillzHUDWidget> HUDWidgetClass;
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when any UI event occurs */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnDeskillzUIEvent OnUIEvent;
	
	// ========================================================================
	// Tournament List
	// ========================================================================
	
	/**
	 * Show the tournament list screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowTournamentList();
	
	/**
	 * Hide the tournament list
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideTournamentList();
	
	/**
	 * Is tournament list visible
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	bool IsTournamentListVisible() const;
	
	// ========================================================================
	// Matchmaking
	// ========================================================================
	
	/**
	 * Show the matchmaking screen
	 * @param TournamentId Tournament to find match in
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowMatchmaking(const FString& TournamentId);
	
	/**
	 * Hide matchmaking screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideMatchmaking();
	
	/**
	 * Update matchmaking status
	 * @param Status Status text to display
	 * @param Progress Progress (0-1) for progress bar
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void UpdateMatchmakingStatus(const FString& Status, float Progress = -1.0f);
	
	// ========================================================================
	// Results
	// ========================================================================
	
	/**
	 * Show match results screen
	 * @param Result The match result to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowResults(const struct FDeskillzMatchResult& Result);
	
	/**
	 * Hide results screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideResults();
	
	// ========================================================================
	// Wallet
	// ========================================================================
	
	/**
	 * Show wallet/balance screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowWallet();
	
	/**
	 * Hide wallet screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideWallet();
	
	/**
	 * Update wallet balance display
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void RefreshWalletBalance();
	
	// ========================================================================
	// HUD
	// ========================================================================
	
	/**
	 * Show the in-game HUD
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowHUD();
	
	/**
	 * Hide the HUD
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideHUD();
	
	/**
	 * Update HUD score display
	 * @param Score Current score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void UpdateHUDScore(int64 Score);
	
	/**
	 * Update HUD timer display
	 * @param RemainingSeconds Seconds remaining
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void UpdateHUDTimer(float RemainingSeconds);
	
	// ========================================================================
	// Leaderboard
	// ========================================================================
	
	/**
	 * Show leaderboard
	 * @param TournamentId Optional tournament ID (empty for global)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowLeaderboard(const FString& TournamentId = TEXT(""));
	
	/**
	 * Hide leaderboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideLeaderboard();
	
	// ========================================================================
	// Popups & Toasts
	// ========================================================================
	
	/**
	 * Show a popup dialog
	 * @param Title Popup title
	 * @param Message Popup message
	 * @param bShowCancel Show cancel button
	 * @param ConfirmText Confirm button text
	 * @param CancelText Cancel button text
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowPopup(const FString& Title, const FString& Message, bool bShowCancel = false, 
		const FString& ConfirmText = TEXT("OK"), const FString& CancelText = TEXT("Cancel"));
	
	/**
	 * Show a toast notification
	 * @param Message Toast message
	 * @param Duration Display duration in seconds
	 * @param bIsError Is this an error toast
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowToast(const FString& Message, float Duration = 3.0f, bool bIsError = false);
	
	/**
	 * Show loading indicator
	 * @param Message Loading message
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowLoading(const FString& Message = TEXT("Loading..."));
	
	/**
	 * Hide loading indicator
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideLoading();
	
	// ========================================================================
	// General
	// ========================================================================
	
	/**
	 * Hide all Deskillz UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideAllUI();
	
	/**
	 * Get current active widget
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	UUserWidget* GetActiveWidget() const;
	
	/**
	 * Is any Deskillz UI visible
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	bool IsAnyUIVisible() const;
	
	/**
	 * Set UI input mode (game only, UI only, or both)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetUIInputMode(bool bUIOnly);
	
protected:
	// ========================================================================
	// Widget Instances
	// ========================================================================
	
	UPROPERTY()
	UDeskillzTournamentListWidget* TournamentListWidget;
	
	UPROPERTY()
	UDeskillzMatchmakingWidget* MatchmakingWidget;
	
	UPROPERTY()
	UDeskillzResultsWidget* ResultsWidget;
	
	UPROPERTY()
	UDeskillzWalletWidget* WalletWidget;
	
	UPROPERTY()
	UDeskillzHUDWidget* HUDWidget;
	
	UPROPERTY()
	UDeskillzLeaderboardWidget* LeaderboardWidget;
	
	UPROPERTY()
	UDeskillzPopupWidget* PopupWidget;
	
	UPROPERTY()
	UUserWidget* LoadingWidget;
	
	/** Currently active full-screen widget */
	UPROPERTY()
	UUserWidget* ActiveWidget;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Create widget of specified class */
	template<typename T>
	T* CreateWidget(TSubclassOf<T> WidgetClass);
	
	/** Show widget with animation */
	void ShowWidget(UUserWidget* Widget, EDeskillzUILayer Layer = EDeskillzUILayer::Content);
	
	/** Hide widget with animation */
	void HideWidget(UUserWidget* Widget);
	
	/** Get owning player controller */
	APlayerController* GetPlayerController() const;
	
	/** Broadcast UI event */
	void BroadcastUIEvent(const FString& EventName);
};