// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzLeaderboardWidget.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/ComboBoxString.h"

UDeskillzLeaderboardWidget::UDeskillzLeaderboardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzLeaderboardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup period dropdown
	if (PeriodComboBox)
	{
		PeriodComboBox->ClearOptions();
		PeriodComboBox->AddOption(TEXT("Today"));
		PeriodComboBox->AddOption(TEXT("This Week"));
		PeriodComboBox->AddOption(TEXT("This Month"));
		PeriodComboBox->AddOption(TEXT("All Time"));
		PeriodComboBox->SetSelectedIndex(3); // All Time default
		PeriodComboBox->OnSelectionChanged.AddDynamic(this, &UDeskillzLeaderboardWidget::OnPeriodChanged);
	}
	
	// Setup buttons
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDeskillzLeaderboardWidget::OnBackClicked);
	}
	
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UDeskillzLeaderboardWidget::OnRefreshClicked);
	}
	
	// Set title
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Leaderboard")));
	}
}

void UDeskillzLeaderboardWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	if (TitleText)
	{
		TitleText->SetColorAndOpacity(Theme.TextColor);
	}
	
	if (EmptyStateText)
	{
		EmptyStateText->SetColorAndOpacity(Theme.MutedTextColor);
	}
}

bool UDeskillzLeaderboardWidget::HandleBackButton_Implementation()
{
	if (UIManager)
	{
		UIManager->HideLeaderboard();
	}
	return true;
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzLeaderboardWidget::SetTournamentId(const FString& TournamentId)
{
	CurrentTournamentId = TournamentId;
	
	// Update title
	if (TitleText)
	{
		FString Title = TournamentId.IsEmpty() ? TEXT("Global Leaderboard") : TEXT("Tournament Leaderboard");
		TitleText->SetText(FText::FromString(Title));
	}
}

void UDeskillzLeaderboardWidget::RefreshLeaderboard()
{
	if (bIsLoading)
	{
		return;
	}
	
	SetLoading(true);
	
	UDeskillzSDK* SDK = UDeskillzSDK::Get();
	if (!SDK)
	{
		SetLoading(false);
		return;
	}
	
	// In full implementation, call SDK to get leaderboard
	// SDK->GetLeaderboard(CurrentTournamentId, CurrentPeriod, ...);
	
	// Simulate test data
	TArray<FDeskillzLeaderboardEntry> TestEntries;
	
	// Generate sample leaderboard
	TArray<FString> SampleNames = {
		TEXT("ProGamer99"), TEXT("NightHawk"), TEXT("SkillMaster"), TEXT("CryptoKing"),
		TEXT("LuckyShot"), TEXT("VictoryRoad"), TEXT("ChampionX"), TEXT("GameWizard"),
		TEXT("TopPlayer"), TEXT("EliteGamer"), TEXT("StarPlayer"), TEXT("MegaWinner")
	};
	
	for (int32 i = 0; i < 20; i++)
	{
		FDeskillzLeaderboardEntry Entry;
		Entry.Rank = i + 1;
		Entry.PlayerId = FString::Printf(TEXT("player_%d"), i);
		Entry.Username = SampleNames[i % SampleNames.Num()];
		Entry.Score = 10000 - (i * 350) + FMath::RandRange(-50, 50);
		Entry.Wins = FMath::RandRange(10, 100 - i * 3);
		Entry.TotalMatches = Entry.Wins + FMath::RandRange(5, 30);
		Entry.WinRate = Entry.TotalMatches > 0 ? (float)Entry.Wins / Entry.TotalMatches * 100.0f : 0.0f;
		Entry.Earnings = (20 - i) * 0.05 + FMath::FRandRange(0.0, 0.1);
		Entry.bIsCurrentPlayer = (i == 7); // Simulate current player at rank 8
		
		TestEntries.Add(Entry);
		
		if (Entry.bIsCurrentPlayer)
		{
			CurrentPlayerEntry = Entry;
		}
	}
	
	OnLeaderboardLoaded(true, TestEntries);
}

void UDeskillzLeaderboardWidget::SetPeriod(EDeskillzLeaderboardPeriod Period)
{
	if (CurrentPeriod != Period)
	{
		CurrentPeriod = Period;
		
		if (PeriodComboBox)
		{
			PeriodComboBox->SetSelectedIndex(static_cast<int32>(Period));
		}
		
		RefreshLeaderboard();
	}
}

int32 UDeskillzLeaderboardWidget::GetCurrentPlayerRank() const
{
	return CurrentPlayerEntry.Rank;
}

void UDeskillzLeaderboardWidget::ScrollToCurrentPlayer()
{
	if (!LeaderboardScrollBox)
	{
		return;
	}
	
	// Find current player index
	int32 PlayerIndex = -1;
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		if (Entries[i].bIsCurrentPlayer)
		{
			PlayerIndex = i;
			break;
		}
	}
	
	if (PlayerIndex >= 0)
	{
		// Scroll to position (approximate)
		float ScrollOffset = PlayerIndex * 60.0f; // Assume ~60px per row
		LeaderboardScrollBox->SetScrollOffset(ScrollOffset);
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzLeaderboardWidget::PopulateLeaderboard()
{
	if (!EntriesContainer)
	{
		return;
	}
	
	EntriesContainer->ClearChildren();
	
	// Update top 3 if enabled
	if (bShowTopThreeSeparately)
	{
		UpdateTopThree();
	}
	
	// Determine starting index
	int32 StartIndex = bShowTopThreeSeparately ? 3 : 0;
	
	// Create rows for remaining entries
	int32 Count = FMath::Min(Entries.Num(), MaxEntries);
	for (int32 i = StartIndex; i < Count; i++)
	{
		UUserWidget* Row = CreateEntryRow(Entries[i]);
		if (Row)
		{
			EntriesContainer->AddChildToVerticalBox(Row);
		}
	}
	
	// Show/hide empty state
	if (EmptyStateText)
	{
		EmptyStateText->SetVisibility(Entries.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	// Update current player display
	UpdateCurrentPlayerDisplay();
	
	UE_LOG(LogDeskillz, Log, TEXT("Populated leaderboard with %d entries"), Entries.Num());
}

UUserWidget* UDeskillzLeaderboardWidget::CreateEntryRow(const FDeskillzLeaderboardEntry& Entry)
{
	// In full implementation, create from EntryRowClass
	// For now, return nullptr (use Blueprint implementation)
	return nullptr;
}

void UDeskillzLeaderboardWidget::UpdateTopThree()
{
	if (!TopThreeContainer)
	{
		return;
	}
	
	TopThreeContainer->SetVisibility(Entries.Num() >= 3 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
	// In full implementation, update FirstPlaceWidget, SecondPlaceWidget, ThirdPlaceWidget
	// with corresponding entry data and special styling (gold, silver, bronze)
}

void UDeskillzLeaderboardWidget::UpdateCurrentPlayerDisplay()
{
	if (CurrentPlayerRankText && CurrentPlayerEntry.Rank > 0)
	{
		FString RankStr = FString::Printf(TEXT("Your Rank: #%d%s"), 
			CurrentPlayerEntry.Rank, 
			*GetRankSuffix(CurrentPlayerEntry.Rank));
		CurrentPlayerRankText->SetText(FText::FromString(RankStr));
		CurrentPlayerRankText->SetColorAndOpacity(CurrentTheme.PrimaryColor);
	}
}

void UDeskillzLeaderboardWidget::SetLoading(bool bLoading)
{
	bIsLoading = bLoading;
	
	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (LeaderboardScrollBox)
	{
		LeaderboardScrollBox->SetVisibility(bLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	
	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(!bLoading);
	}
}

void UDeskillzLeaderboardWidget::OnLeaderboardLoaded(bool bSuccess, const TArray<FDeskillzLeaderboardEntry>& LoadedEntries)
{
	SetLoading(false);
	
	if (bSuccess)
	{
		Entries = LoadedEntries;
		PopulateLeaderboard();
		
		UE_LOG(LogDeskillz, Log, TEXT("Leaderboard loaded: %d entries"), Entries.Num());
	}
	else
	{
		UE_LOG(LogDeskillz, Error, TEXT("Failed to load leaderboard"));
		
		if (UIManager)
		{
			UIManager->ShowToast(TEXT("Failed to load leaderboard"), 3.0f, true);
		}
	}
}

void UDeskillzLeaderboardWidget::OnPeriodChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (PeriodComboBox)
	{
		int32 Index = PeriodComboBox->GetSelectedIndex();
		CurrentPeriod = static_cast<EDeskillzLeaderboardPeriod>(Index);
		RefreshLeaderboard();
	}
}

void UDeskillzLeaderboardWidget::OnEntryClicked(const FDeskillzLeaderboardEntry& Entry)
{
	PlayClickSound();
	OnEntrySelected.Broadcast(Entry);
	
	UE_LOG(LogDeskillz, Log, TEXT("Leaderboard entry selected: %s (Rank %d)"), *Entry.Username, Entry.Rank);
}

void UDeskillzLeaderboardWidget::OnBackClicked()
{
	PlayClickSound();
	HandleBackButton();
}

void UDeskillzLeaderboardWidget::OnRefreshClicked()
{
	PlayClickSound();
	RefreshLeaderboard();
}

FString UDeskillzLeaderboardWidget::GetRankSuffix(int32 Rank)
{
	if (Rank % 100 >= 11 && Rank % 100 <= 13)
	{
		return TEXT("th");
	}
	
	switch (Rank % 10)
	{
		case 1: return TEXT("st");
		case 2: return TEXT("nd");
		case 3: return TEXT("rd");
		default: return TEXT("th");
	}
}

FLinearColor UDeskillzLeaderboardWidget::GetRankColor(int32 Rank) const
{
	switch (Rank)
	{
		case 1: return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);  // Gold
		case 2: return FLinearColor(0.75f, 0.75f, 0.75f, 1.0f); // Silver
		case 3: return FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);   // Bronze
		default: return CurrentTheme.TextColor;
	}
}
