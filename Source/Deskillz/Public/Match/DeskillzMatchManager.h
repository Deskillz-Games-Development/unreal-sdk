// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzMatchManager.generated.h"

class UDeskillzSDK;

// ============================================================================
// Match State Machine
// ============================================================================

/**
 * Internal match state for state machine
 */
UENUM(BlueprintType)
enum class EDeskillzMatchState : uint8
{
	None                 UMETA(DisplayName = "None"),
	WaitingForOpponent   UMETA(DisplayName = "Waiting for Opponent"),
	OpponentFound        UMETA(DisplayName = "Opponent Found"),
	Countdown            UMETA(DisplayName = "Countdown"),
	Playing              UMETA(DisplayName = "Playing"),
	Submitting           UMETA(DisplayName = "Submitting Score"),
	WaitingForResult     UMETA(DisplayName = "Waiting for Result"),
	Completed            UMETA(DisplayName = "Completed"),
	Aborted              UMETA(DisplayName = "Aborted")
};

/**
 * Match progress data for UI updates
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchProgress
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	EDeskillzMatchState State = EDeskillzMatchState::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	float ElapsedTime = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	float RemainingTime = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	float ProgressPercent = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int64 CurrentScore = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int64 OpponentScore = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int32 CountdownSeconds = 0;
	
	FDeskillzMatchProgress() = default;
};

/**
 * Score checkpoint for tracking score history
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzScoreCheckpoint
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int64 Score = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	float Timestamp = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString EventType;
	
	FDeskillzScoreCheckpoint() = default;
	
	FDeskillzScoreCheckpoint(int64 InScore, float InTimestamp, const FString& InEventType = TEXT("update"))
		: Score(InScore), Timestamp(InTimestamp), EventType(InEventType) {}
};

// ============================================================================
// Match Delegates
// ============================================================================

/** Called when match state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchStateChanged, EDeskillzMatchState, OldState, EDeskillzMatchState, NewState);

/** Called every frame during match with progress */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchProgressUpdate, const FDeskillzMatchProgress&, Progress);

/** Called when countdown ticks */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchCountdown, int32, SecondsRemaining);

/** Called when match time warning (e.g., 30 seconds left) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchTimeWarning, float, SecondsRemaining);

/** Called when opponent score updates (sync matches) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOpponentScoreUpdate, int64, OpponentScore);

/** Called when match is about to expire */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchExpiring);

// ============================================================================
// Match Manager
// ============================================================================

/**
 * Deskillz Match Manager
 * 
 * Manages the complete match lifecycle from matchmaking to results.
 * Handles state transitions, timing, score tracking, and synchronization.
 * 
 * State Machine Flow:
 * None -> WaitingForOpponent -> OpponentFound -> Countdown -> Playing -> 
 * Submitting -> WaitingForResult -> Completed
 * 
 * (Any state can transition to Aborted)
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzMatchManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UDeskillzMatchManager();
	
	// ========================================================================
	// Subsystem Lifecycle
	// ========================================================================
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	// ========================================================================
	// Static Accessors
	// ========================================================================
	
	/**
	 * Get the Match Manager instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Match Manager"))
	static UDeskillzMatchManager* Get(const UObject* WorldContextObject);
	
	// ========================================================================
	// Match Lifecycle - Entry Points
	// ========================================================================
	
	/**
	 * Enter a tournament match
	 * Starts matchmaking and handles the full match flow
	 * @param TournamentId Tournament to enter
	 * @param Currency Currency to pay entry fee
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void EnterMatch(const FString& TournamentId, EDeskillzCurrency Currency = EDeskillzCurrency::USDT);
	
	/**
	 * Enter with existing match info (for reconnection)
	 * @param MatchInfo Pre-existing match data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void EnterMatchWithInfo(const FDeskillzMatchInfo& MatchInfo);
	
	/**
	 * Signal ready to start (after loading game scene)
	 * Call this when your game is loaded and ready to begin
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void SignalReady();
	
	/**
	 * Start the countdown (both players ready)
	 * @param CountdownDuration Seconds to count down
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void StartCountdown(int32 CountdownDuration = 3);
	
	/**
	 * Begin gameplay (after countdown)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void BeginGameplay();
	
	/**
	 * End match and submit score
	 * @param FinalScore The player's final score
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void EndMatch(int64 FinalScore);
	
	/**
	 * Abort match (forfeit)
	 * @param Reason Reason for aborting
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void AbortMatch(const FString& Reason = TEXT(""));
	
	// ========================================================================
	// Score Management
	// ========================================================================
	
	/**
	 * Update the current score
	 * @param NewScore Updated score value
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void UpdateScore(int64 NewScore);
	
	/**
	 * Add points to current score
	 * @param Points Points to add (can be negative)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void AddScore(int64 Points);
	
	/**
	 * Multiply current score
	 * @param Multiplier Score multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void MultiplyScore(float Multiplier);
	
	/**
	 * Create a score checkpoint (for verification)
	 * @param EventType Type of event (e.g., "level_complete", "bonus")
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void CreateScoreCheckpoint(const FString& EventType = TEXT("checkpoint"));
	
	/**
	 * Get the current score
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	int64 GetCurrentScore() const { return CurrentScore; }
	
	/**
	 * Get opponent's current score (sync matches only)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	int64 GetOpponentScore() const { return OpponentScore; }
	
	/**
	 * Get score checkpoint history
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	TArray<FDeskillzScoreCheckpoint> GetScoreHistory() const { return ScoreHistory; }
	
	// ========================================================================
	// Match State & Timing
	// ========================================================================
	
	/**
	 * Get current match state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	EDeskillzMatchState GetMatchState() const { return MatchState; }
	
	/**
	 * Get current match info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	FDeskillzMatchInfo GetMatchInfo() const { return CurrentMatchInfo; }
	
	/**
	 * Get match progress data
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	FDeskillzMatchProgress GetProgress() const;
	
	/**
	 * Get elapsed time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	float GetElapsedTime() const;
	
	/**
	 * Get remaining time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	float GetRemainingTime() const;
	
	/**
	 * Get progress as percentage (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	float GetProgressPercent() const;
	
	/**
	 * Check if match is currently active (playing)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	bool IsMatchActive() const { return MatchState == EDeskillzMatchState::Playing; }
	
	/**
	 * Check if in any match state (not None)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	bool IsInMatch() const { return MatchState != EDeskillzMatchState::None; }
	
	/**
	 * Check if this is a synchronous (real-time) match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	bool IsSynchronousMatch() const;
	
	// ========================================================================
	// Match Validation
	// ========================================================================
	
	/**
	 * Validate that score is within acceptable range
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	bool ValidateScore(int64 Score) const;
	
	/**
	 * Check if match time has expired
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	bool IsTimeExpired() const;
	
	/**
	 * Get the random seed for this match (for deterministic gameplay)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Match")
	int64 GetRandomSeed() const { return CurrentMatchInfo.RandomSeed; }
	
	// ========================================================================
	// Time Warnings Configuration
	// ========================================================================
	
	/**
	 * Set time warning thresholds (triggers OnMatchTimeWarning)
	 * @param Thresholds Array of seconds at which to trigger warnings
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Match")
	void SetTimeWarningThresholds(const TArray<float>& Thresholds);
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Match state changed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnMatchStateChanged OnMatchStateChanged;
	
	/** Match progress updated (every tick during match) */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnMatchProgressUpdate OnMatchProgressUpdate;
	
	/** Countdown tick */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnMatchCountdown OnCountdown;
	
	/** Time warning triggered */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnMatchTimeWarning OnTimeWarning;
	
	/** Opponent score updated (sync matches) */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnOpponentScoreUpdate OnOpponentScoreUpdate;
	
	/** Match about to expire */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnMatchExpiring OnMatchExpiring;
	
	/** Match started */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnDeskillzMatchStarted OnMatchStarted;
	
	/** Match completed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnDeskillzMatchCompleted OnMatchCompleted;
	
	/** Score submitted */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Match|Events")
	FOnDeskillzScoreSubmitted OnScoreSubmitted;
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Current match state */
	UPROPERTY()
	EDeskillzMatchState MatchState = EDeskillzMatchState::None;
	
	/** Current match info */
	UPROPERTY()
	FDeskillzMatchInfo CurrentMatchInfo;
	
	/** Current score */
	int64 CurrentScore = 0;
	
	/** Opponent score (sync matches) */
	int64 OpponentScore = 0;
	
	/** Match start time */
	FDateTime MatchStartTime;
	
	/** Gameplay start time (after countdown) */
	FDateTime GameplayStartTime;
	
	/** Score checkpoint history */
	UPROPERTY()
	TArray<FDeskillzScoreCheckpoint> ScoreHistory;
	
	/** Time warning thresholds */
	TArray<float> TimeWarnings;
	
	/** Triggered time warnings */
	TSet<float> TriggeredWarnings;
	
	/** Countdown remaining */
	int32 CountdownRemaining = 0;
	
	/** Is player ready */
	bool bIsPlayerReady = false;
	
	/** Is opponent ready (sync matches) */
	bool bIsOpponentReady = false;
	
	/** Reference to main SDK */
	UPROPERTY()
	TWeakObjectPtr<UDeskillzSDK> SDKRef;
	
	// ========================================================================
	// Timers
	// ========================================================================
	
	FTimerHandle MatchTickHandle;
	FTimerHandle CountdownHandle;
	FTimerHandle TimeoutHandle;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Transition to new state */
	void TransitionToState(EDeskillzMatchState NewState);
	
	/** Match tick (called every frame during active match) */
	void MatchTick();
	
	/** Countdown tick */
	void CountdownTick();
	
	/** Check and trigger time warnings */
	void CheckTimeWarnings();
	
	/** Handle match timeout */
	void OnMatchTimeout();
	
	/** Reset match state */
	void ResetMatchState();
	
	/** Validate state transition */
	bool CanTransitionTo(EDeskillzMatchState NewState) const;
	
	/** Get SDK reference */
	UDeskillzSDK* GetSDK();
	
	/** Handle matchmaking complete */
	void OnMatchmakingComplete(const FDeskillzMatchInfo& MatchInfo);
	
	/** Handle score submission complete */
	void OnScoreSubmissionComplete(bool bSuccess, const FDeskillzError& Error);
	
	/** Handle match result received */
	void OnMatchResultReceived(const FDeskillzMatchResult& Result);
};
