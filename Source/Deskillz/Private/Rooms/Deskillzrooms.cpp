// Copyright Deskillz Games. All Rights Reserved.

#include "DeskillzRooms.h"
#include "DeskillzRoomClient.h"
#include "DeskillzSDK.h"
#include "DeskillzConfig.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

// =============================================================================
// Static Error Codes
// =============================================================================

const FString FRoomError::NotAuthenticated = TEXT("NOT_AUTHENTICATED");
const FString FRoomError::NotInRoom = TEXT("NOT_IN_ROOM");
const FString FRoomError::NotHost = TEXT("NOT_HOST");
const FString FRoomError::RoomFull = TEXT("ROOM_FULL");
const FString FRoomError::RoomNotFound = TEXT("ROOM_NOT_FOUND");
const FString FRoomError::InvalidCode = TEXT("INVALID_CODE");
const FString FRoomError::AlreadyInRoom = TEXT("ALREADY_IN_ROOM");
const FString FRoomError::NotReady = TEXT("NOT_ALL_READY");
const FString FRoomError::InsufficientFunds = TEXT("INSUFFICIENT_FUNDS");
const FString FRoomError::RoomExpired = TEXT("ROOM_EXPIRED");
const FString FRoomError::RoomCancelled = TEXT("ROOM_CANCELLED");
const FString FRoomError::NetworkError = TEXT("NETWORK_ERROR");
const FString FRoomError::ServerError = TEXT("SERVER_ERROR");

// =============================================================================
// FPrivateRoom Implementation
// =============================================================================

bool FPrivateRoom::AreAllPlayersReady() const
{
	if (Players.Num() < MinPlayers)
	{
		return false;
	}

	for (const FRoomPlayer& Player : Players)
	{
		if (!Player.bIsReady)
		{
			return false;
		}
	}

	return true;
}

int32 FPrivateRoom::GetReadyPlayerCount() const
{
	int32 Count = 0;
	for (const FRoomPlayer& Player : Players)
	{
		if (Player.bIsReady)
		{
			Count++;
		}
	}
	return Count;
}

const FRoomPlayer* FPrivateRoom::FindPlayer(const FString& PlayerId) const
{
	for (const FRoomPlayer& Player : Players)
	{
		if (Player.Id == PlayerId)
		{
			return &Player;
		}
	}
	return nullptr;
}

// =============================================================================
// UDeskillzRooms Implementation
// =============================================================================

UDeskillzRooms::UDeskillzRooms()
	: RoomClient(nullptr)
	, bIsInitialized(false)
{
}

void UDeskillzRooms::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create room client
	RoomClient = NewObject<UDeskillzRoomClient>(this);
	RoomClient->Initialize();

	// Bind WebSocket events
	RoomClient->OnStateUpdate.AddUObject(this, &UDeskillzRooms::HandleRoomStateUpdate);
	RoomClient->OnPlayerJoined.AddUObject(this, &UDeskillzRooms::HandlePlayerJoined);
	RoomClient->OnPlayerLeft.AddUObject(this, &UDeskillzRooms::HandlePlayerLeft);
	RoomClient->OnPlayerKicked.AddUObject(this, &UDeskillzRooms::HandlePlayerKicked);
	RoomClient->OnPlayerReady.AddUObject(this, &UDeskillzRooms::HandlePlayerReady);
	RoomClient->OnAllReady.AddUObject(this, &UDeskillzRooms::HandleAllReady);
	RoomClient->OnCountdownStarted.AddUObject(this, &UDeskillzRooms::HandleCountdownStarted);
	RoomClient->OnCountdownTick.AddUObject(this, &UDeskillzRooms::HandleCountdownTick);
	RoomClient->OnLaunching.AddUObject(this, &UDeskillzRooms::HandleLaunching);
	RoomClient->OnCancelled.AddUObject(this, &UDeskillzRooms::HandleCancelled);
	RoomClient->OnKicked.AddUObject(this, &UDeskillzRooms::HandleKicked);
	RoomClient->OnChat.AddUObject(this, &UDeskillzRooms::HandleChat);
	RoomClient->OnError.AddUObject(this, &UDeskillzRooms::HandleError);

	bIsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Initialized"));
}

void UDeskillzRooms::Deinitialize()
{
	if (IsInRoom())
	{
		DisconnectFromRoom();
	}

	if (RoomClient)
	{
		RoomClient->Shutdown();
		RoomClient = nullptr;
	}

	bIsInitialized = false;

	UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Deinitialized"));

	Super::Deinitialize();
}

bool UDeskillzRooms::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

UDeskillzRooms* UDeskillzRooms::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UDeskillzRooms>();
}

// =============================================================================
// Properties
// =============================================================================

bool UDeskillzRooms::IsInRoom() const
{
	return !CurrentRoom.Id.IsEmpty();
}

bool UDeskillzRooms::IsHost() const
{
	if (!IsInRoom())
	{
		return false;
	}

	UDeskillzSDK* SDK = UDeskillzSDK::Get(this);
	if (!SDK)
	{
		return false;
	}

	return CurrentRoom.Host.Id == SDK->GetCurrentUserId();
}

bool UDeskillzRooms::IsConnected() const
{
	return RoomClient ? RoomClient->IsConnected() : false;
}

// =============================================================================
// Room Creation
// =============================================================================

void UDeskillzRooms::CreateRoom(
	const FCreateRoomConfig& Config,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!ValidateAuthentication(OnError))
	{
		return;
	}

	if (IsInRoom())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::AlreadyInRoom, TEXT("Already in a room. Leave first.")));
		return;
	}

	RoomClient->CreateRoom(Config,
		FOnRoomSuccess::CreateLambda([this, OnSuccess](const FPrivateRoom& Room)
		{
			CurrentRoom = Room;
			ConnectToRoom(Room.Id);
			OnSuccess.ExecuteIfBound(Room);
			OnRoomJoined.Broadcast(Room);
			UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Created room: %s"), *Room.RoomCode);
		}),
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

void UDeskillzRooms::QuickCreateRoom(
	const FString& RoomName,
	float EntryFee,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	FCreateRoomConfig Config;
	Config.Name = RoomName;
	Config.EntryFee = EntryFee;
	Config.EntryCurrency = TEXT("USDT");
	Config.MinPlayers = 2;
	Config.MaxPlayers = 2;
	Config.Visibility = ERoomVisibility::Unlisted;

	CreateRoom(Config, OnSuccess, OnError);
}

// =============================================================================
// Room Discovery
// =============================================================================

void UDeskillzRooms::GetPublicRooms(
	FOnRoomListSuccess OnSuccess,
	FOnRoomError OnError)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(this);
	if (!SDK)
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("SDK not initialized")));
		return;
	}

	FString GameId = SDK->GetGameId();
	if (GameId.IsEmpty())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Game ID not configured")));
		return;
	}

	RoomClient->GetPublicRooms(GameId, OnSuccess,
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

void UDeskillzRooms::GetMyRooms(
	FOnRoomListSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!ValidateAuthentication(OnError))
	{
		return;
	}

	RoomClient->GetMyRooms(OnSuccess,
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

void UDeskillzRooms::GetRoomByCode(
	const FString& RoomCode,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (RoomCode.IsEmpty())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::InvalidCode, TEXT("Room code is required")));
		return;
	}

	RoomClient->GetRoomByCode(RoomCode, OnSuccess,
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

// =============================================================================
// Join / Leave
// =============================================================================

void UDeskillzRooms::JoinRoom(
	const FString& RoomCode,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!ValidateAuthentication(OnError))
	{
		return;
	}

	if (IsInRoom())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::AlreadyInRoom, TEXT("Already in a room. Leave first.")));
		return;
	}

	if (RoomCode.IsEmpty())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::InvalidCode, TEXT("Room code is required")));
		return;
	}

	RoomClient->JoinRoom(RoomCode,
		FOnRoomSuccess::CreateLambda([this, OnSuccess](const FPrivateRoom& Room)
		{
			CurrentRoom = Room;
			ConnectToRoom(Room.Id);
			OnSuccess.ExecuteIfBound(Room);
			OnRoomJoined.Broadcast(Room);
			UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Joined room: %s"), *Room.RoomCode);
		}),
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

void UDeskillzRooms::LeaveRoom(
	FOnRoomActionSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!IsInRoom())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotInRoom, TEXT("Not in a room")));
		return;
	}

	FString RoomId = CurrentRoom.Id;
	FString RoomCode = CurrentRoom.RoomCode;

	RoomClient->LeaveRoom(RoomId,
		FOnRoomActionSuccess::CreateLambda([this, RoomCode, OnSuccess]()
		{
			DisconnectFromRoom();
			CurrentRoom = FPrivateRoom();
			OnSuccess.ExecuteIfBound();
			OnRoomLeft.Broadcast();
			UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Left room: %s"), *RoomCode);
		}),
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

// =============================================================================
// Ready Status
// =============================================================================

void UDeskillzRooms::SetReady(bool bIsReady)
{
	if (!IsInRoom())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzRooms] Cannot set ready: not in a room"));
		return;
	}

	RoomClient->SendReady(CurrentRoom.Id, bIsReady);
}

void UDeskillzRooms::ToggleReady()
{
	if (!IsInRoom())
	{
		return;
	}

	UDeskillzSDK* SDK = UDeskillzSDK::Get(this);
	if (!SDK)
	{
		return;
	}

	const FRoomPlayer* CurrentPlayer = CurrentRoom.FindPlayer(SDK->GetCurrentUserId());
	if (CurrentPlayer)
	{
		SetReady(!CurrentPlayer->bIsReady);
	}
}

// =============================================================================
// Host Actions
// =============================================================================

void UDeskillzRooms::StartMatch(
	FOnRoomActionSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!IsInRoom())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotInRoom, TEXT("Not in a room")));
		return;
	}

	if (!IsHost())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotHost, TEXT("Only the host can start the match")));
		return;
	}

	if (!CanStartMatch())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotReady, TEXT("Not all players are ready")));
		return;
	}

	RoomClient->SendStart(CurrentRoom.Id);
	OnSuccess.ExecuteIfBound();
}

void UDeskillzRooms::KickPlayer(
	const FString& PlayerId,
	FOnRoomActionSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!IsInRoom())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotInRoom, TEXT("Not in a room")));
		return;
	}

	if (!IsHost())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotHost, TEXT("Only the host can kick players")));
		return;
	}

	RoomClient->KickPlayer(CurrentRoom.Id, PlayerId,
		FOnRoomActionSuccess::CreateLambda([OnSuccess, PlayerId]()
		{
			OnSuccess.ExecuteIfBound();
			UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Kicked player: %s"), *PlayerId);
		}),
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

void UDeskillzRooms::CancelRoom(
	FOnRoomActionSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!IsInRoom())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotInRoom, TEXT("Not in a room")));
		return;
	}

	if (!IsHost())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotHost, TEXT("Only the host can cancel the room")));
		return;
	}

	RoomClient->CancelRoom(CurrentRoom.Id,
		FOnRoomActionSuccess::CreateLambda([this, OnSuccess]()
		{
			DisconnectFromRoom();
			CurrentRoom = FPrivateRoom();
			OnSuccess.ExecuteIfBound();
			UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Room cancelled"));
		}),
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

// =============================================================================
// Chat
// =============================================================================

void UDeskillzRooms::SendChat(const FString& Message)
{
	if (!IsInRoom() || Message.IsEmpty())
	{
		return;
	}

	RoomClient->SendChat(CurrentRoom.Id, Message);
}

// =============================================================================
// Utility
// =============================================================================

void UDeskillzRooms::RefreshRoom(
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	if (!IsInRoom())
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::NotInRoom, TEXT("Not in a room")));
		return;
	}

	RoomClient->GetRoomById(CurrentRoom.Id,
		FOnRoomSuccess::CreateLambda([this, OnSuccess](const FPrivateRoom& Room)
		{
			CurrentRoom = Room;
			OnSuccess.ExecuteIfBound(Room);
			OnRoomUpdated.Broadcast(Room);
		}),
		FOnRoomError::CreateLambda([this, OnError](const FRoomError& Error)
		{
			OnError.ExecuteIfBound(Error);
			OnRoomErrorOccurred.Broadcast(Error);
		}));
}

FRoomPlayer UDeskillzRooms::GetCurrentPlayer() const
{
	if (!IsInRoom())
	{
		return FRoomPlayer();
	}

	UDeskillzSDK* SDK = UDeskillzSDK::Get(this);
	if (!SDK)
	{
		return FRoomPlayer();
	}

	const FRoomPlayer* Player = CurrentRoom.FindPlayer(SDK->GetCurrentUserId());
	return Player ? *Player : FRoomPlayer();
}

bool UDeskillzRooms::CanStartMatch() const
{
	return IsHost() && CurrentRoom.AreAllPlayersReady() && CurrentRoom.CurrentPlayers >= CurrentRoom.MinPlayers;
}

// =============================================================================
// Internal
// =============================================================================

void UDeskillzRooms::ConnectToRoom(const FString& RoomId)
{
	if (RoomClient)
	{
		RoomClient->Connect(RoomId);
	}
}

void UDeskillzRooms::DisconnectFromRoom()
{
	if (RoomClient)
	{
		RoomClient->Disconnect();
	}
}

bool UDeskillzRooms::ValidateAuthentication(FOnRoomError OnError)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(this);
	if (!SDK || !SDK->IsAuthenticated())
	{
		OnError.ExecuteIfBound(FRoomError(
			FRoomError::NotAuthenticated,
			TEXT("Player not authenticated. Initialize SDK first.")
		));
		return false;
	}
	return true;
}

// =============================================================================
// Event Handlers
// =============================================================================

void UDeskillzRooms::HandleRoomStateUpdate(const FPrivateRoom& Room)
{
	CurrentRoom = Room;
	OnRoomUpdated.Broadcast(Room);
}

void UDeskillzRooms::HandlePlayerJoined(const FRoomPlayer& Player)
{
	bool bFound = false;
	for (const FRoomPlayer& Existing : CurrentRoom.Players)
	{
		if (Existing.Id == Player.Id)
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		CurrentRoom.Players.Add(Player);
		CurrentRoom.CurrentPlayers++;
	}

	OnPlayerJoined.Broadcast(Player);
	OnRoomUpdated.Broadcast(CurrentRoom);
}

void UDeskillzRooms::HandlePlayerLeft(const FString& PlayerId)
{
	CurrentRoom.Players.RemoveAll([PlayerId](const FRoomPlayer& P) { return P.Id == PlayerId; });
	CurrentRoom.CurrentPlayers = FMath::Max(0, CurrentRoom.CurrentPlayers - 1);

	OnPlayerLeft.Broadcast(PlayerId);
	OnRoomUpdated.Broadcast(CurrentRoom);
}

void UDeskillzRooms::HandlePlayerKicked(const FString& PlayerId)
{
	CurrentRoom.Players.RemoveAll([PlayerId](const FRoomPlayer& P) { return P.Id == PlayerId; });
	CurrentRoom.CurrentPlayers = FMath::Max(0, CurrentRoom.CurrentPlayers - 1);

	OnPlayerLeft.Broadcast(PlayerId);
	OnRoomUpdated.Broadcast(CurrentRoom);
}

void UDeskillzRooms::HandlePlayerReady(const FString& PlayerId, bool bIsReady, bool bAllReady)
{
	for (FRoomPlayer& Player : CurrentRoom.Players)
	{
		if (Player.Id == PlayerId)
		{
			Player.bIsReady = bIsReady;
			break;
		}
	}

	OnPlayerReadyChanged.Broadcast(PlayerId, bIsReady);
	OnRoomUpdated.Broadcast(CurrentRoom);

	if (bAllReady)
	{
		OnAllPlayersReady.Broadcast();
	}
}

void UDeskillzRooms::HandleAllReady(int32 PlayerCount)
{
	OnAllPlayersReady.Broadcast();
}

void UDeskillzRooms::HandleCountdownStarted(int32 Seconds)
{
	CurrentRoom.Status = ERoomStatus::Countdown;
	OnCountdownStarted.Broadcast(Seconds);
	OnRoomUpdated.Broadcast(CurrentRoom);
}

void UDeskillzRooms::HandleCountdownTick(int32 Seconds)
{
	OnCountdownTick.Broadcast(Seconds);
}

void UDeskillzRooms::HandleLaunching(const FMatchLaunchData& LaunchData)
{
	CurrentRoom.Status = ERoomStatus::Launching;

	FMatchLaunchData ModifiedData = LaunchData;
	ModifiedData.RoomCode = CurrentRoom.RoomCode;

	OnMatchLaunching.Broadcast(ModifiedData);

	UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Match launching: %s"), *LaunchData.MatchId);
}

void UDeskillzRooms::HandleCancelled(const FString& Reason)
{
	CurrentRoom = FPrivateRoom();
	DisconnectFromRoom();

	OnRoomCancelled.Broadcast(Reason);
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Room cancelled: %s"), *Reason);
}

void UDeskillzRooms::HandleKicked(const FString& Reason)
{
	CurrentRoom = FPrivateRoom();
	DisconnectFromRoom();

	OnKicked.Broadcast(Reason);
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRooms] Kicked from room: %s"), *Reason);
}

void UDeskillzRooms::HandleChat(const FString& SenderId, const FString& Username, const FString& Message)
{
	OnChatReceived.Broadcast(SenderId, Username, Message);
}

void UDeskillzRooms::HandleError(const FString& ErrorMessage)
{
	OnError.Broadcast(FRoomError(FRoomError::ServerError, ErrorMessage));
}