// Copyright Deskillz Games. All Rights Reserved.

#include "Platform/DeskillzAppLifecycle.h"
#include "Analytics/DeskillzAnalytics.h"
#include "Deskillz.h"
#include "Misc/CoreDelegates.h"

// Static singleton
static UDeskillzAppLifecycle* GAppLifecycle = nullptr;

UDeskillzAppLifecycle::UDeskillzAppLifecycle()
{
}

UDeskillzAppLifecycle::~UDeskillzAppLifecycle()
{
	Shutdown();
}

UDeskillzAppLifecycle* UDeskillzAppLifecycle::Get()
{
	if (!GAppLifecycle)
	{
		GAppLifecycle = NewObject<UDeskillzAppLifecycle>();
		GAppLifecycle->AddToRoot();
	}
	return GAppLifecycle;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzAppLifecycle::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}
	
	SessionStartTime = GetCurrentTimestamp();
	StateStartTime = SessionStartTime;
	CurrentState = EDeskillzAppState::Active;
	
	RegisterPlatformDelegates();
	
	bIsInitialized = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("App Lifecycle initialized"));
}

void UDeskillzAppLifecycle::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	UnregisterPlatformDelegates();
	
	bIsInitialized = false;
	UE_LOG(LogDeskillz, Log, TEXT("App Lifecycle shutdown"));
}

// ============================================================================
// State
// ============================================================================

float UDeskillzAppLifecycle::GetTimeInCurrentState() const
{
	int64 CurrentTime = GetCurrentTimestamp();
	return static_cast<float>(CurrentTime - StateStartTime) / 1000.0f;
}

float UDeskillzAppLifecycle::GetSessionDuration() const
{
	int64 CurrentTime = GetCurrentTimestamp();
	return static_cast<float>(CurrentTime - SessionStartTime) / 1000.0f;
}

// ============================================================================
// Settings
// ============================================================================

void UDeskillzAppLifecycle::SetBackgroundTimeout(float TimeoutSeconds)
{
	BackgroundTimeout = FMath::Max(0.0f, TimeoutSeconds);
	UE_LOG(LogDeskillz, Log, TEXT("Background timeout set to %.1fs"), BackgroundTimeout);
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzAppLifecycle::RegisterPlatformDelegates()
{
	// Application activation
	ActivateHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(
		this, &UDeskillzAppLifecycle::OnApplicationActivated);
	
	// Application deactivation
	DeactivateHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(
		this, &UDeskillzAppLifecycle::OnApplicationDeactivated);
	
	// Application suspend
	SuspendHandle = FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(
		this, &UDeskillzAppLifecycle::OnApplicationWillSuspend);
	
	// Application resume
	ResumeHandle = FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject(
		this, &UDeskillzAppLifecycle::OnApplicationHasResumed);
	
	// Application terminate
	TerminateHandle = FCoreDelegates::ApplicationWillTerminateDelegate.AddUObject(
		this, &UDeskillzAppLifecycle::OnApplicationWillTerminate);
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Platform lifecycle delegates registered"));
}

void UDeskillzAppLifecycle::UnregisterPlatformDelegates()
{
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(ActivateHandle);
	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(DeactivateHandle);
	FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(SuspendHandle);
	FCoreDelegates::ApplicationHasReactivatedDelegate.Remove(ResumeHandle);
	FCoreDelegates::ApplicationWillTerminateDelegate.Remove(TerminateHandle);
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Platform lifecycle delegates unregistered"));
}

void UDeskillzAppLifecycle::ChangeState(EDeskillzAppState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}
	
	int64 CurrentTime = GetCurrentTimestamp();
	float TimeInPreviousState = static_cast<float>(CurrentTime - StateStartTime) / 1000.0f;
	
	// Update time tracking
	if (CurrentState == EDeskillzAppState::Background)
	{
		TotalBackgroundTime += TimeInPreviousState;
	}
	else if (CurrentState == EDeskillzAppState::Active)
	{
		TotalForegroundTime += TimeInPreviousState;
	}
	
	// Create event
	FDeskillzAppLifecycleEvent Event;
	Event.State = NewState;
	Event.PreviousState = CurrentState;
	Event.TimeInPreviousState = TimeInPreviousState;
	Event.Timestamp = CurrentTime;
	
	EDeskillzAppState PreviousState = CurrentState;
	CurrentState = NewState;
	StateStartTime = CurrentTime;
	
	UE_LOG(LogDeskillz, Log, TEXT("App state: %d -> %d (%.1fs in previous)"), 
		static_cast<int32>(PreviousState), static_cast<int32>(NewState), TimeInPreviousState);
	
	// Broadcast generic event
	OnAppStateChanged.Broadcast(Event);
	
	// Track in analytics
	UDeskillzAnalytics* Analytics = UDeskillzAnalytics::Get();
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent AnalyticsEvent(TEXT("app_state_change"), EDeskillzEventCategory::System);
		AnalyticsEvent.AddParam(TEXT("new_state"), FString::FromInt(static_cast<int32>(NewState)));
		AnalyticsEvent.AddParam(TEXT("previous_state"), FString::FromInt(static_cast<int32>(PreviousState)));
		AnalyticsEvent.AddValue(TEXT("time_in_previous"), TimeInPreviousState);
		
		Analytics->TrackEvent(AnalyticsEvent);
	}
}

void UDeskillzAppLifecycle::OnApplicationActivated()
{
	UE_LOG(LogDeskillz, Log, TEXT("Application activated (foreground)"));
	
	// Check if background timeout was exceeded
	if (BackgroundEntryTime > 0)
	{
		int64 CurrentTime = GetCurrentTimestamp();
		float TimeInBackground = static_cast<float>(CurrentTime - BackgroundEntryTime) / 1000.0f;
		
		if (TimeInBackground > BackgroundTimeout)
		{
			bBackgroundTimeoutExceeded = true;
			UE_LOG(LogDeskillz, Warning, TEXT("Background timeout exceeded: %.1fs > %.1fs"), 
				TimeInBackground, BackgroundTimeout);
		}
		
		BackgroundEntryTime = 0;
	}
	
	ChangeState(EDeskillzAppState::Active);
	OnAppDidEnterForeground.Broadcast();
}

void UDeskillzAppLifecycle::OnApplicationDeactivated()
{
	UE_LOG(LogDeskillz, Log, TEXT("Application deactivated (background)"));
	
	BackgroundEntryTime = GetCurrentTimestamp();
	BackgroundTransitionCount++;
	
	OnAppWillEnterBackground.Broadcast();
	ChangeState(EDeskillzAppState::Background);
}

void UDeskillzAppLifecycle::OnApplicationWillSuspend()
{
	UE_LOG(LogDeskillz, Log, TEXT("Application suspending"));
	ChangeState(EDeskillzAppState::Suspending);
}

void UDeskillzAppLifecycle::OnApplicationHasResumed()
{
	UE_LOG(LogDeskillz, Log, TEXT("Application resumed"));
	ChangeState(EDeskillzAppState::Resuming);
	
	// Immediately transition to active
	ChangeState(EDeskillzAppState::Active);
	OnAppDidEnterForeground.Broadcast();
}

void UDeskillzAppLifecycle::OnApplicationWillTerminate()
{
	UE_LOG(LogDeskillz, Log, TEXT("Application terminating"));
	
	ChangeState(EDeskillzAppState::Terminating);
	OnAppWillTerminate.Broadcast();
	
	// Track session end
	UDeskillzAnalytics* Analytics = UDeskillzAnalytics::Get();
	if (Analytics && Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("app_terminate"), EDeskillzEventCategory::System);
		Event.AddValue(TEXT("session_duration"), GetSessionDuration());
		Event.AddValue(TEXT("foreground_time"), TotalForegroundTime);
		Event.AddValue(TEXT("background_time"), TotalBackgroundTime);
		Event.AddValue(TEXT("background_count"), static_cast<double>(BackgroundTransitionCount));
		
		Analytics->TrackEvent(Event);
		Analytics->Flush();
	}
}

int64 UDeskillzAppLifecycle::GetCurrentTimestamp() const
{
	return FDateTime::UtcNow().ToUnixTimestamp() * 1000;
}
