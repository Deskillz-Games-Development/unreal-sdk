// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzRoomTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "DeskillzRoomClient.generated.h"

class IWebSocket;

// =============================================================================
// Internal Delegates for WebSocket Events
// =============================================================================

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomStateReceived, const FPrivateRoom&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomPlayerJoined, const FRoomPlayer&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomPlayerLeft, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomPlayerKicked, const FString&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnRoomPlayerReady, const FString&, bool, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomAllReady, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomCountdownStarted, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomCountdownTick, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomLaunching, const FMatchLaunchData&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomCancelled, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomKicked, const FString&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnRoomChat, const FString&, const FString&, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRoomClientError, const FString&);

/**
 * Internal HTTP/WebSocket client for Private Room operations.
 * Handles API calls and real-time WebSocket events.
 */
UCLASS()
class DESKILLZ_API UDeskillzRoomClient : public UObject
{
	GENERATED_BODY()

public:
	UDeskillzRoomClient();

	// =========================================================================
	// Lifecycle
	// =========================================================================

	/** Initialize the client */
	void Initialize();

	/** Shutdown and cleanup */
	void Shutdown();

	// =========================================================================
	// WebSocket Connection
	// =========================================================================

	/** Connect to room WebSocket */
	void Connect(const FString& RoomId);

	/** Disconnect from WebSocket */
	void Disconnect();

	/** Check if connected */
	bool IsConnected() const;

	// =========================================================================
	// HTTP API - Room Creation
	// =========================================================================

	/** Create a new room */
	void CreateRoom(
		const FCreateRoomConfig& Config,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// HTTP API - Room Discovery
	// =========================================================================

	/** Get public rooms for a game */
	void GetPublicRooms(
		const FString& GameId,
		FOnRoomListSuccess OnSuccess,
		FOnRoomError OnError);

	/** Get rooms for current user */
	void GetMyRooms(
		FOnRoomListSuccess OnSuccess,
		FOnRoomError OnError);

	/** Get room by code */
	void GetRoomByCode(
		const FString& RoomCode,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	/** Get room by ID */
	void GetRoomById(
		const FString& RoomId,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// HTTP API - Join/Leave
	// =========================================================================

	/** Join a room by code */
	void JoinRoom(
		const FString& RoomCode,
		FOnRoomSuccess OnSuccess,
		FOnRoomError OnError);

	/** Leave a room */
	void LeaveRoom(
		const FString& RoomId,
		FOnRoomActionSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// HTTP API - Host Actions
	// =========================================================================

	/** Kick a player */
	void KickPlayer(
		const FString& RoomId,
		const FString& PlayerId,
		FOnRoomActionSuccess OnSuccess,
		FOnRoomError OnError);

	/** Cancel a room */
	void CancelRoom(
		const FString& RoomId,
		FOnRoomActionSuccess OnSuccess,
		FOnRoomError OnError);

	// =========================================================================
	// WebSocket Messages
	// =========================================================================

	/** Send ready status */
	void SendReady(const FString& RoomId, bool bIsReady);

	/** Send chat message */
	void SendChat(const FString& RoomId, const FString& Message);

	/** Send start request */
	void SendStart(const FString& RoomId);

	/** Send kick request */
	void SendKick(const FString& RoomId, const FString& TargetUserId);

	/** Send cancel request */
	void SendCancel(const FString& RoomId);

	// =========================================================================
	// WebSocket Events
	// =========================================================================

	FOnRoomStateReceived OnStateUpdate;
	FOnRoomPlayerJoined OnPlayerJoined;
	FOnRoomPlayerLeft OnPlayerLeft;
	FOnRoomPlayerKicked OnPlayerKicked;
	FOnRoomPlayerReady OnPlayerReady;
	FOnRoomAllReady OnAllReady;
	FOnRoomCountdownStarted OnCountdownStarted;
	FOnRoomCountdownTick OnCountdownTick;
	FOnRoomLaunching OnLaunching;
	FOnRoomCancelled OnCancelled;
	FOnRoomKicked OnKicked;
	FOnRoomChat OnChat;
	FOnRoomClientError OnError;

protected:
	// =========================================================================
	// HTTP Helpers
	// =========================================================================

	/** Get base API URL */
	FString GetBaseUrl() const;

	/** Get WebSocket URL */
	FString GetWebSocketUrl() const;

	/** Get auth token */
	FString GetAuthToken() const;

	/** Create HTTP request with headers */
	TSharedRef<IHttpRequest> CreateRequest(const FString& Verb, const FString& Endpoint);

	/** Parse error from response */
	FRoomError ParseError(FHttpResponsePtr Response) const;

	/** Parse room from JSON */
	FPrivateRoom ParseRoom(const TSharedPtr<FJsonObject>& JsonObject) const;

	/** Parse player from JSON */
	FRoomPlayer ParsePlayer(const TSharedPtr<FJsonObject>& JsonObject) const;

	// =========================================================================
	// WebSocket Helpers
	// =========================================================================

	/** Send WebSocket message */
	void SendWebSocketMessage(const FString& Event, const TSharedPtr<FJsonObject>& Data);

	/** Handle WebSocket connected */
	void HandleWebSocketConnected();

	/** Handle WebSocket disconnected */
	void HandleWebSocketDisconnected();

	/** Handle WebSocket message */
	void HandleWebSocketMessage(const FString& Message);

	/** Handle WebSocket error */
	void HandleWebSocketError(const FString& Error);

	/** Process WebSocket event */
	void ProcessWebSocketEvent(const FString& EventType, const TSharedPtr<FJsonObject>& Data);

	/** Subscribe to room */
	void SubscribeToRoom();

	/** Unsubscribe from room */
	void UnsubscribeFromRoom();

	/** Attempt reconnection */
	void AttemptReconnect();

private:
	// =========================================================================
	// State
	// =========================================================================

	/** WebSocket connection */
	TSharedPtr<IWebSocket> WebSocket;

	/** Current room ID */
	FString CurrentRoomId;

	/** Whether initialized */
	bool bIsInitialized;

	/** Reconnection state */
	bool bShouldReconnect;
	int32 ReconnectAttempts;
	FTimerHandle ReconnectTimerHandle;

	/** Constants */
	static const int32 MaxReconnectAttempts;
	static const float ReconnectDelay;
	static const FString RoomsEndpoint;
};