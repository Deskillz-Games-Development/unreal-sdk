// =============================================================================
// Deskillz SDK for Unreal Engine - Cash Out Modal Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Social/DeskillzCashOutModalWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/CircularThrobber.h"

void UDeskillzCashOutModalWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind action buttons
    if (CashOutButton)
    {
        CashOutButton->OnClicked.AddDynamic(this, &UDeskillzCashOutModalWidget::OnCashOutClicked);
    }
    if (StayButton)
    {
        StayButton->OnClicked.AddDynamic(this, &UDeskillzCashOutModalWidget::OnStayClicked);
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UDeskillzCashOutModalWidget::OnCloseClicked);
    }

    // Bind overlay click (to close)
    if (BackgroundOverlay)
    {
        // Note: Image doesn't have OnClicked, would need a button overlay in practice
    }
}

void UDeskillzCashOutModalWidget::ShowModal(const FDeskillzCashOutSummary& Summary)
{
    SessionSummary = Summary;
    UpdateUIFromSummary();
    SetVisibility(ESlateVisibility::Visible);
}

void UDeskillzCashOutModalWidget::HideModal()
{
    SetVisibility(ESlateVisibility::Collapsed);
    SetProcessing(false);
}

void UDeskillzCashOutModalWidget::OnOperationComplete(bool bSuccess, float AmountReceived)
{
    SetProcessing(false);
    OnCashOutComplete.Broadcast(bSuccess, AmountReceived);

    if (bSuccess)
    {
        HideModal();
    }
}

float UDeskillzCashOutModalWidget::GetWinRate() const
{
    if (SessionSummary.RoundsPlayed <= 0) return 0.0f;
    return (static_cast<float>(SessionSummary.RoundsWon) / static_cast<float>(SessionSummary.RoundsPlayed)) * 100.0f;
}

void UDeskillzCashOutModalWidget::OnCashOutClicked()
{
    SetProcessing(true);
    OnCashOutConfirmed.Broadcast();
}

void UDeskillzCashOutModalWidget::OnStayClicked()
{
    OnCashOutCancelled.Broadcast();
    HideModal();
}

void UDeskillzCashOutModalWidget::OnCloseClicked()
{
    OnCashOutCancelled.Broadcast();
    HideModal();
}

void UDeskillzCashOutModalWidget::OnOverlayClicked()
{
    OnCashOutCancelled.Broadcast();
    HideModal();
}

void UDeskillzCashOutModalWidget::UpdateUIFromSummary()
{
    // Update summary values
    if (TotalBuyInText)
    {
        TotalBuyInText->SetText(FText::FromString(FormatCurrency(SessionSummary.TotalBuyIn)));
    }
    if (CurrentBalanceText)
    {
        CurrentBalanceText->SetText(FText::FromString(FormatCurrency(SessionSummary.CurrentBalance)));
    }

    // Update rounds info
    if (RoundsPlayedText)
    {
        RoundsPlayedText->SetText(FText::AsNumber(SessionSummary.RoundsPlayed));
    }
    if (RoundsWonText)
    {
        FString WonStr = FString::Printf(TEXT("%d (%.0f%%)"), SessionSummary.RoundsWon, GetWinRate());
        RoundsWonText->SetText(FText::FromString(WonStr));
    }

    // Update net result
    if (NetResultText)
    {
        NetResultText->SetText(FText::FromString(FormatProfitLoss(SessionSummary.NetProfitLoss)));
    }

    UpdateNetResultColors();

    // Update info text
    if (InfoText)
    {
        InfoText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f %s will be returned to your wallet immediately."),
            SessionSummary.CurrentBalance, *SessionSummary.Currency)));
    }

    // Update cash out button text
    if (CashOutButtonText)
    {
        CashOutButtonText->SetText(FText::FromString(FString::Printf(TEXT("Cash Out $%.2f"), SessionSummary.CurrentBalance)));
    }
}

void UDeskillzCashOutModalWidget::UpdateNetResultColors()
{
    bool bIsProfit = SessionSummary.NetProfitLoss >= 0.0f;

    if (NetResultContainer)
    {
        NetResultContainer->SetBrushColor(bIsProfit ? ProfitBackgroundColor : LossBackgroundColor);
    }

    if (NetResultText)
    {
        NetResultText->SetColorAndOpacity(FSlateColor(bIsProfit ? ProfitTextColor : LossTextColor));
    }

    if (NetResultLabel)
    {
        NetResultLabel->SetText(FText::FromString(bIsProfit ? TEXT("Net Profit") : TEXT("Net Loss")));
    }
}

void UDeskillzCashOutModalWidget::SetProcessing(bool bProcessing)
{
    bIsProcessing = bProcessing;

    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(bProcessing ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (CashOutButton)
    {
        CashOutButton->SetIsEnabled(!bProcessing);
    }
    if (StayButton)
    {
        StayButton->SetIsEnabled(!bProcessing);
    }
    if (CloseButton)
    {
        CloseButton->SetIsEnabled(!bProcessing);
    }
}

FString UDeskillzCashOutModalWidget::FormatCurrency(float Amount) const
{
    return FString::Printf(TEXT("$%.2f %s"), Amount, *SessionSummary.Currency);
}

FString UDeskillzCashOutModalWidget::FormatProfitLoss(float Amount) const
{
    if (Amount >= 0.0f)
    {
        return FString::Printf(TEXT("+$%.2f"), Amount);
    }
    else
    {
        return FString::Printf(TEXT("-$%.2f"), FMath::Abs(Amount));
    }
}