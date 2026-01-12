// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostTypes.h"
#include "DeskillzHostEarningsWidget.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;
class UScrollBox;
class UHorizontalBox;
class UBorder;
class UWidgetSwitcher;

/**
 * Earnings time period
 */
UENUM(BlueprintType)
enum class EDeskillzEarningsPeriod : uint8
{
    Today       UMETA(DisplayName = "Today"),
    Week        UMETA(DisplayName = "This Week"),
    Month       UMETA(DisplayName = "This Month"),
    AllTime     UMETA(DisplayName = "All Time")
};

/**
 * Deskillz Host Earnings Widget
 * 
 * Displays host earnings including:
 * - Available and pending balances
 * - Earnings by time period
 * - Revenue breakdown (esports vs social)
 * - Transaction history
 * - Withdrawal functionality
 * - Earnings chart
 * 
 * Usage:
 * 1. Add to parent widget
 * 2. Call SetEarnings() with earnings data
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostEarningsWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostEarningsWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Data Binding
    // ========================================================================

    /**
     * Set earnings data
     * @param Earnings Host earnings summary
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetEarnings(const FDeskillzHostEarnings& Earnings);

    /**
     * Add transaction to history
     * @param Transaction New transaction
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void AddTransaction(const FDeskillzHostTransaction& Transaction);

    /**
     * Set transactions list
     * @param Transactions Transaction array
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetTransactions(const TArray<FDeskillzHostTransaction>& Transactions);

    /**
     * Update available balance
     * @param Amount New available amount
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdateAvailableBalance(double Amount);

    /**
     * Update pending balance
     * @param Amount New pending amount
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void UpdatePendingBalance(double Amount);

    /**
     * Set earnings period to display
     * @param Period Time period
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void SetPeriod(EDeskillzEarningsPeriod Period);

    // ========================================================================
    // Actions
    // ========================================================================

    /**
     * Open withdrawal modal
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void OpenWithdrawalModal();

    /**
     * Refresh earnings data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void RefreshEarnings();

    /**
     * Load more transactions
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|UI")
    void LoadMoreTransactions();

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Show withdrawal button */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowWithdrawButton = true;

    /** Show earnings chart */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowChart = true;

    /** Show revenue breakdown */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowBreakdown = true;

    /** Show transaction history */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    bool bShowHistory = true;

    /** Transactions per page */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|UI")
    int32 TransactionsPerPage = 20;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when withdraw clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWithdrawClicked);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnWithdrawClicked OnWithdrawClicked;

    /** Called when period changes */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPeriodChanged, EDeskillzEarningsPeriod, NewPeriod);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnPeriodChanged OnPeriodChanged;

    /** Called when transaction clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransactionClicked, const FDeskillzHostTransaction&, Transaction);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|UI")
    FOnTransactionClicked OnTransactionClicked;

protected:
    // ========================================================================
    // UI Components - Balances
    // ========================================================================

    /** Available balance text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* AvailableBalanceText;

    /** Available label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* AvailableLabelText;

    /** Pending balance text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* PendingBalanceText;

    /** Pending label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* PendingLabelText;

    /** Withdraw button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* WithdrawButton;

    // ========================================================================
    // UI Components - Period
    // ========================================================================

    /** Period buttons container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UHorizontalBox* PeriodButtonsContainer;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* TodayButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* WeekButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* MonthButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* AllTimeButton;

    /** Period earnings text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* PeriodEarningsText;

    // ========================================================================
    // UI Components - Breakdown
    // ========================================================================

    /** Breakdown container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UBorder* BreakdownContainer;

    /** Esports earnings text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EsportsEarningsText;

    /** Social earnings text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* SocialEarningsText;

    /** Bonus earnings text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* BonusEarningsText;

    // ========================================================================
    // UI Components - History
    // ========================================================================

    /** Transaction history container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UScrollBox* TransactionHistoryScroll;

    /** Transaction list */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UVerticalBox* TransactionList;

    /** Load more button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UButton* LoadMoreButton;

    /** Empty history text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|UI")
    UTextBlock* EmptyHistoryText;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current earnings data */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    FDeskillzHostEarnings CurrentEarnings;

    /** Transactions */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    TArray<FDeskillzHostTransaction> Transactions;

    /** Current period */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|UI")
    EDeskillzEarningsPeriod CurrentPeriod = EDeskillzEarningsPeriod::Month;

    /** Current page */
    int32 CurrentPage = 0;

    /** Has more transactions */
    bool bHasMoreTransactions = false;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Update balance display */
    void UpdateBalanceDisplay();

    /** Update period display */
    void UpdatePeriodDisplay();

    /** Update breakdown display */
    void UpdateBreakdownDisplay();

    /** Rebuild transaction list */
    void RebuildTransactionList();

    /** Create transaction item widget */
    UUserWidget* CreateTransactionItem(const FDeskillzHostTransaction& Transaction);

    /** Update period button states */
    void UpdatePeriodButtonStates();

    /** Get earnings for period */
    double GetEarningsForPeriod(EDeskillzEarningsPeriod Period) const;

    // Button handlers
    UFUNCTION()
    void OnWithdrawButtonClicked();

    UFUNCTION()
    void OnTodayClicked();

    UFUNCTION()
    void OnWeekClicked();

    UFUNCTION()
    void OnMonthClicked();

    UFUNCTION()
    void OnAllTimeClicked();

    UFUNCTION()
    void OnLoadMoreClicked();
};