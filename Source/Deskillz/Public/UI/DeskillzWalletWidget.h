// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzWalletWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UScrollBox;
class UVerticalBox;
class UEditableTextBox;

/**
 * Wallet balance info
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzWalletBalance
{
	GENERATED_BODY()
	
	/** Currency symbol (BTC, ETH, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	FString Symbol;
	
	/** Currency full name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	FString Name;
	
	/** Available balance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	double Available = 0.0;
	
	/** Locked/pending balance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	double Locked = 0.0;
	
	/** USD equivalent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	double UsdValue = 0.0;
	
	/** Icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	UTexture2D* Icon = nullptr;
	
	/** 24h change percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wallet")
	float Change24h = 0.0f;
};

/** Delegate for wallet actions */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencySelected, const FString&, Symbol);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDepositRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWithdrawRequested);

/**
 * Wallet Widget
 * 
 * Displays cryptocurrency balances with:
 * - Multi-currency support (BTC, ETH, SOL, XRP, BNB, USDT, USDC)
 * - Balance with USD equivalent
 * - 24h change indicator
 * - Deposit/Withdraw actions
 * - Transaction history link
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzWalletWidget : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzWalletWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Refresh all balances from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void RefreshBalances();
	
	/**
	 * Get balance for specific currency
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	FDeskillzWalletBalance GetBalance(const FString& Symbol) const;
	
	/**
	 * Get total USD value
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	double GetTotalUsdValue() const;
	
	/**
	 * Set selected currency
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void SetSelectedCurrency(const FString& Symbol);
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when a currency is selected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnCurrencySelected OnCurrencySelected;
	
	/** Called when deposit is requested */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnDepositRequested OnDepositRequested;
	
	/** Called when withdraw is requested */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnWithdrawRequested OnWithdrawRequested;
	
protected:
	virtual void NativeConstruct() override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	virtual bool HandleBackButton_Implementation() override;
	
	// ========================================================================
	// UI Bindings
	// ========================================================================
	
	/** Title text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TitleText;
	
	/** Total balance in USD */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TotalBalanceText;
	
	/** Scroll container for currency list */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UScrollBox* CurrencyScrollBox;
	
	/** Container for currency rows */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UVerticalBox* CurrencyContainer;
	
	/** Selected currency icon */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* SelectedCurrencyIcon;
	
	/** Selected currency name */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* SelectedCurrencyText;
	
	/** Selected currency balance */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* SelectedBalanceText;
	
	/** Selected currency USD value */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* SelectedUsdText;
	
	/** Deposit button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* DepositButton;
	
	/** Withdraw button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* WithdrawButton;
	
	/** Transaction history button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* HistoryButton;
	
	/** Back button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* BackButton;
	
	/** Refresh button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* RefreshButton;
	
	/** Loading indicator */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* LoadingIndicator;
	
	/** Wallet address display */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* WalletAddressText;
	
	/** Copy address button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* CopyAddressButton;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Supported currencies */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TArray<FString> SupportedCurrencies = { TEXT("BTC"), TEXT("ETH"), TEXT("SOL"), TEXT("XRP"), TEXT("BNB"), TEXT("USDT"), TEXT("USDC") };
	
	/** Currency row widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TSubclassOf<UUserWidget> CurrencyRowClass;
	
	/** Auto-refresh interval (0 to disable) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	float AutoRefreshInterval = 60.0f;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Cached balances */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	TMap<FString, FDeskillzWalletBalance> Balances;
	
	/** Currently selected currency */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FString SelectedCurrency = TEXT("ETH");
	
	/** Is loading */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bIsLoading = false;
	
	/** User's wallet address */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FString WalletAddress;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Populate currency list */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void PopulateCurrencyList();
	
	/** Create currency row widget */
	UUserWidget* CreateCurrencyRow(const FDeskillzWalletBalance& Balance);
	
	/** Update selected currency display */
	void UpdateSelectedCurrencyDisplay();
	
	/** Update total balance display */
	void UpdateTotalBalance();
	
	/** Set loading state */
	void SetLoading(bool bLoading);
	
	/** Called when balances loaded */
	void OnBalancesLoaded(bool bSuccess, const TMap<FString, FDeskillzWalletBalance>& LoadedBalances);
	
	/** Handle currency row clicked */
	UFUNCTION()
	void OnCurrencyRowClicked(const FString& Symbol);
	
	/** Handle deposit clicked */
	UFUNCTION()
	void OnDepositClicked();
	
	/** Handle withdraw clicked */
	UFUNCTION()
	void OnWithdrawClicked();
	
	/** Handle history clicked */
	UFUNCTION()
	void OnHistoryClicked();
	
	/** Handle back clicked */
	UFUNCTION()
	void OnBackClicked();
	
	/** Handle refresh clicked */
	UFUNCTION()
	void OnRefreshClicked();
	
	/** Handle copy address clicked */
	UFUNCTION()
	void OnCopyAddressClicked();
	
	/** Start auto-refresh */
	void StartAutoRefresh();
	
	/** Stop auto-refresh */
	void StopAutoRefresh();
	
	/** Timer handle */
	FTimerHandle AutoRefreshTimerHandle;
};
