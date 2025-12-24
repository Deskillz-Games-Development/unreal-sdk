// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzAnalytics.generated.h"

/**
 * Analytics event category
 */
UENUM(BlueprintType)
enum class EDeskillzEventCategory : uint8
{
	/** User actions */
	User,
	
	/** Match events */
	Match,
	
	/** Tournament events */
	Tournament,
	
	/** Wallet/payment events */
	Wallet,
	
	/** UI interactions */
	UI,
	
	/** System events */
	System,
	
	/** Custom game events */
	Game,
	
	/** Error events */
	Error
};

/**
 * Analytics event data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzAnalyticsEvent
{
	GENERATED_BODY()
	
	/** Event name */
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString EventName;
	
	/** Event category */
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	EDeskillzEventCategory Category = EDeskillzEventCategory::Game;
	
	/** Event parameters */
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	TMap<FString, FString> Parameters;
	
	/** Numeric values */
	TMap<FString, double> NumericValues;
	
	/** Timestamp (Unix ms) */
	UPROPERTY(BlueprintReadOnly, Category = "Analytics")
	int64 Timestamp = 0;
	
	/** Session ID */
	UPROPERTY(BlueprintReadOnly, Category = "Analytics")
	FString SessionId;
	
	/** User ID */
	UPROPERTY(BlueprintReadOnly, Category = "Analytics")
	FString UserId;
	
	/** Sequence number */
	UPROPERTY(BlueprintReadOnly, Category = "Analytics")
	int32 SequenceNumber = 0;
	
	FDeskillzAnalyticsEvent()
	{
		Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	}
	
	FDeskillzAnalyticsEvent(const FString& InName, EDeskillzEventCategory InCategory = EDeskillzEventCategory::Game)
		: EventName(InName), Category(InCategory)
	{
		Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	}
	
	/** Add string parameter */
	FDeskillzAnalyticsEvent& AddParam(const FString& Key, const FString& Value)
	{
		Parameters.Add(Key, Value);
		return *this;
	}
	
	/** Add numeric parameter */
	FDeskillzAnalyticsEvent& AddValue(const FString& Key, double Value)
	{
		NumericValues.Add(Key, Value);
		return *this;
	}
};

/**
 * User properties for analytics
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzUserProperties
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString UserId;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString Username;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 Level = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 SkillRating = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 TotalMatches = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 TotalWins = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString Country;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString Platform;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString AppVersion;
	
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	bool bIsPremium = false;
	
	/** Custom properties */
	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	TMap<FString, FString> CustomProperties;
};

/**
 * Analytics configuration
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzAnalyticsConfig
{
	GENERATED_BODY()
	
	/** Enable analytics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	bool bEnabled = true;
	
	/** Enable debug logging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	bool bDebugMode = false;
	
	/** Batch size before flush */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 BatchSize = 10;
	
	/** Flush interval (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	float FlushInterval = 30.0f;
	
	/** Max events in queue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 MaxQueueSize = 1000;
	
	/** Persist events offline */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	bool bPersistOffline = true;
	
	/** Sample rate (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SampleRate = 1.0f;
	
	/** Track automatic events */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	bool bTrackAutoEvents = true;
	
	/** API endpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analytics")
	FString AnalyticsEndpoint = TEXT("/api/v1/analytics/events");
};

/** Analytics event delegate */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalyticsEvent, const FDeskillzAnalyticsEvent&, Event);

/**
 * Deskillz Analytics System
 * 
 * Comprehensive analytics tracking for:
 * - User behavior and engagement
 * - Match and tournament metrics
 * - Wallet/transaction events
 * - Custom game events
 * - Error tracking
 * 
 * Features:
 * - Event batching and queuing
 * - Offline persistence
 * - Automatic session tracking
 * - User property management
 * - Sampling support
 * 
 * Usage:
 *   UDeskillzAnalytics* Analytics = UDeskillzAnalytics::Get();
 *   Analytics->TrackEvent("level_complete", EDeskillzEventCategory::Game)
 *       .AddParam("level", "5")
 *       .AddValue("score", 1500.0);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzAnalytics : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzAnalytics();
	~UDeskillzAnalytics();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Analytics instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Analytics", meta = (DisplayName = "Get Deskillz Analytics"))
	static UDeskillzAnalytics* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize analytics with configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void Initialize(const FDeskillzAnalyticsConfig& Config);
	
	/**
	 * Initialize with defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void InitializeDefault();
	
	/**
	 * Shutdown analytics
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void Shutdown();
	
	/**
	 * Is analytics enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Analytics")
	bool IsEnabled() const { return AnalyticsConfig.bEnabled && bIsInitialized; }
	
	// ========================================================================
	// Event Tracking
	// ========================================================================
	
	/**
	 * Track an event
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackEvent(const FDeskillzAnalyticsEvent& Event);
	
	/**
	 * Track event by name (C++)
	 */
	FDeskillzAnalyticsEvent& Track(const FString& EventName, EDeskillzEventCategory Category = EDeskillzEventCategory::Game);
	
	/**
	 * Track event by name (Blueprint)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics", meta = (DisplayName = "Track Event"))
	void K2_TrackEvent(const FString& EventName, EDeskillzEventCategory Category, 
		const TMap<FString, FString>& Parameters);
	
	// ========================================================================
	// Pre-defined Events
	// ========================================================================
	
	/**
	 * Track session start
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackSessionStart();
	
	/**
	 * Track session end
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackSessionEnd();
	
	/**
	 * Track user login
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackLogin(const FString& Method);
	
	/**
	 * Track user registration
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackRegistration(const FString& Method);
	
	/**
	 * Track match start
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackMatchStart(const FString& MatchId, const FString& TournamentId, double EntryFee);
	
	/**
	 * Track match complete
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackMatchComplete(const FString& MatchId, int64 Score, bool bWon, double PrizeWon);
	
	/**
	 * Track tournament entry
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackTournamentEntry(const FString& TournamentId, double EntryFee, const FString& Currency);
	
	/**
	 * Track wallet deposit
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackDeposit(double Amount, const FString& Currency);
	
	/**
	 * Track wallet withdrawal
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackWithdrawal(double Amount, const FString& Currency);
	
	/**
	 * Track screen view
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackScreenView(const FString& ScreenName);
	
	/**
	 * Track button click
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackButtonClick(const FString& ButtonName, const FString& ScreenName);
	
	/**
	 * Track error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void TrackError(const FString& ErrorCode, const FString& ErrorMessage, const FString& Context);
	
	// ========================================================================
	// User Properties
	// ========================================================================
	
	/**
	 * Set user properties
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void SetUserProperties(const FDeskillzUserProperties& Properties);
	
	/**
	 * Set user ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void SetUserId(const FString& UserId);
	
	/**
	 * Set user property
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void SetUserProperty(const FString& Key, const FString& Value);
	
	/**
	 * Clear user data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void ClearUserData();
	
	// ========================================================================
	// Queue Management
	// ========================================================================
	
	/**
	 * Flush events to server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void Flush();
	
	/**
	 * Get queued event count
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Analytics")
	int32 GetQueuedEventCount() const { return EventQueue.Num(); }
	
	/**
	 * Clear event queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void ClearQueue();
	
	// ========================================================================
	// Session
	// ========================================================================
	
	/**
	 * Get current session ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Analytics")
	FString GetSessionId() const { return SessionId; }
	
	/**
	 * Get session duration (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Analytics")
	float GetSessionDuration() const;
	
	/**
	 * Start new session
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Analytics")
	void StartNewSession();
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when event is tracked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Analytics")
	FOnAnalyticsEvent OnEventTracked;
	
protected:
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Analytics configuration */
	UPROPERTY()
	FDeskillzAnalyticsConfig AnalyticsConfig;
	
	/** Is initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** Current session ID */
	UPROPERTY()
	FString SessionId;
	
	/** Session start time */
	UPROPERTY()
	int64 SessionStartTime = 0;
	
	/** Current user ID */
	UPROPERTY()
	FString UserId;
	
	/** User properties */
	UPROPERTY()
	FDeskillzUserProperties UserProperties;
	
	/** Event sequence counter */
	UPROPERTY()
	int32 EventSequence = 0;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Event queue */
	TArray<FDeskillzAnalyticsEvent> EventQueue;
	
	/** Pending event for chaining */
	FDeskillzAnalyticsEvent* PendingEvent = nullptr;
	
	/** Flush timer handle */
	FTimerHandle FlushTimerHandle;
	
	/** Is flushing */
	bool bIsFlushing = false;
	
	/** Critical section for queue access */
	FCriticalSection QueueLock;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Generate session ID */
	FString GenerateSessionId() const;
	
	/** Should sample this event */
	bool ShouldSampleEvent() const;
	
	/** Enqueue event */
	void EnqueueEvent(const FDeskillzAnalyticsEvent& Event);
	
	/** Check and flush if needed */
	void CheckFlush();
	
	/** Do flush to server */
	void DoFlush();
	
	/** Handle flush response */
	void OnFlushComplete(bool bSuccess);
	
	/** Start flush timer */
	void StartFlushTimer();
	
	/** Stop flush timer */
	void StopFlushTimer();
	
	/** Persist queue to disk */
	void PersistQueue();
	
	/** Load persisted queue */
	void LoadPersistedQueue();
	
	/** Get device info */
	TMap<FString, FString> GetDeviceInfo() const;
	
	/** Serialize event to JSON */
	TSharedPtr<FJsonObject> EventToJson(const FDeskillzAnalyticsEvent& Event) const;
};
