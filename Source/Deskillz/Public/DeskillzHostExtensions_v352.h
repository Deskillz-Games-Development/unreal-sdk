// Copyright Deskillz Games. All Rights Reserved.
// DeskillzHostExtensions_v352.h - v3.5.2 Host + Capabilities Extensions
// Path: Source/Deskillz/Public/DeskillzHostExtensions_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes_v352.h"
#include "DeskillzHostExtensions_v352.generated.h"

/** Host dashboard composite response */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostDashboard
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	double TotalEarnings = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	double MonthlyEarnings = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	double PendingSettlement = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	double AvailableWithdrawal = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	double EsportsEarnings = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	double SocialEarnings = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	FString EsportsTier;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	FString SocialTier;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	float EsportsHostShare = 15.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	float SocialHostShare = 15.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	int32 TotalPlayersHosted = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	int32 TotalRoomsCompleted = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	int32 ActiveRoomCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	int32 HostLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	FString HostTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	bool bIsVerified = false;
};

/** Host withdrawal response */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHostWithdrawalResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	FString TransactionId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	FString EstimatedArrival;
};

/** Age verification status */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzAgeVerificationStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host")
	bool bIsVerified = false;
};

// Delegates
DECLARE_DELEGATE_OneParam(FOnHostDashboard, const FDeskillzHostDashboard&);
DECLARE_DELEGATE_OneParam(FOnHostWithdrawal, const FDeskillzHostWithdrawalResponse&);
DECLARE_DELEGATE_OneParam(FOnAgeVerification, const FDeskillzAgeVerificationStatus&);

UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzHostExtensions_v352 : public UObject
{
	GENERATED_BODY()

public:
	static UDeskillzHostExtensions_v352* Get();

	/** Get full host dashboard (8.1) */
	void GetDashboard(const FOnHostDashboard& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Withdraw all available earnings (8.2) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
	void WithdrawAllEarnings(const FOnHostWithdrawal& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Request withdrawal with specific params (8.3) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Host")
	void RequestWithdrawal(double Amount, const FString& Currency, const FString& WalletAddress,
		const FOnHostWithdrawal& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Check age verification status (8.4) */
	void CheckAgeVerified(const FOnAgeVerification& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get game capabilities (9.1) */
	void GetGameCapabilities(const FString& GameId,
		const FOnGameCapabilities& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Cached capabilities */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	FDeskillzGameCapabilities CachedCapabilities;

	/** Whether capabilities have been fetched */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	bool bCapabilitiesFetched = false;

private:
	static FDeskillzError ParseError(const FString& Body, int32 StatusCode);
};