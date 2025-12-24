// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzResultsWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UProgressBar;
class UCanvasPanel;
class UOverlay;

/** Delegate for result actions */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayAgainClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnViewLeaderboardClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitClicked);

/**
 * Match Results Widget
 * 
 * Displays match outcome with:
 * - Win/Loss/Draw banner with animation
 * - Score comparison
 * - Prize won (if any)
 * - Stats breakdown
 * - Play again / Exit options
 * - Leaderboard preview
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzResultsWidget : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzResultsWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Set the match result to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetMatchResult(const FDeskillzMatchResult& Result);
	
	/**
	 * Play the result reveal animation
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void PlayResultAnimation();
	
	/**
	 * Get the displayed result
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	FDeskillzMatchResult GetMatchResult() const { return DisplayedResult; }
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when play again is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnPlayAgainClicked OnPlayAgain;
	
	/** Called when view leaderboard is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnViewLeaderboardClicked OnViewLeaderboard;
	
	/** Called when exit is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnExitClicked OnExit;
	
protected:
	virtual void NativeConstruct() override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	virtual bool HandleBackButton_Implementation() override;
	
	// ========================================================================
	// UI Bindings
	// ========================================================================
	
	/** Result banner (WIN/LOSE/DRAW) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* ResultBannerText;
	
	/** Result subtitle (e.g., "Great game!") */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* ResultSubtitleText;
	
	/** Player score text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* PlayerScoreText;
	
	/** Player name text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* PlayerNameText;
	
	/** Player avatar */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* PlayerAvatar;
	
	/** Opponent score text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* OpponentScoreText;
	
	/** Opponent name text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* OpponentNameText;
	
	/** Opponent avatar */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* OpponentAvatar;
	
	/** Prize won text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* PrizeWonText;
	
	/** Prize container (hidden if no prize) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* PrizeContainer;
	
	/** Skill rating change text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* RatingChangeText;
	
	/** New rank text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* NewRankText;
	
	/** Play again button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* PlayAgainButton;
	
	/** View leaderboard button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* LeaderboardButton;
	
	/** Exit button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* ExitButton;
	
	/** Win overlay (confetti, etc.) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UOverlay* WinEffectsOverlay;
	
	/** Stats container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* StatsContainer;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Win message options */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TArray<FString> WinMessages = { TEXT("Victory!"), TEXT("You Win!"), TEXT("Champion!") };
	
	/** Lose message options */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TArray<FString> LoseMessages = { TEXT("Defeat"), TEXT("Better luck next time!"), TEXT("Close game!") };
	
	/** Draw message options */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TArray<FString> DrawMessages = { TEXT("Draw!"), TEXT("Tie Game!"), TEXT("Evenly Matched!") };
	
	/** Score count-up duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float ScoreAnimationDuration = 1.5f;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Displayed match result */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FDeskillzMatchResult DisplayedResult;
	
	/** Is animation playing */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bIsAnimating = false;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Update UI with result data */
	void UpdateResultDisplay();
	
	/** Get random message for outcome */
	FString GetOutcomeMessage(EDeskillzMatchOutcome Outcome) const;
	
	/** Get color for outcome */
	FLinearColor GetOutcomeColor(EDeskillzMatchOutcome Outcome) const;
	
	/** Handle play again clicked */
	UFUNCTION()
	void OnPlayAgainClicked();
	
	/** Handle leaderboard clicked */
	UFUNCTION()
	void OnLeaderboardClicked();
	
	/** Handle exit clicked */
	UFUNCTION()
	void OnExitClicked();
	
	/** Animate score count-up */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void AnimateScoreCountUp();
	virtual void AnimateScoreCountUp_Implementation();
	
	/** Play win effects */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void PlayWinEffects();
	virtual void PlayWinEffects_Implementation();
};
