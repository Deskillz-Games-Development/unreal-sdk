// Copyright Deskillz Games. All Rights Reserved.
// DeskillzMatchmakingWidget_Updated.h - UPDATED for centralized lobby architecture
//
// CHANGES FROM ORIGINAL:
// - Removed in-game matchmaking logic
// - Now shows informational message that matchmaking is handled by main app
// - Kept basic UI structure for showing match status when launched via deep link
// - Simplified to only display received match info

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "DeskillzLobbyTypes.h"
#include "DeskillzMatchmakingWidget_Updated.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UBorder;

/**
 * Widget State for Lobby-Based Architecture
 */
UENUM(BlueprintType)
enum class EDeskillzWidgetState : uint8
{
	/** Waiting for deep link launch */
	WaitingForLaunch,
	
	/** Received match data, showing info */
	MatchReceived,
	
	/** Match is starting */
	Starting,
	
	/** Error state */
	Error
};

/**
 * Matchmaking Widget (Centralized Lobby Version)
 * 
 * In the centralized lobby architecture, matchmaking is handled by the main
 * Deskillz website/app. This widget now serves as:
 * 
 * 1. An informational display showing "Join matches via deskillz.games"
 * 2. A match info display when the game is launched via deep link
 * 3. A pre-match screen showing opponent info before gameplay starts
 * 
 * REMOVED FEATURES (now in main app):
 * - Tournament browsing
 * - Queue joining
 * - Matchmaking progress
 * - Match finding animation
 * 
 * KEPT FEATURES:
 * - Opponent display
 * - Match info display
 * - Entry fee confirmation
 * - Start match button
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzMatchmakingWidget_Updated : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzMatchmakingWidget_Updated(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Show the "join via main app" message
	 * Display when player tries to access tournaments from game
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowJoinViaMainAppMessage();
	
	/**
	 * Display match info from deep link launch
	 * @param LaunchData The launch data received
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void DisplayMatchInfo(const FDeskillzMatchLaunchData& LaunchData);
	
	/**
	 * Get current widget state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	EDeskillzWidgetState GetWidgetState() const { return CurrentState; }
	
	/**
	 * Show error message
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowError(const FString& ErrorMessage);
	
	/**
	 * Open main Deskillz app/website
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void OpenMainApp();
	
protected:
	virtual void NativeConstruct() override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	
	// ========================================================================
	// UI Bindings
	// ========================================================================
	
	/** Main message text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* MessageText;
	
	/** Secondary info text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* InfoText;
	
	/** "Open Deskillz" button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* OpenAppButton;
	
	/** Player avatar */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* PlayerAvatar;
	
	/** Player name */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* PlayerNameText;
	
	/** Opponent avatar */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* OpponentAvatar;
	
	/** Opponent name */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* OpponentNameText;
	
	/** VS text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* VersusText;
	
	/** Entry fee display */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* EntryFeeText;
	
	/** Prize pool display */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* PrizeText;
	
	/** Match duration display */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* DurationText;
	
	/** Start match button (when match info received) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* StartMatchButton;
	
	/** Close/Back button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* CloseButton;
	
	/** Container for "join via app" message */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UBorder* JoinMessageContainer;
	
	/** Container for match info display */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UBorder* MatchInfoContainer;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Main app URL */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FString MainAppURL = TEXT("https://deskillz.games");
	
	/** Main app deep link scheme */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FString MainAppScheme = TEXT("deskillz://lobby");
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Current widget state */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	EDeskillzWidgetState CurrentState = EDeskillzWidgetState::WaitingForLaunch;
	
	/** Current launch data */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FDeskillzMatchLaunchData CurrentLaunchData;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Set widget state */
	void SetState(EDeskillzWidgetState NewState);
	
	/** Update UI for current state */
	void UpdateUIForState();
	
	/** Show match info UI */
	void ShowMatchInfoUI();
	
	/** Show waiting/message UI */
	void ShowMessageUI();
	
	/** Format currency amount */
	FString FormatCurrency(double Amount, EDeskillzCurrency Currency) const;
	
	/** Format duration */
	FString FormatDuration(int32 Seconds) const;
	
	/** Handle open app clicked */
	UFUNCTION()
	void OnOpenAppClicked();
	
	/** Handle start match clicked */
	UFUNCTION()
	void OnStartMatchClicked();
	
	/** Handle close clicked */
	UFUNCTION()
	void OnCloseClicked();
};