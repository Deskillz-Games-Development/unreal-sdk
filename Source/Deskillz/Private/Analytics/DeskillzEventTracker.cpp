// Copyright Deskillz Games. All Rights Reserved.

#include "Analytics/DeskillzEventTracker.h"
#include "Analytics/DeskillzAnalytics.h"
#include "Deskillz.h"

// Static singleton
static UDeskillzEventTracker* GEventTracker = nullptr;

UDeskillzEventTracker::UDeskillzEventTracker()
{
	Analytics = UDeskillzAnalytics::Get();
}

UDeskillzEventTracker* UDeskillzEventTracker::Get()
{
	if (!GEventTracker)
	{
		GEventTracker = NewObject<UDeskillzEventTracker>();
		GEventTracker->AddToRoot();
	}
	return GEventTracker;
}

// ============================================================================
// Timed Events
// ============================================================================

void UDeskillzEventTracker::StartTimedEvent(const FString& EventName, const TMap<FString, FString>& Parameters)
{
	if (ActiveTimedEvents.Contains(EventName))
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Timed event already active: %s"), *EventName);
		return;
	}
	
	FDeskillzTimedEvent TimedEvent;
	TimedEvent.EventName = EventName;
	TimedEvent.StartTime = GetCurrentTimestamp();
	TimedEvent.Parameters = Parameters;
	
	ActiveTimedEvents.Add(EventName, TimedEvent);
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Started timed event: %s"), *EventName);
}

void UDeskillzEventTracker::EndTimedEvent(const FString& EventName, const TMap<FString, FString>& AdditionalParams)
{
	FDeskillzTimedEvent* TimedEvent = ActiveTimedEvents.Find(EventName);
	if (!TimedEvent)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No active timed event: %s"), *EventName);
		return;
	}
	
	TimedEvent->EndTime = GetCurrentTimestamp();
	TimedEvent->Duration = static_cast<float>(TimedEvent->EndTime - TimedEvent->StartTime) / 1000.0f;
	TimedEvent->bIsCompleted = true;
	
	// Merge additional parameters
	for (const auto& Pair : AdditionalParams)
	{
		TimedEvent->Parameters.Add(Pair.Key, Pair.Value);
	}
	
	// Track to analytics
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(EventName + TEXT("_completed"), EDeskillzEventCategory::Game);
		Event.Parameters = TimedEvent->Parameters;
		Event.AddValue(TEXT("duration_seconds"), TimedEvent->Duration);
		
		Analytics->TrackEvent(Event);
	}
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Ended timed event: %s (%.2fs)"), *EventName, TimedEvent->Duration);
	
	ActiveTimedEvents.Remove(EventName);
	SessionEventCount++;
}

void UDeskillzEventTracker::CancelTimedEvent(const FString& EventName)
{
	if (ActiveTimedEvents.Remove(EventName) > 0)
	{
		UE_LOG(LogDeskillz, Verbose, TEXT("Cancelled timed event: %s"), *EventName);
	}
}

float UDeskillzEventTracker::GetTimedEventDuration(const FString& EventName) const
{
	const FDeskillzTimedEvent* TimedEvent = ActiveTimedEvents.Find(EventName);
	if (!TimedEvent)
	{
		return 0.0f;
	}
	
	int64 CurrentTime = GetCurrentTimestamp();
	return static_cast<float>(CurrentTime - TimedEvent->StartTime) / 1000.0f;
}

bool UDeskillzEventTracker::IsTimedEventActive(const FString& EventName) const
{
	return ActiveTimedEvents.Contains(EventName);
}

// ============================================================================
// Conversion Funnels
// ============================================================================

void UDeskillzEventTracker::StartFunnel(const FString& FunnelName, const TArray<FString>& Steps)
{
	if (ActiveFunnels.Contains(FunnelName))
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Funnel already active: %s"), *FunnelName);
		return;
	}
	
	FDeskillzFunnel Funnel;
	Funnel.FunnelName = FunnelName;
	Funnel.ExpectedSteps = Steps;
	Funnel.StartTime = GetCurrentTimestamp();
	
	ActiveFunnels.Add(FunnelName, Funnel);
	
	// Track funnel start
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("funnel_started"), EDeskillzEventCategory::User);
		Event.AddParam(TEXT("funnel"), FunnelName);
		Event.AddValue(TEXT("total_steps"), static_cast<double>(Steps.Num()));
		
		Analytics->TrackEvent(Event);
	}
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Started funnel: %s with %d steps"), *FunnelName, Steps.Num());
}

void UDeskillzEventTracker::RecordFunnelStep(const FString& FunnelName, const FString& StepName,
	const TMap<FString, FString>& Data)
{
	FDeskillzFunnel* Funnel = ActiveFunnels.Find(FunnelName);
	if (!Funnel)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No active funnel: %s"), *FunnelName);
		return;
	}
	
	// Find step index
	int32 StepIndex = Funnel->ExpectedSteps.IndexOfByKey(StepName);
	if (StepIndex == INDEX_NONE)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Unknown step '%s' in funnel '%s'"), *StepName, *FunnelName);
		return;
	}
	
	// Calculate time from previous step
	int64 CurrentTime = GetCurrentTimestamp();
	float TimeFromPrevious = 0.0f;
	if (Funnel->CompletedSteps.Num() > 0)
	{
		TimeFromPrevious = static_cast<float>(CurrentTime - Funnel->CompletedSteps.Last().Timestamp) / 1000.0f;
	}
	else
	{
		TimeFromPrevious = static_cast<float>(CurrentTime - Funnel->StartTime) / 1000.0f;
	}
	
	// Create step record
	FDeskillzFunnelStep Step;
	Step.StepName = StepName;
	Step.StepIndex = StepIndex;
	Step.Timestamp = CurrentTime;
	Step.TimeFromPrevious = TimeFromPrevious;
	Step.Data = Data;
	
	Funnel->CompletedSteps.Add(Step);
	
	// Track step completion
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("funnel_step"), EDeskillzEventCategory::User);
		Event.AddParam(TEXT("funnel"), FunnelName);
		Event.AddParam(TEXT("step"), StepName);
		Event.AddValue(TEXT("step_index"), static_cast<double>(StepIndex));
		Event.AddValue(TEXT("time_from_previous"), TimeFromPrevious);
		Event.AddValue(TEXT("progress"), GetFunnelProgress(FunnelName));
		
		for (const auto& Pair : Data)
		{
			Event.AddParam(Pair.Key, Pair.Value);
		}
		
		Analytics->TrackEvent(Event);
	}
	
	// Check if funnel is complete
	if (Funnel->CompletedSteps.Num() == Funnel->ExpectedSteps.Num())
	{
		Funnel->bIsCompleted = true;
		
		float TotalTime = static_cast<float>(CurrentTime - Funnel->StartTime) / 1000.0f;
		
		if (Analytics && Analytics->IsEnabled())
		{
			FDeskillzAnalyticsEvent Event(TEXT("funnel_completed"), EDeskillzEventCategory::User);
			Event.AddParam(TEXT("funnel"), FunnelName);
			Event.AddValue(TEXT("total_time"), TotalTime);
			Event.AddValue(TEXT("steps_completed"), static_cast<double>(Funnel->CompletedSteps.Num()));
			
			Analytics->TrackEvent(Event);
		}
		
		UE_LOG(LogDeskillz, Log, TEXT("Funnel completed: %s (%.2fs)"), *FunnelName, TotalTime);
		ActiveFunnels.Remove(FunnelName);
	}
	
	SessionEventCount++;
}

void UDeskillzEventTracker::AbandonFunnel(const FString& FunnelName, const FString& Reason)
{
	FDeskillzFunnel* Funnel = ActiveFunnels.Find(FunnelName);
	if (!Funnel)
	{
		return;
	}
	
	Funnel->bIsAbandoned = true;
	
	// Track abandonment
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("funnel_abandoned"), EDeskillzEventCategory::User);
		Event.AddParam(TEXT("funnel"), FunnelName);
		Event.AddParam(TEXT("reason"), Reason);
		Event.AddParam(TEXT("last_step"), GetCurrentFunnelStep(FunnelName));
		Event.AddValue(TEXT("progress"), GetFunnelProgress(FunnelName));
		Event.AddValue(TEXT("steps_completed"), static_cast<double>(Funnel->CompletedSteps.Num()));
		
		Analytics->TrackEvent(Event);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Funnel abandoned: %s (reason: %s)"), *FunnelName, *Reason);
	ActiveFunnels.Remove(FunnelName);
}

float UDeskillzEventTracker::GetFunnelProgress(const FString& FunnelName) const
{
	const FDeskillzFunnel* Funnel = ActiveFunnels.Find(FunnelName);
	if (!Funnel || Funnel->ExpectedSteps.Num() == 0)
	{
		return 0.0f;
	}
	
	return static_cast<float>(Funnel->CompletedSteps.Num()) / static_cast<float>(Funnel->ExpectedSteps.Num());
}

FString UDeskillzEventTracker::GetCurrentFunnelStep(const FString& FunnelName) const
{
	const FDeskillzFunnel* Funnel = ActiveFunnels.Find(FunnelName);
	if (!Funnel || Funnel->CompletedSteps.Num() == 0)
	{
		return TEXT("");
	}
	
	return Funnel->CompletedSteps.Last().StepName;
}

// ============================================================================
// Revenue Tracking
// ============================================================================

void UDeskillzEventTracker::TrackRevenue(const FDeskillzRevenueEvent& RevenueEvent)
{
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("revenue"), EDeskillzEventCategory::Wallet);
		Event.AddParam(TEXT("product_id"), RevenueEvent.ProductId);
		Event.AddParam(TEXT("product_name"), RevenueEvent.ProductName);
		Event.AddParam(TEXT("currency"), RevenueEvent.Currency);
		Event.AddParam(TEXT("transaction_type"), RevenueEvent.TransactionType);
		Event.AddValue(TEXT("amount"), RevenueEvent.Amount);
		Event.AddValue(TEXT("quantity"), static_cast<double>(RevenueEvent.Quantity));
		
		Analytics->TrackEvent(Event);
	}
	
	SessionRevenue += RevenueEvent.Amount;
	SessionEventCount++;
	
	UE_LOG(LogDeskillz, Log, TEXT("Revenue tracked: %.2f %s (%s)"), 
		RevenueEvent.Amount, *RevenueEvent.Currency, *RevenueEvent.ProductId);
}

void UDeskillzEventTracker::TrackPurchase(const FString& ProductId, double Amount, const FString& Currency)
{
	FDeskillzRevenueEvent Event;
	Event.ProductId = ProductId;
	Event.Amount = Amount;
	Event.Currency = Currency;
	Event.TransactionType = TEXT("purchase");
	
	TrackRevenue(Event);
}

void UDeskillzEventTracker::TrackEntryFee(const FString& TournamentId, double Amount, const FString& Currency)
{
	FDeskillzRevenueEvent Event;
	Event.ProductId = TournamentId;
	Event.ProductName = TEXT("Tournament Entry");
	Event.Amount = Amount;
	Event.Currency = Currency;
	Event.TransactionType = TEXT("entry_fee");
	
	TrackRevenue(Event);
}

void UDeskillzEventTracker::TrackPrizePayout(const FString& MatchId, double Amount, const FString& Currency)
{
	FDeskillzRevenueEvent Event;
	Event.ProductId = MatchId;
	Event.ProductName = TEXT("Prize Payout");
	Event.Amount = -Amount; // Negative for payouts
	Event.Currency = Currency;
	Event.TransactionType = TEXT("prize_payout");
	
	TrackRevenue(Event);
}

// ============================================================================
// Counters
// ============================================================================

void UDeskillzEventTracker::IncrementCounter(const FString& CounterName, int32 Amount)
{
	int32& Counter = Counters.FindOrAdd(CounterName);
	Counter += Amount;
}

void UDeskillzEventTracker::SetCounter(const FString& CounterName, int32 Value)
{
	Counters.Add(CounterName, Value);
}

int32 UDeskillzEventTracker::GetCounter(const FString& CounterName) const
{
	const int32* Counter = Counters.Find(CounterName);
	return Counter ? *Counter : 0;
}

void UDeskillzEventTracker::ResetCounter(const FString& CounterName)
{
	Counters.Remove(CounterName);
}

void UDeskillzEventTracker::TrackCounter(const FString& CounterName)
{
	int32 Value = GetCounter(CounterName);
	
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("counter"), EDeskillzEventCategory::Game);
		Event.AddParam(TEXT("counter_name"), CounterName);
		Event.AddValue(TEXT("value"), static_cast<double>(Value));
		
		Analytics->TrackEvent(Event);
	}
	
	SessionEventCount++;
}

// ============================================================================
// A/B Testing
// ============================================================================

void UDeskillzEventTracker::SetABTestVariant(const FString& TestName, const FString& VariantName)
{
	ABTestVariants.Add(TestName, VariantName);
	
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("ab_test_assigned"), EDeskillzEventCategory::System);
		Event.AddParam(TEXT("test_name"), TestName);
		Event.AddParam(TEXT("variant"), VariantName);
		
		Analytics->TrackEvent(Event);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("A/B test: %s = %s"), *TestName, *VariantName);
}

FString UDeskillzEventTracker::GetABTestVariant(const FString& TestName) const
{
	const FString* Variant = ABTestVariants.Find(TestName);
	return Variant ? *Variant : TEXT("");
}

void UDeskillzEventTracker::TrackABTestConversion(const FString& TestName, const FString& ConversionEvent)
{
	FString Variant = GetABTestVariant(TestName);
	if (Variant.IsEmpty())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No variant assigned for test: %s"), *TestName);
		return;
	}
	
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("ab_test_conversion"), EDeskillzEventCategory::System);
		Event.AddParam(TEXT("test_name"), TestName);
		Event.AddParam(TEXT("variant"), Variant);
		Event.AddParam(TEXT("conversion_event"), ConversionEvent);
		
		Analytics->TrackEvent(Event);
	}
	
	SessionEventCount++;
}

// ============================================================================
// Gameplay Events
// ============================================================================

void UDeskillzEventTracker::TrackLevelStart(const FString& LevelName, int32 Difficulty)
{
	// Start timed event for level
	TMap<FString, FString> Params;
	Params.Add(TEXT("level_name"), LevelName);
	Params.Add(TEXT("difficulty"), FString::FromInt(Difficulty));
	
	StartTimedEvent(TEXT("level_") + LevelName, Params);
	
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("level_start"), EDeskillzEventCategory::Game);
		Event.AddParam(TEXT("level_name"), LevelName);
		Event.AddValue(TEXT("difficulty"), static_cast<double>(Difficulty));
		
		Analytics->TrackEvent(Event);
	}
	
	SessionEventCount++;
}

void UDeskillzEventTracker::TrackLevelComplete(const FString& LevelName, int64 Score, float Duration, int32 Stars)
{
	// End timed event
	TMap<FString, FString> Params;
	Params.Add(TEXT("score"), FString::Printf(TEXT("%lld"), Score));
	Params.Add(TEXT("stars"), FString::FromInt(Stars));
	
	EndTimedEvent(TEXT("level_") + LevelName, Params);
	
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("level_complete"), EDeskillzEventCategory::Game);
		Event.AddParam(TEXT("level_name"), LevelName);
		Event.AddValue(TEXT("score"), static_cast<double>(Score));
		Event.AddValue(TEXT("duration"), Duration);
		Event.AddValue(TEXT("stars"), static_cast<double>(Stars));
		
		Analytics->TrackEvent(Event);
	}
	
	IncrementCounter(TEXT("levels_completed"));
	SessionEventCount++;
}

void UDeskillzEventTracker::TrackLevelFail(const FString& LevelName, const FString& Reason, float Duration)
{
	// Cancel timed event
	CancelTimedEvent(TEXT("level_") + LevelName);
	
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("level_fail"), EDeskillzEventCategory::Game);
		Event.AddParam(TEXT("level_name"), LevelName);
		Event.AddParam(TEXT("reason"), Reason);
		Event.AddValue(TEXT("duration"), Duration);
		
		Analytics->TrackEvent(Event);
	}
	
	IncrementCounter(TEXT("levels_failed"));
	SessionEventCount++;
}

void UDeskillzEventTracker::TrackAchievement(const FString& AchievementId, const FString& AchievementName)
{
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("achievement_unlocked"), EDeskillzEventCategory::Game);
		Event.AddParam(TEXT("achievement_id"), AchievementId);
		Event.AddParam(TEXT("achievement_name"), AchievementName);
		
		Analytics->TrackEvent(Event);
	}
	
	IncrementCounter(TEXT("achievements_unlocked"));
	SessionEventCount++;
}

void UDeskillzEventTracker::TrackItemUsed(const FString& ItemId, const FString& ItemName, const FString& Context)
{
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("item_used"), EDeskillzEventCategory::Game);
		Event.AddParam(TEXT("item_id"), ItemId);
		Event.AddParam(TEXT("item_name"), ItemName);
		Event.AddParam(TEXT("context"), Context);
		
		Analytics->TrackEvent(Event);
	}
	
	IncrementCounter(TEXT("items_used"));
	SessionEventCount++;
}

// ============================================================================
// Session Tracking
// ============================================================================

void UDeskillzEventTracker::ClearSessionData()
{
	SessionRevenue = 0.0;
	SessionEventCount = 0;
	ActiveTimedEvents.Empty();
	ActiveFunnels.Empty();
	Counters.Empty();
}

int64 UDeskillzEventTracker::GetCurrentTimestamp() const
{
	return FDateTime::UtcNow().ToUnixTimestamp() * 1000;
}
