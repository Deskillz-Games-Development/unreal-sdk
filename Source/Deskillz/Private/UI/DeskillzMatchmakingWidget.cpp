// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzMatchmakingWidget.h"
#include "Core/DeskillzSDK.h"
#include "Match/DeskillzMatchmaking.h"
#include "Deskillz.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/CircularThrobber.h"
#include "Components/Overlay.h"

UDeskillzMatchmakingWidget::UDeskillzMatchmakingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzMatchmakingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup buttons
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UDeskillzMatchmakingWidget::OnCancelClicked);
	}
	
	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UDeskillzMatchmakingWidget::OnPlayClicked);
		PlayButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Initial state
	SetState(EDeskillzMatchmakingState::Idle);
}

void UDeskillzMatchmakingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Update based on state
	switch (CurrentState)
	{
		case EDeskillzMatchmakingState::Searching:
		{
			// Update search timer
			SearchTime += InDeltaTime;
			UpdateTimerDisplay();
			
			// Check timeout
			if (SearchTime >= MatchmakingTimeout)
			{
				SetState(EDeskillzMatchmakingState::Failed);
				ShowErrorUI(TEXT("Matchmaking timed out"));
			}
			
			// Animate search indicator
			AnimationAngle += InDeltaTime * 90.0f; // 90 degrees per second
			if (AnimationAngle >= 360.0f)
			{
				AnimationAngle -= 360.0f;
			}
		}
		break;
		
		case EDeskillzMatchmakingState::Found:
		{
			// Auto-start countdown
			if (bAutoStartMatch && AutoStartCountdown > 0.0f)
			{
				AutoStartCountdown -= InDeltaTime;
				
				if (InfoText)
				{
					InfoText->SetText(FText::FromString(
						FString::Printf(TEXT("Starting in %d..."), FMath::CeilToInt(AutoStartCountdown))
					));
				}
				
				if (AutoStartCountdown <= 0.0f)
				{
					StartMatch();
				}
			}
		}
		break;
		
		default:
			break;
	}
}

void UDeskillzMatchmakingWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	if (StatusText)
	{
		StatusText->SetColorAndOpacity(Theme.TextColor);
	}
	
	if (InfoText)
	{
		InfoText->SetColorAndOpacity(Theme.MutedTextColor);
	}
	
	if (TimerText)
	{
		TimerText->SetColorAndOpacity(Theme.MutedTextColor);
	}
	
	if (VersusText)
	{
		VersusText->SetColorAndOpacity(Theme.PrimaryColor);
	}
	
	if (EntryFeeText)
	{
		EntryFeeText->SetColorAndOpacity(Theme.WarningColor);
	}
	
	if (PrizeText)
	{
		PrizeText->SetColorAndOpacity(Theme.SuccessColor);
	}
	
	if (ProgressBar)
	{
		ProgressBar->SetFillColorAndOpacity(Theme.PrimaryColor);
	}
}

bool UDeskillzMatchmakingWidget::HandleBackButton_Implementation()
{
	if (CurrentState == EDeskillzMatchmakingState::Searching ||
		CurrentState == EDeskillzMatchmakingState::Connecting)
	{
		CancelMatchmaking();
		return true;
	}
	
	if (UIManager)
	{
		UIManager->HideMatchmaking();
		UIManager->ShowTournamentList();
	}
	
	return true;
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzMatchmakingWidget::SetTournamentId(const FString& TournamentId)
{
	CurrentTournamentId = TournamentId;
	UE_LOG(LogDeskillz, Log, TEXT("Matchmaking set for tournament: %s"), *TournamentId);
}

void UDeskillzMatchmakingWidget::StartMatchmaking()
{
	if (CurrentState == EDeskillzMatchmakingState::Searching)
	{
		return;
	}
	
	SearchTime = 0.0f;
	SetState(EDeskillzMatchmakingState::Connecting);
	
	UDeskillzSDK* SDK = UDeskillzSDK::Get();
	if (!SDK)
	{
		SetState(EDeskillzMatchmakingState::Failed);
		ShowErrorUI(TEXT("SDK not initialized"));
		return;
	}
	
	// Start matchmaking via SDK
	UDeskillzMatchmaking* Matchmaking = UDeskillzMatchmaking::Get(this);
	if (Matchmaking)
	{
		// Bind to matchmaking events
		Matchmaking->OnMatchFound.AddDynamic(this, &UDeskillzMatchmakingWidget::OnMatchmakingComplete);
		
		// Start searching
		Matchmaking->StartMatchmaking(CurrentTournamentId);
		SetState(EDeskillzMatchmakingState::Searching);
	}
	else
	{
		// Simulate for testing
		SetState(EDeskillzMatchmakingState::Searching);
		
		// Simulate finding a match after 3 seconds
		FTimerHandle TimerHandle;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				FDeskillzMatch SimMatch;
				SimMatch.Id = TEXT("match_sim_001");
				SimMatch.TournamentId = CurrentTournamentId;
				SimMatch.Status = EDeskillzMatchStatus::Ready;
				SimMatch.EntryFee = 0.001;
				SimMatch.EntryCurrency = TEXT("ETH");
				SimMatch.PrizeAmount = 0.002;
				SimMatch.PrizeCurrency = TEXT("ETH");
				
				// Player
				FDeskillzPlayerInfo Player;
				Player.Id = TEXT("player_local");
				Player.Username = TEXT("You");
				Player.SkillRating = 1500;
				SimMatch.Players.Add(Player);
				
				// Opponent
				FDeskillzPlayerInfo Opponent;
				Opponent.Id = TEXT("opponent_001");
				Opponent.Username = TEXT("Challenger123");
				Opponent.SkillRating = 1520;
				SimMatch.Players.Add(Opponent);
				
				OnMatchmakingComplete(true, SimMatch);
			}, 3.0f, false);
		}
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Started matchmaking for tournament: %s"), *CurrentTournamentId);
}

void UDeskillzMatchmakingWidget::CancelMatchmaking()
{
	if (CurrentState != EDeskillzMatchmakingState::Searching &&
		CurrentState != EDeskillzMatchmakingState::Connecting)
	{
		return;
	}
	
	// Cancel via SDK
	UDeskillzMatchmaking* Matchmaking = UDeskillzMatchmaking::Get(this);
	if (Matchmaking)
	{
		Matchmaking->CancelMatchmaking();
	}
	
	SetState(EDeskillzMatchmakingState::Cancelled);
	OnCancelled.Broadcast();
	
	UE_LOG(LogDeskillz, Log, TEXT("Matchmaking cancelled"));
}

void UDeskillzMatchmakingWidget::UpdateStatus(const FString& Status, float Progress)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Status));
	}
	
	if (ProgressBar)
	{
		if (Progress < 0.0f)
		{
			// Indeterminate - hide progress bar, show spinner
			ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
			if (LoadingSpinner)
			{
				LoadingSpinner->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			// Determinate progress
			ProgressBar->SetVisibility(ESlateVisibility::Visible);
			ProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
			if (LoadingSpinner)
			{
				LoadingSpinner->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzMatchmakingWidget::SetState(EDeskillzMatchmakingState NewState)
{
	if (CurrentState != NewState)
	{
		EDeskillzMatchmakingState OldState = CurrentState;
		CurrentState = NewState;
		
		UpdateUIForState();
		OnStateChanged.Broadcast(NewState);
		
		UE_LOG(LogDeskillz, Verbose, TEXT("Matchmaking state: %d -> %d"), 
			static_cast<int32>(OldState), static_cast<int32>(NewState));
	}
}

void UDeskillzMatchmakingWidget::UpdateUIForState()
{
	switch (CurrentState)
	{
		case EDeskillzMatchmakingState::Idle:
			UpdateStatus(TEXT("Ready to play"), -1.0f);
			break;
			
		case EDeskillzMatchmakingState::Connecting:
			UpdateStatus(TEXT("Connecting..."), -1.0f);
			ShowSearchingUI();
			break;
			
		case EDeskillzMatchmakingState::Searching:
			UpdateStatus(TEXT("Finding opponent..."), -1.0f);
			ShowSearchingUI();
			break;
			
		case EDeskillzMatchmakingState::Found:
			UpdateStatus(TEXT("Opponent found!"), 1.0f);
			ShowMatchFoundUI();
			break;
			
		case EDeskillzMatchmakingState::Confirming:
			UpdateStatus(TEXT("Confirming match..."), -1.0f);
			break;
			
		case EDeskillzMatchmakingState::Starting:
			UpdateStatus(TEXT("Starting match..."), -1.0f);
			break;
			
		case EDeskillzMatchmakingState::Failed:
			// Error message set by ShowErrorUI
			break;
			
		case EDeskillzMatchmakingState::Cancelled:
			UpdateStatus(TEXT("Matchmaking cancelled"), -1.0f);
			break;
	}
}

void UDeskillzMatchmakingWidget::ShowSearchingUI()
{
	// Show searching overlay
	if (SearchingOverlay)
	{
		SearchingOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Hide match found overlay
	if (MatchFoundOverlay)
	{
		MatchFoundOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Show cancel, hide play
	if (CancelButton)
	{
		CancelButton->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (PlayButton)
	{
		PlayButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Show spinner
	if (LoadingSpinner)
	{
		LoadingSpinner->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Hide progress bar
	if (ProgressBar)
	{
		ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDeskillzMatchmakingWidget::ShowMatchFoundUI()
{
	// Hide searching overlay
	if (SearchingOverlay)
	{
		SearchingOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Show match found overlay
	if (MatchFoundOverlay)
	{
		MatchFoundOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Hide spinner
	if (LoadingSpinner)
	{
		LoadingSpinner->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Update player info
	if (FoundMatch.Players.Num() >= 2)
	{
		// Local player
		if (PlayerNameText)
		{
			PlayerNameText->SetText(FText::FromString(FoundMatch.Players[0].Username));
		}
		
		// Opponent
		if (OpponentNameText)
		{
			OpponentNameText->SetText(FText::FromString(FoundMatch.Players[1].Username));
		}
	}
	
	// Update VS text
	if (VersusText)
	{
		VersusText->SetText(FText::FromString(TEXT("VS")));
	}
	
	// Update entry fee
	if (EntryFeeText)
	{
		FString FeeText = FormatCurrency(FoundMatch.EntryFee, FoundMatch.EntryCurrency);
		EntryFeeText->SetText(FText::FromString(TEXT("Entry: ") + FeeText));
	}
	
	// Update prize
	if (PrizeText)
	{
		FString PrizeTextStr = FormatCurrency(FoundMatch.PrizeAmount, FoundMatch.PrizeCurrency);
		PrizeText->SetText(FText::FromString(TEXT("Prize: ") + PrizeTextStr));
	}
	
	// Show play button
	if (PlayButton)
	{
		PlayButton->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Optionally hide cancel
	if (CancelButton && bAutoStartMatch)
	{
		CancelButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Start auto-start countdown
	if (bAutoStartMatch)
	{
		AutoStartCountdown = AutoStartDelay;
	}
	
	// Play animation
	PlayMatchFoundAnimation();
	PlaySuccessSound();
}

void UDeskillzMatchmakingWidget::ShowErrorUI(const FString& ErrorMessage)
{
	UpdateStatus(ErrorMessage, -1.0f);
	
	// Hide spinner
	if (LoadingSpinner)
	{
		LoadingSpinner->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Show cancel as "Back"
	if (CancelButton)
	{
		CancelButton->SetVisibility(ESlateVisibility::Visible);
		// In full implementation, change button text to "Back"
	}
	
	PlayErrorSound();
}

void UDeskillzMatchmakingWidget::UpdateTimerDisplay()
{
	if (TimerText)
	{
		FString TimeStr = FormatTime(SearchTime);
		TimerText->SetText(FText::FromString(TimeStr));
	}
}

void UDeskillzMatchmakingWidget::OnCancelClicked()
{
	PlayClickSound();
	
	if (CurrentState == EDeskillzMatchmakingState::Searching ||
		CurrentState == EDeskillzMatchmakingState::Connecting)
	{
		CancelMatchmaking();
	}
	
	// Go back
	HandleBackButton();
}

void UDeskillzMatchmakingWidget::OnPlayClicked()
{
	PlayClickSound();
	StartMatch();
}

void UDeskillzMatchmakingWidget::OnMatchmakingComplete(bool bSuccess, const FDeskillzMatch& Match)
{
	if (bSuccess)
	{
		FoundMatch = Match;
		SetState(EDeskillzMatchmakingState::Found);
		OnMatchFound.Broadcast(Match);
		
		UE_LOG(LogDeskillz, Log, TEXT("Match found: %s"), *Match.Id);
	}
	else
	{
		SetState(EDeskillzMatchmakingState::Failed);
		ShowErrorUI(TEXT("Failed to find match"));
		
		UE_LOG(LogDeskillz, Error, TEXT("Matchmaking failed"));
	}
}

void UDeskillzMatchmakingWidget::StartMatch()
{
	SetState(EDeskillzMatchmakingState::Starting);
	
	UDeskillzSDK* SDK = UDeskillzSDK::Get();
	if (SDK)
	{
		// Start the match through SDK
		SDK->StartMatch(FoundMatch.Id);
	}
	
	// Hide matchmaking UI
	if (UIManager)
	{
		UIManager->HideMatchmaking();
		UIManager->ShowHUD();
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Starting match: %s"), *FoundMatch.Id);
}

void UDeskillzMatchmakingWidget::PlayMatchFoundAnimation_Implementation()
{
	// Default implementation - override in Blueprint for custom animation
	// This would typically include:
	// - Slide in opponent from right
	// - Scale up VS text
	// - Particle effects
	// - Sound effect
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Playing match found animation"));
}
