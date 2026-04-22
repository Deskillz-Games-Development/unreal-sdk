// Copyright Deskillz Games. All Rights Reserved.
// DeskillzTypes_v352.h - v3.5.2 Additive Types
// Include alongside existing DeskillzTypes.h
// Path: Source/Deskillz/Public/Core/DeskillzTypes_v352.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Core/DeskillzTypes.h"
#include "DeskillzTypes_v352.generated.h"

// ============================================================================
// SOCIAL WIN CONDITION (Phase 2.1)
// ============================================================================

UENUM(BlueprintType)
enum class EDeskillzSocialWinCondition : uint8
{
	FIRST_TO_POINTS   UMETA(DisplayName = "First to Points"),
	FIXED_ROUNDS      UMETA(DisplayName = "Fixed Rounds"),
	TIMED_SESSION     UMETA(DisplayName = "Timed Session"),
	SINGLE_GAME       UMETA(DisplayName = "Single Game"),
	OPEN_ENDED        UMETA(DisplayName = "Open Ended")
};

// ============================================================================
// USER ENROLLMENT STATUS (Phase 2.9)
// ============================================================================

UENUM(BlueprintType)
enum class EDeskillzEnrollmentStatus : uint8
{
	NOT_REGISTERED    UMETA(DisplayName = "Not Registered"),
	REGISTERED        UMETA(DisplayName = "Registered"),
	CHECKIN_OPEN      UMETA(DisplayName = "Check-In Open"),
	CHECKED_IN        UMETA(DisplayName = "Checked In"),
	SEATED            UMETA(DisplayName = "Seated"),
	PLAYING           UMETA(DisplayName = "Playing"),
	WON               UMETA(DisplayName = "Won"),
	ELIMINATED        UMETA(DisplayName = "Eliminated"),
	DQ_NO_SHOW        UMETA(DisplayName = "DQ No Show"),
	DQ_DISCONNECT     UMETA(DisplayName = "DQ Disconnect"),
	STANDBY           UMETA(DisplayName = "Standby"),
	SUBBED_IN         UMETA(DisplayName = "Subbed In")
};

// ============================================================================
// HOST ROLE (Phase 2.13-2.14)
// ============================================================================

UENUM(BlueprintType)
enum class EDeskillzHostRole : uint8
{
	PLAYER            UMETA(DisplayName = "Player (Host plays)"),
	SPECTATOR         UMETA(DisplayName = "Spectator (Host watches)")
};

// ============================================================================
// ESPORT MATCH MODE
// ============================================================================

UENUM(BlueprintType)
enum class EDeskillzEsportMatchMode : uint8
{
	SINGLE_MATCH      UMETA(DisplayName = "Single Match"),
	BEST_OF_3         UMETA(DisplayName = "Best of 3"),
	BEST_OF_5         UMETA(DisplayName = "Best of 5")
};

// ============================================================================
// SOCIAL GAME TYPE
// ============================================================================

UENUM(BlueprintType)
enum class EDeskillzSocialGameType : uint8
{
	BIG_TWO           UMETA(DisplayName = "Big 2"),
	MAHJONG           UMETA(DisplayName = "Mahjong"),
	CHINESE_POKER_13  UMETA(DisplayName = "Thirteen Cards"),
	DOU_DIZHU         UMETA(DisplayName = "Dou Dizhu")
};

// ============================================================================
// QUICK PLAY QUEUE STATE
// ============================================================================

UENUM(BlueprintType)
enum class EDeskillzQuickPlayQueueState : uint8
{
	IDLE              UMETA(DisplayName = "Idle"),
	QUEUED            UMETA(DisplayName = "Queued"),
	FOUND             UMETA(DisplayName = "Found"),
	READY             UMETA(DisplayName = "Ready"),
	PLAYING           UMETA(DisplayName = "Playing")
};

// ============================================================================
// WALLET BALANCE ENTRY (Phase 2.16)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzWalletBalanceEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	FString Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	FString Symbol;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	double Amount = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	double UsdValue = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	FString Color;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	FString Network;
};

// ============================================================================
// PLAYER STATS (Phase 2.17)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	int32 TotalMatches = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	int32 Wins = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	int32 Losses = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	double TotalEarnings = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	int32 TournamentsPlayed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	int32 TournamentsWon = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	int32 CurrentStreak = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	int32 BestStreak = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Stats")
	float WinRate = 0.0f;
};

// ============================================================================
// MATCH RECORD (Phase 2.18)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString GameName;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int64 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int32 Rank = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	double PrizeWon = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString Outcome;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString OpponentName;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FDateTime PlayedAt;
};

// ============================================================================
// QUICK PLAY CONFIG (Phase 2.2)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQuickPlayConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString GameId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	bool bEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 MinPlayers = 2;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 MaxPlayers = 8;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	double EntryFee = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 MatchDurationSeconds = 120;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 QueueTimeoutSeconds = 60;

	// Social win condition fields (v3.5.1)
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString SocialWinCondition;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	TArray<int32> SocialPointTargets;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	TArray<int32> SocialRoundTargets;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 SocialDefaultTarget = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	bool bSocialAllowFreePlay = false;
};

// ============================================================================
// DISPUTE RECORD (Phase 2.7)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzDisputeRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString RoomCode;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString Reason;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FString Resolution;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	TArray<FString> Evidence;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Dispute")
	FDateTime CreatedAt;

	bool IsResolved() const { return Status == TEXT("RESOLVED") || Status == TEXT("REJECTED"); }
};

// ============================================================================
// ACTIVE SESSION PAYLOAD (Phase 2.8)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzActiveSessionPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	bool bHasActiveSession = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	FString Type;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	FString RoomId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	FString RoomCode;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	FString GameId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Session")
	FString Status;

	bool IsRoom() const { return Type == TEXT("room"); }
	bool IsTournament() const { return Type == TEXT("tournament"); }
	bool IsQuickPlay() const { return Type == TEXT("quickplay"); }
};

// ============================================================================
// GAME CAPABILITIES (Phase 2.12 / 9.2)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzGameCapabilities
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	bool bSupports1v1 = true;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	bool bSupportsFFA = true;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	bool bSupportsSingleElimination = true;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	bool bSupportsSync = true;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	bool bSupportsAsync = true;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	bool bSupportsTurnBased = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	int32 MinMatchDurationSeconds = 30;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	int32 MaxMatchDurationSeconds = 600;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	int32 MaxTournamentSize = 256;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	int32 MinPlayersPerMatch = 2;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Capabilities")
	int32 MaxPlayersPerMatch = 8;

	static FDeskillzGameCapabilities Default() { return FDeskillzGameCapabilities(); }
};

// ============================================================================
// TOURNAMENT LISTING (Phase 2.3)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournamentListing
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString GameId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString GameName;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	double EntryFee = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	double PrizePool = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 MinPlayersPerTable = 2;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString SocialGameType;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FDateTime ScheduledStart;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	bool bIsFeatured = false;

	bool IsFull() const { return MaxPlayers > 0 && CurrentPlayers >= MaxPlayers; }
	bool IsFree() const { return EntryFee <= 0.0; }
	bool IsSocialGame() const { return !SocialGameType.IsEmpty(); }
};

// ============================================================================
// TOURNAMENT REGISTRATION (Phase 2.4)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournamentRegistration
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString TournamentName;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString BookingStatus;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	double EntryFee = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FDateTime RegisteredAt;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FDateTime ScheduledStart;
};

// ============================================================================
// TOURNAMENT ENROLLMENT STATE (Phase 2.5)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzEnrollmentState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString BookingStatus;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	bool bIsRegistered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	bool bIsCheckedIn = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	bool bCanCheckIn = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	bool bCanLeave = false;
};

// ============================================================================
// TOURNAMENT SCHEDULE (Phase 2.6)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTableSeat
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 SeatNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Username;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	bool bIsNPC = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 FinalScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	bool bIsWinner = false;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournamentTable
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 TableNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	TArray<FDeskillzTableSeat> Seats;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournamentRound
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 RoundNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 TotalTables = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 PlayersRemaining = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	TArray<FDeskillzTournamentTable> Tables;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournamentSchedule
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 TotalRounds = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 CurrentRound = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 TotalPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 PlayersRemaining = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	TArray<FDeskillzTournamentRound> Rounds;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTableAssignment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString TournamentId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 RoundNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString TableId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 TableNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 SeatNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	TArray<FDeskillzTableSeat> Opponents;
};

// ============================================================================
// QUICK PLAY REQUEST/RESPONSE (Phase 2.15)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQuickPlayJoinParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|QuickPlay")
	FString GameId;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|QuickPlay")
	double EntryFee = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|QuickPlay")
	FString Currency = TEXT("USDT_BSC");

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|QuickPlay")
	int32 PlayerCount = 2;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQuickPlayJoinResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString QueueKey;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString GameId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 Position = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 EstimatedWaitSeconds = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString MatchId;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQuickPlayLaunchData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString MatchSessionId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString GameId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString DeepLink;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString Token;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	double EntryFee = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	double PrizePool = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 MatchDurationSecs = 120;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQuickPlayScoreResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int64 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	bool bAllScoresSubmitted = false;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQuickPlayPlayerResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString Username;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int64 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	int32 Rank = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	double PrizeWon = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	bool bIsWinner = false;
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzQuickPlayMatchResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString MatchId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString WinnerId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	double PrizePool = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	FString Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|QuickPlay")
	TArray<FDeskillzQuickPlayPlayerResult> Players;
};

// ============================================================================
// ROOM CREATION OPTIONS (Phase 2.13-2.14)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzCreateEsportRoomOpts
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	double EntryFee = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Currency = TEXT("USDT_BSC");

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 MinPlayers = 2;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 MaxPlayers = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	EDeskillzHostRole HostRole = EDeskillzHostRole::PLAYER;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 MatchDurationSeconds = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	EDeskillzEsportMatchMode MatchMode = EDeskillzEsportMatchMode::SINGLE_MATCH;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Visibility = TEXT("UNLISTED");
};

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzCreateSocialRoomOpts
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString SocialGameType;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	double TableStakes = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Currency = TEXT("USDT_BSC");

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 PlayersPerTable = 4;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 MaxTables = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	EDeskillzHostRole HostRole = EDeskillzHostRole::PLAYER;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	double RakePercent = 5.0;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Visibility = TEXT("UNLISTED");

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	EDeskillzSocialWinCondition WinCondition = EDeskillzSocialWinCondition::OPEN_ENDED;

	UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 WinConditionTarget = 0;
};

// ============================================================================
// ROOM INVITE (Phase 5)
// ============================================================================

USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzRoomInvite
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString RoomId;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString RoomCode;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString RoomName;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString SenderUsername;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	double EntryFee = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Status;
};

// ============================================================================
// CURRENCY DISPLAY HELPER (Phase 2.11)
// ============================================================================

struct DESKILLZ_API FDeskillzCurrencyHelper
{
	static FString GetDisplayName(const FString& CurrencyCode)
	{
		if (CurrencyCode == TEXT("BNB"))       return TEXT("BNB");
		if (CurrencyCode == TEXT("USDT_BSC"))  return TEXT("USDT (BSC)");
		if (CurrencyCode == TEXT("USDT_TRON")) return TEXT("USDT (Tron)");
		if (CurrencyCode == TEXT("USDC_BSC"))  return TEXT("USDC (BSC)");
		if (CurrencyCode == TEXT("USDC_TRON")) return TEXT("USDC (Tron)");
		if (CurrencyCode == TEXT("Free"))      return TEXT("Free");
		return CurrencyCode;
	}

	static FString GetSymbol(const FString& CurrencyCode)
	{
		if (CurrencyCode.Contains(TEXT("USDT"))) return TEXT("USDT");
		if (CurrencyCode.Contains(TEXT("USDC"))) return TEXT("USDC");
		if (CurrencyCode == TEXT("BNB"))         return TEXT("BNB");
		return CurrencyCode;
	}

	static FString GetNetwork(const FString& CurrencyCode)
	{
		if (CurrencyCode.Contains(TEXT("_BSC")))  return TEXT("BSC");
		if (CurrencyCode.Contains(TEXT("_TRON"))) return TEXT("TRON");
		if (CurrencyCode == TEXT("BNB"))          return TEXT("BSC");
		return TEXT("");
	}

	static FString Format(double Amount, const FString& CurrencyCode)
	{
		if (CurrencyCode == TEXT("Free")) return TEXT("Free");
		FString Sym = GetSymbol(CurrencyCode);
		FString Net = GetNetwork(CurrencyCode);
		FString Result = FString::Printf(TEXT("%.2f %s"), Amount, *Sym);
		if (!Net.IsEmpty()) Result += FString::Printf(TEXT(" (%s)"), *Net);
		return Result;
	}
};

// ============================================================================
// v3.5.2 DELEGATES
// ============================================================================

// Tournament delegates
DECLARE_DELEGATE_OneParam(FOnTournamentListings, const TArray<FDeskillzTournamentListing>&);
DECLARE_DELEGATE_OneParam(FOnTournamentRegistration, const FDeskillzTournamentRegistration&);
DECLARE_DELEGATE_OneParam(FOnEnrollmentState, const FDeskillzEnrollmentState&);
DECLARE_DELEGATE_OneParam(FOnTournamentSchedule, const FDeskillzTournamentSchedule&);
DECLARE_DELEGATE_OneParam(FOnTableAssignment, const FDeskillzTableAssignment&);
DECLARE_DELEGATE_OneParam(FOnTournamentRegistrations, const TArray<FDeskillzTournamentRegistration>&);

// Quick Play delegates
DECLARE_DELEGATE_OneParam(FOnQuickPlayJoinResult, const FDeskillzQuickPlayJoinResult&);
DECLARE_DELEGATE_OneParam(FOnQuickPlayConfig, const FDeskillzQuickPlayConfig&);
DECLARE_DELEGATE_OneParam(FOnQuickPlayLaunchData, const FDeskillzQuickPlayLaunchData&);
DECLARE_DELEGATE_OneParam(FOnQuickPlayScoreResult, const FDeskillzQuickPlayScoreResult&);
DECLARE_DELEGATE_OneParam(FOnQuickPlayMatchResult, const FDeskillzQuickPlayMatchResult&);

// Dispute delegates
DECLARE_DELEGATE_OneParam(FOnDisputeRecord, const FDeskillzDisputeRecord&);
DECLARE_DELEGATE_OneParam(FOnDisputeRecords, const TArray<FDeskillzDisputeRecord>&);

// Wallet delegates
DECLARE_DELEGATE_OneParam(FOnWalletBalances, const TArray<FDeskillzWalletBalanceEntry>&);
DECLARE_DELEGATE_OneParam(FOnWalletBalance, const FDeskillzWalletBalanceEntry&);
DECLARE_DELEGATE_OneParam(FOnPlayerStats, const FDeskillzPlayerStats&);
DECLARE_DELEGATE_OneParam(FOnMatchRecords, const TArray<FDeskillzMatchRecord>&);

// Session delegates
DECLARE_DELEGATE_OneParam(FOnActiveSession, const FDeskillzActiveSessionPayload&);
DECLARE_DELEGATE_OneParam(FOnGameCapabilities, const FDeskillzGameCapabilities&);

// Generic
DECLARE_DELEGATE(FOnSuccess);
DECLARE_DELEGATE_OneParam(FOnDeskillzApiError, const FDeskillzError&);

// ============================================================================
// v3.5.2 API ENDPOINTS (Phase 11 -- additive)
// ============================================================================

namespace DeskillzApi_v352
{
	namespace Tournament
	{
		static FString Register(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/tournaments/%s/register"), *Id); }

		static FString CheckIn(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/tournaments/%s/checkin"), *Id); }

		static FString Leave(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/tournaments/%s/leave"), *Id); }

		static FString MyStatus(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/tournaments/%s/my-status"), *Id); }

		static FString Schedule(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/tournaments/%s/schedule"), *Id); }

		static FString MySeat(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/tournaments/%s/my-seat"), *Id); }

		static FString ActiveByGame(const FString& GameId)
		{ return FString::Printf(TEXT("/api/v1/tournaments/game/%s/active"), *GameId); }

		static const FString MyRegistrations = TEXT("/api/v1/tournaments/my-registrations");
	}

	namespace QuickPlay
	{
		static const FString Join = TEXT("/api/v1/lobby/quick-play/join");
		static const FString Leave = TEXT("/api/v1/lobby/quick-play/leave");
		static const FString Status = TEXT("/api/v1/lobby/quick-play/status");

		static FString Config(const FString& GameId)
		{ return FString::Printf(TEXT("/api/v1/quick-play/games/%s"), *GameId); }

		static const FString MatchLaunch = TEXT("/api/v1/lobby/quick-play/match/launch");

		static FString MatchScore(const FString& MatchId)
		{ return FString::Printf(TEXT("/api/v1/lobby/quick-play/match/%s/score"), *MatchId); }

		static FString MatchResults(const FString& MatchId)
		{ return FString::Printf(TEXT("/api/v1/lobby/quick-play/match/%s/results"), *MatchId); }

		static FString MatchComplete(const FString& MatchId)
		{ return FString::Printf(TEXT("/api/v1/lobby/quick-play/match/%s/complete"), *MatchId); }

		static const FString SocialCreate = TEXT("/api/v1/lobby/quick-play/social/create");

		static FString SocialRound(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/lobby/quick-play/social/%s/round"), *RoomId); }

		static FString SocialRebuy(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/lobby/quick-play/social/%s/rebuy"), *RoomId); }

		static FString SocialCashOut(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/lobby/quick-play/social/%s/cashout"), *RoomId); }

		static FString SocialEnd(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/lobby/quick-play/social/%s/end"), *RoomId); }
	}

	namespace Dispute
	{
		static const FString File = TEXT("/api/v1/disputes");
		static const FString Mine = TEXT("/api/v1/disputes/me");

		static FString Details(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/disputes/%s"), *Id); }

		static FString Evidence(const FString& Id)
		{ return FString::Printf(TEXT("/api/v1/disputes/%s/evidence"), *Id); }

		static const FString RecentMatches = TEXT("/api/v1/matches/history/me");
	}

	namespace Room
	{
		static const FString Create = TEXT("/api/v1/private-rooms/create");
		static const FString MyActive = TEXT("/api/v1/private-rooms/my-active");
		static const FString MyInvites = TEXT("/api/v1/private-rooms/invites/my");

		static FString BuyIn(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/private-rooms/%s/buy-in"), *RoomId); }

		static FString CashOut(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/private-rooms/%s/cash-out"), *RoomId); }

		static FString Rebuy(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/private-rooms/%s/rebuy"), *RoomId); }

		static FString Round(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/private-rooms/%s/round"), *RoomId); }

		static FString Settle(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/private-rooms/%s/settle"), *RoomId); }

		static FString Invite(const FString& RoomId)
		{ return FString::Printf(TEXT("/api/v1/private-rooms/%s/invite"), *RoomId); }

		static FString RespondInvite(const FString& InviteId)
		{ return FString::Printf(TEXT("/api/v1/private-rooms/invites/%s/respond"), *InviteId); }
	}

	namespace Wallet
	{
		static const FString Balance = TEXT("/api/v1/wallet/balance");

		static FString BalanceForCurrency(const FString& Cur)
		{ return FString::Printf(TEXT("/api/v1/wallet/balance/%s"), *Cur); }

		static const FString Deposit = TEXT("/api/v1/wallet/deposit");
		static const FString Withdraw = TEXT("/api/v1/wallet/withdraw");
		static const FString Transactions = TEXT("/api/v1/wallet/transactions");
	}

	namespace Host
	{
		static const FString Dashboard = TEXT("/api/v1/host/dashboard");
		static const FString Withdraw = TEXT("/api/v1/host/withdraw");
		static const FString VerifyAgeStatus = TEXT("/api/v1/host/verify-age/status");
	}

	namespace Capabilities
	{
		static FString ForGame(const FString& GameId)
		{ return FString::Printf(TEXT("/api/v1/games/%s/capabilities"), *GameId); }
	}
}