// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "DeskillzHUDWidget.generated.h"

class UTextBlock;
class UButton;
class UProgressBar;
class UImage;
class UCanvasPanel;

/** Delegate for HUD actions */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForfeitClicked);

/**
 * In-Game HUD Widget
 * 
 * Displays during gameplay:
 * - Score display
 * - Timer/countdown
 * - Match info
 * - Pause/Menu button
 * - Opponent score (if synchronous)
 * - Progress indicators
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHUDWidget : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzHUDWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Set the current score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetScore(int64 Score);
	
	/**
	 * Set opponent score (for synchronous matches)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetOpponentScore(int64 Score);
	
	/**
	 * Set the timer value
	 * @param RemainingSeconds Seconds remaining, or total time if counting up
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetTimer(float RemainingSeconds);
	
	/**
	 * Set timer direction
	 * @param bCountDown True for countdown, false for count up
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetTimerCountDown(bool bCountDown);
	
	/**
	 * Show/hide opponent score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetOpponentScoreVisible(bool bVisible);
	
	/**
	 * Set match info text
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetMatchInfo(const FString& TournamentName, const FString& OpponentName);
	
	/**
	 * Show low time warning
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowLowTimeWarning(bool bShow);
	
	/**
	 * Add score with animation
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void AddScoreAnimated(int64 Points, const FVector2D& ScreenPosition);
	
	/**
	 * Show combo indicator
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowCombo(int32 ComboCount);
	
	/**
	 * Update progress bar
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetProgress(float Progress);
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when pause is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnPauseClicked OnPause;
	
	/** Called when forfeit is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnForfeitClicked OnForfeit;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	
	// ========================================================================
	// UI Bindings
	// ========================================================================
	
	/** Player score text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* ScoreText;
	
	/** Score label */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* ScoreLabelText;
	
	/** Opponent score text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* OpponentScoreText;
	
	/** Opponent name text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* OpponentNameText;
	
	/** Opponent score container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* OpponentScoreContainer;
	
	/** Timer text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TimerText;
	
	/** Timer icon */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* TimerIcon;
	
	/** Timer container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* TimerContainer;
	
	/** Tournament name text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TournamentNameText;
	
	/** Match progress bar */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UProgressBar* ProgressBar;
	
	/** Pause button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* PauseButton;
	
	/** Combo display text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* ComboText;
	
	/** Combo container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* ComboContainer;
	
	/** Canvas for floating score text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UCanvasPanel* FloatingTextCanvas;
	
	/** Low time warning container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* LowTimeWarning;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Low time threshold (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float LowTimeThreshold = 10.0f;
	
	/** Combo display duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float ComboDuration = 2.0f;
	
	/** Floating text duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float FloatingTextDuration = 1.0f;
	
	/** Score animation speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float ScoreAnimationSpeed = 5.0f;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Current score */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	int64 CurrentScore = 0;
	
	/** Displayed score (for animation) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	float DisplayedScore = 0.0f;
	
	/** Current opponent score */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	int64 CurrentOpponentScore = 0;
	
	/** Current timer value */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	float CurrentTimer = 0.0f;
	
	/** Is countdown mode */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bIsCountDown = true;
	
	/** Is low time warning showing */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bLowTimeWarning = false;
	
	/** Current combo */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	int32 CurrentCombo = 0;
	
	/** Combo timer */
	float ComboTimer = 0.0f;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Update score display */
	void UpdateScoreDisplay();
	
	/** Update timer display */
	void UpdateTimerDisplay();
	
	/** Handle pause clicked */
	UFUNCTION()
	void OnPauseClicked();
	
	/** Animate score change */
	void AnimateScore(float DeltaTime);
	
	/** Check for low time */
	void CheckLowTime();
	
	/** Spawn floating text */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void SpawnFloatingText(const FString& Text, const FVector2D& Position, const FLinearColor& Color);
	virtual void SpawnFloatingText_Implementation(const FString& Text, const FVector2D& Position, const FLinearColor& Color);
	
	/** Play low time pulse */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void PlayLowTimePulse();
	virtual void PlayLowTimePulse_Implementation();
};
