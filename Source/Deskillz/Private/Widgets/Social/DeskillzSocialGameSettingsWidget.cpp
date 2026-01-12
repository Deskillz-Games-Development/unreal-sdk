// =============================================================================
// Deskillz SDK for Unreal Engine - Social Game Settings Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Social/DeskillzSocialGameSettingsWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/Slider.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"

void UDeskillzSocialGameSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind point value controls
    if (PointValueSlider)
    {
        PointValueSlider->OnValueChanged.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnPointValueSliderChanged);
    }
    if (PointValueInput)
    {
        PointValueInput->OnTextCommitted.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnPointValueInputCommitted);
    }

    // Bind rake controls
    if (RakePercentSlider)
    {
        RakePercentSlider->OnValueChanged.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnRakeSliderChanged);
    }
    if (RakeCapInput)
    {
        RakeCapInput->OnTextCommitted.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnRakeCapInputCommitted);
    }

    // Bind timer controls
    if (TurnTimerCheckbox)
    {
        TurnTimerCheckbox->OnCheckStateChanged.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnTurnTimerCheckboxChanged);
    }
    if (TurnTimerSlider)
    {
        TurnTimerSlider->OnValueChanged.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnTurnTimerSliderChanged);
    }

    // Bind pause controls
    if (PauseEnabledCheckbox)
    {
        PauseEnabledCheckbox->OnCheckStateChanged.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnPauseCheckboxChanged);
    }
    if (MaxPausesSlider)
    {
        MaxPausesSlider->OnValueChanged.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnMaxPausesSliderChanged);
    }
    if (PauseDurationSlider)
    {
        PauseDurationSlider->OnValueChanged.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnPauseDurationSliderChanged);
    }

    // Bind buttons
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnConfirmClicked);
    }
    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnCancelClicked);
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UDeskillzSocialGameSettingsWidget::OnCloseClicked);
    }

    // Initialize with defaults
    SetSettings(FDeskillzSocialGameSettings());
}

void UDeskillzSocialGameSettingsWidget::ShowModal()
{
    UpdateUIFromSettings();
    SetVisibility(ESlateVisibility::Visible);
}

void UDeskillzSocialGameSettingsWidget::HideModal()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

void UDeskillzSocialGameSettingsWidget::SetSettings(const FDeskillzSocialGameSettings& Settings)
{
    CurrentSettings = Settings;
    UpdateUIFromSettings();
}

FDeskillzSocialGameSettings UDeskillzSocialGameSettingsWidget::GetCurrentSettings() const
{
    return CurrentSettings;
}

void UDeskillzSocialGameSettingsWidget::SetHostTier(int32 TierLevel)
{
    HostTierLevel = FMath::Clamp(TierLevel, 0, 5);
    UpdateRevenuePreview();
}

void UDeskillzSocialGameSettingsWidget::UpdateRevenuePreview()
{
    float HostShare = GetHostSharePercent();

    if (HostSharePreviewText)
    {
        HostSharePreviewText->SetText(FText::FromString(FString::Printf(TEXT("Your Share: %.0f%%"), HostShare)));
    }

    if (EstimatedEarningsText)
    {
        float Estimated = CalculateEstimatedEarnings();
        EstimatedEarningsText->SetText(FText::FromString(FString::Printf(TEXT("Est. per 100 hands: $%.2f"), Estimated)));
    }
}

void UDeskillzSocialGameSettingsWidget::OnPointValueSliderChanged(float Value)
{
    CurrentSettings.PointValue = FMath::Lerp(MinPointValue, MaxPointValue, Value);
    
    if (PointValueInput)
    {
        PointValueInput->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), CurrentSettings.PointValue)));
    }

    UpdateMinBuyInDisplay();
    UpdateRevenuePreview();
    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnPointValueInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    float NewValue = FCString::Atof(*Text.ToString());
    CurrentSettings.PointValue = FMath::Clamp(NewValue, MinPointValue, MaxPointValue);

    if (PointValueSlider)
    {
        float SliderValue = (CurrentSettings.PointValue - MinPointValue) / (MaxPointValue - MinPointValue);
        PointValueSlider->SetValue(SliderValue);
    }

    UpdateMinBuyInDisplay();
    UpdateRevenuePreview();
    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnRakeSliderChanged(float Value)
{
    CurrentSettings.RakePercent = FMath::Lerp(MinRakePercent, MaxRakePercent, Value);

    if (RakePercentText)
    {
        RakePercentText->SetText(FText::FromString(FString::Printf(TEXT("%.1f%%"), CurrentSettings.RakePercent)));
    }

    UpdateRevenuePreview();
    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnRakeCapInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    CurrentSettings.RakeCap = FMath::Max(0.0f, FCString::Atof(*Text.ToString()));
    UpdateRevenuePreview();
    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnTurnTimerCheckboxChanged(bool bIsChecked)
{
    CurrentSettings.bTurnTimerEnabled = bIsChecked;

    if (TurnTimerSlider)
    {
        TurnTimerSlider->SetIsEnabled(bIsChecked);
    }
    if (TurnTimerText)
    {
        TurnTimerText->SetVisibility(bIsChecked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnTurnTimerSliderChanged(float Value)
{
    CurrentSettings.TurnTimerSeconds = FMath::RoundToInt(FMath::Lerp(
        static_cast<float>(MinTurnTimerSeconds), 
        static_cast<float>(MaxTurnTimerSeconds), 
        Value));

    if (TurnTimerText)
    {
        TurnTimerText->SetText(FText::FromString(FString::Printf(TEXT("%ds"), CurrentSettings.TurnTimerSeconds)));
    }

    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnPauseCheckboxChanged(bool bIsChecked)
{
    CurrentSettings.bPauseEnabled = bIsChecked;

    if (MaxPausesSlider)
    {
        MaxPausesSlider->SetIsEnabled(bIsChecked);
    }
    if (PauseDurationSlider)
    {
        PauseDurationSlider->SetIsEnabled(bIsChecked);
    }

    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnMaxPausesSliderChanged(float Value)
{
    CurrentSettings.MaxPausesPerPlayer = FMath::RoundToInt(FMath::Lerp(1.0f, 5.0f, Value));

    if (MaxPausesText)
    {
        MaxPausesText->SetText(FText::AsNumber(CurrentSettings.MaxPausesPerPlayer));
    }

    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnPauseDurationSliderChanged(float Value)
{
    CurrentSettings.MaxPauseDurationMinutes = FMath::RoundToInt(FMath::Lerp(1.0f, 15.0f, Value));

    if (PauseDurationText)
    {
        PauseDurationText->SetText(FText::FromString(FString::Printf(TEXT("%d min"), CurrentSettings.MaxPauseDurationMinutes)));
    }

    OnSettingsChanged.Broadcast(CurrentSettings);
}

void UDeskillzSocialGameSettingsWidget::OnConfirmClicked()
{
    OnSettingsConfirmed.Broadcast(CurrentSettings);
    HideModal();
}

void UDeskillzSocialGameSettingsWidget::OnCancelClicked()
{
    OnSettingsCancelled.Broadcast();
    HideModal();
}

void UDeskillzSocialGameSettingsWidget::OnCloseClicked()
{
    OnSettingsCancelled.Broadcast();
    HideModal();
}

void UDeskillzSocialGameSettingsWidget::UpdateUIFromSettings()
{
    // Point value
    if (PointValueSlider)
    {
        float SliderValue = (CurrentSettings.PointValue - MinPointValue) / (MaxPointValue - MinPointValue);
        PointValueSlider->SetValue(SliderValue);
    }
    if (PointValueInput)
    {
        PointValueInput->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), CurrentSettings.PointValue)));
    }

    // Rake
    if (RakePercentSlider)
    {
        float SliderValue = (CurrentSettings.RakePercent - MinRakePercent) / (MaxRakePercent - MinRakePercent);
        RakePercentSlider->SetValue(SliderValue);
    }
    if (RakePercentText)
    {
        RakePercentText->SetText(FText::FromString(FString::Printf(TEXT("%.1f%%"), CurrentSettings.RakePercent)));
    }
    if (RakeCapInput)
    {
        RakeCapInput->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), CurrentSettings.RakeCap)));
    }

    // Turn timer
    if (TurnTimerCheckbox)
    {
        TurnTimerCheckbox->SetIsChecked(CurrentSettings.bTurnTimerEnabled);
    }
    if (TurnTimerSlider)
    {
        float SliderValue = static_cast<float>(CurrentSettings.TurnTimerSeconds - MinTurnTimerSeconds) / 
                           static_cast<float>(MaxTurnTimerSeconds - MinTurnTimerSeconds);
        TurnTimerSlider->SetValue(SliderValue);
        TurnTimerSlider->SetIsEnabled(CurrentSettings.bTurnTimerEnabled);
    }
    if (TurnTimerText)
    {
        TurnTimerText->SetText(FText::FromString(FString::Printf(TEXT("%ds"), CurrentSettings.TurnTimerSeconds)));
    }

    // Pause
    if (PauseEnabledCheckbox)
    {
        PauseEnabledCheckbox->SetIsChecked(CurrentSettings.bPauseEnabled);
    }
    if (MaxPausesSlider)
    {
        MaxPausesSlider->SetValue(static_cast<float>(CurrentSettings.MaxPausesPerPlayer - 1) / 4.0f);
        MaxPausesSlider->SetIsEnabled(CurrentSettings.bPauseEnabled);
    }
    if (MaxPausesText)
    {
        MaxPausesText->SetText(FText::AsNumber(CurrentSettings.MaxPausesPerPlayer));
    }
    if (PauseDurationSlider)
    {
        PauseDurationSlider->SetValue(static_cast<float>(CurrentSettings.MaxPauseDurationMinutes - 1) / 14.0f);
        PauseDurationSlider->SetIsEnabled(CurrentSettings.bPauseEnabled);
    }
    if (PauseDurationText)
    {
        PauseDurationText->SetText(FText::FromString(FString::Printf(TEXT("%d min"), CurrentSettings.MaxPauseDurationMinutes)));
    }

    UpdateMinBuyInDisplay();
    UpdateRevenuePreview();
}

void UDeskillzSocialGameSettingsWidget::UpdateMinBuyInDisplay()
{
    if (MinBuyInText)
    {
        float MinBuyIn = CurrentSettings.PointValue * 50.0f; // 50 point minimum
        MinBuyInText->SetText(FText::FromString(FString::Printf(TEXT("Min Buy-in: $%.2f"), MinBuyIn)));
    }
}

float UDeskillzSocialGameSettingsWidget::CalculateEstimatedEarnings() const
{
    // Estimate earnings per 100 hands
    // Assumes average pot of 10x point value, rake collected on 70% of hands
    float AveragePot = CurrentSettings.PointValue * 10.0f;
    float RakePerHand = FMath::Min(AveragePot * (CurrentSettings.RakePercent / 100.0f), CurrentSettings.RakeCap);
    float TotalRake = RakePerHand * 70.0f; // 70 hands with rake out of 100
    float HostShare = GetHostSharePercent() / 100.0f;
    
    return TotalRake * HostShare;
}

float UDeskillzSocialGameSettingsWidget::GetHostSharePercent() const
{
    // Revenue share based on host tier
    switch (HostTierLevel)
    {
        case 0: return 50.0f;
        case 1: return 55.0f;
        case 2: return 60.0f;
        case 3: return 65.0f;
        case 4: return 70.0f;
        case 5: return 75.0f;
        default: return 50.0f;
    }
}