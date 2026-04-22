// Copyright Deskillz Games. All Rights Reserved.
// DeskillzDisputeManager_v352.h - v3.5.2 Dispute Manager
// Path: Source/Deskillz/Public/DeskillzDisputeManager_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes_v352.h"
#include "DeskillzDisputeManager_v352.generated.h"

/** File dispute parameters */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzFileDisputeParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Dispute")
	FString DisputeType; // TOURNAMENT, QUICK_PLAY, PRIVATE_ROOM

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Dispute")
	FString TournamentId;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Dispute")
	FString MatchId;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Dispute")
	FString RoomCode;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Dispute")
	FString Reason;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Dispute")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Dispute")
	TArray<FString> Evidence;
};

/** Last match context for auto-suggest */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzLastMatchContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString RoomCode;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString DisputeType;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString OpponentName;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString CompletedAt;

	bool IsValid() const { return !MatchId.IsEmpty(); }
};

UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzDisputeManager_v352 : public UObject
{
	GENERATED_BODY()

public:
	static UDeskillzDisputeManager_v352* Get();

	/** File a dispute (4.1) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Dispute")
	void FileDispute(const FDeskillzFileDisputeParams& Params,
		const FOnDisputeRecord& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get my disputes (4.2) */
	void GetMyDisputes(const FString& StatusFilter,
		const FOnDisputeRecords& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get dispute details (4.3) */
	void GetDisputeDetails(const FString& DisputeId,
		const FOnDisputeRecord& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Add evidence to dispute (4.4) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Dispute")
	void AddEvidence(const FString& DisputeId, const TArray<FString>& Evidence,
		const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Get recent matches for dispute context (4.5) */
	void GetRecentMatches(const FOnMatchRecords& OnSuccess, const FOnDeskillzApiError& OnError);

	/** Persist last match context locally (4.6) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Dispute")
	void PersistLastMatch(const FDeskillzLastMatchContext& Context);

	/** Get last match context (4.6) */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Dispute")
	FDeskillzLastMatchContext GetLastMatch() const;

	/** Clear last match (4.6) */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Dispute")
	void ClearLastMatch();

private:
	static FDeskillzDisputeRecord ParseDispute(const TSharedPtr<FJsonObject>& Json);
	static FDeskillzError ParseError(const FString& Body, int32 StatusCode);

	static const FString LastMatchSaveSlot;
};