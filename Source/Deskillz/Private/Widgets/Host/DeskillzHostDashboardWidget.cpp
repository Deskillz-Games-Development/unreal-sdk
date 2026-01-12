// =============================================================================
// Deskillz SDK for Unreal Engine - Host Dashboard Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Host/DeskillzHostDashboardWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CircularThrobber.h"

void UDeskillzHostDashboardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind tab button events
    if (OverviewTabButton)
    {
        OverviewTabButton->OnClicked.AddDynamic(this, &UDeskillzHostDashboardWidget::OnOverviewTabClicked);
    }
    if (EarningsTabButton)
    {
        EarningsTabButton->OnClicked.AddDynamic(this, &UDeskillzHostDashboardWidget::OnEarningsTabClicked);
    }
    if (RoomsTabButton)
    {
        RoomsTabButton->OnClicked.AddDynamic(this, &UDeskillzHostDashboardWidget::OnRoomsTabClicked);
    }
    if (SettingsTabButton)
    {
        SettingsTabButton->OnClicked.AddDynamic(this, &UDeskillzHostDashboardWidget::OnSettingsTabClicked);
    }

    // Bind action buttons
    if (CreateRoomButton)
    {
        CreateRoomButton->OnClicked.AddDynamic(this, &UDeskillzHostDashboardWidget::OnCreateRoomClicked);
    }
    if (WithdrawButton)
    {
        WithdrawButton->OnClicked.AddDynamic(this, &UDeskillzHostDashboardWidget::OnWithdrawClicked);
    }
    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UDeskillzHostDashboardWidget::OnRefreshClicked);
    }

    // Set initial tab
    SwitchToTab(EDeskillzHostDashboardTab::Overview);
}

void UDeskillzHostDashboardWidget::SetHostData(const FDeskillzHostData& InHostData)
{
    HostData = InHostData;
    UpdateAllDisplays();
    OnHostDataUpdated.Broadcast(HostData);
}

void UDeskillzHostDashboardWidget::RefreshData()
{
    SetLoading(true);
    OnRefreshRequested.Broadcast();
}

void UDeskillzHostDashboardWidget::SwitchToTab(EDeskillzHostDashboardTab Tab)
{
    CurrentTab = Tab;

    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(Tab));
    }

    UpdateTabButtonStates();
    OnTabChanged.Broadcast(Tab);
}

void UDeskillzHostDashboardWidget::SetLoading(bool bLoading)
{
    bIsLoading = bLoading;

    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (ContentSwitcher)
    {
        ContentSwitcher->SetVisibility(bLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
}

void UDeskillzHostDashboardWidget::ShowError(const FString& ErrorMessage)
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::FromString(ErrorMessage));
        ErrorText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UDeskillzHostDashboardWidget::ClearError()
{
    if (ErrorText)
    {
        ErrorText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UDeskillzHostDashboardWidget::OnOverviewTabClicked()
{
    SwitchToTab(EDeskillzHostDashboardTab::Overview);
}

void UDeskillzHostDashboardWidget::OnEarningsTabClicked()
{
    SwitchToTab(EDeskillzHostDashboardTab::Earnings);
}

void UDeskillzHostDashboardWidget::OnRoomsTabClicked()
{
    SwitchToTab(EDeskillzHostDashboardTab::Rooms);
}

void UDeskillzHostDashboardWidget::OnSettingsTabClicked()
{
    SwitchToTab(EDeskillzHostDashboardTab::Settings);
}

void UDeskillzHostDashboardWidget::OnCreateRoomClicked()
{
    OnCreateRoomRequested.Broadcast();
}

void UDeskillzHostDashboardWidget::OnWithdrawClicked()
{
    OnWithdrawRequested.Broadcast();
}

void UDeskillzHostDashboardWidget::OnRefreshClicked()
{
    RefreshData();
}

void UDeskillzHostDashboardWidget::UpdateAllDisplays()
{
    UpdateHostNameDisplay();
    UpdateTierDisplay();
    UpdateEarningsDisplay();
    UpdateStatsDisplay();
}

void UDeskillzHostDashboardWidget::UpdateHostNameDisplay()
{
    if (HostNameText)
    {
        HostNameText->SetText(FText::FromString(HostData.HostName));
    }
}

void UDeskillzHostDashboardWidget::UpdateTierDisplay()
{
    if (TierLevelText)
    {
        TierLevelText->SetText(FText::FromString(FString::Printf(TEXT("Tier %d"), HostData.TierLevel)));
    }
    if (TierNameText)
    {
        TierNameText->SetText(FText::FromString(GetTierName(HostData.TierLevel)));
    }
}

void UDeskillzHostDashboardWidget::UpdateEarningsDisplay()
{
    if (TotalEarningsText)
    {
        TotalEarningsText->SetText(FText::FromString(FormatCurrency(HostData.TotalEarnings)));
    }
    if (PendingPayoutText)
    {
        PendingPayoutText->SetText(FText::FromString(FormatCurrency(HostData.PendingPayout)));
    }
    if (AvailableBalanceText)
    {
        AvailableBalanceText->SetText(FText::FromString(FormatCurrency(HostData.AvailableBalance)));
    }
}

void UDeskillzHostDashboardWidget::UpdateStatsDisplay()
{
    if (TotalRoomsText)
    {
        TotalRoomsText->SetText(FText::AsNumber(HostData.TotalRoomsHosted));
    }
    if (ActiveRoomsText)
    {
        ActiveRoomsText->SetText(FText::AsNumber(HostData.ActiveRooms));
    }
    if (TotalPlayersText)
    {
        TotalPlayersText->SetText(FText::AsNumber(HostData.TotalPlayersServed));
    }
}

void UDeskillzHostDashboardWidget::UpdateTabButtonStates()
{
    // Update visual states for tab buttons
    const FLinearColor ActiveColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);
    const FLinearColor InactiveColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    if (OverviewTabButton)
    {
        OverviewTabButton->SetColorAndOpacity(CurrentTab == EDeskillzHostDashboardTab::Overview ? ActiveColor : InactiveColor);
    }
    if (EarningsTabButton)
    {
        EarningsTabButton->SetColorAndOpacity(CurrentTab == EDeskillzHostDashboardTab::Earnings ? ActiveColor : InactiveColor);
    }
    if (RoomsTabButton)
    {
        RoomsTabButton->SetColorAndOpacity(CurrentTab == EDeskillzHostDashboardTab::Rooms ? ActiveColor : InactiveColor);
    }
    if (SettingsTabButton)
    {
        SettingsTabButton->SetColorAndOpacity(CurrentTab == EDeskillzHostDashboardTab::Settings ? ActiveColor : InactiveColor);
    }
}

FString UDeskillzHostDashboardWidget::GetTierName(int32 Tier) const
{
    switch (Tier)
    {
        case 0: return TEXT("Starter");
        case 1: return TEXT("Bronze");
        case 2: return TEXT("Silver");
        case 3: return TEXT("Gold");
        case 4: return TEXT("Platinum");
        case 5: return TEXT("Diamond");
        default: return TEXT("Unknown");
    }
}

FString UDeskillzHostDashboardWidget::FormatCurrency(float Amount) const
{
    return FString::Printf(TEXT("$%.2f"), Amount);
}