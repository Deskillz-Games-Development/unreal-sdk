// Copyright Deskillz Games. All Rights Reserved.
// DeskillzMatchmakingWidget_Updated.cpp - UPDATED for centralized lobby architecture

#include "Lobby/DeskillzMatchmakingWidget_Updated.h"
#include "Lobby/DeskillzBridge.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "HAL/PlatformProcess.h"

UDeskillzMatchmakingWidget_Updated::UDeskillzMatchmakingWidget_Updated(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Constructor
}

void UDeskillzMatchmakingWidget_Updated::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bind button clicks
	if (OpenAppButton)
	{
		OpenAppButton->OnClicked.AddDynamic(this, &UDeskillzMatchmakingWidget_Updated::OnOpenAppClicked);
	}
	
	if (StartMatchButton)
	{
		StartMatchButton->OnClicked.AddDynamic(this, &UDeskillzMatchmakingWidget_Updated::OnStartMatchClicked);
	}
	
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UDeskillzMatchmakingWidget_Updated::OnCloseClicked);
	}
	
	// Default to showing "join via main app" message
	ShowJoinViaMainAppMessage();
}

void UDeskillzMatchmakingWidget_Updated::ShowJoinViaMainAppMessage()
{
	SetState(EDeskillzWidgetState::WaitingForLaunch);
	
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(TEXT("Join Tournaments via Deskillz")));
	}
	
	if (InfoText)
	{
		InfoText->SetText(FText::FromString(
			TEXT("Browse tournaments, join matches, and compete for crypto prizes at deskillz.games\n\n")
			TEXT("When you find a match, the game will automatically launch with your opponent.")));
	}
	
	ShowMessageUI();
}

void UDeskillzMatchmakingWidget_Updated::DisplayMatchInfo(const FDeskillzMatchLaunchData& LaunchData)
{
	if (!LaunchData.bIsValid)
	{
		ShowError(TEXT("Invalid match data received"));
		return;
	}
	
	CurrentLaunchData = LaunchData;
	SetState(EDeskillzWidgetState::MatchReceived);
	
	// Update match info displays
	if (EntryFeeText)
	{
		FString FeeStr = FormatCurrency(LaunchData.EntryFee, LaunchData.Currency);
		EntryFeeText->SetText(FText::FromString(FString::Printf(TEXT("Entry: %s"), *FeeStr)));
	}
	
	if (PrizeText)
	{
		FString PrizeStr = FormatCurrency(LaunchData.PrizePool, LaunchData.Currency);
		PrizeText->SetText(FText::FromString(FString::Printf(TEXT("Prize: %s"), *PrizeStr)));
	}
	
	if (DurationText)
	{
		FString DurationStr = FormatDuration(LaunchData.DurationSeconds);
		DurationText->SetText(FText::FromString(FString::Printf(TEXT("Duration: %s"), *DurationStr)));
	}
	
	// Update opponent info if available
	if (LaunchData.HasOpponent())
	{
		if (OpponentNameText)
		{
			OpponentNameText->SetText(FText::FromString(LaunchData.Opponent.Username));
		}
		
		// TODO: Load opponent avatar from URL
		// if (OpponentAvatar) { ... }
	}
	else
	{
		if (OpponentNameText)
		{
			OpponentNameText->SetText(FText::FromString(TEXT("Opponent")));
		}
	}
	
	// Show match info UI
	ShowMatchInfoUI();
}

void UDeskillzMatchmakingWidget_Updated::ShowError(const FString& ErrorMessage)
{
	SetState(EDeskillzWidgetState::Error);
	
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(TEXT("Error")));
	}
	
	if (InfoText)
	{
		InfoText->SetText(FText::FromString(ErrorMessage));
	}
	
	ShowMessageUI();
}

void UDeskillzMatchmakingWidget_Updated::OpenMainApp()
{
	// Try deep link first
	FString DeepLink = MainAppScheme;
	
#if PLATFORM_IOS || PLATFORM_ANDROID
	// Try to open app
	FPlatformProcess::LaunchURL(*DeepLink, nullptr, nullptr);
#else
	// Desktop fallback to web
	FPlatformProcess::LaunchURL(*MainAppURL, nullptr, nullptr);
#endif
}

void UDeskillzMatchmakingWidget_Updated::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	// Apply theme colors to text elements
	// This would use Theme.PrimaryColor, Theme.TextColor, etc.
}

void UDeskillzMatchmakingWidget_Updated::SetState(EDeskillzWidgetState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}
	
	CurrentState = NewState;
	UpdateUIForState();
}

void UDeskillzMatchmakingWidget_Updated::UpdateUIForState()
{
	switch (CurrentState)
	{
		case EDeskillzWidgetState::WaitingForLaunch:
			ShowMessageUI();
			break;
			
		case EDeskillzWidgetState::MatchReceived:
			ShowMatchInfoUI();
			break;
			
		case EDeskillzWidgetState::Starting:
			if (StartMatchButton)
			{
				StartMatchButton->SetIsEnabled(false);
			}
			break;
			
		case EDeskillzWidgetState::Error:
			ShowMessageUI();
			break;
	}
}

void UDeskillzMatchmakingWidget_Updated::ShowMatchInfoUI()
{
	// Show match info container, hide message container
	if (JoinMessageContainer)
	{
		JoinMessageContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (MatchInfoContainer)
	{
		MatchInfoContainer->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (OpenAppButton)
	{
		OpenAppButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (StartMatchButton)
	{
		StartMatchButton->SetVisibility(ESlateVisibility::Visible);
		StartMatchButton->SetIsEnabled(true);
	}
}

void UDeskillzMatchmakingWidget_Updated::ShowMessageUI()
{
	// Show message container, hide match info
	if (JoinMessageContainer)
	{
		JoinMessageContainer->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (MatchInfoContainer)
	{
		MatchInfoContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (OpenAppButton)
	{
		OpenAppButton->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (StartMatchButton)
	{
		StartMatchButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FString UDeskillzMatchmakingWidget_Updated::FormatCurrency(double Amount, EDeskillzCurrency Currency) const
{
	FString Symbol;
	
	switch (Currency)
	{
		case EDeskillzCurrency::BTC:  Symbol = TEXT("BTC"); break;
		case EDeskillzCurrency::ETH:  Symbol = TEXT("ETH"); break;
		case EDeskillzCurrency::SOL:  Symbol = TEXT("SOL"); break;
		case EDeskillzCurrency::XRP:  Symbol = TEXT("XRP"); break;
		case EDeskillzCurrency::BNB:  Symbol = TEXT("BNB"); break;
		case EDeskillzCurrency::USDC: Symbol = TEXT("USDC"); break;
		case EDeskillzCurrency::USDT: 
		default: Symbol = TEXT("USDT"); break;
	}
	
	// Format based on currency type (crypto vs stablecoin)
	if (Currency == EDeskillzCurrency::USDT || Currency == EDeskillzCurrency::USDC)
	{
		return FString::Printf(TEXT("$%.2f %s"), Amount, *Symbol);
	}
	else
	{
		return FString::Printf(TEXT("%.6f %s"), Amount, *Symbol);
	}
}

FString UDeskillzMatchmakingWidget_Updated::FormatDuration(int32 Seconds) const
{
	if (Seconds < 60)
	{
		return FString::Printf(TEXT("%d sec"), Seconds);
	}
	else if (Seconds < 3600)
	{
		int32 Minutes = Seconds / 60;
		int32 RemainingSecs = Seconds % 60;
		if (RemainingSecs > 0)
		{
			return FString::Printf(TEXT("%d min %d sec"), Minutes, RemainingSecs);
		}
		return FString::Printf(TEXT("%d min"), Minutes);
	}
	else
	{
		int32 Hours = Seconds / 3600;
		int32 RemainingMins = (Seconds % 3600) / 60;
		return FString::Printf(TEXT("%d hr %d min"), Hours, RemainingMins);
	}
}

void UDeskillzMatchmakingWidget_Updated::OnOpenAppClicked()
{
	OpenMainApp();
}

void UDeskillzMatchmakingWidget_Updated::OnStartMatchClicked()
{
	SetState(EDeskillzWidgetState::Starting);
	
	// Initialize the bridge with launch data
	UDeskillzBridge* Bridge = UDeskillzBridge::Get();
	if (Bridge)
	{
		Bridge->Initialize(CurrentLaunchData);
		Bridge->ReportMatchStarted();
	}
	
	// Close this widget - game should handle starting gameplay
	RemoveFromParent();
}

void UDeskillzMatchmakingWidget_Updated::OnCloseClicked()
{
	if (CurrentState == EDeskillzWidgetState::MatchReceived)
	{
		// Player is cancelling match - notify backend
		UDeskillzBridge* Bridge = UDeskillzBridge::Get();
		if (Bridge && Bridge->IsInitialized())
		{
			Bridge->AbortMatch(TEXT("Player cancelled"));
		}
	}
	
	RemoveFromParent();
}