// Copyright Deskillz Games. All Rights Reserved.
// DeskillzLobbyTypes.h - Lobby-specific types for centralized lobby architecture

#pragma once

#include "CoreMinimal.h"
#include "DeskillzTypes.h"
#include "DeskillzLobbyTypes.generated.h"

// ============================================================================
// Deep Link Launch Types (for Centralized Lobby)
// ============================================================================

/**
 * Match launch parameters received from main Deskillz app via deep link
 * 
 * When a player joins a match through the centralized lobby (website/app),
 * the game is launched with these parameters via deep link.
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchLaunchData
{
	GENERATED_BODY()
	
	/** Unique match identifier */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	FString MatchId;
	
	/** Tournament identifier (if tournament match) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	FString TournamentId;
	
	/** Player's authentication token for this session */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	FString PlayerToken;
	
	/** Match type (synchronous or asynchronous) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	EDeskillzMatchType MatchType = EDeskillzMatchType::Asynchronous;
	
	/** Match duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	int32 DurationSeconds = 180;
	
	/** Random seed for deterministic gameplay */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	int64 RandomSeed = 0;
	
	/** Entry fee amount */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	double EntryFee = 0.0;
	
	/** Entry fee currency */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	EDeskillzCurrency Currency = EDeskillzCurrency::USDT;
	
	/** Prize pool amount */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	double PrizePool = 0.0;
	
	/** Number of rounds */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	int32 Rounds = 1;
	
	/** Room code (for private matches) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	FString RoomCode;
	
	/** Is this a private room match */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	bool bIsPrivateRoom = false;
	
	/** Score type (points, time, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	FString ScoreType = TEXT("points");
	
	/** Custom parameters from tournament/match settings */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	TMap<FString, FString> CustomParams;
	
	/** Opponent information (if available) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	FDeskillzPlayer Opponent;
	
	/** Is this data valid */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	bool bIsValid = false;
	
	/** Timestamp when received */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Lobby")
	int64 Timestamp = 0;
	
	FDeskillzMatchLaunchData() = default;
	
	/** Check if this is a tournament match */
	bool IsTournamentMatch() const { return !TournamentId.IsEmpty(); }
	
	/** Check if this is a synchronous match */
	bool IsSynchronous() const { return MatchType == EDeskillzMatchType::Synchronous; }
	
	/** Check if opponent info is available */
	bool HasOpponent() const { return !Opponent.PlayerId.IsEmpty(); }
};

/**
 * Match completion data to send back to main app
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchCompletionData
{
	GENERATED_BODY()
	
	/** Match identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Lobby")
	FString MatchId;
	
	/** Final score */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Lobby")
	int64 FinalScore = 0;
	
	/** Match result */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Lobby")
	EDeskillzMatchResult Result = EDeskillzMatchResult::Pending;
	
	/** Match duration (actual time played) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Lobby")
	float MatchDuration = 0.0f;
	
	/** Was the match completed normally */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Lobby")
	bool bCompletedNormally = true;
	
	/** Abort reason (if not completed normally) */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Lobby")
	FString AbortReason;
	
	/** Additional stats to report */
	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Lobby")
	TMap<FString, FString> Stats;
	
	FDeskillzMatchCompletionData() = default;
};

/**
 * Return destination after match
 */
UENUM(BlueprintType)
enum class EDeskillzReturnDestination : uint8
{
	/** Return to lobby/home */
	Lobby                UMETA(DisplayName = "Lobby"),
	
	/** Return to match results */
	Results              UMETA(DisplayName = "Results"),
	
	/** Return to tournament details */
	Tournament           UMETA(DisplayName = "Tournament"),
	
	/** Return to leaderboard */
	Leaderboard          UMETA(DisplayName = "Leaderboard"),
	
	/** Return to rematch option */
	Rematch              UMETA(DisplayName = "Rematch")
};

// ============================================================================
// Lobby Delegates
// ============================================================================

/** Called when a match is received from deep link */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchLaunchReceived, const FDeskillzMatchLaunchData&, LaunchData);

/** Called when match is ready to start (after validation) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchReady, const FDeskillzMatchLaunchData&, LaunchData);

/** Called when returning to main app */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReturnToMainApp, EDeskillzReturnDestination, Destination, const FString&, Data);

/** Called when deep link validation fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLaunchValidationFailed, const FString&, Reason, const FDeskillzMatchLaunchData&, LaunchData);