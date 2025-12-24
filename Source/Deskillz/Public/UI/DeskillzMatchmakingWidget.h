// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzMatchmakingWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UProgressBar;
class UCircularThrobber;
class UBorder;
class UOverlay;

/**
 * Matchmaking state
 */
UENUM(BlueprintType)
enum class EDeskillzMatchmakingState : uint8
{
	Idle,
	Connecting,
	Searching,
	Found,
	Confirming,
	Starting,
	Failed,
	Cancelled
};

/** Delegate for matchmaking events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStateChanged, EDeskillzMatchmakingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFound, const FDeskillzMatch&, Match);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchmakingCancelled);

/**
 * Matchmaking Widget
 * 
 * Displays matchmaking process with:
 * - Animated searching indicator
 * - Progress updates
 * - Opponent preview when found
 * - Entry fee confirmation
 * - Cancel option
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzMatchmakingWidget : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzMatchmakingWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Set tournament ID for matchmaking
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetTournamentId(const FString& TournamentId);
	
	/**
	 * Start the matchmaking process
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void StartMatchmaking();
	
	/**
	 * Cancel matchmaking
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void CancelMatchmaking();
	
	/**
	 * Update status display
	 * @param Status Status message
	 * @param Progress Progress value (0-1), negative for indeterminate
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void UpdateStatus(const FString& Status, float Progress = -1.0f);
	
	/**
	 * Get current matchmaking state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	EDeskillzMatchmakingState GetMatchmakingState() const { return CurrentState; }
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when matchmaking state changes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnMatchmakingStateChanged OnStateChanged;
	
	/** Called when match is found */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnMatchFound OnMatchFound;
	
	/** Called when matchmaking is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnMatchmakingCancelled OnCancelled;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	virtual bool HandleBackButton_Implementation() override;
	
	// ========================================================================
	// UI Bindings
	// ========================================================================
	
	/** Main status text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* StatusText;
	
	/** Secondary info text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* InfoText;
	
	/** Timer text (time spent searching) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TimerText;
	
	/** Progress bar */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UProgressBar* ProgressBar;
	
	/** Loading spinner */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UCircularThrobber* LoadingSpinner;
	
	/** Cancel button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* CancelButton;
	
	/** Play/Start button (shown after match found) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* PlayButton;
	
	/** Player avatar image */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* PlayerAvatar;
	
	/** Player name text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* PlayerNameText;
	
	/** Opponent avatar image */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* OpponentAvatar;
	
	/** Opponent name text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* OpponentNameText;
	
	/** VS text between players */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* VersusText;
	
	/** Entry fee display */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* EntryFeeText;
	
	/** Prize display */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* PrizeText;
	
	/** Match found overlay */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UOverlay* MatchFoundOverlay;
	
	/** Searching overlay */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UOverlay* SearchingOverlay;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Matchmaking timeout (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float MatchmakingTimeout = 60.0f;
	
	/** Auto-start match after finding opponent */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	bool bAutoStartMatch = false;
	
	/** Countdown before auto-start */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float AutoStartDelay = 3.0f;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Current tournament ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FString CurrentTournamentId;
	
	/** Current matchmaking state */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	EDeskillzMatchmakingState CurrentState = EDeskillzMatchmakingState::Idle;
	
	/** Found match data */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FDeskillzMatch FoundMatch;
	
	/** Time spent searching */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	float SearchTime = 0.0f;
	
	/** Auto-start countdown */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	float AutoStartCountdown = 0.0f;
	
	/** Animation rotation angle */
	float AnimationAngle = 0.0f;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Set matchmaking state */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetState(EDeskillzMatchmakingState NewState);
	
	/** Update UI for current state */
	void UpdateUIForState();
	
	/** Show searching UI */
	void ShowSearchingUI();
	
	/** Show match found UI */
	void ShowMatchFoundUI();
	
	/** Show error UI */
	void ShowErrorUI(const FString& ErrorMessage);
	
	/** Update timer display */
	void UpdateTimerDisplay();
	
	/** Handle cancel clicked */
	UFUNCTION()
	void OnCancelClicked();
	
	/** Handle play clicked */
	UFUNCTION()
	void OnPlayClicked();
	
	/** Called when match is found from SDK */
	void OnMatchmakingComplete(bool bSuccess, const FDeskillzMatch& Match);
	
	/** Start the actual match */
	void StartMatch();
	
	/** Play match found animation */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void PlayMatchFoundAnimation();
	virtual void PlayMatchFoundAnimation_Implementation();
};
