// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DeskillzRoomTypes.generated.h"

// =============================================================================
// ENUMS
// =============================================================================

/**
 * Room visibility options
 */
UENUM(BlueprintType)
enum class ERoomVisibility : uint8
{
	/** Listed publicly, anyone can see and join */
	PublicListed UMETA(DisplayName = "Public Listed"),
	
	/** Not listed, but anyone with code can join */
	Unlisted UMETA(DisplayName = "Unlisted"),
	
	/** Invite only, requires host approval */
	Private UMETA(DisplayName = "Private")
};

/**
 * Room status states
 */
UENUM(BlueprintType)
enum class ERoomStatus : uint8
{
	/** Room is waiting for players */
	Waiting UMETA(DisplayName = "Waiting"),
	
	/** All players ready, awaiting start */
	ReadyCheck UMETA(DisplayName = "Ready Check"),
	
	/** Countdown in progress */
	Countdown UMETA(DisplayName = "Countdown"),
	
	/** Match is launching */
	Launching UMETA(DisplayName = "Launching"),
	
	/** Match in progress */
	InProgress UMETA(DisplayName = "In Progress"),
	
	/** Match completed */
	Completed UMETA(DisplayName = "Completed"),
	
	/** Room was cancelled */
	Cancelled UMETA(DisplayName = "Cancelled"),
	
	/** Room expired (24h limit) */
	Expired UMETA(DisplayName = "Expired")
};

/**
 * Room game mode
 */
UENUM(BlueprintType)
enum class ERoomMode : uint8
{
	/** Synchronous (real-time) gameplay */
	Sync UMETA(DisplayName = "Synchronous"),
	
	/** Asynchronous (turn-based) gameplay */
	Async UMETA(DisplayName = "Asynchronous")
};

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * Room host information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FRoomHost
{
	GENERATED_BODY()

	/** Host user ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Id;

	/** Host username */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Username;

	/** Host avatar URL */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString AvatarUrl;

	FRoomHost() {}
};

/**
 * Room game information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FRoomGame
{
	GENERATED_BODY()

	/** Game ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Id;

	/** Game name */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Name;

	/** Game icon URL */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString IconUrl;

	FRoomGame() {}
};

/**
 * Player in a room
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FRoomPlayer
{
	GENERATED_BODY()

	/** Player user ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Id;

	/** Player username */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Username;

	/** Player avatar URL */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString AvatarUrl;

	/** Whether player is ready */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	bool bIsReady;

	/** Whether player is room admin/host */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	bool bIsAdmin;

	/** When player joined the room */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FDateTime JoinedAt;

	FRoomPlayer()
		: bIsReady(false)
		, bIsAdmin(false)
	{}
};

/**
 * Private room data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FPrivateRoom
{
	GENERATED_BODY()

	/** Unique room identifier */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Id;

	/** Room code for sharing (e.g., DSKZ-AB3C) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString RoomCode;

	/** Room display name */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Name;

	/** Optional room description */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Description;

	/** Room host information */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FRoomHost Host;

	/** Game information */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FRoomGame Game;

	/** Game mode (Sync/Async) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	ERoomMode Mode;

	/** Entry fee amount */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	float EntryFee;

	/** Entry fee currency code */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString EntryCurrency;

	/** Current prize pool */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	float PrizePool;

	/** Minimum players to start */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	int32 MinPlayers;

	/** Maximum players allowed */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	int32 MaxPlayers;

	/** Current number of players */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	int32 CurrentPlayers;

	/** Current room status */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	ERoomStatus Status;

	/** Room visibility setting */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	ERoomVisibility Visibility;

	/** Whether join requests require approval */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	bool bInviteRequired;

	/** List of players in room */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	TArray<FRoomPlayer> Players;

	/** When the room was created */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FDateTime CreatedAt;

	/** When the room expires */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FDateTime ExpiresAt;

	FPrivateRoom()
		: Mode(ERoomMode::Sync)
		, EntryFee(0.0f)
		, PrizePool(0.0f)
		, MinPlayers(2)
		, MaxPlayers(10)
		, CurrentPlayers(0)
		, Status(ERoomStatus::Waiting)
		, Visibility(ERoomVisibility::Unlisted)
		, bInviteRequired(false)
	{}

	/** Check if room is full */
	bool IsFull() const { return CurrentPlayers >= MaxPlayers; }

	/** Check if room can be joined */
	bool CanJoin() const { return Status == ERoomStatus::Waiting && !IsFull(); }

	/** Check if all players are ready */
	bool AreAllPlayersReady() const;

	/** Get number of ready players */
	int32 GetReadyPlayerCount() const;

	/** Find a player by ID */
	const FRoomPlayer* FindPlayer(const FString& PlayerId) const;
};

/**
 * Configuration for creating a room
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FCreateRoomConfig
{
	GENERATED_BODY()

	/** Room display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Name;

	/** Optional description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString Description;

	/** Entry fee amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	float EntryFee;

	/** Entry fee currency code (BTC, ETH, USDT, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	FString EntryCurrency;

	/** Minimum players to start (default: 2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 MinPlayers;

	/** Maximum players allowed (default: 10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 MaxPlayers;

	/** Room visibility (default: Unlisted) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	ERoomVisibility Visibility;

	/** Game mode (default: Sync) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	ERoomMode Mode;

	/** Match duration in seconds (optional, 0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 MatchDuration;

	/** Number of rounds (default: 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	int32 RoundsCount;

	/** Whether join requests need approval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms")
	bool bInviteRequired;

	FCreateRoomConfig()
		: EntryFee(0.0f)
		, EntryCurrency(TEXT("USDT"))
		, MinPlayers(2)
		, MaxPlayers(10)
		, Visibility(ERoomVisibility::Unlisted)
		, Mode(ERoomMode::Sync)
		, MatchDuration(0)
		, RoundsCount(1)
		, bInviteRequired(false)
	{}
};

/**
 * Match launch data received when match starts
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FMatchLaunchData
{
	GENERATED_BODY()

	/** Match session ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString MatchId;

	/** Room code the match originated from */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString RoomCode;

	/** Deep link URL (for website players) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString DeepLink;

	/** Launch token for authentication */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Token;

	/** Game session ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString GameSessionId;

	FMatchLaunchData() {}
};

/**
 * Room error information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FRoomError
{
	GENERATED_BODY()

	/** Error code */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Code;

	/** Human-readable error message */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FString Message;

	FRoomError() {}

	FRoomError(const FString& InCode, const FString& InMessage)
		: Code(InCode)
		, Message(InMessage)
	{}

	/** Check if this represents an actual error */
	bool IsError() const { return !Code.IsEmpty(); }

	// Common error codes
	static const FString NotAuthenticated;
	static const FString NotInRoom;
	static const FString NotHost;
	static const FString RoomFull;
	static const FString RoomNotFound;
	static const FString InvalidCode;
	static const FString AlreadyInRoom;
	static const FString NotReady;
	static const FString InsufficientFunds;
	static const FString RoomExpired;
	static const FString RoomCancelled;
	static const FString NetworkError;
	static const FString ServerError;
};

// =============================================================================
// DELEGATES
// =============================================================================

/** Delegate for room operations that return a room */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRoomSuccess, const FPrivateRoom&, Room);

/** Delegate for room list operations */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRoomListSuccess, const TArray<FPrivateRoom>&, Rooms);

/** Delegate for simple success operations */
DECLARE_DYNAMIC_DELEGATE(FOnRoomActionSuccess);

/** Delegate for error handling */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRoomError, const FRoomError&, Error);

/** Delegate for player events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoinedRoom, const FRoomPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeftRoom, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerReadyChanged, const FString&, PlayerId, bool, bIsReady);

/** Delegate for room state events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomJoined, const FPrivateRoom&, Room);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomUpdated, const FPrivateRoom&, Room);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCancelled, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoomLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPlayersReady);

/** Delegate for countdown events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownStarted, int32, Seconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, Seconds);

/** Delegate for match launching */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchLaunching, const FMatchLaunchData&, LaunchData);

/** Delegate for being kicked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKickedFromRoom, const FString&, Reason);

/** Delegate for chat messages */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatReceived, const FString&, SenderId, const FString&, Username, const FString&, Message);

/** Delegate for errors */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomErrorOccurred, const FRoomError&, Error);