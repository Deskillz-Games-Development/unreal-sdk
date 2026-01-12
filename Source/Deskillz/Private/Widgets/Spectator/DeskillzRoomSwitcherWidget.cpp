// =============================================================================
// Deskillz SDK for Unreal Engine - Room Switcher Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Spectator/DeskillzRoomSwitcherWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"

void UDeskillzRoomSwitcherWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind navigation buttons
    if (PreviousRoomButton)
    {
        PreviousRoomButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnPreviousRoomClicked);
    }
    if (NextRoomButton)
    {
        NextRoomButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnNextRoomClicked);
    }
    if (ExpandListButton)
    {
        ExpandListButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnExpandListClicked);
    }
    if (CloseListButton)
    {
        CloseListButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnCloseListClicked);
    }
    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnRefreshClicked);
    }

    // Bind filter buttons
    if (FilterAllButton)
    {
        FilterAllButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnFilterAllClicked);
    }
    if (FilterActiveButton)
    {
        FilterActiveButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnFilterActiveClicked);
    }
    if (FilterHighStakesButton)
    {
        FilterHighStakesButton->OnClicked.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnFilterHighStakesClicked);
    }

    // Bind search
    if (SearchInput)
    {
        SearchInput->OnTextChanged.AddDynamic(this, &UDeskillzRoomSwitcherWidget::OnSearchTextChanged);
    }

    // Initialize state
    SetExpanded(false);
}

void UDeskillzRoomSwitcherWidget::SetAvailableRooms(const TArray<FDeskillzSwitcherRoomInfo>& InRooms)
{
    AllRooms = InRooms;
    ApplyCurrentFilter();
}

void UDeskillzRoomSwitcherWidget::SetCurrentRoom(const FString& RoomId)
{
    CurrentRoomId = RoomId;
    
    // Find current room index
    CurrentRoomIndex = -1;
    for (int32 i = 0; i < FilteredRooms.Num(); ++i)
    {
        if (FilteredRooms[i].RoomId == RoomId)
        {
            CurrentRoomIndex = i;
            break;
        }
    }

    UpdateCurrentRoomDisplay();
    UpdateNavigationButtons();
    RebuildRoomList();
}

void UDeskillzRoomSwitcherWidget::SetFilter(EDeskillzRoomFilter Filter)
{
    CurrentFilter = Filter;
    UpdateFilterButtonStates();
    ApplyCurrentFilter();
}

void UDeskillzRoomSwitcherWidget::SetExpanded(bool bExpand)
{
    bIsExpanded = bExpand;

    if (ExpandedListContainer)
    {
        ExpandedListContainer->SetVisibility(bExpand ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (CompactContainer)
    {
        CompactContainer->SetVisibility(bExpand ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
    if (ExpandListButton)
    {
        ExpandListButton->SetVisibility(bExpand ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
    if (CloseListButton)
    {
        CloseListButton->SetVisibility(bExpand ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UDeskillzRoomSwitcherWidget::RefreshRooms()
{
    OnRefreshRequested.Broadcast();
}

void UDeskillzRoomSwitcherWidget::OnPreviousRoomClicked()
{
    if (CurrentRoomIndex > 0)
    {
        SwitchToRoom(FilteredRooms[CurrentRoomIndex - 1].RoomId);
    }
}

void UDeskillzRoomSwitcherWidget::OnNextRoomClicked()
{
    if (CurrentRoomIndex < FilteredRooms.Num() - 1)
    {
        SwitchToRoom(FilteredRooms[CurrentRoomIndex + 1].RoomId);
    }
}

void UDeskillzRoomSwitcherWidget::OnExpandListClicked()
{
    SetExpanded(true);
}

void UDeskillzRoomSwitcherWidget::OnCloseListClicked()
{
    SetExpanded(false);
}

void UDeskillzRoomSwitcherWidget::OnRefreshClicked()
{
    RefreshRooms();
}

void UDeskillzRoomSwitcherWidget::OnFilterAllClicked()
{
    SetFilter(EDeskillzRoomFilter::All);
}

void UDeskillzRoomSwitcherWidget::OnFilterActiveClicked()
{
    SetFilter(EDeskillzRoomFilter::Active);
}

void UDeskillzRoomSwitcherWidget::OnFilterHighStakesClicked()
{
    SetFilter(EDeskillzRoomFilter::HighStakes);
}

void UDeskillzRoomSwitcherWidget::OnSearchTextChanged(const FText& Text)
{
    SearchQuery = Text.ToString();
    ApplyCurrentFilter();
}

void UDeskillzRoomSwitcherWidget::OnRoomItemClicked(const FString& RoomId)
{
    SwitchToRoom(RoomId);
    SetExpanded(false);
}

void UDeskillzRoomSwitcherWidget::ApplyCurrentFilter()
{
    FilteredRooms.Empty();

    for (const FDeskillzSwitcherRoomInfo& Room : AllRooms)
    {
        bool bPassesFilter = false;

        switch (CurrentFilter)
        {
            case EDeskillzRoomFilter::All:
                bPassesFilter = true;
                break;
            case EDeskillzRoomFilter::Active:
                bPassesFilter = Room.bIsActive && Room.PlayerCount > 0;
                break;
            case EDeskillzRoomFilter::HighStakes:
                bPassesFilter = Room.MinBuyIn >= HighStakesThreshold;
                break;
        }

        // Apply search filter
        if (bPassesFilter && !SearchQuery.IsEmpty())
        {
            bPassesFilter = Room.RoomName.Contains(SearchQuery, ESearchCase::IgnoreCase) ||
                           Room.HostName.Contains(SearchQuery, ESearchCase::IgnoreCase) ||
                           Room.GameName.Contains(SearchQuery, ESearchCase::IgnoreCase);
        }

        if (bPassesFilter)
        {
            FilteredRooms.Add(Room);
        }
    }

    // Update current room index
    CurrentRoomIndex = -1;
    for (int32 i = 0; i < FilteredRooms.Num(); ++i)
    {
        if (FilteredRooms[i].RoomId == CurrentRoomId)
        {
            CurrentRoomIndex = i;
            break;
        }
    }

    RebuildRoomList();
    UpdateNavigationButtons();
    UpdateRoomCountDisplay();
}

void UDeskillzRoomSwitcherWidget::RebuildRoomList()
{
    if (!RoomListContainer) return;

    RoomListContainer->ClearChildren();

    for (const FDeskillzSwitcherRoomInfo& Room : FilteredRooms)
    {
        UWidget* RoomItem = CreateRoomListItem(Room);
        if (RoomItem)
        {
            RoomListContainer->AddChild(RoomItem);
        }
    }
}

void UDeskillzRoomSwitcherWidget::UpdateCurrentRoomDisplay()
{
    // Find current room data
    const FDeskillzSwitcherRoomInfo* CurrentRoom = nullptr;
    for (const FDeskillzSwitcherRoomInfo& Room : AllRooms)
    {
        if (Room.RoomId == CurrentRoomId)
        {
            CurrentRoom = &Room;
            break;
        }
    }

    if (CurrentRoom)
    {
        if (CurrentRoomNameText)
        {
            CurrentRoomNameText->SetText(FText::FromString(CurrentRoom->RoomName));
        }
        if (CurrentRoomHostText)
        {
            CurrentRoomHostText->SetText(FText::FromString(FString::Printf(TEXT("Host: %s"), *CurrentRoom->HostName)));
        }
        if (CurrentRoomPlayersText)
        {
            CurrentRoomPlayersText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), 
                CurrentRoom->PlayerCount, CurrentRoom->MaxPlayers)));
        }
        if (CurrentRoomStakesText)
        {
            CurrentRoomStakesText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f"), CurrentRoom->MinBuyIn)));
        }
    }
}

void UDeskillzRoomSwitcherWidget::UpdateNavigationButtons()
{
    if (PreviousRoomButton)
    {
        PreviousRoomButton->SetIsEnabled(CurrentRoomIndex > 0);
    }
    if (NextRoomButton)
    {
        NextRoomButton->SetIsEnabled(CurrentRoomIndex < FilteredRooms.Num() - 1);
    }

    if (RoomPositionText)
    {
        if (CurrentRoomIndex >= 0)
        {
            RoomPositionText->SetText(FText::FromString(FString::Printf(TEXT("%d of %d"), 
                CurrentRoomIndex + 1, FilteredRooms.Num())));
        }
        else
        {
            RoomPositionText->SetText(FText::FromString(FString::Printf(TEXT("- of %d"), FilteredRooms.Num())));
        }
    }
}

void UDeskillzRoomSwitcherWidget::UpdateFilterButtonStates()
{
    const FLinearColor ActiveColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);
    const FLinearColor InactiveColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);

    if (FilterAllButton)
    {
        FilterAllButton->SetColorAndOpacity(CurrentFilter == EDeskillzRoomFilter::All ? ActiveColor : InactiveColor);
    }
    if (FilterActiveButton)
    {
        FilterActiveButton->SetColorAndOpacity(CurrentFilter == EDeskillzRoomFilter::Active ? ActiveColor : InactiveColor);
    }
    if (FilterHighStakesButton)
    {
        FilterHighStakesButton->SetColorAndOpacity(CurrentFilter == EDeskillzRoomFilter::HighStakes ? ActiveColor : InactiveColor);
    }
}

void UDeskillzRoomSwitcherWidget::UpdateRoomCountDisplay()
{
    if (RoomCountText)
    {
        RoomCountText->SetText(FText::FromString(FString::Printf(TEXT("%d rooms"), FilteredRooms.Num())));
    }
}

void UDeskillzRoomSwitcherWidget::SwitchToRoom(const FString& RoomId)
{
    if (RoomId == CurrentRoomId) return;

    OnRoomSwitchRequested.Broadcast(RoomId);
}

UWidget* UDeskillzRoomSwitcherWidget::CreateRoomListItem(const FDeskillzSwitcherRoomInfo& Room)
{
    UBorder* Container = NewObject<UBorder>(this);
    if (!Container) return nullptr;

    // Highlight current room
    bool bIsCurrent = (Room.RoomId == CurrentRoomId);
    
    if (bIsCurrent)
    {
        Container->SetBrushColor(FLinearColor(0.15f, 0.25f, 0.35f, 1.0f));
    }
    else if (Room.bIsActive)
    {
        Container->SetBrushColor(FLinearColor(0.1f, 0.12f, 0.15f, 1.0f));
    }
    else
    {
        Container->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.08f, 0.6f));
    }

    return Container;
}

FLinearColor UDeskillzRoomSwitcherWidget::GetStakesColor(float MinBuyIn) const
{
    if (MinBuyIn >= 100.0f)
    {
        return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);      // Gold - High stakes
    }
    else if (MinBuyIn >= 25.0f)
    {
        return FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);    // Silver - Medium stakes
    }
    else
    {
        return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);       // Gray - Low stakes
    }
}