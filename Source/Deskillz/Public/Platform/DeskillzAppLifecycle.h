// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzAppLifecycle.generated.h"

/**
 * Application state
 */
UENUM(BlueprintType)
enum class EDeskillzAppState : uint8
{
	/** App is active and in foreground */
	Active,
	
	/** App is in background */
	Background,
	
	/** App is being suspended */
	Suspending,
	
	/** App is resuming from background */
	Resuming,
	
	/** App is being terminated */
	Terminating
};

/**
 * App lifecycle event data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzAppLifecycleEvent
{
	GENERATED_BODY()
	
	/** New state */
	UPROPERTY(BlueprintReadOnly, Category = "Lifecycle")
	EDeskillzAppState State = EDeskillzAppState::Active;
	
	/** Previous state */
	UPROPERTY(BlueprintReadOnly, Category = "Lifecycle")
	EDeskillzAppState PreviousState = EDeskillzAppState::Active;
	
	/** Time in previous state (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Lifecycle")
	float TimeInPreviousState = 0.0f;
	
	/** Timestamp */
	UPROPERTY(BlueprintReadOnly, Category = "Lifecycle")
	int64 Timestamp = 0;
};

/** Lifecycle delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAppStateChanged, const FDeskillzAppLifecycleEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAppWillEnterBackground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAppDidEnterForeground);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAppWillTerminate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAppLowMemory);

/**
 * Deskillz App Lifecycle Manager
 * 
 * Manages application lifecycle events:
 * - Foreground/background transitions
 * - App suspension and resumption
 * - Memory warnings
 * - App termination
 * - Session tracking
 * 
 * Features:
 * - Automatic session management
 * - Background time tracking
 * - Pause/resume game state
 * - Memory warning handling
 * 
 * Usage:
 *   UDeskillzAppLifecycle* Lifecycle = UDeskillzAppLifecycle::Get();
 *   
 *   Lifecycle->OnAppWillEnterBackground.AddDynamic(this, &AMyActor::PauseGame);
 *   Lifecycle->OnAppDidEnterForeground.AddDynamic(this, &AMyActor::ResumeGame);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzAppLifecycle : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzAppLifecycle();
	~UDeskillzAppLifecycle();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the App Lifecycle instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle", meta = (DisplayName = "Get Deskillz App Lifecycle"))
	static UDeskillzAppLifecycle* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize lifecycle management
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lifecycle")
	void Initialize();
	
	/**
	 * Shutdown lifecycle management
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lifecycle")
	void Shutdown();
	
	// ========================================================================
	// State
	// ========================================================================
	
	/**
	 * Get current app state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	EDeskillzAppState GetCurrentState() const { return CurrentState; }
	
	/**
	 * Is app in foreground
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	bool IsInForeground() const { return CurrentState == EDeskillzAppState::Active; }
	
	/**
	 * Is app in background
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	bool IsInBackground() const { return CurrentState == EDeskillzAppState::Background; }
	
	/**
	 * Get time in current state (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	float GetTimeInCurrentState() const;
	
	/**
	 * Get total background time this session (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	float GetTotalBackgroundTime() const { return TotalBackgroundTime; }
	
	/**
	 * Get total foreground time this session (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	float GetTotalForegroundTime() const { return TotalForegroundTime; }
	
	// ========================================================================
	// Session
	// ========================================================================
	
	/**
	 * Get session start time
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	int64 GetSessionStartTime() const { return SessionStartTime; }
	
	/**
	 * Get session duration (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	float GetSessionDuration() const;
	
	/**
	 * Get number of background transitions
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	int32 GetBackgroundTransitionCount() const { return BackgroundTransitionCount; }
	
	// ========================================================================
	// Settings
	// ========================================================================
	
	/**
	 * Set background timeout (after which match is considered abandoned)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lifecycle")
	void SetBackgroundTimeout(float TimeoutSeconds);
	
	/**
	 * Get background timeout
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	float GetBackgroundTimeout() const { return BackgroundTimeout; }
	
	/**
	 * Was app in background longer than timeout
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Lifecycle")
	bool WasBackgroundTimeoutExceeded() const { return bBackgroundTimeoutExceeded; }
	
	/**
	 * Clear background timeout flag
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Lifecycle")
	void ClearBackgroundTimeoutFlag() { bBackgroundTimeoutExceeded = false; }
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when app state changes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lifecycle")
	FOnAppStateChanged OnAppStateChanged;
	
	/** Called when app is about to enter background */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lifecycle")
	FOnAppWillEnterBackground OnAppWillEnterBackground;
	
	/** Called when app returns to foreground */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lifecycle")
	FOnAppDidEnterForeground OnAppDidEnterForeground;
	
	/** Called when app is about to terminate */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lifecycle")
	FOnAppWillTerminate OnAppWillTerminate;
	
	/** Called on low memory warning */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Lifecycle")
	FOnAppLowMemory OnAppLowMemory;
	
protected:
	/** Is initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** Current app state */
	UPROPERTY()
	EDeskillzAppState CurrentState = EDeskillzAppState::Active;
	
	/** Time when current state started */
	UPROPERTY()
	int64 StateStartTime = 0;
	
	/** Session start time */
	UPROPERTY()
	int64 SessionStartTime = 0;
	
	/** Total time in background this session */
	UPROPERTY()
	float TotalBackgroundTime = 0.0f;
	
	/** Total time in foreground this session */
	UPROPERTY()
	float TotalForegroundTime = 0.0f;
	
	/** Number of times went to background */
	UPROPERTY()
	int32 BackgroundTransitionCount = 0;
	
	/** Background timeout threshold */
	UPROPERTY()
	float BackgroundTimeout = 300.0f; // 5 minutes default
	
	/** Was background timeout exceeded */
	UPROPERTY()
	bool bBackgroundTimeoutExceeded = false;
	
	/** Time when entered background */
	UPROPERTY()
	int64 BackgroundEntryTime = 0;
	
	/** Delegate handles */
	FDelegateHandle ActivateHandle;
	FDelegateHandle DeactivateHandle;
	FDelegateHandle SuspendHandle;
	FDelegateHandle ResumeHandle;
	FDelegateHandle TerminateHandle;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Register platform delegates */
	void RegisterPlatformDelegates();
	
	/** Unregister platform delegates */
	void UnregisterPlatformDelegates();
	
	/** Handle state change */
	void ChangeState(EDeskillzAppState NewState);
	
	/** Platform callbacks */
	void OnApplicationActivated();
	void OnApplicationDeactivated();
	void OnApplicationWillSuspend();
	void OnApplicationHasResumed();
	void OnApplicationWillTerminate();
	
	/** Get current timestamp */
	int64 GetCurrentTimestamp() const;
};
