// =============================================================================
// Deskillz SDK for Unreal Engine - Host Badge Grid Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Host/DeskillzHostBadgeGridWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/UniformGridPanel.h"
#include "Components/WrapBox.h"

void UDeskillzHostBadgeGridWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind filter buttons
    if (AllBadgesButton)
    {
        AllBadgesButton->OnClicked.AddDynamic(this, &UDeskillzHostBadgeGridWidget::OnAllFilterClicked);
    }
    if (EarnedBadgesButton)
    {
        EarnedBadgesButton->OnClicked.AddDynamic(this, &UDeskillzHostBadgeGridWidget::OnEarnedFilterClicked);
    }
    if (LockedBadgesButton)
    {
        LockedBadgesButton->OnClicked.AddDynamic(this, &UDeskillzHostBadgeGridWidget::OnLockedFilterClicked);
    }
}

void UDeskillzHostBadgeGridWidget::SetBadges(const TArray<FDeskillzHostBadge>& InBadges)
{
    AllBadges = InBadges;
    ApplyCurrentFilter();
}

void UDeskillzHostBadgeGridWidget::SetFilter(EDeskillzBadgeFilter Filter)
{
    CurrentFilter = Filter;
    ApplyCurrentFilter();
    UpdateFilterButtonStates();
}

void UDeskillzHostBadgeGridWidget::SetGridColumns(int32 Columns)
{
    GridColumns = FMath::Clamp(Columns, 2, 6);
    RebuildGrid();
}

void UDeskillzHostBadgeGridWidget::OnAllFilterClicked()
{
    SetFilter(EDeskillzBadgeFilter::All);
}

void UDeskillzHostBadgeGridWidget::OnEarnedFilterClicked()
{
    SetFilter(EDeskillzBadgeFilter::Earned);
}

void UDeskillzHostBadgeGridWidget::OnLockedFilterClicked()
{
    SetFilter(EDeskillzBadgeFilter::Locked);
}

void UDeskillzHostBadgeGridWidget::OnBadgeClicked(const FDeskillzHostBadge& Badge)
{
    OnBadgeSelected.Broadcast(Badge);
}

void UDeskillzHostBadgeGridWidget::ApplyCurrentFilter()
{
    FilteredBadges.Empty();

    for (const FDeskillzHostBadge& Badge : AllBadges)
    {
        bool bInclude = false;

        switch (CurrentFilter)
        {
            case EDeskillzBadgeFilter::All:
                bInclude = true;
                break;
            case EDeskillzBadgeFilter::Earned:
                bInclude = Badge.bIsEarned;
                break;
            case EDeskillzBadgeFilter::Locked:
                bInclude = !Badge.bIsEarned;
                break;
        }

        if (bInclude)
        {
            FilteredBadges.Add(Badge);
        }
    }

    RebuildGrid();
    UpdateCountDisplay();
}

void UDeskillzHostBadgeGridWidget::RebuildGrid()
{
    if (!BadgeGrid) return;

    BadgeGrid->ClearChildren();

    for (int32 i = 0; i < FilteredBadges.Num(); ++i)
    {
        const FDeskillzHostBadge& Badge = FilteredBadges[i];
        
        // Create badge item widget
        UWidget* BadgeWidget = CreateBadgeWidget(Badge);
        if (BadgeWidget)
        {
            BadgeGrid->AddChild(BadgeWidget);
        }
    }
}

void UDeskillzHostBadgeGridWidget::UpdateFilterButtonStates()
{
    const FLinearColor ActiveColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);
    const FLinearColor InactiveColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    if (AllBadgesButton)
    {
        AllBadgesButton->SetColorAndOpacity(CurrentFilter == EDeskillzBadgeFilter::All ? ActiveColor : InactiveColor);
    }
    if (EarnedBadgesButton)
    {
        EarnedBadgesButton->SetColorAndOpacity(CurrentFilter == EDeskillzBadgeFilter::Earned ? ActiveColor : InactiveColor);
    }
    if (LockedBadgesButton)
    {
        LockedBadgesButton->SetColorAndOpacity(CurrentFilter == EDeskillzBadgeFilter::Locked ? ActiveColor : InactiveColor);
    }
}

void UDeskillzHostBadgeGridWidget::UpdateCountDisplay()
{
    int32 EarnedCount = 0;
    for (const FDeskillzHostBadge& Badge : AllBadges)
    {
        if (Badge.bIsEarned)
        {
            EarnedCount++;
        }
    }

    if (BadgeCountText)
    {
        BadgeCountText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d Badges"), EarnedCount, AllBadges.Num())));
    }

    if (FilteredCountText)
    {
        FString FilterName;
        switch (CurrentFilter)
        {
            case EDeskillzBadgeFilter::All: FilterName = TEXT("All"); break;
            case EDeskillzBadgeFilter::Earned: FilterName = TEXT("Earned"); break;
            case EDeskillzBadgeFilter::Locked: FilterName = TEXT("Locked"); break;
        }
        FilteredCountText->SetText(FText::FromString(FString::Printf(TEXT("%s: %d"), *FilterName, FilteredBadges.Num())));
    }
}

UWidget* UDeskillzHostBadgeGridWidget::CreateBadgeWidget(const FDeskillzHostBadge& Badge)
{
    // Create a border container for the badge
    UBorder* BadgeContainer = NewObject<UBorder>(this);
    if (!BadgeContainer) return nullptr;

    // Set visual style based on earned status
    if (Badge.bIsEarned)
    {
        BadgeContainer->SetBrushColor(FLinearColor(0.1f, 0.15f, 0.2f, 1.0f));
    }
    else
    {
        BadgeContainer->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.5f));
    }

    return BadgeContainer;
}

FLinearColor UDeskillzHostBadgeGridWidget::GetBadgeRarityColor(EDeskillzBadgeRarity Rarity) const
{
    switch (Rarity)
    {
        case EDeskillzBadgeRarity::Common:
            return FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);      // Gray
        case EDeskillzBadgeRarity::Uncommon:
            return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);      // Green
        case EDeskillzBadgeRarity::Rare:
            return FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);      // Blue
        case EDeskillzBadgeRarity::Epic:
            return FLinearColor(0.6f, 0.2f, 0.8f, 1.0f);      // Purple
        case EDeskillzBadgeRarity::Legendary:
            return FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);      // Orange
        default:
            return FLinearColor::White;
    }
}