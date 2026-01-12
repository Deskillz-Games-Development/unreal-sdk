// =============================================================================
// Deskillz SDK for Unreal Engine - Turn Timer Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Social/DeskillzTurnTimerWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"

void UDeskillzTurnTimerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Initialize in stopped state
    StopTimer();
}

void UDeskillzTurnTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsRunning && !bIsPaused)
    {
        RemainingTime -= InDeltaTime;

        if (RemainingTime <= 0.0f)
        {
            RemainingTime = 0.0f;
            HandleTimerExpired();
        }
        else
        {
            UpdateTimerDisplay();

            // Check warning threshold
            if (!bWarningTriggered && RemainingTime <= WarningThreshold)
            {
                bWarningTriggered = true;
                OnWarningThreshold.Broadcast();
                PlayWarningAnimation();
            }
        }
    }
}

void UDeskillzTurnTimerWidget::StartTimer(float Duration, const FString& PlayerName)
{
    TotalDuration = Duration;
    RemainingTime = Duration;
    CurrentPlayerName = PlayerName;
    bIsRunning = true;
    bIsPaused = false;
    bWarningTriggered = false;

    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(PlayerName));
    }

    UpdateTimerDisplay();
    SetVisibility(ESlateVisibility::Visible);
    OnTimerStarted.Broadcast(Duration);
}

void UDeskillzTurnTimerWidget::StopTimer()
{
    bIsRunning = false;
    bIsPaused = false;
    RemainingTime = 0.0f;
    OnTimerStopped.Broadcast();
}

void UDeskillzTurnTimerWidget::PauseTimer()
{
    if (bIsRunning && !bIsPaused)
    {
        bIsPaused = true;
        OnTimerPaused.Broadcast();

        if (PausedIndicator)
        {
            PausedIndicator->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void UDeskillzTurnTimerWidget::ResumeTimer()
{
    if (bIsRunning && bIsPaused)
    {
        bIsPaused = false;
        OnTimerResumed.Broadcast();

        if (PausedIndicator)
        {
            PausedIndicator->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UDeskillzTurnTimerWidget::AddTime(float AdditionalSeconds)
{
    RemainingTime += AdditionalSeconds;
    TotalDuration += AdditionalSeconds;
    UpdateTimerDisplay();
}

void UDeskillzTurnTimerWidget::SetWarningThreshold(float Threshold)
{
    WarningThreshold = Threshold;
}

void UDeskillzTurnTimerWidget::UpdateTimerDisplay()
{
    // Update time text
    if (TimerText)
    {
        int32 Seconds = FMath::CeilToInt(RemainingTime);
        
        if (bShowMilliseconds && RemainingTime < 10.0f)
        {
            int32 Millis = FMath::FloorToInt((RemainingTime - FMath::FloorToFloat(RemainingTime)) * 10.0f);
            TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d.%d"), Seconds, Millis)));
        }
        else if (Seconds >= 60)
        {
            int32 Minutes = Seconds / 60;
            int32 RemainingSecs = Seconds % 60;
            TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, RemainingSecs)));
        }
        else
        {
            TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Seconds)));
        }
    }

    // Update progress bar
    if (TimerProgressBar && TotalDuration > 0.0f)
    {
        float Progress = RemainingTime / TotalDuration;
        TimerProgressBar->SetPercent(Progress);
    }

    // Update circular progress
    if (CircularProgressImage)
    {
        UpdateCircularProgress();
    }

    // Update colors based on remaining time
    UpdateTimerColors();
}

void UDeskillzTurnTimerWidget::UpdateTimerColors()
{
    FLinearColor CurrentColor;
    
    float TimePercent = (TotalDuration > 0.0f) ? (RemainingTime / TotalDuration) : 0.0f;

    if (RemainingTime <= CriticalThreshold)
    {
        CurrentColor = CriticalColor;
    }
    else if (RemainingTime <= WarningThreshold)
    {
        CurrentColor = WarningColor;
    }
    else
    {
        CurrentColor = NormalColor;
    }

    // Apply color to elements
    if (TimerText)
    {
        TimerText->SetColorAndOpacity(FSlateColor(CurrentColor));
    }
    if (TimerProgressBar)
    {
        TimerProgressBar->SetFillColorAndOpacity(CurrentColor);
    }
    if (TimerBackground)
    {
        // Slightly darken background
        FLinearColor BgColor = CurrentColor * 0.2f;
        BgColor.A = 0.8f;
        TimerBackground->SetBrushColor(BgColor);
    }
}

void UDeskillzTurnTimerWidget::UpdateCircularProgress()
{
    if (!CircularProgressImage) return;

    // Update material parameter for circular progress
    // This assumes a material with a "Progress" parameter
    float Progress = (TotalDuration > 0.0f) ? (RemainingTime / TotalDuration) : 0.0f;
    
    UMaterialInstanceDynamic* DynMaterial = CircularProgressImage->GetDynamicMaterial();
    if (DynMaterial)
    {
        DynMaterial->SetScalarParameterValue(TEXT("Progress"), Progress);
    }
}

void UDeskillzTurnTimerWidget::PlayWarningAnimation()
{
    if (!bEnableAnimations) return;

    // Pulse animation for warning state
    // This would typically use a UWidgetAnimation in practice
}

void UDeskillzTurnTimerWidget::PlayExpiredAnimation()
{
    if (!bEnableAnimations) return;

    // Flash animation for expired state
}

void UDeskillzTurnTimerWidget::HandleTimerExpired()
{
    bIsRunning = false;
    OnTimerExpired.Broadcast(CurrentPlayerName);
    PlayExpiredAnimation();

    if (bAutoHideOnExpire)
    {
        // Could add delay before hiding
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

FString UDeskillzTurnTimerWidget::FormatTime(float Seconds) const
{
    int32 TotalSeconds = FMath::CeilToInt(Seconds);
    
    if (TotalSeconds >= 3600)
    {
        int32 Hours = TotalSeconds / 3600;
        int32 Minutes = (TotalSeconds % 3600) / 60;
        int32 Secs = TotalSeconds % 60;
        return FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Secs);
    }
    else if (TotalSeconds >= 60)
    {
        int32 Minutes = TotalSeconds / 60;
        int32 Secs = TotalSeconds % 60;
        return FString::Printf(TEXT("%d:%02d"), Minutes, Secs);
    }
    else
    {
        return FString::Printf(TEXT("%d"), TotalSeconds);
    }
}