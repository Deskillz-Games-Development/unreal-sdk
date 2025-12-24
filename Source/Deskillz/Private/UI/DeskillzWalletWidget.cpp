// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzWalletWidget.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "HAL/PlatformApplicationMisc.h"
#include "TimerManager.h"
#include "Engine/World.h"

UDeskillzWalletWidget::UDeskillzWalletWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzWalletWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup buttons
	if (DepositButton)
	{
		DepositButton->OnClicked.AddDynamic(this, &UDeskillzWalletWidget::OnDepositClicked);
	}
	
	if (WithdrawButton)
	{
		WithdrawButton->OnClicked.AddDynamic(this, &UDeskillzWalletWidget::OnWithdrawClicked);
	}
	
	if (HistoryButton)
	{
		HistoryButton->OnClicked.AddDynamic(this, &UDeskillzWalletWidget::OnHistoryClicked);
	}
	
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDeskillzWalletWidget::OnBackClicked);
	}
	
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UDeskillzWalletWidget::OnRefreshClicked);
	}
	
	if (CopyAddressButton)
	{
		CopyAddressButton->OnClicked.AddDynamic(this, &UDeskillzWalletWidget::OnCopyAddressClicked);
	}
	
	// Set title
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Wallet")));
	}
	
	// Start auto-refresh
	if (AutoRefreshInterval > 0.0f)
	{
		StartAutoRefresh();
	}
}

void UDeskillzWalletWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	if (TitleText)
	{
		TitleText->SetColorAndOpacity(Theme.TextColor);
	}
	
	if (TotalBalanceText)
	{
		TotalBalanceText->SetColorAndOpacity(Theme.PrimaryColor);
	}
	
	if (WalletAddressText)
	{
		WalletAddressText->SetColorAndOpacity(Theme.MutedTextColor);
	}
}

bool UDeskillzWalletWidget::HandleBackButton_Implementation()
{
	StopAutoRefresh();
	
	if (UIManager)
	{
		UIManager->HideWallet();
	}
	
	return true;
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzWalletWidget::RefreshBalances()
{
	if (bIsLoading)
	{
		return;
	}
	
	SetLoading(true);
	
	UDeskillzSDK* SDK = UDeskillzSDK::Get();
	if (!SDK)
	{
		SetLoading(false);
		return;
	}
	
	// In full implementation, call SDK to get balances
	// SDK->GetWalletBalances(...);
	
	// For now, simulate with test data
	TMap<FString, FDeskillzWalletBalance> TestBalances;
	
	// BTC
	FDeskillzWalletBalance Btc;
	Btc.Symbol = TEXT("BTC");
	Btc.Name = TEXT("Bitcoin");
	Btc.Available = 0.00234567;
	Btc.Locked = 0.0;
	Btc.UsdValue = Btc.Available * 42000.0;
	Btc.Change24h = 2.5f;
	TestBalances.Add(TEXT("BTC"), Btc);
	
	// ETH
	FDeskillzWalletBalance Eth;
	Eth.Symbol = TEXT("ETH");
	Eth.Name = TEXT("Ethereum");
	Eth.Available = 0.15678;
	Eth.Locked = 0.01;
	Eth.UsdValue = Eth.Available * 2200.0;
	Eth.Change24h = -1.2f;
	TestBalances.Add(TEXT("ETH"), Eth);
	
	// SOL
	FDeskillzWalletBalance Sol;
	Sol.Symbol = TEXT("SOL");
	Sol.Name = TEXT("Solana");
	Sol.Available = 5.234;
	Sol.Locked = 0.0;
	Sol.UsdValue = Sol.Available * 95.0;
	Sol.Change24h = 5.8f;
	TestBalances.Add(TEXT("SOL"), Sol);
	
	// XRP
	FDeskillzWalletBalance Xrp;
	Xrp.Symbol = TEXT("XRP");
	Xrp.Name = TEXT("Ripple");
	Xrp.Available = 150.0;
	Xrp.Locked = 0.0;
	Xrp.UsdValue = Xrp.Available * 0.55;
	Xrp.Change24h = 0.3f;
	TestBalances.Add(TEXT("XRP"), Xrp);
	
	// BNB
	FDeskillzWalletBalance Bnb;
	Bnb.Symbol = TEXT("BNB");
	Bnb.Name = TEXT("BNB");
	Bnb.Available = 0.5;
	Bnb.Locked = 0.0;
	Bnb.UsdValue = Bnb.Available * 310.0;
	Bnb.Change24h = -0.5f;
	TestBalances.Add(TEXT("BNB"), Bnb);
	
	// USDT
	FDeskillzWalletBalance Usdt;
	Usdt.Symbol = TEXT("USDT");
	Usdt.Name = TEXT("Tether");
	Usdt.Available = 25.50;
	Usdt.Locked = 0.0;
	Usdt.UsdValue = Usdt.Available;
	Usdt.Change24h = 0.0f;
	TestBalances.Add(TEXT("USDT"), Usdt);
	
	// USDC
	FDeskillzWalletBalance Usdc;
	Usdc.Symbol = TEXT("USDC");
	Usdc.Name = TEXT("USD Coin");
	Usdc.Available = 50.00;
	Usdc.Locked = 5.00;
	Usdc.UsdValue = Usdc.Available;
	Usdc.Change24h = 0.0f;
	TestBalances.Add(TEXT("USDC"), Usdc);
	
	// Simulate wallet address
	WalletAddress = TEXT("0x1234...5678");
	
	OnBalancesLoaded(true, TestBalances);
}

FDeskillzWalletBalance UDeskillzWalletWidget::GetBalance(const FString& Symbol) const
{
	if (const FDeskillzWalletBalance* Balance = Balances.Find(Symbol))
	{
		return *Balance;
	}
	return FDeskillzWalletBalance();
}

double UDeskillzWalletWidget::GetTotalUsdValue() const
{
	double Total = 0.0;
	for (const auto& Pair : Balances)
	{
		Total += Pair.Value.UsdValue;
	}
	return Total;
}

void UDeskillzWalletWidget::SetSelectedCurrency(const FString& Symbol)
{
	if (SelectedCurrency != Symbol && Balances.Contains(Symbol))
	{
		SelectedCurrency = Symbol;
		UpdateSelectedCurrencyDisplay();
		OnCurrencySelected.Broadcast(Symbol);
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzWalletWidget::PopulateCurrencyList()
{
	if (!CurrencyContainer)
	{
		return;
	}
	
	CurrencyContainer->ClearChildren();
	
	// Sort by USD value
	TArray<FDeskillzWalletBalance> SortedBalances;
	for (const auto& Pair : Balances)
	{
		SortedBalances.Add(Pair.Value);
	}
	
	SortedBalances.Sort([](const FDeskillzWalletBalance& A, const FDeskillzWalletBalance& B)
	{
		return A.UsdValue > B.UsdValue;
	});
	
	// Create rows
	for (const FDeskillzWalletBalance& Balance : SortedBalances)
	{
		UUserWidget* Row = CreateCurrencyRow(Balance);
		if (Row)
		{
			CurrencyContainer->AddChildToVerticalBox(Row);
		}
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Populated wallet with %d currencies"), SortedBalances.Num());
}

UUserWidget* UDeskillzWalletWidget::CreateCurrencyRow(const FDeskillzWalletBalance& Balance)
{
	// In full implementation, create from CurrencyRowClass
	// For now, return nullptr (use Blueprint implementation)
	return nullptr;
}

void UDeskillzWalletWidget::UpdateSelectedCurrencyDisplay()
{
	const FDeskillzWalletBalance* Balance = Balances.Find(SelectedCurrency);
	if (!Balance)
	{
		return;
	}
	
	if (SelectedCurrencyText)
	{
		SelectedCurrencyText->SetText(FText::FromString(Balance->Name + TEXT(" (") + Balance->Symbol + TEXT(")")));
	}
	
	if (SelectedBalanceText)
	{
		SelectedBalanceText->SetText(FText::FromString(FormatCurrency(Balance->Available, Balance->Symbol, 8)));
	}
	
	if (SelectedUsdText)
	{
		FString UsdStr = FString::Printf(TEXT("≈ $%.2f USD"), Balance->UsdValue);
		SelectedUsdText->SetText(FText::FromString(UsdStr));
	}
	
	if (SelectedCurrencyIcon && Balance->Icon)
	{
		SelectedCurrencyIcon->SetBrushFromTexture(Balance->Icon);
	}
}

void UDeskillzWalletWidget::UpdateTotalBalance()
{
	if (TotalBalanceText)
	{
		double Total = GetTotalUsdValue();
		FString TotalStr = FString::Printf(TEXT("$%.2f"), Total);
		TotalBalanceText->SetText(FText::FromString(TotalStr));
	}
}

void UDeskillzWalletWidget::SetLoading(bool bLoading)
{
	bIsLoading = bLoading;
	
	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (CurrencyScrollBox)
	{
		CurrencyScrollBox->SetVisibility(bLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	
	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(!bLoading);
	}
}

void UDeskillzWalletWidget::OnBalancesLoaded(bool bSuccess, const TMap<FString, FDeskillzWalletBalance>& LoadedBalances)
{
	SetLoading(false);
	
	if (bSuccess)
	{
		Balances = LoadedBalances;
		PopulateCurrencyList();
		UpdateTotalBalance();
		UpdateSelectedCurrencyDisplay();
		
		// Update wallet address display
		if (WalletAddressText)
		{
			WalletAddressText->SetText(FText::FromString(WalletAddress));
		}
		
		UE_LOG(LogDeskillz, Log, TEXT("Wallet balances loaded: %d currencies"), Balances.Num());
	}
	else
	{
		UE_LOG(LogDeskillz, Error, TEXT("Failed to load wallet balances"));
		
		if (UIManager)
		{
			UIManager->ShowToast(TEXT("Failed to load balances"), 3.0f, true);
		}
	}
}

void UDeskillzWalletWidget::OnCurrencyRowClicked(const FString& Symbol)
{
	PlayClickSound();
	SetSelectedCurrency(Symbol);
}

void UDeskillzWalletWidget::OnDepositClicked()
{
	PlayClickSound();
	OnDepositRequested.Broadcast();
	
	// In full implementation, show deposit modal
	if (UIManager)
	{
		UIManager->ShowPopup(
			TEXT("Deposit"),
			TEXT("Deposit functionality coming soon. Use your wallet address to receive funds."),
			false,
			TEXT("OK")
		);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Deposit requested for: %s"), *SelectedCurrency);
}

void UDeskillzWalletWidget::OnWithdrawClicked()
{
	PlayClickSound();
	OnWithdrawRequested.Broadcast();
	
	// In full implementation, show withdraw modal
	if (UIManager)
	{
		UIManager->ShowPopup(
			TEXT("Withdraw"),
			TEXT("Withdraw functionality coming soon. Your funds are secure."),
			false,
			TEXT("OK")
		);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Withdraw requested for: %s"), *SelectedCurrency);
}

void UDeskillzWalletWidget::OnHistoryClicked()
{
	PlayClickSound();
	
	// In full implementation, show transaction history
	if (UIManager)
	{
		UIManager->ShowPopup(
			TEXT("Transaction History"),
			TEXT("View your transaction history in the Deskillz web app."),
			false,
			TEXT("OK")
		);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Transaction history requested"));
}

void UDeskillzWalletWidget::OnBackClicked()
{
	PlayClickSound();
	HandleBackButton();
}

void UDeskillzWalletWidget::OnRefreshClicked()
{
	PlayClickSound();
	RefreshBalances();
}

void UDeskillzWalletWidget::OnCopyAddressClicked()
{
	PlayClickSound();
	
	// Copy to clipboard
	FPlatformApplicationMisc::ClipboardCopy(*WalletAddress);
	
	if (UIManager)
	{
		UIManager->ShowToast(TEXT("Address copied to clipboard"), 2.0f, false);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Wallet address copied: %s"), *WalletAddress);
}

void UDeskillzWalletWidget::StartAutoRefresh()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoRefreshTimerHandle,
			this,
			&UDeskillzWalletWidget::RefreshBalances,
			AutoRefreshInterval,
			true
		);
	}
}

void UDeskillzWalletWidget::StopAutoRefresh()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoRefreshTimerHandle);
	}
}
