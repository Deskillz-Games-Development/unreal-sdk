// Copyright Deskillz Games. All Rights Reserved.
// DeskillzQuickPlayManager_v352.h - v3.5.2 Quick Play Manager
// Path: Source/Deskillz/Public/DeskillzQuickPlayManager_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes_v352.h"
#include "DeskillzQuickPlayManager_v352.generated.h"

UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzQuickPlayManager_v352 : public UObject
{
	GENERATED_BODY()

public:
	static UDeskillzQuickPlayManager_v352* Get();

	// ========================================================================
	// ESPORT QUICK PLAY (6.1 - 6.8)
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Deskillz|QuickPlay")
	void JoinQueue(const FDeskillzQuickPlayJoinParams& Params,
		const FOnQuickPlayJoinResult& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|QuickPlay")
	void LeaveQueue(const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	void GetConfig(const FString& GameId,
		const FOnQuickPlayConfig& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|QuickPlay")
	void LaunchMatch(const FString& MatchSessionId,
		const FOnQuickPlayLaunchData& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|QuickPlay")
	void SubmitScore(const FString& MatchId, int64 Score,
		const FOnQuickPlayScoreResult& OnSuccess, const FOnDeskillzApiError& OnError);

	void GetMatchResults(const FString& MatchId,
		const FOnQuickPlayMatchResult& OnSuccess, const FOnDeskillzApiError& OnError);

	UFUNCTION(BlueprintCallable, Category = "Deskillz|QuickPlay")
	void ForceCompleteMatch(const FString& MatchId,
		const FOnQuickPlayMatchResult& OnSuccess, const FOnDeskillzApiError& OnError);

	// ========================================================================
	// SOCIAL QUICK PLAY (6.9 - 6.11)
	// ========================================================================

	void CreateSocialRoom(const FString& GameId, double PointValueUsd,
		const FString& Currency, int32 SeatsPerTable,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	void SubmitSocialRound(const FString& RoomId, const FString& PayloadJson,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	void SocialRebuy(const FString& RoomId, double Amount,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	void SocialCashOut(const FString& RoomId,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	void EndSocialGame(const FString& RoomId,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	// ========================================================================
	// STATE
	// ========================================================================

	UFUNCTION(BlueprintPure, Category = "Deskillz|QuickPlay")
	bool IsInQueue() const { return bIsInQueue; }

	UFUNCTION(BlueprintPure, Category = "Deskillz|QuickPlay")
	FDeskillzQuickPlayLaunchData GetCurrentMatch() const { return CurrentMatch; }

	// ========================================================================
	// DELEGATES (6.12)
	// ========================================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueMatched, const FString&, MatchSessionId);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|QuickPlay")
	FOnQueueMatched OnQueueMatched;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQueueTimeout);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|QuickPlay")
	FOnQueueTimeout OnQueueTimeout;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickPlayMatchCompleted, const FString&, MatchId);
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|QuickPlay")
	FOnQuickPlayMatchCompleted OnMatchCompleted;

	// Socket event handlers
	void HandleQueueMatched(const FString& MatchSessionId);
	void HandleQueueTimeout();
	void HandleMatchCompleted(const FString& MatchId);

private:
	bool bIsInQueue = false;
	FDeskillzQuickPlayLaunchData CurrentMatch;

	static FDeskillzError ParseError(const FString& Body, int32 StatusCode);
};