// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzTournamentListWidget.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ComboBoxString.h"
#include "TimerManager.h"
#include "Engine/World.h"

UDeskillzTournamentListWidget::UDeskillzTournamentListWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzTournamentListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup filter dropdown
	if (FilterComboBox)
	{
		FilterComboBox->ClearOptions();
		FilterComboBox->AddOption(TEXT("All Tournaments"));
		FilterComboBox->AddOption(TEXT("Free Entry"));
		FilterComboBox->AddOption(TEXT("Paid Entry"));
		FilterComboBox->AddOption(TEXT("Featured"));
		FilterComboBox->AddOption(TEXT("Ending Soon"));
		FilterComboBox->SetSelectedIndex(0);
		FilterComboBox->OnSelectionChanged.AddDynamic(this, &UDeskillzTournamentListWidget::OnFilterChanged);
	}
	
	// Setup sort dropdown
	if (SortComboBox)
	{
		SortComboBox->ClearOptions();
		SortComboBox->AddOption(TEXT("Featured"));
		SortComboBox->AddOption(TEXT("Prize: High to Low"));
		SortComboBox->AddOption(TEXT("Prize: Low to High"));
		SortComboBox->AddOption(TEXT("Entry: Low to High"));
		SortComboBox->AddOption(TEXT("Entry: High to Low"));
		SortComboBox->AddOption(TEXT("Most Players"));
		SortComboBox->AddOption(TEXT("Ending Soon"));
		SortComboBox->SetSelectedIndex(0);
		SortComboBox->OnSelectionChanged.AddDynamic(this, &UDeskillzTournamentListWidget::OnSortChanged);
	}
	
	// Setup buttons
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UDeskillzTournamentListWidget::OnRefreshClicked);
	}
	
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDeskillzTournamentListWidget::OnBackClicked);
	}
	
	// Set title
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Tournaments")));
	}
	
	// Initial balance update
	UpdateBalanceDisplay();
	
	// Start auto-refresh
	if (AutoRefreshInterval > 0.0f)
	{
		StartAutoRefresh();
	}
}

void UDeskillzTournamentListWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	// Apply theme colors to UI elements
	if (TitleText)
	{
		TitleText->SetColorAndOpacity(Theme.TextColor);
	}
	
	if (BalanceText)
	{
		BalanceText->SetColorAndOpacity(Theme.PrimaryColor);
	}
	
	if (EmptyStateText)
	{
		EmptyStateText->SetColorAndOpacity(Theme.MutedTextColor);
	}
}

bool UDeskillzTournamentListWidget::HandleBackButton_Implementation()
{
	StopAutoRefresh();
	
	if (UIManager)
	{
		UIManager->HideTournamentList();
	}
	
	return true;
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzTournamentListWidget::RefreshTournaments()
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
	
	// Request tournaments from SDK
	// In full implementation, this would call the actual API
	// SDK->GetTournaments(FOnTournamentsLoaded::CreateUObject(this, &UDeskillzTournamentListWidget::OnTournamentsLoaded));
	
	// For now, simulate with test data
	TArray<FDeskillzTournament> TestTournaments;
	
	// Create sample tournaments
	for (int32 i = 0; i < 10; i++)
	{
		FDeskillzTournament Tournament;
		Tournament.Id = FString::Printf(TEXT("tournament_%d"), i);
		Tournament.Name = FString::Printf(TEXT("Championship %d"), i + 1);
		Tournament.Description = TEXT("Compete for the top prize!");
		Tournament.GameId = TEXT("game_001");
		Tournament.EntryFee = i == 0 ? 0.0 : (i * 0.001);
		Tournament.EntryCurrency = TEXT("ETH");
		Tournament.PrizePool = i * 0.01 + 0.05;
		Tournament.PrizeCurrency = TEXT("ETH");
		Tournament.MaxPlayers = 100 + (i * 50);
		Tournament.CurrentPlayers = FMath::RandRange(10, Tournament.MaxPlayers - 10);
		Tournament.Status = EDeskillzTournamentStatus::Active;
		Tournament.bIsFeatured = (i == 0 || i == 3);
		Tournament.StartTimeMs = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
		Tournament.EndTimeMs = Tournament.StartTimeMs + (86400000 * (i + 1)); // 1-10 days
		
		TestTournaments.Add(Tournament);
	}
	
	OnTournamentsLoaded(true, TestTournaments);
}

void UDeskillzTournamentListWidget::SetFilter(EDeskillzTournamentFilter Filter)
{
	if (CurrentFilter != Filter)
	{
		CurrentFilter = Filter;
		
		if (FilterComboBox)
		{
			FilterComboBox->SetSelectedIndex(static_cast<int32>(Filter));
		}
		
		PopulateTournamentList();
	}
}

void UDeskillzTournamentListWidget::SetSort(EDeskillzTournamentSort Sort)
{
	if (CurrentSort != Sort)
	{
		CurrentSort = Sort;
		
		if (SortComboBox)
		{
			SortComboBox->SetSelectedIndex(static_cast<int32>(Sort));
		}
		
		PopulateTournamentList();
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzTournamentListWidget::PopulateTournamentList()
{
	if (!TournamentContainer)
	{
		return;
	}
	
	// Clear existing
	TournamentContainer->ClearChildren();
	
	// Get filtered/sorted list
	TArray<FDeskillzTournament> FilteredTournaments = GetFilteredTournaments();
	
	// Show empty state if no tournaments
	if (EmptyStateText)
	{
		EmptyStateText->SetVisibility(FilteredTournaments.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	// Create cards for each tournament
	int32 Count = FMath::Min(FilteredTournaments.Num(), MaxTournamentsToShow);
	for (int32 i = 0; i < Count; i++)
	{
		UUserWidget* Card = CreateTournamentCard(FilteredTournaments[i]);
		if (Card)
		{
			TournamentContainer->AddChildToVerticalBox(Card);
		}
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Populated tournament list with %d tournaments"), Count);
}

UUserWidget* UDeskillzTournamentListWidget::CreateTournamentCard(const FDeskillzTournament& Tournament)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return nullptr;
	}
	
	// Create card widget
	// In full implementation, this would use TournamentCardClass
	// For now, create programmatically or return nullptr to use Blueprint implementation
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Created card for tournament: %s"), *Tournament.Name);
	
	return nullptr; // Override in Blueprint or implement custom card class
}

TArray<FDeskillzTournament> UDeskillzTournamentListWidget::GetFilteredTournaments() const
{
	TArray<FDeskillzTournament> Result;
	
	// Apply filter
	for (const FDeskillzTournament& Tournament : Tournaments)
	{
		bool bPassFilter = false;
		
		switch (CurrentFilter)
		{
			case EDeskillzTournamentFilter::All:
				bPassFilter = true;
				break;
			case EDeskillzTournamentFilter::Free:
				bPassFilter = Tournament.EntryFee <= 0.0;
				break;
			case EDeskillzTournamentFilter::Paid:
				bPassFilter = Tournament.EntryFee > 0.0;
				break;
			case EDeskillzTournamentFilter::Featured:
				bPassFilter = Tournament.bIsFeatured;
				break;
			case EDeskillzTournamentFilter::Ending_Soon:
				{
					int64 NowMs = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
					int64 TimeLeftMs = Tournament.EndTimeMs - NowMs;
					bPassFilter = TimeLeftMs > 0 && TimeLeftMs < 86400000; // Less than 24 hours
				}
				break;
		}
		
		if (bPassFilter)
		{
			Result.Add(Tournament);
		}
	}
	
	// Apply sort
	Result.Sort([this](const FDeskillzTournament& A, const FDeskillzTournament& B)
	{
		switch (CurrentSort)
		{
			case EDeskillzTournamentSort::Featured:
				if (A.bIsFeatured != B.bIsFeatured)
				{
					return A.bIsFeatured;
				}
				return A.PrizePool > B.PrizePool;
				
			case EDeskillzTournamentSort::Prize_High:
				return A.PrizePool > B.PrizePool;
				
			case EDeskillzTournamentSort::Prize_Low:
				return A.PrizePool < B.PrizePool;
				
			case EDeskillzTournamentSort::Entry_Low:
				return A.EntryFee < B.EntryFee;
				
			case EDeskillzTournamentSort::Entry_High:
				return A.EntryFee > B.EntryFee;
				
			case EDeskillzTournamentSort::Players:
				return A.CurrentPlayers > B.CurrentPlayers;
				
			case EDeskillzTournamentSort::Ending_Soon:
				return A.EndTimeMs < B.EndTimeMs;
				
			default:
				return false;
		}
	});
	
	return Result;
}

void UDeskillzTournamentListWidget::OnTournamentCardClicked(const FString& TournamentId)
{
	SelectedTournamentId = TournamentId;
	OnTournamentSelected.Broadcast(TournamentId);
	
	PlayClickSound();
	
	UE_LOG(LogDeskillz, Log, TEXT("Tournament selected: %s"), *TournamentId);
}

void UDeskillzTournamentListWidget::OnEnterTournament(const FString& TournamentId)
{
	PlayClickSound();
	
	// Find tournament
	const FDeskillzTournament* Tournament = Tournaments.FindByPredicate([&TournamentId](const FDeskillzTournament& T)
	{
		return T.Id == TournamentId;
	});
	
	if (!Tournament)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Tournament not found: %s"), *TournamentId);
		return;
	}
	
	// Show matchmaking
	if (UIManager)
	{
		UIManager->ShowMatchmaking(TournamentId);
	}
}

void UDeskillzTournamentListWidget::OnFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (FilterComboBox)
	{
		int32 Index = FilterComboBox->GetSelectedIndex();
		CurrentFilter = static_cast<EDeskillzTournamentFilter>(Index);
		PopulateTournamentList();
	}
}

void UDeskillzTournamentListWidget::OnSortChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SortComboBox)
	{
		int32 Index = SortComboBox->GetSelectedIndex();
		CurrentSort = static_cast<EDeskillzTournamentSort>(Index);
		PopulateTournamentList();
	}
}

void UDeskillzTournamentListWidget::OnRefreshClicked()
{
	PlayClickSound();
	RefreshTournaments();
}

void UDeskillzTournamentListWidget::OnBackClicked()
{
	PlayClickSound();
	HandleBackButton();
}

void UDeskillzTournamentListWidget::OnTournamentsLoaded(bool bSuccess, const TArray<FDeskillzTournament>& LoadedTournaments)
{
	SetLoading(false);
	
	if (bSuccess)
	{
		Tournaments = LoadedTournaments;
		PopulateTournamentList();
		
		UE_LOG(LogDeskillz, Log, TEXT("Loaded %d tournaments"), Tournaments.Num());
	}
	else
	{
		UE_LOG(LogDeskillz, Error, TEXT("Failed to load tournaments"));
		
		if (UIManager)
		{
			UIManager->ShowToast(TEXT("Failed to load tournaments"), 3.0f, true);
		}
	}
}

void UDeskillzTournamentListWidget::SetLoading(bool bLoading)
{
	bIsLoading = bLoading;
	
	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (TournamentScrollBox)
	{
		TournamentScrollBox->SetVisibility(bLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	
	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(!bLoading);
	}
}

void UDeskillzTournamentListWidget::UpdateBalanceDisplay()
{
	if (!BalanceText)
	{
		return;
	}
	
	// In full implementation, get from wallet
	// For now, show placeholder
	BalanceText->SetText(FText::FromString(TEXT("0.0000 ETH")));
}

void UDeskillzTournamentListWidget::StartAutoRefresh()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoRefreshTimerHandle,
			this,
			&UDeskillzTournamentListWidget::RefreshTournaments,
			AutoRefreshInterval,
			true // Looping
		);
	}
}

void UDeskillzTournamentListWidget::StopAutoRefresh()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoRefreshTimerHandle);
	}
}
