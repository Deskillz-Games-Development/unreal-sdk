// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzMatchmaking.generated.h"

class UDeskillzSDK;
class IWebSocket;

// ============================================================================
// Matchmaking Status
// ============================================================================

/**
 * Matchmaking queue status
 */
UENUM(BlueprintType)
enum class EDeskillzMatchmakingStatus : uint8
{
	Idle             UMETA(DisplayName = "Idle"),
	Queued           UMETA(DisplayName = "Queued"),
	Searching        UMETA(DisplayName = "Searching"),
	Expanding        UMETA(DisplayName = "Expanding Search"),
	Found            UMETA(DisplayName = "Match Found"),
	Connecting       UMETA(DisplayName = "Connecting to Match"),
	Ready            UMETA(DisplayName = "Ready"),
	Cancelled        UMETA(DisplayName = "Cancelled"),
	Failed           UMETA(DisplayName = "Failed"),
	Timeout          UMETA(DisplayName = "Timeout")
};

/**
 * Matchmaking search parameters
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchmakingParams
{
	GENERATED_BODY()
	
	/** Tournament to match in */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	FString TournamentId;
	
	/** Preferred match type (can be overridden by tournament) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	EDeskillzMatchType PreferredMatchType = EDeskillzMatchType::Asynchronous;
	
	/** Rating range tolerance (0 = exact match) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	int32 RatingTolerance = 200;
	
	/** Maximum wait time before expanding search (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	float ExpandSearchAfter = 15.0f;
	
	/** Maximum total wait time (seconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	float MaxWaitTime = 120.0f;
	
	/** Allow bot match if no opponent found */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	bool bAllowBotMatch = false;
	
	/** Region preference (empty = any) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	FString PreferredRegion;
	
	/** Custom match data to pass */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Matchmaking")
	TMap<FString, FString> CustomData;
	
	FDeskillzMatchmakingParams() = default;
};

/**
 * Matchmaking queue info
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQueueInfo
{
	GENERATED_BODY()
	
	/** Current status */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	EDeskillzMatchmakingStatus Status = EDeskillzMatchmakingStatus::Idle;
	
	/** Time in queue (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	float WaitTime = 0.0f;
	
	/** Estimated wait time (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	float EstimatedWaitTime = 30.0f;
	
	/** Number of players searching in this queue */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	int32 PlayersInQueue = 0;
	
	/** Queue position (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	int32 QueuePosition = 0;
	
	/** Current rating range being searched */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	int32 CurrentRatingRange = 0;
	
	/** Has search been expanded */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	bool bSearchExpanded = false;
	
	/** Queue ticket ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	FString TicketId;
	
	FDeskillzQueueInfo() = default;
};

/**
 * Potential opponent preview (shown before confirming match)
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzOpponentPreview
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	FDeskillzPlayer Player;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	int32 RatingDifference = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	float WinProbability = 0.5f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	int32 RecentFormWins = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	int32 RecentFormLosses = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Matchmaking")
	bool bIsBot = false;
	
	FDeskillzOpponentPreview() = default;
};

// ============================================================================
// Matchmaking Delegates
// ============================================================================

/** Called when matchmaking status changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchmakingStatusChanged, EDeskillzMatchmakingStatus, OldStatus, EDeskillzMatchmakingStatus, NewStatus);

/** Called with queue updates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingQueueUpdate, const FDeskillzQueueInfo&, QueueInfo);

/** Called when opponent found */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingOpponentFound, const FDeskillzOpponentPreview&, Opponent);

/** Called when match is ready to start */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingMatchReady, const FDeskillzMatchInfo&, MatchInfo);

/** Called when matchmaking fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingFailed, const FDeskillzError&, Error);

/** Called when matchmaking times out */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingTimeout, float, TotalWaitTime);

// ============================================================================
// Matchmaking Manager
// ============================================================================

/**
 * Deskillz Matchmaking Manager
 * 
 * Handles matchmaking queue, opponent finding, and match creation.
 * Supports both synchronous (real-time) and asynchronous (turn-based) matching.
 * 
 * Features:
 * - Rating-based matchmaking with tolerance expansion
 * - Region-aware matching
 * - Estimated wait times
 * - Queue position tracking
 * - Bot fallback option
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzMatchmaking : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzMatchmaking();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize matchmaking with SDK reference
	 */
	void Initialize(UDeskillzSDK* SDK);
	
	/**
	 * Cleanup matchmaking
	 */
	void Cleanup();
	
	// ========================================================================
	// Static Factory
	// ========================================================================
	
	/**
	 * Create and initialize a matchmaking instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Matchmaking", meta = (WorldContext = "WorldContextObject"))
	static UDeskillzMatchmaking* CreateMatchmaking(const UObject* WorldContextObject);
	
	// ========================================================================
	// Queue Management
	// ========================================================================
	
	/**
	 * Join matchmaking queue
	 * @param Params Matchmaking parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Matchmaking")
	void JoinQueue(const FDeskillzMatchmakingParams& Params);
	
	/**
	 * Join queue with just tournament ID (uses default params)
	 * @param TournamentId Tournament to find match in
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Matchmaking")
	void JoinQueueSimple(const FString& TournamentId);
	
	/**
	 * Leave matchmaking queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Matchmaking")
	void LeaveQueue();
	
	/**
	 * Accept found match
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Matchmaking")
	void AcceptMatch();
	
	/**
	 * Decline found match (return to queue)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Matchmaking")
	void DeclineMatch();
	
	// ========================================================================
	// Status & Info
	// ========================================================================
	
	/**
	 * Get current matchmaking status
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Matchmaking")
	EDeskillzMatchmakingStatus GetStatus() const { return CurrentStatus; }
	
	/**
	 * Get current queue info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Matchmaking")
	FDeskillzQueueInfo GetQueueInfo() const { return QueueInfo; }
	
	/**
	 * Get found opponent preview (if status is Found)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Matchmaking")
	FDeskillzOpponentPreview GetOpponentPreview() const { return FoundOpponent; }
	
	/**
	 * Get ready match info (if status is Ready)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Matchmaking")
	FDeskillzMatchInfo GetMatchInfo() const { return ReadyMatch; }
	
	/**
	 * Check if currently in queue
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Matchmaking")
	bool IsInQueue() const;
	
	/**
	 * Check if match has been found
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Matchmaking")
	bool IsMatchFound() const { return CurrentStatus == EDeskillzMatchmakingStatus::Found; }
	
	/**
	 * Get time spent in queue
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Matchmaking")
	float GetWaitTime() const;
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Matchmaking status changed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Matchmaking|Events")
	FOnMatchmakingStatusChanged OnStatusChanged;
	
	/** Queue info updated */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Matchmaking|Events")
	FOnMatchmakingQueueUpdate OnQueueUpdate;
	
	/** Opponent found */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Matchmaking|Events")
	FOnMatchmakingOpponentFound OnOpponentFound;
	
	/** Match ready to start */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Matchmaking|Events")
	FOnMatchmakingMatchReady OnMatchReady;
	
	/** Matchmaking failed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Matchmaking|Events")
	FOnMatchmakingFailed OnFailed;
	
	/** Matchmaking timed out */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Matchmaking|Events")
	FOnMatchmakingTimeout OnTimeout;
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Current status */
	UPROPERTY()
	EDeskillzMatchmakingStatus CurrentStatus = EDeskillzMatchmakingStatus::Idle;
	
	/** Current queue info */
	UPROPERTY()
	FDeskillzQueueInfo QueueInfo;
	
	/** Found opponent */
	UPROPERTY()
	FDeskillzOpponentPreview FoundOpponent;
	
	/** Ready match */
	UPROPERTY()
	FDeskillzMatchInfo ReadyMatch;
	
	/** Current search params */
	FDeskillzMatchmakingParams CurrentParams;
	
	/** Queue join time */
	FDateTime QueueJoinTime;
	
	/** SDK reference */
	UPROPERTY()
	TWeakObjectPtr<UDeskillzSDK> SDKRef;
	
	/** Timer handles */
	FTimerHandle QueueTickHandle;
	FTimerHandle TimeoutHandle;
	FTimerHandle ExpandHandle;
	
	/** World reference for timers */
	TWeakObjectPtr<UWorld> WorldRef;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Transition to new status */
	void TransitionToStatus(EDeskillzMatchmakingStatus NewStatus);
	
	/** Queue tick - update wait time and check expansion */
	void QueueTick();
	
	/** Expand search range */
	void ExpandSearch();
	
	/** Handle queue timeout */
	void OnQueueTimeout();
	
	/** Handle WebSocket message */
	void HandleWebSocketMessage(const FString& MessageType, TSharedPtr<FJsonObject> Data);
	
	/** Parse opponent from JSON */
	FDeskillzOpponentPreview ParseOpponent(TSharedPtr<FJsonObject> Data);
	
	/** Parse match from JSON */
	FDeskillzMatchInfo ParseMatch(TSharedPtr<FJsonObject> Data);
	
	/** Reset state */
	void Reset();
	
	/** Clear timers */
	void ClearTimers();
	
	/** Calculate estimated wait time */
	float CalculateEstimatedWait() const;
	
	/** Calculate win probability against opponent */
	float CalculateWinProbability(int32 PlayerRating, int32 OpponentRating) const;
};
