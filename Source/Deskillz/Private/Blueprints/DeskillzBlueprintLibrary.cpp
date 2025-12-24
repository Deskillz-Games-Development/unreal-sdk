// Copyright Deskillz Games. All Rights Reserved.

#include "Blueprints/DeskillzBlueprintLibrary.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Helper to get SDK instance safely
static UDeskillzSDK* GetSDK()
{
	// Try to get from game instance
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		return UDeskillzSDK::Get(World->GetFirstPlayerController());
	}
	return nullptr;
}

// ============================================================================
// Quick Start - Most Common Functions
// ============================================================================

void UDeskillzBlueprintLibrary::SubmitScore(int64 Score)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->SubmitScore(Score);
	}
	else
	{
		UE_LOG(LogDeskillz, Warning, TEXT("SubmitScore called but SDK not available"));
	}
}

void UDeskillzBlueprintLibrary::EndMatch()
{
	// Score submission handles match ending
	// This is a convenience function for clarity
	UE_LOG(LogDeskillz, Log, TEXT("EndMatch called - match flow complete"));
}

bool UDeskillzBlueprintLibrary::IsInMatch()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->IsInMatch();
	}
	return false;
}

FDeskillzMatchInfo UDeskillzBlueprintLibrary::GetCurrentMatchInfo()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetCurrentMatch();
	}
	return FDeskillzMatchInfo();
}

float UDeskillzBlueprintLibrary::GetMatchTimeRemaining()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetRemainingTime();
	}
	return 0.0f;
}

// ============================================================================
// Match Flow
// ============================================================================

void UDeskillzBlueprintLibrary::MatchReady()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->StartMatch();
	}
}

void UDeskillzBlueprintLibrary::UpdateScore(int64 CurrentScore)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->UpdateScore(CurrentScore);
	}
}

void UDeskillzBlueprintLibrary::AbortMatch(const FString& Reason)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->AbortMatch(Reason);
	}
}

int64 UDeskillzBlueprintLibrary::GetMatchRandomSeed()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetCurrentMatch().RandomSeed;
	}
	return FDateTime::UtcNow().GetTicks();
}

bool UDeskillzBlueprintLibrary::IsSyncMatch()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetCurrentMatch().IsSynchronous();
	}
	return false;
}

float UDeskillzBlueprintLibrary::GetMatchElapsedTime()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetElapsedTime();
	}
	return 0.0f;
}

// ============================================================================
// Tournaments
// ============================================================================

void UDeskillzBlueprintLibrary::ShowTournaments()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->GetTournaments();
		// TODO: Open tournament UI widget
	}
}

void UDeskillzBlueprintLibrary::JoinTournament(const FString& TournamentId)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->JoinTournament(TournamentId);
	}
}

void UDeskillzBlueprintLibrary::FindMatch(const FString& TournamentId)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->StartMatchmaking(TournamentId);
	}
}

void UDeskillzBlueprintLibrary::CancelMatchmaking()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->CancelMatchmaking();
	}
}

bool UDeskillzBlueprintLibrary::IsMatchmaking()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->IsInMatchmaking();
	}
	return false;
}

// ============================================================================
// Practice Mode
// ============================================================================

void UDeskillzBlueprintLibrary::StartPractice(int32 DurationSeconds)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->StartPractice(DurationSeconds);
	}
}

void UDeskillzBlueprintLibrary::EndPractice()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->EndPractice();
	}
}

bool UDeskillzBlueprintLibrary::IsInPractice()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->IsInPractice();
	}
	return false;
}

// ============================================================================
// Player Info
// ============================================================================

FDeskillzPlayer UDeskillzBlueprintLibrary::GetCurrentPlayer()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetCurrentPlayer();
	}
	return FDeskillzPlayer();
}

FDeskillzPlayer UDeskillzBlueprintLibrary::GetOpponent()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetCurrentMatch().Opponent;
	}
	return FDeskillzPlayer();
}

bool UDeskillzBlueprintLibrary::IsLoggedIn()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->IsAuthenticated();
	}
	return false;
}

void UDeskillzBlueprintLibrary::ShowLogin()
{
	// TODO: Open login UI widget
	UE_LOG(LogDeskillz, Log, TEXT("ShowLogin called - opening login UI"));
}

void UDeskillzBlueprintLibrary::Logout()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->Logout();
	}
}

// ============================================================================
// Wallet
// ============================================================================

double UDeskillzBlueprintLibrary::GetBalance(EDeskillzCurrency Currency)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetBalance(Currency).Amount;
	}
	return 0.0;
}

FString UDeskillzBlueprintLibrary::GetBalanceFormatted(EDeskillzCurrency Currency)
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		FDeskillzWalletBalance Balance = SDK->GetBalance(Currency);
		return Balance.FormattedAmount + TEXT(" ") + Balance.GetCurrencySymbol();
	}
	return TEXT("0.00");
}

bool UDeskillzBlueprintLibrary::HasEnoughFunds(EDeskillzCurrency Currency, double Amount)
{
	return GetBalance(Currency) >= Amount;
}

void UDeskillzBlueprintLibrary::ShowWallet()
{
	// TODO: Open wallet UI widget
	UE_LOG(LogDeskillz, Log, TEXT("ShowWallet called - opening wallet UI"));
}

// ============================================================================
// Leaderboards
// ============================================================================

void UDeskillzBlueprintLibrary::ShowLeaderboard()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->GetGlobalLeaderboard();
		// TODO: Open leaderboard UI widget
	}
}

int32 UDeskillzBlueprintLibrary::GetPlayerRank()
{
	// TODO: Get from cached leaderboard data
	return 0;
}

// ============================================================================
// SDK State
// ============================================================================

bool UDeskillzBlueprintLibrary::IsSDKReady()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->IsReady();
	}
	return false;
}

EDeskillzSDKState UDeskillzBlueprintLibrary::GetSDKState()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetSDKState();
	}
	return EDeskillzSDKState::Uninitialized;
}

FString UDeskillzBlueprintLibrary::GetSDKVersion()
{
	return UDeskillzSDK::GetSDKVersion();
}

EDeskillzEnvironment UDeskillzBlueprintLibrary::GetEnvironment()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		return SDK->GetEnvironment();
	}
	
	// Return from config if SDK not initialized
	if (const UDeskillzConfig* Config = UDeskillzConfig::Get())
	{
		return Config->Environment;
	}
	
	return EDeskillzEnvironment::Sandbox;
}

void UDeskillzBlueprintLibrary::InitializeSDK()
{
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->InitializeSDK();
	}
}

// ============================================================================
// UI Helpers
// ============================================================================

void UDeskillzBlueprintLibrary::ShowResults()
{
	// TODO: Open results UI widget
	UE_LOG(LogDeskillz, Log, TEXT("ShowResults called - opening results UI"));
}

void UDeskillzBlueprintLibrary::ShowPopup(const FString& Title, const FString& Message)
{
	// TODO: Show popup widget
	UE_LOG(LogDeskillz, Log, TEXT("ShowPopup: %s - %s"), *Title, *Message);
}

void UDeskillzBlueprintLibrary::HideAllUI()
{
	// TODO: Hide all Deskillz UI widgets
	UE_LOG(LogDeskillz, Log, TEXT("HideAllUI called"));
}
