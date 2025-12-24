// Copyright Deskillz Games. All Rights Reserved.

#include "Match/DeskillzMatchmaking.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// ============================================================================
// Constructor & Initialization
// ============================================================================

UDeskillzMatchmaking::UDeskillzMatchmaking()
{
	CurrentStatus = EDeskillzMatchmakingStatus::Idle;
}

void UDeskillzMatchmaking::Initialize(UDeskillzSDK* SDK)
{
	if (!SDK)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Cannot initialize matchmaking without SDK"));
		return;
	}
	
	SDKRef = SDK;
	
	// Get world reference for timers
	if (UGameInstance* GI = SDK->GetGameInstance())
	{
		WorldRef = GI->GetWorld();
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Matchmaking initialized"));
}

void UDeskillzMatchmaking::Cleanup()
{
	ClearTimers();
	Reset();
	
	UE_LOG(LogDeskillz, Log, TEXT("Matchmaking cleaned up"));
}

// ============================================================================
// Static Factory
// ============================================================================

UDeskillzMatchmaking* UDeskillzMatchmaking::CreateMatchmaking(const UObject* WorldContextObject)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(WorldContextObject);
	if (!SDK)
	{
		return nullptr;
	}
	
	UDeskillzMatchmaking* Matchmaking = NewObject<UDeskillzMatchmaking>();
	Matchmaking->Initialize(SDK);
	
	return Matchmaking;
}

// ============================================================================
// Queue Management
// ============================================================================

void UDeskillzMatchmaking::JoinQueue(const FDeskillzMatchmakingParams& Params)
{
	if (IsInQueue())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Already in matchmaking queue"));
		return;
	}
	
	UDeskillzSDK* SDK = SDKRef.Get();
	if (!SDK || !SDK->IsReady())
	{
		UE_LOG(LogDeskillz, Error, TEXT("SDK not ready for matchmaking"));
		OnFailed.Broadcast(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("SDK not ready")));
		return;
	}
	
	if (!SDK->IsAuthenticated())
	{
		UE_LOG(LogDeskillz, Error, TEXT("User not authenticated for matchmaking"));
		OnFailed.Broadcast(FDeskillzError(EDeskillzErrorCode::AuthenticationFailed, TEXT("Not authenticated")));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Joining matchmaking queue for tournament: %s"), *Params.TournamentId);
	
	// Store params and reset state
	CurrentParams = Params;
	Reset();
	
	// Record queue join time
	QueueJoinTime = FDateTime::UtcNow();
	
	// Initialize queue info
	QueueInfo.Status = EDeskillzMatchmakingStatus::Queued;
	QueueInfo.CurrentRatingRange = Params.RatingTolerance;
	QueueInfo.EstimatedWaitTime = CalculateEstimatedWait();
	
	// Transition to queued
	TransitionToStatus(EDeskillzMatchmakingStatus::Queued);
	
	// Start queue tick timer
	if (UWorld* World = WorldRef.Get())
	{
		World->GetTimerManager().SetTimer(
			QueueTickHandle,
			this,
			&UDeskillzMatchmaking::QueueTick,
			1.0f,
			true
		);
		
		// Set expand search timer
		World->GetTimerManager().SetTimer(
			ExpandHandle,
			this,
			&UDeskillzMatchmaking::ExpandSearch,
			Params.ExpandSearchAfter,
			false
		);
		
		// Set timeout timer
		World->GetTimerManager().SetTimer(
			TimeoutHandle,
			this,
			&UDeskillzMatchmaking::OnQueueTimeout,
			Params.MaxWaitTime,
			false
		);
	}
	
	// Send join request to server
	SDK->StartMatchmaking(Params.TournamentId);
	
	// Transition to searching
	TransitionToStatus(EDeskillzMatchmakingStatus::Searching);
}

void UDeskillzMatchmaking::JoinQueueSimple(const FString& TournamentId)
{
	FDeskillzMatchmakingParams Params;
	Params.TournamentId = TournamentId;
	JoinQueue(Params);
}

void UDeskillzMatchmaking::LeaveQueue()
{
	if (!IsInQueue())
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Leaving matchmaking queue"));
	
	ClearTimers();
	
	// Notify server
	if (UDeskillzSDK* SDK = SDKRef.Get())
	{
		SDK->CancelMatchmaking();
	}
	
	TransitionToStatus(EDeskillzMatchmakingStatus::Cancelled);
	Reset();
}

void UDeskillzMatchmaking::AcceptMatch()
{
	if (CurrentStatus != EDeskillzMatchmakingStatus::Found)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No match to accept"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Accepting match"));
	
	TransitionToStatus(EDeskillzMatchmakingStatus::Connecting);
	
	// Server will send match ready once both players accept
	// For async matches, this triggers immediately
	if (CurrentParams.PreferredMatchType == EDeskillzMatchType::Asynchronous)
	{
		TransitionToStatus(EDeskillzMatchmakingStatus::Ready);
		OnMatchReady.Broadcast(ReadyMatch);
	}
}

void UDeskillzMatchmaking::DeclineMatch()
{
	if (CurrentStatus != EDeskillzMatchmakingStatus::Found)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No match to decline"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Declining match, returning to queue"));
	
	// Clear found opponent
	FoundOpponent = FDeskillzOpponentPreview();
	
	// Return to searching
	TransitionToStatus(EDeskillzMatchmakingStatus::Searching);
}

// ============================================================================
// Status & Info
// ============================================================================

bool UDeskillzMatchmaking::IsInQueue() const
{
	switch (CurrentStatus)
	{
		case EDeskillzMatchmakingStatus::Queued:
		case EDeskillzMatchmakingStatus::Searching:
		case EDeskillzMatchmakingStatus::Expanding:
		case EDeskillzMatchmakingStatus::Found:
		case EDeskillzMatchmakingStatus::Connecting:
			return true;
		default:
			return false;
	}
}

float UDeskillzMatchmaking::GetWaitTime() const
{
	if (!IsInQueue())
	{
		return 0.0f;
	}
	
	FTimespan Duration = FDateTime::UtcNow() - QueueJoinTime;
	return (float)Duration.GetTotalSeconds();
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzMatchmaking::TransitionToStatus(EDeskillzMatchmakingStatus NewStatus)
{
	if (NewStatus == CurrentStatus)
	{
		return;
	}
	
	EDeskillzMatchmakingStatus OldStatus = CurrentStatus;
	CurrentStatus = NewStatus;
	QueueInfo.Status = NewStatus;
	
	UE_LOG(LogDeskillz, Log, TEXT("Matchmaking status: %d -> %d"), (int32)OldStatus, (int32)NewStatus);
	
	OnStatusChanged.Broadcast(OldStatus, NewStatus);
}

void UDeskillzMatchmaking::QueueTick()
{
	// Update wait time
	QueueInfo.WaitTime = GetWaitTime();
	
	// Update estimated wait (could adjust based on queue position updates)
	QueueInfo.EstimatedWaitTime = FMath::Max(1.0f, QueueInfo.EstimatedWaitTime - 1.0f);
	
	// Broadcast queue update
	OnQueueUpdate.Broadcast(QueueInfo);
}

void UDeskillzMatchmaking::ExpandSearch()
{
	if (!IsInQueue() || CurrentStatus == EDeskillzMatchmakingStatus::Found)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Expanding search range"));
	
	// Increase rating tolerance
	QueueInfo.CurrentRatingRange += CurrentParams.RatingTolerance;
	QueueInfo.bSearchExpanded = true;
	
	TransitionToStatus(EDeskillzMatchmakingStatus::Expanding);
	
	// Schedule another expansion
	if (UWorld* World = WorldRef.Get())
	{
		World->GetTimerManager().SetTimer(
			ExpandHandle,
			this,
			&UDeskillzMatchmaking::ExpandSearch,
			CurrentParams.ExpandSearchAfter,
			false
		);
	}
	
	// After multiple expansions, consider bot match
	if (QueueInfo.CurrentRatingRange > CurrentParams.RatingTolerance * 3 && CurrentParams.bAllowBotMatch)
	{
		UE_LOG(LogDeskillz, Log, TEXT("Creating bot match due to extended wait time"));
		
		// Create bot opponent
		FoundOpponent = FDeskillzOpponentPreview();
		FoundOpponent.bIsBot = true;
		FoundOpponent.Player.Username = TEXT("Practice Bot");
		FoundOpponent.Player.Rating = SDKRef.IsValid() ? SDKRef->GetCurrentPlayer().Rating : 1000;
		FoundOpponent.WinProbability = 0.5f;
		
		TransitionToStatus(EDeskillzMatchmakingStatus::Found);
		OnOpponentFound.Broadcast(FoundOpponent);
	}
}

void UDeskillzMatchmaking::OnQueueTimeout()
{
	if (!IsInQueue())
	{
		return;
	}
	
	float TotalWait = GetWaitTime();
	UE_LOG(LogDeskillz, Log, TEXT("Matchmaking timeout after %.1f seconds"), TotalWait);
	
	ClearTimers();
	
	TransitionToStatus(EDeskillzMatchmakingStatus::Timeout);
	OnTimeout.Broadcast(TotalWait);
	
	// If bot matches allowed, create one instead of failing
	if (CurrentParams.bAllowBotMatch)
	{
		FoundOpponent.bIsBot = true;
		FoundOpponent.Player.Username = TEXT("Practice Bot");
		TransitionToStatus(EDeskillzMatchmakingStatus::Found);
		OnOpponentFound.Broadcast(FoundOpponent);
	}
	else
	{
		Reset();
	}
}

void UDeskillzMatchmaking::HandleWebSocketMessage(const FString& MessageType, TSharedPtr<FJsonObject> Data)
{
	if (MessageType == TEXT("matchmaking_update"))
	{
		// Queue position update
		QueueInfo.PlayersInQueue = Data->GetIntegerField(TEXT("playersInQueue"));
		QueueInfo.QueuePosition = Data->GetIntegerField(TEXT("queuePosition"));
		QueueInfo.EstimatedWaitTime = Data->GetNumberField(TEXT("estimatedWait"));
		
		OnQueueUpdate.Broadcast(QueueInfo);
	}
	else if (MessageType == TEXT("opponent_found"))
	{
		// Opponent found
		FoundOpponent = ParseOpponent(Data);
		
		TransitionToStatus(EDeskillzMatchmakingStatus::Found);
		OnOpponentFound.Broadcast(FoundOpponent);
	}
	else if (MessageType == TEXT("match_ready"))
	{
		// Match is ready to start
		ReadyMatch = ParseMatch(Data);
		
		ClearTimers();
		TransitionToStatus(EDeskillzMatchmakingStatus::Ready);
		OnMatchReady.Broadcast(ReadyMatch);
	}
	else if (MessageType == TEXT("matchmaking_failed"))
	{
		// Matchmaking failed
		FString ErrorMessage = Data->GetStringField(TEXT("message"));
		
		ClearTimers();
		TransitionToStatus(EDeskillzMatchmakingStatus::Failed);
		OnFailed.Broadcast(FDeskillzError(EDeskillzErrorCode::Unknown, ErrorMessage));
		Reset();
	}
}

FDeskillzOpponentPreview UDeskillzMatchmaking::ParseOpponent(TSharedPtr<FJsonObject> Data)
{
	FDeskillzOpponentPreview Opponent;
	
	if (!Data.IsValid())
	{
		return Opponent;
	}
	
	// Parse player info
	if (TSharedPtr<FJsonObject> PlayerObj = Data->GetObjectField(TEXT("player")))
	{
		Opponent.Player.PlayerId = PlayerObj->GetStringField(TEXT("id"));
		Opponent.Player.Username = PlayerObj->GetStringField(TEXT("username"));
		Opponent.Player.AvatarUrl = PlayerObj->GetStringField(TEXT("avatarUrl"));
		Opponent.Player.Rating = PlayerObj->GetIntegerField(TEXT("rating"));
		Opponent.Player.GamesPlayed = PlayerObj->GetIntegerField(TEXT("gamesPlayed"));
		Opponent.Player.Wins = PlayerObj->GetIntegerField(TEXT("wins"));
		
		if (Opponent.Player.GamesPlayed > 0)
		{
			Opponent.Player.WinRate = (float)Opponent.Player.Wins / (float)Opponent.Player.GamesPlayed;
		}
	}
	
	// Calculate rating difference and win probability
	if (UDeskillzSDK* SDK = SDKRef.Get())
	{
		int32 PlayerRating = SDK->GetCurrentPlayer().Rating;
		Opponent.RatingDifference = Opponent.Player.Rating - PlayerRating;
		Opponent.WinProbability = CalculateWinProbability(PlayerRating, Opponent.Player.Rating);
	}
	
	// Parse recent form
	Opponent.RecentFormWins = Data->GetIntegerField(TEXT("recentWins"));
	Opponent.RecentFormLosses = Data->GetIntegerField(TEXT("recentLosses"));
	Opponent.bIsBot = Data->GetBoolField(TEXT("isBot"));
	
	return Opponent;
}

FDeskillzMatchInfo UDeskillzMatchmaking::ParseMatch(TSharedPtr<FJsonObject> Data)
{
	FDeskillzMatchInfo Match;
	
	if (!Data.IsValid())
	{
		return Match;
	}
	
	Match.MatchId = Data->GetStringField(TEXT("matchId"));
	Match.TournamentId = Data->GetStringField(TEXT("tournamentId"));
	Match.DurationSeconds = Data->GetIntegerField(TEXT("duration"));
	Match.RandomSeed = (int64)Data->GetNumberField(TEXT("randomSeed"));
	Match.Status = EDeskillzMatchStatus::Ready;
	
	// Match type
	FString TypeStr = Data->GetStringField(TEXT("matchType"));
	Match.MatchType = (TypeStr == TEXT("synchronous")) 
		? EDeskillzMatchType::Synchronous 
		: EDeskillzMatchType::Asynchronous;
	
	// Opponent
	Match.Opponent = FoundOpponent.Player;
	
	// Local player
	if (UDeskillzSDK* SDK = SDKRef.Get())
	{
		Match.LocalPlayer = SDK->GetCurrentPlayer();
	}
	
	return Match;
}

void UDeskillzMatchmaking::Reset()
{
	CurrentStatus = EDeskillzMatchmakingStatus::Idle;
	QueueInfo = FDeskillzQueueInfo();
	FoundOpponent = FDeskillzOpponentPreview();
	ReadyMatch = FDeskillzMatchInfo();
	CurrentParams = FDeskillzMatchmakingParams();
}

void UDeskillzMatchmaking::ClearTimers()
{
	if (UWorld* World = WorldRef.Get())
	{
		World->GetTimerManager().ClearTimer(QueueTickHandle);
		World->GetTimerManager().ClearTimer(TimeoutHandle);
		World->GetTimerManager().ClearTimer(ExpandHandle);
	}
}

float UDeskillzMatchmaking::CalculateEstimatedWait() const
{
	// Base estimate - would be refined by server data
	float BaseEstimate = 30.0f;
	
	// Adjust based on rating (extreme ratings = longer wait)
	if (UDeskillzSDK* SDK = SDKRef.Get())
	{
		int32 Rating = SDK->GetCurrentPlayer().Rating;
		if (Rating < 800 || Rating > 1600)
		{
			BaseEstimate *= 1.5f;
		}
	}
	
	// Adjust based on tolerance
	float ToleranceFactor = (float)CurrentParams.RatingTolerance / 200.0f;
	BaseEstimate /= ToleranceFactor;
	
	return FMath::Max(5.0f, BaseEstimate);
}

float UDeskillzMatchmaking::CalculateWinProbability(int32 PlayerRating, int32 OpponentRating) const
{
	// ELO-based win probability
	// P(A wins) = 1 / (1 + 10^((Rb - Ra) / 400))
	float RatingDiff = (float)(OpponentRating - PlayerRating);
	float Exponent = RatingDiff / 400.0f;
	float Probability = 1.0f / (1.0f + FMath::Pow(10.0f, Exponent));
	
	return FMath::Clamp(Probability, 0.01f, 0.99f);
}
