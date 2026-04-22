// Copyright Deskillz Games. All Rights Reserved.
// DeskillzWalletManager_v352.h - v3.5.2 Wallet & Profile Manager
// Path: Source/Deskillz/Public/DeskillzWalletManager_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes_v352.h"
#include "DeskillzWalletManager_v352.generated.h"

UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzWalletManager_v352 : public UObject
{
	GENERATED_BODY()

public:
	static UDeskillzWalletManager_v352* Get();

	/** Get all wallet balances (7.1) */
	void GetBalance(const FOnWalletBalances& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get balance for specific currency (7.2) */
	void GetBalanceForCurrency(const FString& Currency,
		const FOnWalletBalance& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Initiate deposit (7.3) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Wallet")
	void Deposit(const FString& Currency, double Amount,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Initiate withdrawal (7.4) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Wallet")
	void Withdraw(const FString& Currency, double Amount, const FString& WalletAddress,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get player stats (7.5) */
	void GetPlayerStats(const FOnPlayerStats& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get match history (7.6) */
	void GetMatchHistory(int32 Page, int32 Limit,
		const FOnMatchRecords& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get game leaderboard (7.7) */
	void GetGameLeaderboard(const FString& GameId, const FString& Period, int32 Limit,
		const FOnDeskillzLeaderboardReceived& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get transaction history (7.8) */
	void GetTransactions(int32 Limit, int32 Offset,
		const FString& Type, const FString& Currency,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Cached balances */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	TArray<FDeskillzWalletBalanceEntry> CachedBalances;

private:
	static FDeskillzError ParseError(const FString& Body, int32 StatusCode);
};