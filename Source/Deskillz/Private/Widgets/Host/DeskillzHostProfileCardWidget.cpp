// =============================================================================
// Deskillz SDK for Unreal Engine - Host Profile Card Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Host/DeskillzHostProfileCardWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"

void UDeskillzHostProfileCardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (EditProfileButton)
    {
        EditProfileButton->OnClicked.AddDynamic(this, &UDeskillzHostProfileCardWidget::OnEditProfileClicked);
    }
    if (ViewStatsButton)
    {
        ViewStatsButton->OnClicked.AddDynamic(this, &UDeskillzHostProfileCardWidget::OnViewStatsClicked);
    }
    if (ShareProfileButton)
    {
        ShareProfileButton->OnClicked.AddDynamic(this, &UDeskillzHostProfileCardWidget::OnShareProfileClicked);
    }
}

void UDeskillzHostProfileCardWidget::SetProfileData(const FDeskillzHostProfileData& InProfileData)
{
    ProfileData = InProfileData;
    UpdateAllDisplays();
}

void UDeskillzHostProfileCardWidget::SetCompactMode(bool bCompact)
{
    bCompactMode = bCompact;
    UpdateLayoutMode();
}

void UDeskillzHostProfileCardWidget::OnEditProfileClicked()
{
    OnEditProfileRequested.Broadcast();
}

void UDeskillzHostProfileCardWidget::OnViewStatsClicked()
{
    OnViewStatsRequested.Broadcast();
}

void UDeskillzHostProfileCardWidget::OnShareProfileClicked()
{
    OnShareProfileRequested.Broadcast();
}

void UDeskillzHostProfileCardWidget::UpdateAllDisplays()
{
    UpdateNameDisplay();
    UpdateAvatarDisplay();
    UpdateTierDisplay();
    UpdateStatsDisplay();
    UpdateVerificationDisplay();
}

void UDeskillzHostProfileCardWidget::UpdateNameDisplay()
{
    if (HostNameText)
    {
        HostNameText->SetText(FText::FromString(ProfileData.DisplayName));
    }
    if (HostUsernameText)
    {
        HostUsernameText->SetText(FText::FromString(FString::Printf(TEXT("@%s"), *ProfileData.Username)));
    }
}

void UDeskillzHostProfileCardWidget::UpdateAvatarDisplay()
{
    if (AvatarImage && ProfileData.AvatarTexture)
    {
        AvatarImage->SetBrushFromTexture(ProfileData.AvatarTexture);
    }

    // Update tier border color
    if (TierBorderImage)
    {
        TierBorderImage->SetColorAndOpacity(GetTierColor(ProfileData.TierLevel));
    }
}

void UDeskillzHostProfileCardWidget::UpdateTierDisplay()
{
    if (TierBadgeImage)
    {
        TierBadgeImage->SetColorAndOpacity(GetTierColor(ProfileData.TierLevel));
    }
    if (TierNameText)
    {
        TierNameText->SetText(FText::FromString(GetTierName(ProfileData.TierLevel)));
    }
    if (TierProgressBar)
    {
        TierProgressBar->SetPercent(ProfileData.TierProgress);
    }
    if (TierProgressText)
    {
        TierProgressText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), ProfileData.TierProgress * 100.0f)));
    }
}

void UDeskillzHostProfileCardWidget::UpdateStatsDisplay()
{
    if (RoomsHostedText)
    {
        RoomsHostedText->SetText(FText::AsNumber(ProfileData.TotalRoomsHosted));
    }
    if (PlayersServedText)
    {
        PlayersServedText->SetText(FText::AsNumber(ProfileData.TotalPlayersServed));
    }
    if (RatingText)
    {
        RatingText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), ProfileData.HostRating)));
    }
    if (MemberSinceText)
    {
        MemberSinceText->SetText(FText::FromString(ProfileData.MemberSince));
    }
}

void UDeskillzHostProfileCardWidget::UpdateVerificationDisplay()
{
    if (VerifiedBadge)
    {
        VerifiedBadge->SetVisibility(ProfileData.bIsVerified ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    if (VerifiedText)
    {
        VerifiedText->SetVisibility(ProfileData.bIsVerified ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UDeskillzHostProfileCardWidget::UpdateLayoutMode()
{
    // Hide detailed stats in compact mode
    if (StatsContainer)
    {
        StatsContainer->SetVisibility(bCompactMode ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
    if (ActionButtonsContainer)
    {
        ActionButtonsContainer->SetVisibility(bCompactMode ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
}

FString UDeskillzHostProfileCardWidget::GetTierName(int32 Tier) const
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

FLinearColor UDeskillzHostProfileCardWidget::GetTierColor(int32 Tier) const
{
    switch (Tier)
    {
        case 0: return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);      // Gray - Starter
        case 1: return FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);      // Bronze
        case 2: return FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);   // Silver
        case 3: return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);     // Gold
        case 4: return FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);      // Platinum
        case 5: return FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);     // Diamond
        default: return FLinearColor::White;
    }
}