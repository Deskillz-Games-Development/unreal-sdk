// =============================================================================
// Deskillz SDK for Unreal Engine - Pause Request Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Social/DeskillzPauseRequestWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"

void UDeskillzPauseRequestWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind buttons
    if (RequestPauseButton)
    {
        RequestPauseButton->OnClicked.AddDynamic(this, &UDeskillzPauseRequestWidget::OnRequestPauseClicked);
    }
    if (CancelPauseButton)
    {
        CancelPauseButton->OnClicked.AddDynamic(this, &UDeskillzPauseRequestWidget::OnCancelPauseClicked);
    }
    if (ResumePauseButton)
    {
        ResumePauseButton->OnClicked.AddDynamic(this, &UDeskillzPauseRequestWidget::OnResumeClicked);
    }
    if (ApprovePauseButton)
    {
        ApprovePauseButton->OnClicked.AddDynamic(this, &UDeskillzPauseRequestWidget::OnApproveClicked);
    }
    if (DenyPauseButton)
    {
        DenyPauseButton->OnClicked.AddDynamic(this, &UDeskillzPauseRequestWidget::OnDenyClicked);
    }

    // Initialize state
    SetPauseState(EDeskillzPauseState::Idle);
}

void UDeskillzPauseRequestWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update pause duration timer
    if (CurrentState == EDeskillzPauseState::Paused && bPauseTimerRunning)
    {
        PauseElapsedTime += InDeltaTime;
        UpdatePauseTimerDisplay();

        // Check max duration
        if (PauseElapsedTime >= MaxPauseDuration)
        {
            HandlePauseExpired();
        }
    }

    // Update vote timer
    if (CurrentState == EDeskillzPauseState::Voting && bVoteTimerRunning)
    {
        VoteTimeRemaining -= InDeltaTime;
        UpdateVoteTimerDisplay();

        if (VoteTimeRemaining <= 0.0f)
        {
            HandleVoteExpired();
        }
    }

    // Update cooldown timer
    if (bOnCooldown)
    {
        CooldownRemaining -= InDeltaTime;
        if (CooldownRemaining <= 0.0f)
        {
            bOnCooldown = false;
            UpdateCooldownDisplay();
        }
    }
}

void UDeskillzPauseRequestWidget::SetPauseConfig(const FDeskillzPauseConfig& Config)
{
    PauseConfig = Config;
    UpdatePauseCountDisplay();
}

void UDeskillzPauseRequestWidget::SetPauseState(EDeskillzPauseState NewState)
{
    CurrentState = NewState;
    UpdateStateDisplay();
    OnPauseStateChanged.Broadcast(NewState);
}

void UDeskillzPauseRequestWidget::ShowPauseRequest(const FString& RequesterName)
{
    CurrentRequesterName = RequesterName;
    SetPauseState(EDeskillzPauseState::Voting);
    
    // Start vote timer
    VoteTimeRemaining = VoteTimeout;
    bVoteTimerRunning = true;

    if (RequesterNameText)
    {
        RequesterNameText->SetText(FText::FromString(FString::Printf(TEXT("%s requests a pause"), *RequesterName)));
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UDeskillzPauseRequestWidget::ShowPauseActive(const FString& PausedByName, float Duration)
{
    CurrentRequesterName = PausedByName;
    MaxPauseDuration = Duration;
    PauseElapsedTime = 0.0f;
    bPauseTimerRunning = true;

    SetPauseState(EDeskillzPauseState::Paused);

    if (PausedByText)
    {
        PausedByText->SetText(FText::FromString(FString::Printf(TEXT("Paused by %s"), *PausedByName)));
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UDeskillzPauseRequestWidget::HidePauseUI()
{
    bPauseTimerRunning = false;
    bVoteTimerRunning = false;
    SetPauseState(EDeskillzPauseState::Idle);
    SetVisibility(ESlateVisibility::Collapsed);
}

void UDeskillzPauseRequestWidget::SetOnCooldown(float CooldownSeconds)
{
    bOnCooldown = true;
    CooldownRemaining = CooldownSeconds;
    UpdateCooldownDisplay();
}

void UDeskillzPauseRequestWidget::OnRequestPauseClicked()
{
    if (bOnCooldown || CurrentState != EDeskillzPauseState::Idle) return;
    if (PauseConfig.PausesUsed >= PauseConfig.MaxPauses) return;

    SetPauseState(EDeskillzPauseState::Requesting);
    OnPauseRequested.Broadcast();
}

void UDeskillzPauseRequestWidget::OnCancelPauseClicked()
{
    SetPauseState(EDeskillzPauseState::Idle);
    OnPauseCancelled.Broadcast();
}

void UDeskillzPauseRequestWidget::OnResumeClicked()
{
    bPauseTimerRunning = false;
    OnResumeRequested.Broadcast();
}

void UDeskillzPauseRequestWidget::OnApproveClicked()
{
    bVoteTimerRunning = false;
    OnPauseApproved.Broadcast();
}

void UDeskillzPauseRequestWidget::OnDenyClicked()
{
    bVoteTimerRunning = false;
    OnPauseDenied.Broadcast();
}

void UDeskillzPauseRequestWidget::UpdateStateDisplay()
{
    // Hide all state-specific containers
    if (IdleContainer) IdleContainer->SetVisibility(ESlateVisibility::Collapsed);
    if (RequestingContainer) RequestingContainer->SetVisibility(ESlateVisibility::Collapsed);
    if (VotingContainer) VotingContainer->SetVisibility(ESlateVisibility::Collapsed);
    if (PausedContainer) PausedContainer->SetVisibility(ESlateVisibility::Collapsed);

    // Show appropriate container
    switch (CurrentState)
    {
        case EDeskillzPauseState::Idle:
            if (IdleContainer) IdleContainer->SetVisibility(ESlateVisibility::Visible);
            break;
        case EDeskillzPauseState::Requesting:
            if (RequestingContainer) RequestingContainer->SetVisibility(ESlateVisibility::Visible);
            break;
        case EDeskillzPauseState::Voting:
            if (VotingContainer) VotingContainer->SetVisibility(ESlateVisibility::Visible);
            break;
        case EDeskillzPauseState::Paused:
            if (PausedContainer) PausedContainer->SetVisibility(ESlateVisibility::Visible);
            break;
    }

    // Update button states
    UpdateButtonStates();
}

void UDeskillzPauseRequestWidget::UpdateButtonStates()
{
    bool bCanRequest = !bOnCooldown && 
                       CurrentState == EDeskillzPauseState::Idle && 
                       PauseConfig.PausesUsed < PauseConfig.MaxPauses;

    if (RequestPauseButton)
    {
        RequestPauseButton->SetIsEnabled(bCanRequest);
    }
}

void UDeskillzPauseRequestWidget::UpdatePauseTimerDisplay()
{
    float Remaining = MaxPauseDuration - PauseElapsedTime;

    if (PauseTimerText)
    {
        int32 Minutes = FMath::FloorToInt(Remaining / 60.0f);
        int32 Seconds = FMath::FloorToInt(FMath::Fmod(Remaining, 60.0f));
        PauseTimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds)));
    }

    if (PauseProgressBar && MaxPauseDuration > 0.0f)
    {
        PauseProgressBar->SetPercent(Remaining / MaxPauseDuration);
    }
}

void UDeskillzPauseRequestWidget::UpdateVoteTimerDisplay()
{
    if (VoteTimerText)
    {
        int32 Seconds = FMath::CeilToInt(VoteTimeRemaining);
        VoteTimerText->SetText(FText::FromString(FString::Printf(TEXT("%ds"), Seconds)));
    }
}

void UDeskillzPauseRequestWidget::UpdatePauseCountDisplay()
{
    if (PauseCountText)
    {
        PauseCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d pauses"), 
            PauseConfig.PausesUsed, PauseConfig.MaxPauses)));
    }

    // Disable request button if no pauses remaining
    if (RequestPauseButton)
    {
        RequestPauseButton->SetIsEnabled(PauseConfig.PausesUsed < PauseConfig.MaxPauses && !bOnCooldown);
    }
}

void UDeskillzPauseRequestWidget::UpdateCooldownDisplay()
{
    if (CooldownText)
    {
        if (bOnCooldown)
        {
            int32 Minutes = FMath::FloorToInt(CooldownRemaining / 60.0f);
            int32 Seconds = FMath::FloorToInt(FMath::Fmod(CooldownRemaining, 60.0f));
            CooldownText->SetText(FText::FromString(FString::Printf(TEXT("Cooldown: %d:%02d"), Minutes, Seconds)));
            CooldownText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            CooldownText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    UpdateButtonStates();
}

void UDeskillzPauseRequestWidget::HandlePauseExpired()
{
    bPauseTimerRunning = false;
    OnPauseExpired.Broadcast();
    HidePauseUI();
}

void UDeskillzPauseRequestWidget::HandleVoteExpired()
{
    bVoteTimerRunning = false;
    OnVoteExpired.Broadcast();
    SetPauseState(EDeskillzPauseState::Idle);
}

FString UDeskillzPauseRequestWidget::FormatDuration(float Seconds) const
{
    int32 TotalSeconds = FMath::FloorToInt(Seconds);
    int32 Minutes = TotalSeconds / 60;
    int32 Secs = TotalSeconds % 60;
    return FString::Printf(TEXT("%d:%02d"), Minutes, Secs);
}