// Copyright Deskillz Games. All Rights Reserved.
// DeskillzRoomExtensions_v352.h - v3.5.2 Room Extensions
// Path: Source/Deskillz/Public/DeskillzRoomExtensions_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes_v352.h"
#include "DeskillzRoomExtensions_v352.generated.h"

UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzRoomExtensions_v352 : public UObject
{
	GENERATED_BODY()

public:
	static UDeskillzRoomExtensions_v352* Get();

	// ========================================================================
	// CREATE WITH HOST ROLE (5.1, 5.2)
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void CreateEsportRoom(const FDeskillzCreateEsportRoomOpts& Opts,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void CreateSocialRoom(const FDeskillzCreateSocialRoomOpts& Opts,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	// ========================================================================
	// FINANCIAL (5.3 - 5.5)
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void BuyIn(const FString& RoomId, double Amount, const FString& Currency,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void CashOut(const FString& RoomId,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void Rebuy(const FString& RoomId, double Amount, const FString& Currency,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	// ========================================================================
	// ROUND & SETTLEMENT (5.6, 5.7)
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void SubmitRound(const FString& RoomId, const FString& PayloadJson,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void TriggerSettlement(const FString& RoomId,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	// ========================================================================
	// INVITES (5.8 - 5.10)
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void InvitePlayer(const FString& RoomId, const FString& TargetUsernameOrId,
		const FString& Message,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	void GetMyInvites(
		const TFunction<void(const TArray<FDeskillzRoomInvite>&)>& OnSuccess,
		const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void RespondToInvite(const FString& InviteId, bool bAccept,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	// ========================================================================
	// DELEGATES
	// ========================================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteReceived, const FDeskillzRoomInvite&, Invite);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms")
	FOnInviteReceived OnInviteReceived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuyInComplete, double, ChipBalance);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms")
	FOnBuyInComplete OnBuyInComplete;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCashOutComplete, double, Amount);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms")
	FOnCashOutComplete OnCashOutComplete;

	// Socket handler
	void HandleInviteReceived(const FDeskillzRoomInvite& Invite);

private:
	static FDeskillzError ParseError(const FString& Body, int32 StatusCode);
};