// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostTypes.h"
#include "DeskillzHostBadgeGridWidget.generated.h"

class UUniformGridPanel;
class UScrollBox;
class UTextBlock;
class UHorizontalBox;
class UButton;

/**
 * Badge filter options
 */
UENUM(BlueprintType)
enum class EDeskillzBadgeFilter : uint8
{
    All             UMETA(DisplayName = "All"),
    Achievement     UMETA(DisplayName = "Achievement"),
    Performance     UMETA(DisplayName = "Performance"),
    Exclusive       UMETA(DisplayName = "Exclusive"),
    Active          UMETA(DisplayName = "Active Only"),
    Expired         UMETA(DisplayName = "Expired")
};

/**
 * Deskillz Host Badge Grid Widget
 * 
 * Displays host badges in a grid layout with:
 * - Badge icons and names
 * - Bonus percentages
 * - Earned/expiration dates
 * - Filter by category
 * - Total bonus calculation
 * 
 * Usage:
 * 1. Add to parent widget
 * 2. Call SetBadges() with badge array
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostBadgeGridWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostBadgeGridWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Data Binding
    // ========================================================================

    /**
     * Set badges to display
     * @param Badges Array of host badges
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetBadges(const TArray<FDeskillzHostBadge>& Badges);

    /**
     * Add a single badge (e.g., newly earned)
     * @param Badge Badge to add
     * @param bPlayAnimation Play earn animation
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void AddBadge(const FDeskillzHostBadge& Badge, bool bPlayAnimation = true);

    /**
     * Remove a badge
     * @param BadgeId Badge ID to remove
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void RemoveBadge(const FString& BadgeId);

    /**
     * Update badge (e.g., expiration)
     * @param Badge Updated badge data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateBadge(const FDeskillzHostBadge& Badge);

    /**
     * Set filter
     * @param Filter Filter to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetFilter(EDeskillzBadgeFilter Filter);

    /**
     * Get current filter
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|UI")
    EDeskillzBadgeFilter GetCurrentFilter() const { return CurrentFilter; }

    /**
     * Get total bonus from active badges
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|UI")
    float GetTotalBonus() const;

    /**
     * Get badge count
     * @param bActiveOnly Count only active badges
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|UI")
    int32 GetBadgeCount(bool bActiveOnly = false) const;

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Number of columns in grid */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    int32 GridColumns = 4;

    /** Show filter buttons */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowFilters = true;

    /** Show total bonus */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowTotalBonus = true;

    /** Show empty badge slots */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowEmptySlots = false;

    /** Badge item widget class */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    TSubclassOf<UUserWidget> BadgeItemWidgetClass;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when badge is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBadgeClicked, const FDeskillzHostBadge&, Badge);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnBadgeClicked OnBadgeClicked;

    /** Called when filter changes */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFilterChanged, EDeskillzBadgeFilter, NewFilter);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnFilterChanged OnFilterChanged;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Badge grid panel */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UUniformGridPanel* BadgeGrid;

    /** Scroll box container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UScrollBox* ScrollBox;

    /** Filter buttons container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UHorizontalBox* FilterButtonsContainer;

    /** Filter: All */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* FilterAllButton;

    /** Filter: Achievement */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* FilterAchievementButton;

    /** Filter: Performance */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* FilterPerformanceButton;

    /** Filter: Exclusive */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* FilterExclusiveButton;

    /** Total bonus text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* TotalBonusText;

    /** Badge count text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* BadgeCountText;

    /** Empty state text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EmptyStateText;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** All badges */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    TArray<FDeskillzHostBadge> AllBadges;

    /** Filtered badges */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    TArray<FDeskillzHostBadge> FilteredBadges;

    /** Current filter */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    EDeskillzBadgeFilter CurrentFilter = EDeskillzBadgeFilter::All;

    /** Badge item widgets */
    UPROPERTY()
    TArray<UUserWidget*> BadgeItemWidgets;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Rebuild badge grid */
    void RebuildGrid();

    /** Apply current filter */
    void ApplyFilter();

    /** Update summary text */
    void UpdateSummary();

    /** Create badge item widget */
    UUserWidget* CreateBadgeItem(const FDeskillzHostBadge& Badge);

    /** Handle badge item clicked */
    UFUNCTION()
    void OnBadgeItemClicked(const FDeskillzHostBadge& Badge);

    // Filter button handlers
    UFUNCTION()
    void OnFilterAllClicked();

    UFUNCTION()
    void OnFilterAchievementClicked();

    UFUNCTION()
    void OnFilterPerformanceClicked();

    UFUNCTION()
    void OnFilterExclusiveClicked();
};