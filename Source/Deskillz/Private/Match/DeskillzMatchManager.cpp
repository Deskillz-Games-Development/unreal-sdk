// Copyright Deskillz Games. All Rights Reserved.

#include "Match/DeskillzMatchManager.h"
#include "Core/DeskillzSDK.h"
#include "Core/DeskillzConfig.h"
#include "Deskillz.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// Constructor & Lifecycle
// ============================================================================

UDeskillzMatchManager::UDeskillzMatchManager()
{
	// Default time warnings at 60, 30, 10, 5 seconds
	TimeWarnings = { 60.0f, 30.0f, 10.0f, 5.0f };
}

void UDeskillzMatchManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogDeskillz, Log, TEXT("Match Manager Subsystem Initialized"));
	
	// Get SDK reference
	SDKRef = UDeskillzSDK::Get(GetGameInstance());
}

void UDeskillzMatchManager::Deinitialize()
{
	// Cleanup any active match
	if (IsInMatch())
	{
		AbortMatch(TEXT("Match Manager Deinitializing"));
	}
	
	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTickHandle);
		World->GetTimerManager().ClearTimer(CountdownHandle);
		World->GetTimerManager().ClearTimer(TimeoutHandle);
	}
	
	Super::Deinitialize();
}

bool UDeskillzMatchManager::ShouldCreateSubsystem(UObject* Outer) const
{
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	return Config && Config->bEnableSDK;
}

// ============================================================================
// Static Accessors
// ============================================================================

UDeskillzMatchManager* UDeskillzMatchManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	
	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}
	
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
	
	return GameInstance->GetSubsystem<UDeskillzMatchManager>();
}

// ============================================================================
// Match Lifecycle - Entry Points
// ============================================================================

void UDeskillzMatchManager::EnterMatch(const FString& TournamentId, EDeskillzCurrency Currency)
{
	if (IsInMatch())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Already in a match. Cannot enter new match."));
		return;
	}
	
	UDeskillzSDK* SDK = GetSDK();
	if (!SDK || !SDK->IsReady())
	{
		UE_LOG(LogDeskillz, Error, TEXT("SDK not ready. Cannot enter match."));
		return;
	}
	
	if (!SDK->IsAuthenticated())
	{
		UE_LOG(LogDeskillz, Error, TEXT("User not authenticated. Cannot enter match."));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Entering match for tournament: %s"), *TournamentId);
	
	// Reset state
	ResetMatchState();
	
	// Store tournament ID
	CurrentMatchInfo.TournamentId = TournamentId;
	
	// Transition to waiting state
	TransitionToState(EDeskillzMatchState::WaitingForOpponent);
	
	// Bind to SDK events
	SDK->OnMatchStarted.AddDynamic(this, &UDeskillzMatchManager::OnMatchmakingComplete);
	SDK->OnScoreSubmitted.AddDynamic(this, &UDeskillzMatchManager::OnScoreSubmissionComplete);
	SDK->OnMatchCompleted.AddDynamic(this, &UDeskillzMatchManager::OnMatchResultReceived);
	
	// Start matchmaking
	SDK->StartMatchmaking(TournamentId);
}

void UDeskillzMatchManager::EnterMatchWithInfo(const FDeskillzMatchInfo& MatchInfo)
{
	if (IsInMatch())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Already in a match. Cannot enter with info."));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Entering match with existing info: %s"), *MatchInfo.MatchId);
	
	ResetMatchState();
	CurrentMatchInfo = MatchInfo;
	
	// Determine appropriate state based on match info
	if (MatchInfo.Status == EDeskillzMatchStatus::InProgress)
	{
		TransitionToState(EDeskillzMatchState::Playing);
		BeginGameplay();
	}
	else if (MatchInfo.Status == EDeskillzMatchStatus::Ready)
	{
		TransitionToState(EDeskillzMatchState::OpponentFound);
	}
	else
	{
		TransitionToState(EDeskillzMatchState::WaitingForOpponent);
	}
}

void UDeskillzMatchManager::SignalReady()
{
	if (MatchState != EDeskillzMatchState::OpponentFound)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot signal ready in current state: %d"), (int32)MatchState);
		return;
	}
	
	bIsPlayerReady = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("Player signaled ready"));
	
	// For async matches, we can start immediately
	// For sync matches, wait for opponent ready signal via WebSocket
	if (!IsSynchronousMatch() || bIsOpponentReady)
	{
		StartCountdown();
	}
}

void UDeskillzMatchManager::StartCountdown(int32 CountdownDuration)
{
	if (MatchState != EDeskillzMatchState::OpponentFound)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Starting countdown: %d seconds"), CountdownDuration);
	
	CountdownRemaining = CountdownDuration;
	TransitionToState(EDeskillzMatchState::Countdown);
	
	// Start countdown timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CountdownHandle,
			this,
			&UDeskillzMatchManager::CountdownTick,
			1.0f,
			true
		);
	}
	
	// Broadcast initial countdown
	OnCountdown.Broadcast(CountdownRemaining);
}

void UDeskillzMatchManager::BeginGameplay()
{
	if (MatchState != EDeskillzMatchState::Countdown && MatchState != EDeskillzMatchState::OpponentFound)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot begin gameplay in current state: %d"), (int32)MatchState);
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Beginning gameplay for match: %s"), *CurrentMatchInfo.MatchId);
	
	// Clear countdown timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownHandle);
	}
	
	// Set gameplay start time
	GameplayStartTime = FDateTime::UtcNow();
	CurrentMatchInfo.Status = EDeskillzMatchStatus::InProgress;
	
	// Transition to playing
	TransitionToState(EDeskillzMatchState::Playing);
	
	// Start match tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MatchTickHandle,
			this,
			&UDeskillzMatchManager::MatchTick,
			0.1f, // 10 Hz update
			true
		);
		
		// Set timeout for match duration
		float MatchDuration = (float)CurrentMatchInfo.DurationSeconds;
		World->GetTimerManager().SetTimer(
			TimeoutHandle,
			this,
			&UDeskillzMatchManager::OnMatchTimeout,
			MatchDuration,
			false
		);
	}
	
	// Notify SDK
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->StartMatch();
	}
	
	// Broadcast match started
	OnMatchStarted.Broadcast(CurrentMatchInfo, FDeskillzError::None());
}

void UDeskillzMatchManager::EndMatch(int64 FinalScore)
{
	if (!IsMatchActive())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No active match to end"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Ending match with score: %lld"), FinalScore);
	
	// Stop timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTickHandle);
		World->GetTimerManager().ClearTimer(TimeoutHandle);
	}
	
	// Update final score
	CurrentScore = FinalScore;
	
	// Create final checkpoint
	CreateScoreCheckpoint(TEXT("final"));
	
	// Transition to submitting
	TransitionToState(EDeskillzMatchState::Submitting);
	
	// Submit score via SDK
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->SubmitScore(FinalScore);
	}
}

void UDeskillzMatchManager::AbortMatch(const FString& Reason)
{
	if (!IsInMatch())
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Aborting match: %s. Reason: %s"), *CurrentMatchInfo.MatchId, *Reason);
	
	// Stop all timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTickHandle);
		World->GetTimerManager().ClearTimer(CountdownHandle);
		World->GetTimerManager().ClearTimer(TimeoutHandle);
	}
	
	// Notify SDK
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->AbortMatch(Reason);
	}
	
	// Transition to aborted
	TransitionToState(EDeskillzMatchState::Aborted);
	
	// Create result for abort
	FDeskillzMatchResult AbortResult;
	AbortResult.MatchId = CurrentMatchInfo.MatchId;
	AbortResult.Result = EDeskillzMatchResult::Forfeit;
	AbortResult.PlayerScore = CurrentScore;
	
	OnMatchCompleted.Broadcast(AbortResult, FDeskillzError::None());
	
	// Reset state
	ResetMatchState();
}

// ============================================================================
// Score Management
// ============================================================================

void UDeskillzMatchManager::UpdateScore(int64 NewScore)
{
	if (!IsMatchActive())
	{
		return;
	}
	
	// Validate score
	if (!ValidateScore(NewScore))
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Invalid score: %lld"), NewScore);
		return;
	}
	
	int64 OldScore = CurrentScore;
	CurrentScore = NewScore;
	
	// Notify SDK for real-time sync
	if (UDeskillzSDK* SDK = GetSDK())
	{
		SDK->UpdateScore(NewScore);
	}
	
	// Log significant score changes
	if (FMath::Abs(NewScore - OldScore) > 1000)
	{
		CreateScoreCheckpoint(TEXT("significant_change"));
	}
}

void UDeskillzMatchManager::AddScore(int64 Points)
{
	UpdateScore(CurrentScore + Points);
}

void UDeskillzMatchManager::MultiplyScore(float Multiplier)
{
	UpdateScore((int64)(CurrentScore * Multiplier));
}

void UDeskillzMatchManager::CreateScoreCheckpoint(const FString& EventType)
{
	float Timestamp = GetElapsedTime();
	
	FDeskillzScoreCheckpoint Checkpoint(CurrentScore, Timestamp, EventType);
	ScoreHistory.Add(Checkpoint);
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Score checkpoint: %lld at %.2fs (%s)"), 
		CurrentScore, Timestamp, *EventType);
}

// ============================================================================
// Match State & Timing
// ============================================================================

FDeskillzMatchProgress UDeskillzMatchManager::GetProgress() const
{
	FDeskillzMatchProgress Progress;
	Progress.State = MatchState;
	Progress.ElapsedTime = GetElapsedTime();
	Progress.RemainingTime = GetRemainingTime();
	Progress.ProgressPercent = GetProgressPercent();
	Progress.CurrentScore = CurrentScore;
	Progress.OpponentScore = OpponentScore;
	Progress.CountdownSeconds = CountdownRemaining;
	return Progress;
}

float UDeskillzMatchManager::GetElapsedTime() const
{
	if (MatchState != EDeskillzMatchState::Playing)
	{
		return 0.0f;
	}
	
	FTimespan Duration = FDateTime::UtcNow() - GameplayStartTime;
	return (float)Duration.GetTotalSeconds();
}

float UDeskillzMatchManager::GetRemainingTime() const
{
	if (MatchState != EDeskillzMatchState::Playing)
	{
		return (float)CurrentMatchInfo.DurationSeconds;
	}
	
	float Elapsed = GetElapsedTime();
	return FMath::Max(0.0f, (float)CurrentMatchInfo.DurationSeconds - Elapsed);
}

float UDeskillzMatchManager::GetProgressPercent() const
{
	if (CurrentMatchInfo.DurationSeconds <= 0)
	{
		return 0.0f;
	}
	
	float Elapsed = GetElapsedTime();
	return FMath::Clamp(Elapsed / (float)CurrentMatchInfo.DurationSeconds, 0.0f, 1.0f);
}

bool UDeskillzMatchManager::IsSynchronousMatch() const
{
	return CurrentMatchInfo.MatchType == EDeskillzMatchType::Synchronous;
}

// ============================================================================
// Match Validation
// ============================================================================

bool UDeskillzMatchManager::ValidateScore(int64 Score) const
{
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	if (!Config)
	{
		return true;
	}
	
	return Score >= Config->MinScore && Score <= Config->MaxScore;
}

bool UDeskillzMatchManager::IsTimeExpired() const
{
	return GetRemainingTime() <= 0.0f;
}

// ============================================================================
// Time Warnings
// ============================================================================

void UDeskillzMatchManager::SetTimeWarningThresholds(const TArray<float>& Thresholds)
{
	TimeWarnings = Thresholds;
	TimeWarnings.Sort([](float A, float B) { return A > B; }); // Sort descending
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzMatchManager::TransitionToState(EDeskillzMatchState NewState)
{
	if (!CanTransitionTo(NewState))
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Invalid state transition: %d -> %d"), (int32)MatchState, (int32)NewState);
		return;
	}
	
	EDeskillzMatchState OldState = MatchState;
	MatchState = NewState;
	
	UE_LOG(LogDeskillz, Log, TEXT("Match state: %d -> %d"), (int32)OldState, (int32)NewState);
	
	OnMatchStateChanged.Broadcast(OldState, NewState);
}

bool UDeskillzMatchManager::CanTransitionTo(EDeskillzMatchState NewState) const
{
	// Always allow transition to Aborted
	if (NewState == EDeskillzMatchState::Aborted)
	{
		return MatchState != EDeskillzMatchState::None;
	}
	
	// Define valid transitions
	switch (MatchState)
	{
		case EDeskillzMatchState::None:
			return NewState == EDeskillzMatchState::WaitingForOpponent;
			
		case EDeskillzMatchState::WaitingForOpponent:
			return NewState == EDeskillzMatchState::OpponentFound;
			
		case EDeskillzMatchState::OpponentFound:
			return NewState == EDeskillzMatchState::Countdown || 
			       NewState == EDeskillzMatchState::Playing; // Direct for async
			
		case EDeskillzMatchState::Countdown:
			return NewState == EDeskillzMatchState::Playing;
			
		case EDeskillzMatchState::Playing:
			return NewState == EDeskillzMatchState::Submitting;
			
		case EDeskillzMatchState::Submitting:
			return NewState == EDeskillzMatchState::WaitingForResult || 
			       NewState == EDeskillzMatchState::Completed;
			
		case EDeskillzMatchState::WaitingForResult:
			return NewState == EDeskillzMatchState::Completed;
			
		default:
			return false;
	}
}

void UDeskillzMatchManager::MatchTick()
{
	if (!IsMatchActive())
	{
		return;
	}
	
	// Check time warnings
	CheckTimeWarnings();
	
	// Broadcast progress update
	OnMatchProgressUpdate.Broadcast(GetProgress());
	
	// Check for timeout
	if (IsTimeExpired())
	{
		OnMatchTimeout();
	}
}

void UDeskillzMatchManager::CountdownTick()
{
	CountdownRemaining--;
	
	UE_LOG(LogDeskillz, Log, TEXT("Countdown: %d"), CountdownRemaining);
	
	OnCountdown.Broadcast(CountdownRemaining);
	
	if (CountdownRemaining <= 0)
	{
		// Clear timer and begin gameplay
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CountdownHandle);
		}
		BeginGameplay();
	}
}

void UDeskillzMatchManager::CheckTimeWarnings()
{
	float Remaining = GetRemainingTime();
	
	for (float Threshold : TimeWarnings)
	{
		if (Remaining <= Threshold && !TriggeredWarnings.Contains(Threshold))
		{
			TriggeredWarnings.Add(Threshold);
			OnTimeWarning.Broadcast(Remaining);
			
			// Special case for expiring warning
			if (Threshold <= 10.0f)
			{
				OnMatchExpiring.Broadcast();
			}
			
			break; // Only trigger one warning per tick
		}
	}
}

void UDeskillzMatchManager::OnMatchTimeout()
{
	if (!IsMatchActive())
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Match timeout - auto-submitting score"));
	
	// Auto-submit current score
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	if (Config && Config->bAutoSubmitScores)
	{
		EndMatch(CurrentScore);
	}
}

void UDeskillzMatchManager::ResetMatchState()
{
	MatchState = EDeskillzMatchState::None;
	CurrentMatchInfo = FDeskillzMatchInfo();
	CurrentScore = 0;
	OpponentScore = 0;
	ScoreHistory.Empty();
	TriggeredWarnings.Empty();
	CountdownRemaining = 0;
	bIsPlayerReady = false;
	bIsOpponentReady = false;
}

UDeskillzSDK* UDeskillzMatchManager::GetSDK()
{
	if (SDKRef.IsValid())
	{
		return SDKRef.Get();
	}
	
	SDKRef = UDeskillzSDK::Get(GetGameInstance());
	return SDKRef.Get();
}

// ============================================================================
// SDK Event Handlers
// ============================================================================

void UDeskillzMatchManager::OnMatchmakingComplete(const FDeskillzMatchInfo& MatchInfo, const FDeskillzError& Error)
{
	if (Error.IsError())
	{
		UE_LOG(LogDeskillz, Error, TEXT("Matchmaking failed: %s"), *Error.Message);
		TransitionToState(EDeskillzMatchState::Aborted);
		ResetMatchState();
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Opponent found: %s"), *MatchInfo.Opponent.Username);
	
	CurrentMatchInfo = MatchInfo;
	MatchStartTime = FDateTime::UtcNow();
	
	TransitionToState(EDeskillzMatchState::OpponentFound);
}

void UDeskillzMatchManager::OnScoreSubmissionComplete(bool bSuccess, const FDeskillzError& Error)
{
	OnScoreSubmitted.Broadcast(bSuccess, Error);
	
	if (!bSuccess)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Score submission failed: %s"), *Error.Message);
		// Could retry or handle error
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Score submitted successfully"));
	
	// For async matches, we might get result immediately
	// For sync matches, wait for opponent
	if (IsSynchronousMatch())
	{
		TransitionToState(EDeskillzMatchState::WaitingForResult);
	}
}

void UDeskillzMatchManager::OnMatchResultReceived(const FDeskillzMatchResult& Result, const FDeskillzError& Error)
{
	if (Error.IsError())
	{
		UE_LOG(LogDeskillz, Error, TEXT("Failed to get match result: %s"), *Error.Message);
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Match result received: %s, Score: %lld vs %lld"), 
		Result.IsWin() ? TEXT("WIN") : TEXT("LOSS/DRAW"),
		Result.PlayerScore, Result.OpponentScore);
	
	TransitionToState(EDeskillzMatchState::Completed);
	
	// Broadcast completion
	OnMatchCompleted.Broadcast(Result, FDeskillzError::None());
	
	// Reset for next match
	ResetMatchState();
}
