// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostTypes.h"
#include "DeskillzHostDashboardWidget.generated.h"

class UDeskillzHostProfileCardWidget;
class UDeskillzHostTierProgressWidget;
class UDeskillzHostBadgeGridWidget;
class UDeskillzHostEarningsWidget;
class UDeskillzHostActiveRoomsWidget;
class UVerticalBox;
class UHorizontalBox;
class UScrollBox;
class UButton;
class UTextBlock;
class UImage;
class UWidgetSwitcher;

/**
 * Dashboard tab enum
 */
UENUM(BlueprintType)
enum class EDeskillzHostDashboardTab : uint8
{
    Overview    UMETA(DisplayName = "Overview"),
    Earnings    UMETA(DisplayName = "Earnings"),
    Rooms       UMETA(DisplayName = "Rooms"),
    Badges      UMETA(DisplayName = "Badges"),
    Settings    UMETA(DisplayName = "Settings")
};

/**
 * Deskillz Host Dashboard Widget
 * 
 * Main dashboard for host features including:
 * - Profile card with tier display
 * - Earnings summary and history
 * - Active rooms management
 * - Badge collection
 * - Tier progress tracking
 * 
 * Usage:
 * 1. Create widget via UI Manager or direct spawn
 * 2. Call RefreshData() to load latest host info
 * 3. Use tab system to navigate sections
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostDashboardWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostDashboardWidget(const FObjectInitializer& ObjectInitializer);

    // ========================================================================
    // Initialization
    // ========================================================================

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /**
     * Refresh all dashboard data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void RefreshData();

    /**
     * Set the host profile to display
     * @param Profile Host profile data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetProfile(const FDeskillzHostProfile& Profile);

    /**
     * Set earnings data
     * @param Earnings Earnings summary
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetEarnings(const FDeskillzHostEarnings& Earnings);

    /**
     * Set active rooms
     * @param Rooms Active room list
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetActiveRooms(const TArray<FDeskillzActiveRoomSummary>& Rooms);

    /**
     * Set host statistics
     * @param Stats Host stats
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetStats(const FDeskillzHostStats& Stats);

    // ========================================================================
    // Navigation
    // ========================================================================

    /**
     * Switch to a specific tab
     * @param Tab Tab to switch to
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SwitchToTab(EDeskillzHostDashboardTab Tab);

    /**
     * Get current active tab
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|UI")
    EDeskillzHostDashboardTab GetActiveTab() const { return ActiveTab; }

    // ========================================================================
    // Actions
    // ========================================================================

    /**
     * Open withdrawal dialog
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void OpenWithdrawalDialog();

    /**
     * Create a new room
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void CreateNewRoom();

    /**
     * View room details
     * @param RoomId Room to view
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void ViewRoomDetails(const FString& RoomId);

    /**
     * View badge details
     * @param Badge Badge to view
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void ViewBadgeDetails(const FDeskillzHostBadge& Badge);

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when tab changes */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabChanged, EDeskillzHostDashboardTab, NewTab);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnTabChanged OnTabChanged;

    /** Called when withdrawal requested */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWithdrawalRequested);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnWithdrawalRequested OnWithdrawalRequested;

    /** Called when create room requested */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateRoomRequested);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnCreateRoomRequested OnCreateRoomRequested;

protected:
    // ========================================================================
    // UI Components (Bind in Blueprint)
    // ========================================================================

    /** Profile card widget */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UDeskillzHostProfileCardWidget* ProfileCard;

    /** Tier progress widget */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UDeskillzHostTierProgressWidget* TierProgress;

    /** Badge grid widget */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UDeskillzHostBadgeGridWidget* BadgeGrid;

    /** Earnings widget */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UDeskillzHostEarningsWidget* EarningsPanel;

    /** Active rooms widget */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UDeskillzHostActiveRoomsWidget* ActiveRoomsPanel;

    /** Tab content switcher */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UWidgetSwitcher* TabContentSwitcher;

    /** Tab buttons container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UHorizontalBox* TabButtonsContainer;

    // Tab Buttons
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* OverviewTabButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* EarningsTabButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* RoomsTabButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* BadgesTabButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* SettingsTabButton;

    // Header elements
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* TitleText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* CloseButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* RefreshButton;

    // Quick stats on overview
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* AvailableBalanceText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* TodayEarningsText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* ActiveRoomsCountText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* WithdrawButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* CreateRoomButton;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current host profile */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    FDeskillzHostProfile CurrentProfile;

    /** Current earnings */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    FDeskillzHostEarnings CurrentEarnings;

    /** Current stats */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    FDeskillzHostStats CurrentStats;

    /** Active rooms */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    TArray<FDeskillzActiveRoomSummary> CurrentActiveRooms;

    /** Currently active tab */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    EDeskillzHostDashboardTab ActiveTab = EDeskillzHostDashboardTab::Overview;

    /** Is data loading */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    bool bIsLoading = false;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Setup event bindings */
    void SetupBindings();

    /** Update overview tab display */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateOverviewDisplay();

    /** Update tab button states */
    void UpdateTabButtonStates();

    /** Handle profile loaded */
    UFUNCTION()
    void OnProfileLoaded(const FDeskillzHostProfile& Profile, const FDeskillzError& Error);

    /** Handle earnings loaded */
    UFUNCTION()
    void OnEarningsLoaded(const FDeskillzHostEarnings& Earnings, const FDeskillzError& Error);

    /** Handle active rooms updated */
    UFUNCTION()
    void OnActiveRoomsUpdated(const TArray<FDeskillzActiveRoomSummary>& Rooms);

    // Button handlers
    UFUNCTION()
    void OnOverviewTabClicked();

    UFUNCTION()
    void OnEarningsTabClicked();

    UFUNCTION()
    void OnRoomsTabClicked();

    UFUNCTION()
    void OnBadgesTabClicked();

    UFUNCTION()
    void OnSettingsTabClicked();

    UFUNCTION()
    void OnCloseClicked();

    UFUNCTION()
    void OnRefreshClicked();

    UFUNCTION()
    void OnWithdrawClicked();

    UFUNCTION()
    void OnCreateRoomClicked();
};