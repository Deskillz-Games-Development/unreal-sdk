// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzHUDWidget.h"
#include "Deskillz.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

UDeskillzHUDWidget::UDeskillzHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup pause button
	if (PauseButton)
	{
		PauseButton->OnClicked.AddDynamic(this, &UDeskillzHUDWidget::OnPauseClicked);
	}
	
	// Initialize displays
	if (ScoreLabelText)
	{
		ScoreLabelText->SetText(FText::FromString(TEXT("SCORE")));
	}
	
	// Hide optional elements initially
	if (OpponentScoreContainer)
	{
		OpponentScoreContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (ComboContainer)
	{
		ComboContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (LowTimeWarning)
	{
		LowTimeWarning->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Initialize score display
	UpdateScoreDisplay();
	UpdateTimerDisplay();
}

void UDeskillzHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Animate score if different from target
	AnimateScore(InDeltaTime);
	
	// Update combo timer
	if (CurrentCombo > 0 && ComboTimer > 0.0f)
	{
		ComboTimer -= InDeltaTime;
		if (ComboTimer <= 0.0f)
		{
			CurrentCombo = 0;
			if (ComboContainer)
			{
				ComboContainer->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UDeskillzHUDWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	if (ScoreText)
	{
		ScoreText->SetColorAndOpacity(Theme.PrimaryColor);
	}
	
	if (ScoreLabelText)
	{
		ScoreLabelText->SetColorAndOpacity(Theme.MutedTextColor);
	}
	
	if (TimerText)
	{
		TimerText->SetColorAndOpacity(Theme.TextColor);
	}
	
	if (ComboText)
	{
		ComboText->SetColorAndOpacity(Theme.SecondaryColor);
	}
	
	if (TournamentNameText)
	{
		TournamentNameText->SetColorAndOpacity(Theme.MutedTextColor);
	}
	
	if (ProgressBar)
	{
		ProgressBar->SetFillColorAndOpacity(Theme.PrimaryColor);
	}
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzHUDWidget::SetScore(int64 Score)
{
	CurrentScore = Score;
	// Score animation handled in tick
}

void UDeskillzHUDWidget::SetOpponentScore(int64 Score)
{
	CurrentOpponentScore = Score;
	
	if (OpponentScoreText)
	{
		OpponentScoreText->SetText(FText::FromString(FormatNumber(Score)));
	}
}

void UDeskillzHUDWidget::SetTimer(float RemainingSeconds)
{
	CurrentTimer = FMath::Max(0.0f, RemainingSeconds);
	UpdateTimerDisplay();
	CheckLowTime();
}

void UDeskillzHUDWidget::SetTimerCountDown(bool bCountDown)
{
	bIsCountDown = bCountDown;
}

void UDeskillzHUDWidget::SetOpponentScoreVisible(bool bVisible)
{
	if (OpponentScoreContainer)
	{
		OpponentScoreContainer->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UDeskillzHUDWidget::SetMatchInfo(const FString& TournamentName, const FString& OpponentName)
{
	if (TournamentNameText)
	{
		TournamentNameText->SetText(FText::FromString(TournamentName));
	}
	
	if (OpponentNameText)
	{
		OpponentNameText->SetText(FText::FromString(OpponentName));
	}
}

void UDeskillzHUDWidget::ShowLowTimeWarning(bool bShow)
{
	bLowTimeWarning = bShow;
	
	if (LowTimeWarning)
	{
		LowTimeWarning->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (bShow)
	{
		PlayLowTimePulse();
	}
}

void UDeskillzHUDWidget::AddScoreAnimated(int64 Points, const FVector2D& ScreenPosition)
{
	// Add to current score
	CurrentScore += Points;
	
	// Spawn floating text
	FString PointsText = FString::Printf(TEXT("+%s"), *FormatNumber(Points));
	SpawnFloatingText(PointsText, ScreenPosition, CurrentTheme.SuccessColor);
}

void UDeskillzHUDWidget::ShowCombo(int32 ComboCount)
{
	CurrentCombo = ComboCount;
	ComboTimer = ComboDuration;
	
	if (ComboContainer)
	{
		ComboContainer->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (ComboText)
	{
		FString ComboStr = FString::Printf(TEXT("%dx COMBO!"), ComboCount);
		ComboText->SetText(FText::FromString(ComboStr));
	}
}

void UDeskillzHUDWidget::SetProgress(float Progress)
{
	if (ProgressBar)
	{
		ProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzHUDWidget::UpdateScoreDisplay()
{
	if (ScoreText)
	{
		int64 DisplayValue = FMath::RoundToInt64(DisplayedScore);
		ScoreText->SetText(FText::FromString(FormatNumber(DisplayValue)));
	}
}

void UDeskillzHUDWidget::UpdateTimerDisplay()
{
	if (TimerText)
	{
		FString TimeStr = FormatTime(CurrentTimer);
		TimerText->SetText(FText::FromString(TimeStr));
		
		// Color based on time remaining
		if (bLowTimeWarning)
		{
			TimerText->SetColorAndOpacity(CurrentTheme.ErrorColor);
		}
		else
		{
			TimerText->SetColorAndOpacity(CurrentTheme.TextColor);
		}
	}
}

void UDeskillzHUDWidget::OnPauseClicked()
{
	PlayClickSound();
	OnPause.Broadcast();
	
	// Show pause menu
	if (UIManager)
	{
		UIManager->ShowPopup(
			TEXT("Paused"),
			TEXT("Match in progress. Forfeit or continue?"),
			true,
			TEXT("Continue"),
			TEXT("Forfeit")
		);
	}
}

void UDeskillzHUDWidget::AnimateScore(float DeltaTime)
{
	if (FMath::Abs(DisplayedScore - static_cast<float>(CurrentScore)) > 0.5f)
	{
		// Lerp towards target
		float Diff = static_cast<float>(CurrentScore) - DisplayedScore;
		float Speed = FMath::Max(FMath::Abs(Diff) * ScoreAnimationSpeed, 100.0f);
		
		if (Diff > 0)
		{
			DisplayedScore += Speed * DeltaTime;
			DisplayedScore = FMath::Min(DisplayedScore, static_cast<float>(CurrentScore));
		}
		else
		{
			DisplayedScore -= Speed * DeltaTime;
			DisplayedScore = FMath::Max(DisplayedScore, static_cast<float>(CurrentScore));
		}
		
		UpdateScoreDisplay();
	}
	else
	{
		DisplayedScore = static_cast<float>(CurrentScore);
	}
}

void UDeskillzHUDWidget::CheckLowTime()
{
	if (bIsCountDown && CurrentTimer <= LowTimeThreshold && CurrentTimer > 0.0f)
	{
		if (!bLowTimeWarning)
		{
			ShowLowTimeWarning(true);
		}
	}
	else if (bLowTimeWarning && CurrentTimer > LowTimeThreshold)
	{
		ShowLowTimeWarning(false);
	}
}

void UDeskillzHUDWidget::SpawnFloatingText_Implementation(const FString& Text, const FVector2D& Position, const FLinearColor& Color)
{
	// Default implementation - override in Blueprint for custom floating text
	// This would typically:
	// 1. Create a text widget
	// 2. Position at screen location
	// 3. Animate upward with fade
	// 4. Destroy after duration
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Floating text: %s at (%f, %f)"), *Text, Position.X, Position.Y);
}

void UDeskillzHUDWidget::PlayLowTimePulse_Implementation()
{
	// Default implementation - override in Blueprint for custom pulse animation
	// This would typically:
	// 1. Scale/flash the timer
	// 2. Play warning sound
	// 3. Add screen vignette
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Low time pulse!"));
}
