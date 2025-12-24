// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzEventTracker.generated.h"

class UDeskillzAnalytics;

/**
 * Timed event data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTimedEvent
{
	GENERATED_BODY()
	
	/** Event name */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	FString EventName;
	
	/** Start timestamp */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	int64 StartTime = 0;
	
	/** End timestamp */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	int64 EndTime = 0;
	
	/** Duration (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	float Duration = 0.0f;
	
	/** Is completed */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	bool bIsCompleted = false;
	
	/** Custom parameters */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	TMap<FString, FString> Parameters;
};

/**
 * Funnel step
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzFunnelStep
{
	GENERATED_BODY()
	
	/** Step name */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	FString StepName;
	
	/** Step index */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	int32 StepIndex = 0;
	
	/** Timestamp when reached */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	int64 Timestamp = 0;
	
	/** Time to reach from previous step (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	float TimeFromPrevious = 0.0f;
	
	/** Custom data */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	TMap<FString, FString> Data;
};

/**
 * Conversion funnel
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzFunnel
{
	GENERATED_BODY()
	
	/** Funnel name */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	FString FunnelName;
	
	/** Expected steps in order */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	TArray<FString> ExpectedSteps;
	
	/** Completed steps */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	TArray<FDeskillzFunnelStep> CompletedSteps;
	
	/** Start time */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	int64 StartTime = 0;
	
	/** Is completed */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	bool bIsCompleted = false;
	
	/** Is abandoned */
	UPROPERTY(BlueprintReadOnly, Category = "Events")
	bool bIsAbandoned = false;
};

/**
 * Revenue event data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzRevenueEvent
{
	GENERATED_BODY()
	
	/** Product/item ID */
	UPROPERTY(BlueprintReadWrite, Category = "Events")
	FString ProductId;
	
	/** Product name */
	UPROPERTY(BlueprintReadWrite, Category = "Events")
	FString ProductName;
	
	/** Revenue amount */
	UPROPERTY(BlueprintReadWrite, Category = "Events")
	double Amount = 0.0;
	
	/** Currency code */
	UPROPERTY(BlueprintReadWrite, Category = "Events")
	FString Currency = TEXT("USD");
	
	/** Quantity */
	UPROPERTY(BlueprintReadWrite, Category = "Events")
	int32 Quantity = 1;
	
	/** Transaction type */
	UPROPERTY(BlueprintReadWrite, Category = "Events")
	FString TransactionType = TEXT("purchase");
};

/**
 * Deskillz Event Tracker
 * 
 * Specialized event tracking utilities:
 * - Timed events (start/stop)
 * - Conversion funnels
 * - Revenue tracking
 * - Custom counters
 * - A/B test tracking
 * 
 * Usage:
 *   UDeskillzEventTracker* Tracker = UDeskillzEventTracker::Get();
 *   
 *   // Timed event
 *   Tracker->StartTimedEvent("level_1");
 *   // ... gameplay ...
 *   Tracker->EndTimedEvent("level_1");
 *   
 *   // Funnel
 *   Tracker->StartFunnel("registration", {"email", "password", "confirm"});
 *   Tracker->RecordFunnelStep("registration", "email");
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzEventTracker : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzEventTracker();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Event Tracker instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events", meta = (DisplayName = "Get Deskillz Event Tracker"))
	static UDeskillzEventTracker* Get();
	
	// ========================================================================
	// Timed Events
	// ========================================================================
	
	/**
	 * Start a timed event
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void StartTimedEvent(const FString& EventName, const TMap<FString, FString>& Parameters);
	
	/**
	 * End a timed event
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void EndTimedEvent(const FString& EventName, const TMap<FString, FString>& AdditionalParams);
	
	/**
	 * Cancel a timed event
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void CancelTimedEvent(const FString& EventName);
	
	/**
	 * Get active timed event duration
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	float GetTimedEventDuration(const FString& EventName) const;
	
	/**
	 * Is timed event active
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	bool IsTimedEventActive(const FString& EventName) const;
	
	// ========================================================================
	// Conversion Funnels
	// ========================================================================
	
	/**
	 * Start a conversion funnel
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void StartFunnel(const FString& FunnelName, const TArray<FString>& Steps);
	
	/**
	 * Record funnel step completion
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void RecordFunnelStep(const FString& FunnelName, const FString& StepName, 
		const TMap<FString, FString>& Data);
	
	/**
	 * Abandon funnel
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void AbandonFunnel(const FString& FunnelName, const FString& Reason);
	
	/**
	 * Get funnel progress
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	float GetFunnelProgress(const FString& FunnelName) const;
	
	/**
	 * Get current funnel step
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	FString GetCurrentFunnelStep(const FString& FunnelName) const;
	
	// ========================================================================
	// Revenue Tracking
	// ========================================================================
	
	/**
	 * Track revenue event
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackRevenue(const FDeskillzRevenueEvent& RevenueEvent);
	
	/**
	 * Track simple purchase
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackPurchase(const FString& ProductId, double Amount, const FString& Currency);
	
	/**
	 * Track entry fee payment
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackEntryFee(const FString& TournamentId, double Amount, const FString& Currency);
	
	/**
	 * Track prize payout
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackPrizePayout(const FString& MatchId, double Amount, const FString& Currency);
	
	// ========================================================================
	// Counters
	// ========================================================================
	
	/**
	 * Increment counter
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void IncrementCounter(const FString& CounterName, int32 Amount = 1);
	
	/**
	 * Set counter value
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void SetCounter(const FString& CounterName, int32 Value);
	
	/**
	 * Get counter value
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	int32 GetCounter(const FString& CounterName) const;
	
	/**
	 * Reset counter
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void ResetCounter(const FString& CounterName);
	
	/**
	 * Track counter (send current value to analytics)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackCounter(const FString& CounterName);
	
	// ========================================================================
	// A/B Testing
	// ========================================================================
	
	/**
	 * Set A/B test variant
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void SetABTestVariant(const FString& TestName, const FString& VariantName);
	
	/**
	 * Get A/B test variant
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	FString GetABTestVariant(const FString& TestName) const;
	
	/**
	 * Track A/B test conversion
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackABTestConversion(const FString& TestName, const FString& ConversionEvent);
	
	// ========================================================================
	// Gameplay Events
	// ========================================================================
	
	/**
	 * Track level start
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackLevelStart(const FString& LevelName, int32 Difficulty = 0);
	
	/**
	 * Track level complete
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackLevelComplete(const FString& LevelName, int64 Score, float Duration, int32 Stars = 0);
	
	/**
	 * Track level fail
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackLevelFail(const FString& LevelName, const FString& Reason, float Duration);
	
	/**
	 * Track achievement unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackAchievement(const FString& AchievementId, const FString& AchievementName);
	
	/**
	 * Track item used
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void TrackItemUsed(const FString& ItemId, const FString& ItemName, const FString& Context);
	
	// ========================================================================
	// Session Tracking
	// ========================================================================
	
	/**
	 * Get total session revenue
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	double GetSessionRevenue() const { return SessionRevenue; }
	
	/**
	 * Get session event count
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Events")
	int32 GetSessionEventCount() const { return SessionEventCount; }
	
	/**
	 * Clear session data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Events")
	void ClearSessionData();
	
protected:
	/** Analytics reference */
	UPROPERTY()
	UDeskillzAnalytics* Analytics;
	
	/** Active timed events */
	TMap<FString, FDeskillzTimedEvent> ActiveTimedEvents;
	
	/** Active funnels */
	TMap<FString, FDeskillzFunnel> ActiveFunnels;
	
	/** Counters */
	TMap<FString, int32> Counters;
	
	/** A/B test variants */
	TMap<FString, FString> ABTestVariants;
	
	/** Session revenue */
	double SessionRevenue = 0.0;
	
	/** Session event count */
	int32 SessionEventCount = 0;
	
	/** Get current timestamp */
	int64 GetCurrentTimestamp() const;
};
