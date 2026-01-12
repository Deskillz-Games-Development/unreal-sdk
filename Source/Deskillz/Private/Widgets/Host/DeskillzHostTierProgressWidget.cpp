// =============================================================================
// Deskillz SDK for Unreal Engine - Host Tier Progress Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Host/DeskillzHostTierProgressWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"

void UDeskillzHostTierProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (ViewBenefitsButton)
    {
        ViewBenefitsButton->OnClicked.AddDynamic(this, &UDeskillzHostTierProgressWidget::OnViewBenefitsClicked);
    }
    if (ViewRequirementsButton)
    {
        ViewRequirementsButton->OnClicked.AddDynamic(this, &UDeskillzHostTierProgressWidget::OnViewRequirementsClicked);
    }
}

void UDeskillzHostTierProgressWidget::SetTierData(const FDeskillzHostTierData& InTierData)
{
    TierData = InTierData;
    UpdateAllDisplays();
}

void UDeskillzHostTierProgressWidget::SetAnimated(bool bAnimate)
{
    bAnimateProgress = bAnimate;
}

void UDeskillzHostTierProgressWidget::PlayProgressAnimation()
{
    if (!bAnimateProgress) return;

    // Start animation from 0 to current progress
    AnimationProgress = 0.0f;
    bIsAnimating = true;
}

void UDeskillzHostTierProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsAnimating)
    {
        AnimationProgress += InDeltaTime * AnimationSpeed;
        
        if (AnimationProgress >= 1.0f)
        {
            AnimationProgress = 1.0f;
            bIsAnimating = false;
        }

        // Update progress bar with animated value
        float AnimatedProgress = FMath::Lerp(0.0f, TierData.ProgressToNextTier, AnimationProgress);
        if (MainProgressBar)
        {
            MainProgressBar->SetPercent(AnimatedProgress);
        }
    }
}

void UDeskillzHostTierProgressWidget::OnViewBenefitsClicked()
{
    OnViewBenefitsRequested.Broadcast(TierData.CurrentTier);
}

void UDeskillzHostTierProgressWidget::OnViewRequirementsClicked()
{
    OnViewRequirementsRequested.Broadcast(TierData.CurrentTier + 1);
}

void UDeskillzHostTierProgressWidget::UpdateAllDisplays()
{
    UpdateCurrentTierDisplay();
    UpdateNextTierDisplay();
    UpdateProgressDisplay();
    UpdateBenefitsDisplay();
    UpdateMilestoneMarkers();
}

void UDeskillzHostTierProgressWidget::UpdateCurrentTierDisplay()
{
    if (CurrentTierText)
    {
        CurrentTierText->SetText(FText::FromString(GetTierName(TierData.CurrentTier)));
    }
    if (CurrentTierIcon)
    {
        CurrentTierIcon->SetColorAndOpacity(GetTierColor(TierData.CurrentTier));
    }
    if (CurrentTierBorder)
    {
        CurrentTierBorder->SetBrushColor(GetTierColor(TierData.CurrentTier));
    }
}

void UDeskillzHostTierProgressWidget::UpdateNextTierDisplay()
{
    bool bHasNextTier = TierData.CurrentTier < 5;

    if (NextTierContainer)
    {
        NextTierContainer->SetVisibility(bHasNextTier ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (MaxTierReachedText)
    {
        MaxTierReachedText->SetVisibility(bHasNextTier ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }

    if (bHasNextTier)
    {
        if (NextTierText)
        {
            NextTierText->SetText(FText::FromString(GetTierName(TierData.CurrentTier + 1)));
        }
        if (NextTierIcon)
        {
            NextTierIcon->SetColorAndOpacity(GetTierColor(TierData.CurrentTier + 1));
        }
    }
}

void UDeskillzHostTierProgressWidget::UpdateProgressDisplay()
{
    if (MainProgressBar)
    {
        if (bAnimateProgress && !bIsAnimating)
        {
            PlayProgressAnimation();
        }
        else if (!bAnimateProgress)
        {
            MainProgressBar->SetPercent(TierData.ProgressToNextTier);
        }
    }

    if (ProgressPercentText)
    {
        ProgressPercentText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), TierData.ProgressToNextTier * 100.0f)));
    }

    // Update requirement progress
    if (RoomsProgressText)
    {
        RoomsProgressText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d rooms"), 
            TierData.CurrentRoomsHosted, TierData.RequiredRoomsForNextTier)));
    }
    if (EarningsProgressText)
    {
        EarningsProgressText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f / $%.2f earned"), 
            TierData.CurrentTotalEarnings, TierData.RequiredEarningsForNextTier)));
    }
    if (RatingProgressText)
    {
        RatingProgressText->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f rating"), 
            TierData.CurrentRating, TierData.RequiredRatingForNextTier)));
    }
}

void UDeskillzHostTierProgressWidget::UpdateBenefitsDisplay()
{
    float revenueShare = GetRevenueSharePercent(TierData.CurrentTier);
    
    if (RevenueShareText)
    {
        RevenueShareText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%% Revenue Share"), revenueShare)));
    }

    float nextRevenueShare = GetRevenueSharePercent(TierData.CurrentTier + 1);
    if (NextRevenueShareText && TierData.CurrentTier < 5)
    {
        NextRevenueShareText->SetText(FText::FromString(FString::Printf(TEXT("Next: %.0f%%"), nextRevenueShare)));
    }
}

void UDeskillzHostTierProgressWidget::UpdateMilestoneMarkers()
{
    if (!MilestoneContainer) return;

    // Create milestone markers for each tier
    for (int32 i = 0; i <= 5; ++i)
    {
        float MarkerPosition = static_cast<float>(i) / 5.0f;
        bool bReached = i <= TierData.CurrentTier;
        
        // Update marker visuals (implementation depends on widget setup)
    }
}

FString UDeskillzHostTierProgressWidget::GetTierName(int32 Tier) const
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

FLinearColor UDeskillzHostTierProgressWidget::GetTierColor(int32 Tier) const
{
    switch (Tier)
    {
        case 0: return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);      // Gray
        case 1: return FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);      // Bronze
        case 2: return FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);   // Silver
        case 3: return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);     // Gold
        case 4: return FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);      // Platinum
        case 5: return FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);     // Diamond
        default: return FLinearColor::White;
    }
}

float UDeskillzHostTierProgressWidget::GetRevenueSharePercent(int32 Tier) const
{
    switch (Tier)
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