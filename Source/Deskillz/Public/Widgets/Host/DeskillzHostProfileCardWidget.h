// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostTypes.h"
#include "DeskillzHostProfileCardWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UBorder;
class UButton;
class UHorizontalBox;

/**
 * Deskillz Host Profile Card Widget
 * 
 * Displays host profile information including:
 * - Avatar and username
 * - Current tier with icon
 * - Level and XP progress
 * - Rating stars
 * - Verification status
 * - Quick stats (streak, rooms, players)
 * 
 * Usage:
 * 1. Add to parent widget
 * 2. Call SetProfile() with host data
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostProfileCardWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostProfileCardWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Data Binding
    // ========================================================================

    /**
     * Set the profile to display
     * @param Profile Host profile data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetProfile(const FDeskillzHostProfile& Profile);

    /**
     * Update tier display
     * @param EsportsTier Esports tier
     * @param SocialTier Social tier
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateTierDisplay(EDeskillzHostTier EsportsTier, EDeskillzHostTier SocialTier);

    /**
     * Update level progress
     * @param Level Current level
     * @param Experience Current XP
     * @param NextLevelXP XP needed for next level
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateLevelProgress(int32 Level, int32 Experience, int32 NextLevelXP);

    /**
     * Update rating display
     * @param Rating Rating (0-5)
     * @param Count Number of ratings
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateRating(float Rating, int32 Count);

    /**
     * Set verification status
     * @param bIsVerified Whether host is verified
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetVerificationStatus(bool bIsVerified);

    /**
     * Update quick stats
     * @param Streak Current streak days
     * @param TotalRooms Total rooms created
     * @param TotalPlayers Total players hosted
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateQuickStats(int32 Streak, int32 TotalRooms, int32 TotalPlayers);

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Show compact version */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bCompactMode = false;

    /** Show tier badge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowTierBadge = true;

    /** Show level progress */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowLevelProgress = true;

    /** Show rating */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowRating = true;

    /** Show quick stats */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowQuickStats = true;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when profile card is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProfileCardClicked);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnProfileCardClicked OnProfileCardClicked;

    /** Called when tier badge is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTierBadgeClicked, EDeskillzHostTier, Tier);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnTierBadgeClicked OnTierBadgeClicked;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Avatar image */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UImage* AvatarImage;

    /** Username text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* UsernameText;

    /** Level title text (e.g., "Veteran Host") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* LevelTitleText;

    /** Level number text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* LevelNumberText;

    /** XP progress bar */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UProgressBar* XPProgressBar;

    /** XP progress text (e.g., "1,234 / 2,000 XP") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* XPProgressText;

    /** Esports tier icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UImage* EsportsTierIcon;

    /** Esports tier text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EsportsTierText;

    /** Social tier icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UImage* SocialTierIcon;

    /** Social tier text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* SocialTierText;

    /** Rating stars container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UHorizontalBox* RatingStarsContainer;

    /** Rating text (e.g., "4.8 (123 ratings)") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* RatingText;

    /** Verification badge */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UImage* VerificationBadge;

    /** Streak text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* StreakText;

    /** Total rooms text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* TotalRoomsText;

    /** Total players text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* TotalPlayersText;

    /** Card background border */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UBorder* CardBackground;

    /** Card button (for click handling) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* CardButton;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current profile data */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    FDeskillzHostProfile CurrentProfile;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Load avatar image async */
    void LoadAvatarImage(const FString& Url);

    /** Get tier color */
    FLinearColor GetTierColor(EDeskillzHostTier Tier) const;

    /** Get tier icon texture */
    UTexture2D* GetTierIconTexture(EDeskillzHostTier Tier) const;

    /** Update star display */
    void UpdateStarDisplay(float Rating);

    /** Handle card clicked */
    UFUNCTION()
    void OnCardClicked();
};