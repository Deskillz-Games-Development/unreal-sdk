// =============================================================================
// Deskillz SDK for Unreal Engine - Buy-In Modal Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Social/DeskillzBuyInModalWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CircularThrobber.h"

void UDeskillzBuyInModalWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind input events
    if (AmountInput)
    {
        AmountInput->OnTextChanged.AddDynamic(this, &UDeskillzBuyInModalWidget::OnAmountInputChanged);
    }
    if (AmountSlider)
    {
        AmountSlider->OnValueChanged.AddDynamic(this, &UDeskillzBuyInModalWidget::OnAmountSliderChanged);
    }
    if (CurrencyDropdown)
    {
        CurrencyDropdown->OnSelectionChanged.AddDynamic(this, &UDeskillzBuyInModalWidget::OnCurrencySelected);
        
        // Populate currencies
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
        PresetMinButton->OnClicked.AddDynamic(this, &UDeskillzBuyInModalWidget::OnPresetMinClicked);
    }
    if (Preset100xButton)
    {
        Preset100xButton->OnClicked.AddDynamic(this, &UDeskillzBuyInModalWidget::OnPreset100xClicked);
    }
    if (Preset200xButton)
    {
        Preset200xButton->OnClicked.AddDynamic(this, &UDeskillzBuyInModalWidget::OnPreset200xClicked);
    }
    if (PresetMaxButton)
    {
        PresetMaxButton->OnClicked.AddDynamic(this, &UDeskillzBuyInModalWidget::OnPresetMaxClicked);
    }

    // Bind action buttons
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UDeskillzBuyInModalWidget::OnConfirmClicked);
    }
    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UDeskillzBuyInModalWidget::OnCancelClicked);
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UDeskillzBuyInModalWidget::OnCancelClicked);
    }
}

void UDeskillzBuyInModalWidget::ShowModal(const FDeskillzBuyInConfig& Config)
{
    BuyInConfig = Config;
    
    // Calculate presets
    PresetMin = Config.MinBuyIn;
    Preset100x = Config.PointValue * 100.0f;
    Preset200x = Config.PointValue * 200.0f;
    PresetMax = FMath::Min(Config.MaxBuyIn, Config.WalletBalance);

    // Update UI
    UpdateUIFromConfig();
    SetSelectedAmount(PresetMin);
    
    SetVisibility(ESlateVisibility::Visible);
}

void UDeskillzBuyInModalWidget::HideModal()
{
    SetVisibility(ESlateVisibility::Collapsed);
    SetProcessing(false);
}

void UDeskillzBuyInModalWidget::OnBuyInComplete(bool bSuccess, const FString& ErrorMessage)
{
    SetProcessing(false);

    if (bSuccess)
    {
        HideModal();
    }
    else
    {
        ShowError(ErrorMessage);
    }
}

void UDeskillzBuyInModalWidget::OnAmountInputChanged(const FText& Text)
{
    float NewAmount = FCString::Atof(*Text.ToString());
    SetSelectedAmount(NewAmount);
}

void UDeskillzBuyInModalWidget::OnAmountSliderChanged(float Value)
{
    float NewAmount = FMath::Lerp(BuyInConfig.MinBuyIn, PresetMax, Value);
    SetSelectedAmount(NewAmount);
    
    // Update text input
    if (AmountInput)
    {
        AmountInput->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), NewAmount)));
    }
}

void UDeskillzBuyInModalWidget::OnCurrencySelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    SelectedCurrency = SelectedItem;
    UpdateButtonStates();
}

void UDeskillzBuyInModalWidget::OnPresetMinClicked()
{
    SetSelectedAmount(PresetMin);
}

void UDeskillzBuyInModalWidget::OnPreset100xClicked()
{
    SetSelectedAmount(Preset100x);
}

void UDeskillzBuyInModalWidget::OnPreset200xClicked()
{
    SetSelectedAmount(Preset200x);
}

void UDeskillzBuyInModalWidget::OnPresetMaxClicked()
{
    SetSelectedAmount(PresetMax);
}

void UDeskillzBuyInModalWidget::OnConfirmClicked()
{
    if (!ValidateSelection()) return;

    SetProcessing(true);
    OnBuyInConfirmed.Broadcast(SelectedAmount, SelectedCurrency);
}

void UDeskillzBuyInModalWidget::OnCancelClicked()
{
    OnBuyInCancelled.Broadcast();
    HideModal();
}

void UDeskillzBuyInModalWidget::UpdateUIFromConfig()
{
    // Update room info
    if (RoomNameText)
    {
        RoomNameText->SetText(FText::FromString(BuyInConfig.RoomName));
    }
    if (GameNameText)
    {
        GameNameText->SetText(FText::FromString(BuyInConfig.GameName));
    }
    if (PointValueText)
    {
        PointValueText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f per point"), BuyInConfig.PointValue)));
    }

    // Update limits
    if (MinBuyInText)
    {
        MinBuyInText->SetText(FText::FromString(FString::Printf(TEXT("Min: $%.2f"), BuyInConfig.MinBuyIn)));
    }
    if (MaxBuyInText)
    {
        MaxBuyInText->SetText(FText::FromString(FString::Printf(TEXT("Max: $%.2f"), BuyInConfig.MaxBuyIn)));
    }

    // Update wallet balance
    if (WalletBalanceText)
    {
        WalletBalanceText->SetText(FText::FromString(FString::Printf(TEXT("Balance: $%.2f"), BuyInConfig.WalletBalance)));
    }

    // Update preset button texts
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
    if (PresetMaxText)
    {
        PresetMaxText->SetText(FText::FromString(FString::Printf(TEXT("$%.0f"), PresetMax)));
    }

    // Update slider range
    if (AmountSlider)
    {
        AmountSlider->SetMinValue(0.0f);
        AmountSlider->SetMaxValue(1.0f);
    }
}

void UDeskillzBuyInModalWidget::UpdateButtonStates()
{
    bool bCanConfirm = ValidateSelection() && !bIsProcessing;

    if (ConfirmButton)
    {
        ConfirmButton->SetIsEnabled(bCanConfirm);
    }

    // Update preset button states based on availability
    if (Preset100xButton)
    {
        Preset100xButton->SetIsEnabled(Preset100x <= BuyInConfig.WalletBalance && Preset100x >= BuyInConfig.MinBuyIn);
    }
    if (Preset200xButton)
    {
        Preset200xButton->SetIsEnabled(Preset200x <= BuyInConfig.WalletBalance && Preset200x >= BuyInConfig.MinBuyIn);
    }
    if (PresetMaxButton)
    {
        PresetMaxButton->SetIsEnabled(PresetMax >= BuyInConfig.MinBuyIn);
    }
}

void UDeskillzBuyInModalWidget::SetSelectedAmount(float Amount)
{
    SelectedAmount = FMath::Clamp(Amount, BuyInConfig.MinBuyIn, PresetMax);

    if (AmountInput)
    {
        AmountInput->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), SelectedAmount)));
    }

    // Update slider position
    if (AmountSlider && PresetMax > BuyInConfig.MinBuyIn)
    {
        float SliderValue = (SelectedAmount - BuyInConfig.MinBuyIn) / (PresetMax - BuyInConfig.MinBuyIn);
        AmountSlider->SetValue(SliderValue);
    }

    // Update starting chips display
    if (StartingChipsText && BuyInConfig.PointValue > 0.0f)
    {
        int32 Chips = FMath::FloorToInt(SelectedAmount / BuyInConfig.PointValue);
        StartingChipsText->SetText(FText::FromString(FString::Printf(TEXT("%d chips"), Chips)));
    }

    UpdateButtonStates();
}

bool UDeskillzBuyInModalWidget::ValidateSelection() const
{
    if (SelectedAmount < BuyInConfig.MinBuyIn) return false;
    if (SelectedAmount > BuyInConfig.MaxBuyIn) return false;
    if (SelectedAmount > BuyInConfig.WalletBalance) return false;
    if (SelectedCurrency.IsEmpty()) return false;
    return true;
}

void UDeskillzBuyInModalWidget::SetProcessing(bool bProcessing)
{
    bIsProcessing = bProcessing;

    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(bProcessing ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (ConfirmButton)
    {
        ConfirmButton->SetIsEnabled(!bProcessing);
    }
    if (CancelButton)
    {
        CancelButton->SetIsEnabled(!bProcessing);
    }
}

void UDeskillzBuyInModalWidget::ShowError(const FString& ErrorMessage)
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::FromString(ErrorMessage));
        ErrorText->SetVisibility(ESlateVisibility::Visible);
    }
}