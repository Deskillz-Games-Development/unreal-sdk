// =============================================================================
// Deskillz SDK for Unreal Engine - Rebuy Modal Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Social/DeskillzRebuyModalWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/CircularThrobber.h"

void UDeskillzRebuyModalWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind input events
    if (AmountInput)
    {
        AmountInput->OnTextChanged.AddDynamic(this, &UDeskillzRebuyModalWidget::OnAmountInputChanged);
    }
    if (CurrencyDropdown)
    {
        CurrencyDropdown->OnSelectionChanged.AddDynamic(this, &UDeskillzRebuyModalWidget::OnCurrencySelected);
        
        for (const FString& Currency : AvailableCurrencies)
        {
            CurrencyDropdown->AddOption(Currency);
        }
        if (AvailableCurrencies.Num() > 0)
        {
            CurrencyDropdown->SetSelectedOption(AvailableCurrencies[0]);
            SelectedCurrency = AvailableCurrencies[0];
        }
    }

    // Bind preset buttons
    if (PresetMinButton)
    {
        PresetMinButton->OnClicked.AddDynamic(this, &UDeskillzRebuyModalWidget::OnPresetMinClicked);
    }
    if (Preset100xButton)
    {
        Preset100xButton->OnClicked.AddDynamic(this, &UDeskillzRebuyModalWidget::OnPreset100xClicked);
    }
    if (Preset200xButton)
    {
        Preset200xButton->OnClicked.AddDynamic(this, &UDeskillzRebuyModalWidget::OnPreset200xClicked);
    }

    // Bind action buttons
    if (RebuyButton)
    {
        RebuyButton->OnClicked.AddDynamic(this, &UDeskillzRebuyModalWidget::OnRebuyClicked);
    }
    if (LeaveButton)
    {
        LeaveButton->OnClicked.AddDynamic(this, &UDeskillzRebuyModalWidget::OnLeaveClicked);
    }
}

void UDeskillzRebuyModalWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bTimerActive && !bIsProcessing)
    {
        RemainingTime -= InDeltaTime;
        UpdateTimerDisplay();

        if (RemainingTime <= 0.0f)
        {
            HandleTimerExpired();
        }
    }
}

void UDeskillzRebuyModalWidget::ShowModal(const FDeskillzRebuySessionSummary& Summary)
{
    SessionSummary = Summary;

    // Calculate presets
    PresetMin = Summary.MinRebuy;
    Preset100x = Summary.PointValue * 100.0f;
    Preset200x = Summary.PointValue * 200.0f;

    // Reset timer
    RemainingTime = DecisionTimeLimit;
    bTimerActive = true;

    UpdateUIFromSummary();
    SetSelectedAmount(PresetMin);

    SetVisibility(ESlateVisibility::Visible);
}

void UDeskillzRebuyModalWidget::HideModal()
{
    bTimerActive = false;
    SetVisibility(ESlateVisibility::Collapsed);
    SetProcessing(false);
}

void UDeskillzRebuyModalWidget::OnRebuyComplete(bool bSuccess, const FString& ErrorMessage)
{
    SetProcessing(false);

    if (bSuccess)
    {
        HideModal();
    }
    else
    {
        // Show error and resume timer
        if (ErrorText)
        {
            ErrorText->SetText(FText::FromString(ErrorMessage));
            ErrorText->SetVisibility(ESlateVisibility::Visible);
        }
        ResumeTimer();
    }
}

void UDeskillzRebuyModalWidget::PauseTimer()
{
    bTimerActive = false;
}

void UDeskillzRebuyModalWidget::ResumeTimer()
{
    bTimerActive = true;
}

void UDeskillzRebuyModalWidget::OnAmountInputChanged(const FText& Text)
{
    float NewAmount = FCString::Atof(*Text.ToString());
    SetSelectedAmount(NewAmount);
}

void UDeskillzRebuyModalWidget::OnCurrencySelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    SelectedCurrency = SelectedItem;
    UpdateButtonStates();
}

void UDeskillzRebuyModalWidget::OnPresetMinClicked()
{
    SetSelectedAmount(PresetMin);
}

void UDeskillzRebuyModalWidget::OnPreset100xClicked()
{
    SetSelectedAmount(Preset100x);
}

void UDeskillzRebuyModalWidget::OnPreset200xClicked()
{
    SetSelectedAmount(Preset200x);
}

void UDeskillzRebuyModalWidget::OnRebuyClicked()
{
    if (!ValidateSelection()) return;

    PauseTimer();
    SetProcessing(true);
    OnRebuyConfirmed.Broadcast(SelectedAmount, SelectedCurrency);
}

void UDeskillzRebuyModalWidget::OnLeaveClicked()
{
    bTimerActive = false;
    OnLeaveGameClicked.Broadcast();
    HideModal();
}

void UDeskillzRebuyModalWidget::UpdateUIFromSummary()
{
    // Update session summary display
    if (TotalBuyInText)
    {
        TotalBuyInText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f"), SessionSummary.TotalBuyIn)));
    }
    if (RoundsPlayedText)
    {
        RoundsPlayedText->SetText(FText::AsNumber(SessionSummary.RoundsPlayed));
    }
    if (NetProfitLossText)
    {
        FString ProfitStr = FString::Printf(TEXT("%s$%.2f"), 
            SessionSummary.NetProfitLoss >= 0 ? TEXT("+") : TEXT("-"),
            FMath::Abs(SessionSummary.NetProfitLoss));
        NetProfitLossText->SetText(FText::FromString(ProfitStr));
        
        // Color based on profit/loss
        FSlateColor ProfitColor = SessionSummary.NetProfitLoss >= 0 ?
            FSlateColor(FLinearColor(0.2f, 0.8f, 0.4f, 1.0f)) :
            FSlateColor(FLinearColor(1.0f, 0.3f, 0.3f, 1.0f));
        NetProfitLossText->SetColorAndOpacity(ProfitColor);
    }

    // Update wallet and limits
    if (WalletBalanceText)
    {
        WalletBalanceText->SetText(FText::FromString(FString::Printf(TEXT("Balance: $%.2f"), SessionSummary.WalletBalance)));
    }
    if (MinRebuyText)
    {
        MinRebuyText->SetText(FText::FromString(FString::Printf(TEXT("Min: $%.2f"), SessionSummary.MinRebuy)));
    }

    // Update preset texts
    if (PresetMinText)
    {
        PresetMinText->SetText(FText::FromString(FString::Printf(TEXT("$%.0f"), PresetMin)));
    }
    if (Preset100xText)
    {
        Preset100xText->SetText(FText::FromString(FString::Printf(TEXT("$%.0f"), Preset100x)));
    }
    if (Preset200xText)
    {
        Preset200xText->SetText(FText::FromString(FString::Printf(TEXT("$%.0f"), Preset200x)));
    }
}

void UDeskillzRebuyModalWidget::UpdateButtonStates()
{
    bool bCanRebuy = ValidateSelection() && !bIsProcessing;

    if (RebuyButton)
    {
        RebuyButton->SetIsEnabled(bCanRebuy);
    }

    // Disable presets that exceed wallet balance
    if (Preset100xButton)
    {
        Preset100xButton->SetIsEnabled(Preset100x <= SessionSummary.WalletBalance);
    }
    if (Preset200xButton)
    {
        Preset200xButton->SetIsEnabled(Preset200x <= SessionSummary.WalletBalance);
    }
}

void UDeskillzRebuyModalWidget::UpdateTimerDisplay()
{
    if (TimerText)
    {
        int32 Seconds = FMath::CeilToInt(RemainingTime);
        TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Seconds)));

        // Change color when low
        if (RemainingTime <= TimerWarningThreshold)
        {
            TimerText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.3f, 0.3f, 1.0f)));
        }
        else
        {
            TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }
    }
}

void UDeskillzRebuyModalWidget::SetSelectedAmount(float Amount)
{
    SelectedAmount = FMath::Max(Amount, SessionSummary.MinRebuy);
    SelectedAmount = FMath::Min(SelectedAmount, SessionSummary.WalletBalance);

    if (AmountInput)
    {
        AmountInput->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), SelectedAmount)));
    }

    UpdateButtonStates();
}

bool UDeskillzRebuyModalWidget::ValidateSelection() const
{
    if (SelectedAmount < SessionSummary.MinRebuy) return false;
    if (SelectedAmount > SessionSummary.WalletBalance) return false;
    if (SelectedCurrency.IsEmpty()) return false;
    return true;
}

void UDeskillzRebuyModalWidget::SetProcessing(bool bProcessing)
{
    bIsProcessing = bProcessing;

    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(bProcessing ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (RebuyButton)
    {
        RebuyButton->SetIsEnabled(!bProcessing);
    }
    if (LeaveButton)
    {
        LeaveButton->SetIsEnabled(!bProcessing);
    }
}

void UDeskillzRebuyModalWidget::HandleTimerExpired()
{
    bTimerActive = false;
    OnDecisionTimerExpired.Broadcast();

    if (bAutoLeaveOnTimerExpiry)
    {
        OnLeaveGameClicked.Broadcast();
        HideModal();
    }
}