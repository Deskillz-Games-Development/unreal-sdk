// Copyright Deskillz Games. All Rights Reserved.

#include "Widgets/Rooms/DeskillzRoomListWidget.h"
#include "Rooms/DeskillzRooms.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/Throbber.h"

// =============================================================================
// UDeskillzRoomListWidget
// =============================================================================

UDeskillzRoomListWidget::UDeskillzRoomListWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentSortOption(ERoomSortOption::Newest)
	, bIsLoading(false)
	, bIsVisible(false)
{
}

void UDeskillzRoomListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeUI();
}

void UDeskillzRoomListWidget::NativeDestruct()
{
	ClearRoomCards();
	Super::NativeDestruct();
}

void UDeskillzRoomListWidget::InitializeUI()
{
	// Bind button clicks
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDeskillzRoomListWidget::OnBackButtonClicked);
	}
	
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UDeskillzRoomListWidget::OnRefreshButtonClicked);
	}
	
	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.AddDynamic(this, &UDeskillzRoomListWidget::OnCreateRoomButtonClicked);
	}
	
	if (JoinCodeButton)
	{
		JoinCodeButton->OnClicked.AddDynamic(this, &UDeskillzRoomListWidget::OnJoinCodeButtonClicked);
	}
	
	// Bind search input
	if (SearchInput)
	{
		SearchInput->OnTextChanged.AddDynamic(this, &UDeskillzRoomListWidget::OnSearchTextChanged);
	}
	
	// Setup sort dropdown
	if (SortDropdown)
	{
		SortDropdown->ClearOptions();
		SortDropdown->AddOption(TEXT("Newest First"));
		SortDropdown->AddOption(TEXT("Entry Fee: Low to High"));
		SortDropdown->AddOption(TEXT("Entry Fee: High to Low"));
		SortDropdown->AddOption(TEXT("Players: Low to High"));
		SortDropdown->AddOption(TEXT("Players: High to Low"));
		SortDropdown->SetSelectedIndex(0);
		SortDropdown->OnSelectionChanged.AddDynamic(this, &UDeskillzRoomListWidget::OnSortSelectionChanged);
	}
	
	// Initially hide loading and empty state
	SetLoading(false);
	ShowEmptyState(false);
}

// =============================================================================
// Public Methods
// =============================================================================

void UDeskillzRoomListWidget::RefreshRooms()
{
	if (bIsLoading)
	{
		return;
	}
	
	SetLoading(true);
	
	UDeskillzRooms* RoomSystem = UDeskillzRooms::Get(this);
	if (!RoomSystem)
	{
		SetLoading(false);
		ShowEmptyState(true);
		return;
	}
	
	FOnRoomListSuccess OnSuccess;
	OnSuccess.BindLambda([this](const TArray<FPrivateRoom>& RoomList)
	{
		Rooms = RoomList;
		ApplyFiltersAndSort();
		SetLoading(false);
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([this](const FRoomError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomListWidget] Failed to load rooms: %s"), *Error.Message);
		Rooms.Empty();
		UpdateRoomCards(Rooms);
		SetLoading(false);
	});
	
	RoomSystem->GetPublicRooms(OnSuccess, OnError);
}

void UDeskillzRoomListWidget::Show()
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);
	RefreshRooms();
}

void UDeskillzRoomListWidget::Hide()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

// =============================================================================
// Filtering and Sorting
// =============================================================================

void UDeskillzRoomListWidget::ApplyFiltersAndSort()
{
	TArray<FPrivateRoom> FilteredRooms = Rooms;
	
	// Apply search filter
	if (!SearchFilter.IsEmpty())
	{
		FilteredRooms = FilteredRooms.FilterByPredicate([this](const FPrivateRoom& Room)
		{
			return Room.Name.Contains(SearchFilter, ESearchCase::IgnoreCase) ||
			       Room.RoomCode.Contains(SearchFilter, ESearchCase::IgnoreCase) ||
			       Room.Host.Username.Contains(SearchFilter, ESearchCase::IgnoreCase);
		});
	}
	
	// Apply sort
	switch (CurrentSortOption)
	{
	case ERoomSortOption::Newest:
		FilteredRooms.Sort([](const FPrivateRoom& A, const FPrivateRoom& B)
		{
			return A.CreatedAt > B.CreatedAt;
		});
		break;
		
	case ERoomSortOption::EntryFeeAsc:
		FilteredRooms.Sort([](const FPrivateRoom& A, const FPrivateRoom& B)
		{
			return A.EntryFee < B.EntryFee;
		});
		break;
		
	case ERoomSortOption::EntryFeeDesc:
		FilteredRooms.Sort([](const FPrivateRoom& A, const FPrivateRoom& B)
		{
			return A.EntryFee > B.EntryFee;
		});
		break;
		
	case ERoomSortOption::PlayersAsc:
		FilteredRooms.Sort([](const FPrivateRoom& A, const FPrivateRoom& B)
		{
			return A.CurrentPlayers < B.CurrentPlayers;
		});
		break;
		
	case ERoomSortOption::PlayersDesc:
		FilteredRooms.Sort([](const FPrivateRoom& A, const FPrivateRoom& B)
		{
			return A.CurrentPlayers > B.CurrentPlayers;
		});
		break;
	}
	
	UpdateRoomCards(FilteredRooms);
}

void UDeskillzRoomListWidget::UpdateRoomCards(const TArray<FPrivateRoom>& FilteredRooms)
{
	ClearRoomCards();
	
	// Show empty state if no rooms
	ShowEmptyState(FilteredRooms.Num() == 0);
	
	if (!RoomListContent)
	{
		return;
	}
	
	// Create cards for each room
	for (const FPrivateRoom& Room : FilteredRooms)
	{
		UDeskillzRoomCardWidget* Card = CreateRoomCard(Room);
		if (Card)
		{
			RoomCards.Add(Card);
			RoomListContent->AddChild(Card);
		}
	}
}

UDeskillzRoomCardWidget* UDeskillzRoomListWidget::CreateRoomCard(const FPrivateRoom& Room)
{
	if (!RoomCardClass)
	{
		// Use default class if not specified
		RoomCardClass = UDeskillzRoomCardWidget::StaticClass();
	}
	
	UDeskillzRoomCardWidget* Card = CreateWidget<UDeskillzRoomCardWidget>(this, RoomCardClass);
	if (Card)
	{
		Card->SetRoom(Room);
		Card->OnJoinClicked.AddDynamic(this, &UDeskillzRoomListWidget::OnRoomCardJoinClicked);
	}
	
	return Card;
}

void UDeskillzRoomListWidget::ClearRoomCards()
{
	for (UDeskillzRoomCardWidget* Card : RoomCards)
	{
		if (Card)
		{
			Card->RemoveFromParent();
		}
	}
	RoomCards.Empty();
}

void UDeskillzRoomListWidget::SetLoading(bool bLoading)
{
	bIsLoading = bLoading;
	
	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(!bLoading);
	}
	
	if (RoomListScrollBox)
	{
		RoomListScrollBox->SetVisibility(bLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UDeskillzRoomListWidget::ShowEmptyState(bool bShow)
{
	if (EmptyStateText)
	{
		EmptyStateText->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

// =============================================================================
// UI Callbacks
// =============================================================================

void UDeskillzRoomListWidget::OnBackButtonClicked()
{
	OnBackClicked.Broadcast();
}

void UDeskillzRoomListWidget::OnRefreshButtonClicked()
{
	RefreshRooms();
}

void UDeskillzRoomListWidget::OnSearchTextChanged(const FText& Text)
{
	SearchFilter = Text.ToString();
	ApplyFiltersAndSort();
}

void UDeskillzRoomListWidget::OnSortSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	int32 Index = SortDropdown ? SortDropdown->GetSelectedIndex() : 0;
	CurrentSortOption = static_cast<ERoomSortOption>(Index);
	ApplyFiltersAndSort();
}

void UDeskillzRoomListWidget::OnCreateRoomButtonClicked()
{
	OnCreateRoomClicked.Broadcast();
}

void UDeskillzRoomListWidget::OnJoinCodeButtonClicked()
{
	OnJoinByCodeClicked.Broadcast();
}

void UDeskillzRoomListWidget::OnRoomCardJoinClicked(const FPrivateRoom& Room)
{
	OnRoomSelected.Broadcast(Room);
}

// =============================================================================
// UDeskillzRoomCardWidget
// =============================================================================

UDeskillzRoomCardWidget::UDeskillzRoomCardWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzRoomCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UDeskillzRoomCardWidget::OnJoinButtonClicked);
	}
}

void UDeskillzRoomCardWidget::SetRoom(const FPrivateRoom& Room)
{
	CurrentRoom = Room;
	UpdateDisplay();
}

void UDeskillzRoomCardWidget::UpdateDisplay()
{
	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(CurrentRoom.Name));
	}
	
	if (RoomCodeText)
	{
		RoomCodeText->SetText(FText::FromString(CurrentRoom.RoomCode));
	}
	
	if (HostText)
	{
		HostText->SetText(FText::FromString(FString::Printf(TEXT("Host: %s"), *CurrentRoom.Host.Username)));
	}
	
	if (PlayersText)
	{
		PlayersText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d players"), 
			CurrentRoom.CurrentPlayers, CurrentRoom.MaxPlayers)));
	}
	
	if (EntryFeeText)
	{
		EntryFeeText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f %s"), 
			CurrentRoom.EntryFee, *CurrentRoom.EntryCurrency)));
	}
	
	if (JoinButton)
	{
		JoinButton->SetIsEnabled(CurrentRoom.CanJoin());
	}
}

void UDeskillzRoomCardWidget::OnJoinButtonClicked()
{
	OnJoinClicked.Broadcast(CurrentRoom);
}