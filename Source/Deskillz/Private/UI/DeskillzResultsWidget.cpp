// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzResultsWidget.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Kismet/KismetMathLibrary.h"

UDeskillzResultsWidget::UDeskillzResultsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzResultsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup buttons
	if (PlayAgainButton)
	{
		PlayAgainButton->OnClicked.AddDynamic(this, &UDeskillzResultsWidget::OnPlayAgainClicked);
	}
	
	if (LeaderboardButton)
	{
		LeaderboardButton->OnClicked.AddDynamic(this, &UDeskillzResultsWidget::OnLeaderboardClicked);
	}
	
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UDeskillzResultsWidget::OnExitClicked);
	}
	
	// Hide win effects initially
	if (WinEffectsOverlay)
	{
		WinEffectsOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDeskillzResultsWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	if (ResultSubtitleText)
	{
		ResultSubtitleText->SetColorAndOpacity(Theme.MutedTextColor);
	}
	
	if (PlayerNameText)
	{
		PlayerNameText->SetColorAndOpacity(Theme.TextColor);
	}
	
	if (OpponentNameText)
	{
		OpponentNameText->SetColorAndOpacity(Theme.TextColor);
	}
	
	// Result banner color is set dynamically based on outcome
}

bool UDeskillzResultsWidget::HandleBackButton_Implementation()
{
	OnExitClicked();
	return true;
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzResultsWidget::SetMatchResult(const FDeskillzMatchResult& Result)
{
	DisplayedResult = Result;
	UpdateResultDisplay();
	
	UE_LOG(LogDeskillz, Log, TEXT("Displaying result for match: %s, Outcome: %d"), 
		*Result.MatchId, static_cast<int32>(Result.Outcome));
}

void UDeskillzResultsWidget::PlayResultAnimation()
{
	if (bIsAnimating)
	{
		return;
	}
	
	bIsAnimating = true;
	
	// Start with elements hidden/zero
	if (PlayerScoreText)
	{
		PlayerScoreText->SetText(FText::FromString(TEXT("0")));
	}
	
	if (OpponentScoreText)
	{
		OpponentScoreText->SetText(FText::FromString(TEXT("0")));
	}
	
	// Play show animation
	PlayShowAnimation();
	
	// Animate score count-up
	AnimateScoreCountUp();
	
	// Play win effects if won
	if (DisplayedResult.Outcome == EDeskillzMatchOutcome::Win)
	{
		PlayWinEffects();
		PlaySuccessSound();
	}
	else if (DisplayedResult.Outcome == EDeskillzMatchOutcome::Loss)
	{
		// Could play a subtle lose effect
	}
	
	bIsAnimating = false;
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzResultsWidget::UpdateResultDisplay()
{
	// Result banner
	if (ResultBannerText)
	{
		FString Message = GetOutcomeMessage(DisplayedResult.Outcome);
		ResultBannerText->SetText(FText::FromString(Message));
		ResultBannerText->SetColorAndOpacity(GetOutcomeColor(DisplayedResult.Outcome));
	}
	
	// Subtitle based on outcome
	if (ResultSubtitleText)
	{
		FString Subtitle;
		switch (DisplayedResult.Outcome)
		{
			case EDeskillzMatchOutcome::Win:
				Subtitle = TEXT("Great performance!");
				break;
			case EDeskillzMatchOutcome::Loss:
				Subtitle = TEXT("Don't give up!");
				break;
			case EDeskillzMatchOutcome::Draw:
				Subtitle = TEXT("So close!");
				break;
			default:
				Subtitle = TEXT("");
				break;
		}
		ResultSubtitleText->SetText(FText::FromString(Subtitle));
	}
	
	// Player info
	if (PlayerScoreText)
	{
		PlayerScoreText->SetText(FText::FromString(FormatNumber(DisplayedResult.PlayerScore)));
	}
	
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(DisplayedResult.PlayerName.IsEmpty() ? TEXT("You") : DisplayedResult.PlayerName));
	}
	
	// Opponent info
	if (OpponentScoreText)
	{
		OpponentScoreText->SetText(FText::FromString(FormatNumber(DisplayedResult.OpponentScore)));
	}
	
	if (OpponentNameText)
	{
		OpponentNameText->SetText(FText::FromString(DisplayedResult.OpponentName.IsEmpty() ? TEXT("Opponent") : DisplayedResult.OpponentName));
	}
	
	// Prize won
	if (PrizeContainer)
	{
		bool bHasPrize = DisplayedResult.PrizeWon > 0.0;
		PrizeContainer->SetVisibility(bHasPrize ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		
		if (bHasPrize && PrizeWonText)
		{
			FString PrizeStr = TEXT("+") + FormatCurrency(DisplayedResult.PrizeWon, DisplayedResult.PrizeCurrency);
			PrizeWonText->SetText(FText::FromString(PrizeStr));
			PrizeWonText->SetColorAndOpacity(CurrentTheme.SuccessColor);
		}
	}
	
	// Rating change
	if (RatingChangeText)
	{
		int32 Change = DisplayedResult.NewRating - DisplayedResult.OldRating;
		FString RatingStr;
		
		if (Change > 0)
		{
			RatingStr = FString::Printf(TEXT("+%d"), Change);
			RatingChangeText->SetColorAndOpacity(CurrentTheme.SuccessColor);
		}
		else if (Change < 0)
		{
			RatingStr = FString::Printf(TEXT("%d"), Change);
			RatingChangeText->SetColorAndOpacity(CurrentTheme.ErrorColor);
		}
		else
		{
			RatingStr = TEXT("±0");
			RatingChangeText->SetColorAndOpacity(CurrentTheme.MutedTextColor);
		}
		
		RatingChangeText->SetText(FText::FromString(RatingStr));
	}
	
	// New rank
	if (NewRankText)
	{
		NewRankText->SetText(FText::FromString(FString::Printf(TEXT("Rating: %d"), DisplayedResult.NewRating)));
	}
}

FString UDeskillzResultsWidget::GetOutcomeMessage(EDeskillzMatchOutcome Outcome) const
{
	const TArray<FString>* Messages = nullptr;
	
	switch (Outcome)
	{
		case EDeskillzMatchOutcome::Win:
			Messages = &WinMessages;
			break;
		case EDeskillzMatchOutcome::Loss:
			Messages = &LoseMessages;
			break;
		case EDeskillzMatchOutcome::Draw:
			Messages = &DrawMessages;
			break;
		default:
			return TEXT("Match Complete");
	}
	
	if (Messages && Messages->Num() > 0)
	{
		int32 Index = FMath::RandRange(0, Messages->Num() - 1);
		return (*Messages)[Index];
	}
	
	return TEXT("Match Complete");
}

FLinearColor UDeskillzResultsWidget::GetOutcomeColor(EDeskillzMatchOutcome Outcome) const
{
	switch (Outcome)
	{
		case EDeskillzMatchOutcome::Win:
			return CurrentTheme.SuccessColor;
		case EDeskillzMatchOutcome::Loss:
			return CurrentTheme.ErrorColor;
		case EDeskillzMatchOutcome::Draw:
			return CurrentTheme.WarningColor;
		default:
			return CurrentTheme.TextColor;
	}
}

void UDeskillzResultsWidget::OnPlayAgainClicked()
{
	PlayClickSound();
	OnPlayAgain.Broadcast();
	
	// Return to tournament list
	if (UIManager)
	{
		UIManager->HideResults();
		UIManager->ShowTournamentList();
	}
}

void UDeskillzResultsWidget::OnLeaderboardClicked()
{
	PlayClickSound();
	OnViewLeaderboard.Broadcast();
	
	// Show leaderboard
	if (UIManager)
	{
		UIManager->ShowLeaderboard(DisplayedResult.TournamentId);
	}
}

void UDeskillzResultsWidget::OnExitClicked()
{
	PlayClickSound();
	OnExit.Broadcast();
	
	// Hide all Deskillz UI
	if (UIManager)
	{
		UIManager->HideAllUI();
	}
}

void UDeskillzResultsWidget::AnimateScoreCountUp_Implementation()
{
	// Default implementation - count up over ScoreAnimationDuration
	// In full implementation, use timer to animate from 0 to final score
	
	// For now, just set final values
	if (PlayerScoreText)
	{
		PlayerScoreText->SetText(FText::FromString(FormatNumber(DisplayedResult.PlayerScore)));
	}
	
	if (OpponentScoreText)
	{
		OpponentScoreText->SetText(FText::FromString(FormatNumber(DisplayedResult.OpponentScore)));
	}
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Score animation complete"));
}

void UDeskillzResultsWidget::PlayWinEffects_Implementation()
{
	// Default implementation - show win effects overlay
	if (WinEffectsOverlay)
	{
		WinEffectsOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	
	// In full implementation:
	// - Confetti particle effect
	// - Screen shake
	// - Trophy animation
	// - Victory sound
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Win effects playing"));
}
