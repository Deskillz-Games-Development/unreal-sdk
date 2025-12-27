// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeskillzRoomTypes.h"
#include "DeskillzRooms.generated.h"

class UDeskillzRoomClient;

/**
 * Deskillz SDK - Private Room Management
 * 
 * Allows players to create, join, and manage private rooms for custom matches.
 * Rooms can be created from both the website and game apps, with full cross-platform
 * support via WebSocket synchronization.
 * 
 * Usage (Blueprint):
 * 1. Get via "Get Game Instance Subsystem" > "Deskillz Rooms"
 * 2. Call CreateRoom, JoinRoom, SetReady, etc.
 * 3. Bind to events like OnRoomJoined, OnPlayerJoined, etc.
 * 
 * Usage (C++):
 * @code
 * UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
 * 
 * FCreateRoomConfig Config;
 * Config.Name = "My Room";
 * Config.EntryFee = 5.0f;
 * Config.EntryCurrency = "USDT";
 * 
 * Rooms->CreateRoom(Config,
 *     FOnRoomSuccess::CreateLambda([](const FPrivateRoom& Room) {
 *         UE_LOG(LogTemp, Log, TEXT("Room created: %s"), *Room.RoomCode);
 *     }),
 *     FOnRoomError::CreateLambda([](const FRoomError& Error) {
 *         UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Error.Message);
 *     }));
 * @endcode
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzRooms : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDeskillzRooms();

	// =========================================================================
	// Subsystem Lifecycle
	// =========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// =========================================================================
	// Static Accessors
	// =========================================================================

	/**
	 * Get the Deskillz Rooms instance
	 * @param WorldContextObject Any UObject to get world context from
	 * @return The Rooms instance or nullptr if not available
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Deskillz Rooms"))
	static UDeskillzRooms* Get(const UObject* WorldContextObject);

	// =========================================================================
	// Properties
	// =========================================================================

	/** Currently connected room, or invalid if not in a room */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Rooms")
	FPrivateRoom CurrentRoom;

	/** Whether currently in a room */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms")
	bool IsInRoom() const;

	/** Whether current user is the room host */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms")
	bool IsHost() const;

	/** Whether WebSocket is connected */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms")
	bool IsConnected() const;

	// =========================================================================
	// Room Creation
	// =========================================================================

	/**
	 * Create a new private room for the current game.
	 * @param Config Room configuration
	 * @param OnSuccess Called with room data on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void CreateRoom(
		const FCreateRoomConfig& Config,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	/**
	 * Quick create a room with minimal configuration.
	 * Uses default settings for a 2-player match.
	 * @param RoomName Room display name
	 * @param EntryFee Entry fee amount
	 * @param OnSuccess Called with room data on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void QuickCreateRoom(
		const FString& RoomName,
		float EntryFee,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// Room Discovery
	// =========================================================================

	/**
	 * Get public rooms for the current game.
	 * @param OnSuccess Called with list of rooms on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void GetPublicRooms(
		FOnRoomListSuccess OnSuccess,
		FOnRoomError OnError);

	/**
	 * Get rooms created by or joined by current user.
	 * @param OnSuccess Called with list of rooms on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void GetMyRooms(
		FOnRoomListSuccess OnSuccess,
		FOnRoomError OnError);

	/**
	 * Get room details by code (preview before joining).
	 * @param RoomCode Room code (e.g., "DSKZ-AB3C")
	 * @param OnSuccess Called with room data on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void GetRoomByCode(
		const FString& RoomCode,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// Join / Leave
	// =========================================================================

	/**
	 * Join a room by its code (e.g., "DSKZ-AB3C").
	 * @param RoomCode Room code to join
	 * @param OnSuccess Called with room data on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void JoinRoom(
		const FString& RoomCode,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	/**
	 * Leave the current room.
	 * @param OnSuccess Called on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void LeaveRoom(
		FOnRoomActionSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// Ready Status
	// =========================================================================

	/**
	 * Set ready status for current player.
	 * @param bIsReady Ready status to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void SetReady(bool bIsReady);

	/**
	 * Toggle ready status.
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void ToggleReady();

	// =========================================================================
	// Host Actions
	// =========================================================================

	/**
	 * Start the match (host only). Requires all players to be ready.
	 * @param OnSuccess Called on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void StartMatch(
		FOnRoomActionSuccess OnSuccess,
		FOnRoomError OnError);

	/**
	 * Kick a player from the room (host only).
	 * @param PlayerId Player ID to kick
	 * @param OnSuccess Called on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void KickPlayer(
		const FString& PlayerId,
		FOnRoomActionSuccess OnSuccess,
		FOnRoomError OnError);

	/**
	 * Cancel the room (host only).
	 * @param OnSuccess Called on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void CancelRoom(
		FOnRoomActionSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// Chat
	// =========================================================================

	/**
	 * Send a chat message to the room.
	 * @param Message Message to send (max 500 characters)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void SendChat(const FString& Message);

	// =========================================================================
	// Utility
	// =========================================================================

	/**
	 * Refresh the current room state from the server.
	 * @param OnSuccess Called with updated room data on success
	 * @param OnError Called on error
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms")
	void RefreshRoom(
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	/**
	 * Get current player's data in the room.
	 * @return Player data or nullptr if not in room
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms")
	FRoomPlayer GetCurrentPlayer() const;

	/**
	 * Check if match can be started (all players ready, enough players).
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms")
	bool CanStartMatch() const;

	// =========================================================================
	// Events - Bindable in Blueprint and C++
	// =========================================================================

	/** Called when successfully joined a room */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnRoomJoined OnRoomJoined;

	/** Called when room state updates */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnRoomUpdated OnRoomUpdated;

	/** Called when a player joins the room */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnPlayerJoinedRoom OnPlayerJoined;

	/** Called when a player leaves the room */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnPlayerLeftRoom OnPlayerLeft;

	/** Called when a player's ready status changes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnPlayerReadyChanged OnPlayerReadyChanged;

	/** Called when all players are ready */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnAllPlayersReady OnAllPlayersReady;

	/** Called when countdown starts */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnCountdownStarted OnCountdownStarted;

	/** Called each countdown tick */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnCountdownTick OnCountdownTick;

	/** Called when match is launching */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnMatchLaunching OnMatchLaunching;

	/** Called when room is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnRoomCancelled OnRoomCancelled;

	/** Called when leaving a room */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnRoomLeft OnRoomLeft;

	/** Called when kicked from room */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnKickedFromRoom OnKicked;

	/** Called when chat message is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnChatReceived OnChatReceived;

	/** Called when an error occurs */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|Events")
	FOnRoomErrorOccurred OnError;

protected:
	// =========================================================================
	// Internal
	// =========================================================================

	/** HTTP/WebSocket client */
	UPROPERTY()
	UDeskillzRoomClient* RoomClient;

	/** Whether initialized */
	bool bIsInitialized;

	/** Connect WebSocket to room */
	void ConnectToRoom(const FString& RoomId);

	/** Disconnect WebSocket */
	void DisconnectFromRoom();

	/** Handle WebSocket events */
	void HandleRoomStateUpdate(const FPrivateRoom& Room);
	void HandlePlayerJoined(const FRoomPlayer& Player);
	void HandlePlayerLeft(const FString& PlayerId);
	void HandlePlayerKicked(const FString& PlayerId);
	void HandlePlayerReady(const FString& PlayerId, bool bIsReady, bool bAllReady);
	void HandleAllReady(int32 PlayerCount);
	void HandleCountdownStarted(int32 Seconds);
	void HandleCountdownTick(int32 Seconds);
	void HandleLaunching(const FMatchLaunchData& LaunchData);
	void HandleCancelled(const FString& Reason);
	void HandleKicked(const FString& Reason);
	void HandleChat(const FString& SenderId, const FString& Username, const FString& Message);
	void HandleError(const FString& ErrorMessage);

	/** Validate authentication */
	bool ValidateAuthentication(FOnRoomError OnError);
};