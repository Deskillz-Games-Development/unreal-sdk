// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostTypes.h"
#include "DeskillzHostTierProgressWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UHorizontalBox;
class UBorder;
class UButton;

/**
 * Deskillz Host Tier Progress Widget
 * 
 * Displays tier progress for both Esports and Social revenue types:
 * - Current tier with icon
 * - Progress bar to next tier
 * - Required players count
 * - Days until tier evaluation
 * - Tier benefits comparison
 * 
 * Usage:
 * 1. Add to parent widget
 * 2. Call SetTierData() to update display
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostTierProgressWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostTierProgressWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Data Binding
    // ========================================================================

    /**
     * Set tier progress data
     * @param EsportsTier Current esports tier
     * @param SocialTier Current social tier
     * @param Stats Current host stats
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetTierData(EDeskillzHostTier EsportsTier, EDeskillzHostTier SocialTier, const FDeskillzHostStats& Stats);

    /**
     * Update esports tier progress
     * @param CurrentTier Current tier
     * @param UniquePlayersThisMonth Players this month
     * @param TierProgress Progress percentage (0-100)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateEsportsTierProgress(EDeskillzHostTier CurrentTier, int32 UniquePlayersThisMonth, float TierProgress);

    /**
     * Update social tier progress
     * @param CurrentTier Current tier
     * @param UniquePlayersThisMonth Players this month
     * @param TierProgress Progress percentage (0-100)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateSocialTierProgress(EDeskillzHostTier CurrentTier, int32 UniquePlayersThisMonth, float TierProgress);

    /**
     * Set days until tier evaluation
     * @param Days Days remaining
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetDaysUntilEvaluation(int32 Days);

    /**
     * Show tier details popup
     * @param Tier Tier to show details for
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void ShowTierDetails(EDeskillzHostTier Tier);

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Revenue type to display (or both) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowBothTypes = true;

    /** Show tier benefits comparison */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowBenefitsComparison = true;

    /** Show evaluation countdown */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowEvaluationCountdown = true;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when tier icon is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierClicked, EDeskillzHostTier, Tier, EDeskillzRoomRevenueType, RevenueType);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnTierClicked OnTierClicked;

    /** Called when info button is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInfoClicked);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnInfoClicked OnInfoClicked;

protected:
    // ========================================================================
    // UI Components - Esports
    // ========================================================================

    /** Esports section container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UBorder* EsportsContainer;

    /** Esports tier icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UImage* EsportsTierIcon;

    /** Esports tier name */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EsportsTierNameText;

    /** Esports share percentage */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EsportsShareText;

    /** Esports progress bar */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UProgressBar* EsportsProgressBar;

    /** Esports progress text (e.g., "150/200 players") */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EsportsProgressText;

    /** Esports next tier text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EsportsNextTierText;

    // ========================================================================
    // UI Components - Social
    // ========================================================================

    /** Social section container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UBorder* SocialContainer;

    /** Social tier icon */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UImage* SocialTierIcon;

    /** Social tier name */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* SocialTierNameText;

    /** Social share percentage */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* SocialShareText;

    /** Social progress bar */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UProgressBar* SocialProgressBar;

    /** Social progress text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* SocialProgressText;

    /** Social next tier text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* SocialNextTierText;

    // ========================================================================
    // UI Components - Evaluation
    // ========================================================================

    /** Evaluation countdown container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UBorder* EvaluationContainer;

    /** Days remaining text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* DaysRemainingText;

    /** Evaluation info text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EvaluationInfoText;

    /** Info button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* InfoButton;

    // ========================================================================
    // UI Components - Tier Ladder
    // ========================================================================

    /** Tier ladder container showing all tiers */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UHorizontalBox* TierLadderContainer;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current esports tier */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    EDeskillzHostTier CurrentEsportsTier = EDeskillzHostTier::Bronze;

    /** Current social tier */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    EDeskillzHostTier CurrentSocialTier = EDeskillzHostTier::Bronze;

    /** Days until evaluation */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    int32 DaysUntilEvaluation = 30;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Get tier share percentage */
    float GetTierSharePercent(EDeskillzHostTier Tier, EDeskillzRoomRevenueType RevenueType) const;

    /** Get next tier */
    EDeskillzHostTier GetNextTier(EDeskillzHostTier CurrentTier) const;

    /** Get required players for tier */
    int32 GetRequiredPlayers(EDeskillzHostTier Tier) const;

    /** Get tier color */
    FLinearColor GetTierColor(EDeskillzHostTier Tier) const;

    /** Update tier ladder display */
    void UpdateTierLadder();

    /** Handle esports tier clicked */
    UFUNCTION()
    void OnEsportsTierClicked();

    /** Handle social tier clicked */
    UFUNCTION()
    void OnSocialTierClicked();

    /** Handle info clicked */
    UFUNCTION()
    void OnInfoButtonClicked();
};