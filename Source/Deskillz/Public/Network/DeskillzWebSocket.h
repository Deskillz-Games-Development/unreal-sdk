// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IWebSocket.h"
#include "DeskillzWebSocket.generated.h"

/**
 * WebSocket connection state
 */
UENUM(BlueprintType)
enum class EDeskillzWebSocketState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	Reconnecting,
	Error
};

/**
 * WebSocket message type
 */
UENUM(BlueprintType)
enum class EDeskillzMessageType : uint8
{
	Text,
	Binary,
	Ping,
	Pong
};

/**
 * WebSocket message
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzWebSocketMessage
{
	GENERATED_BODY()
	
	/** Message type */
	UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
	EDeskillzMessageType Type = EDeskillzMessageType::Text;
	
	/** Message data (text) */
	UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
	FString Data;
	
	/** Binary data */
	TArray<uint8> BinaryData;
	
	/** Event type (from JSON) */
	UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
	FString EventType;
	
	/** Timestamp */
	UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
	int64 Timestamp = 0;
};

/** WebSocket delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzWSConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzWSDisconnected, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzWSError, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzWSMessage, const FDeskillzWebSocketMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzWSStateChanged, EDeskillzWebSocketState, NewState);

/** Native delegate for message handling */
DECLARE_DELEGATE_OneParam(FOnDeskillzWSMessageNative, const FDeskillzWebSocketMessage&);

/**
 * Deskillz WebSocket Client
 * 
 * Real-time communication for:
 * - Live match updates
 * - Synchronous multiplayer
 * - Tournament notifications
 * - Chat messages
 * - Presence updates
 * 
 * Features:
 * - Auto-reconnection
 * - Heartbeat/ping-pong
 * - Message queuing during disconnect
 * - Event-based messaging
 * 
 * Usage:
 *   UDeskillzWebSocket* WS = UDeskillzWebSocket::Get();
 *   WS->OnMessage.AddDynamic(this, &MyClass::HandleMessage);
 *   WS->Connect("wss://api.deskillz.games/ws");
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzWebSocket : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzWebSocket();
	~UDeskillzWebSocket();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the WebSocket instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network", meta = (DisplayName = "Get Deskillz WebSocket"))
	static UDeskillzWebSocket* Get();
	
	// ========================================================================
	// Connection
	// ========================================================================
	
	/**
	 * Connect to WebSocket server
	 * @param Url WebSocket URL (wss://...)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void Connect(const FString& Url);
	
	/**
	 * Connect with authentication
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void ConnectWithAuth(const FString& Url, const FString& AuthToken);
	
	/**
	 * Disconnect from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void Disconnect();
	
	/**
	 * Check if connected
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	bool IsConnected() const;
	
	/**
	 * Get current connection state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	EDeskillzWebSocketState GetState() const { return CurrentState; }
	
	// ========================================================================
	// Messaging
	// ========================================================================
	
	/**
	 * Send text message
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	bool Send(const FString& Message);
	
	/**
	 * Send JSON message
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	bool SendJson(const FString& EventType, const TMap<FString, FString>& Data);
	
	/**
	 * Send binary data
	 */
	bool SendBinary(const TArray<uint8>& Data);
	
	/**
	 * Send JSON object
	 */
	bool SendJsonObject(const TSharedPtr<FJsonObject>& JsonObject);
	
	// ========================================================================
	// Event Subscription
	// ========================================================================
	
	/**
	 * Subscribe to specific event type
	 */
	void SubscribeToEvent(const FString& EventType, const FOnDeskillzWSMessageNative& Callback);
	
	/**
	 * Unsubscribe from event type
	 */
	void UnsubscribeFromEvent(const FString& EventType);
	
	/**
	 * Subscribe to event (Blueprint)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void K2_SubscribeToEvent(const FString& EventType);
	
	// ========================================================================
	// Room/Channel
	// ========================================================================
	
	/**
	 * Join a room/channel
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void JoinRoom(const FString& RoomId);
	
	/**
	 * Leave a room/channel
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void LeaveRoom(const FString& RoomId);
	
	/**
	 * Send message to room
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SendToRoom(const FString& RoomId, const FString& Message);
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/**
	 * Set auto-reconnect
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetAutoReconnect(bool bEnable, float DelaySeconds = 5.0f, int32 MaxAttempts = 10);
	
	/**
	 * Set heartbeat interval
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetHeartbeatInterval(float Seconds);
	
	/**
	 * Set auth token
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetAuthToken(const FString& Token);
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when connected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnDeskillzWSConnected OnConnected;
	
	/** Called when disconnected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnDeskillzWSDisconnected OnDisconnected;
	
	/** Called on error */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnDeskillzWSError OnError;
	
	/** Called when message received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnDeskillzWSMessage OnMessage;
	
	/** Called when state changes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnDeskillzWSStateChanged OnStateChanged;
	
protected:
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** WebSocket URL */
	UPROPERTY()
	FString ServerUrl;
	
	/** Auth token */
	UPROPERTY()
	FString AuthToken;
	
	/** Auto-reconnect enabled */
	UPROPERTY()
	bool bAutoReconnect = true;
	
	/** Reconnect delay */
	UPROPERTY()
	float ReconnectDelay = 5.0f;
	
	/** Max reconnect attempts */
	UPROPERTY()
	int32 MaxReconnectAttempts = 10;
	
	/** Current reconnect attempt */
	UPROPERTY()
	int32 CurrentReconnectAttempt = 0;
	
	/** Heartbeat interval */
	UPROPERTY()
	float HeartbeatInterval = 30.0f;
	
	/** Current state */
	UPROPERTY()
	EDeskillzWebSocketState CurrentState = EDeskillzWebSocketState::Disconnected;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** WebSocket instance */
	TSharedPtr<IWebSocket> WebSocket;
	
	/** Message queue (for when disconnected) */
	TArray<FString> MessageQueue;
	
	/** Joined rooms */
	TSet<FString> JoinedRooms;
	
	/** Event subscriptions */
	TMap<FString, FOnDeskillzWSMessageNative> EventCallbacks;
	
	/** Heartbeat timer handle */
	FTimerHandle HeartbeatTimerHandle;
	
	/** Reconnect timer handle */
	FTimerHandle ReconnectTimerHandle;
	
	/** Last ping time */
	double LastPingTime = 0.0;
	
	/** Last pong time */
	double LastPongTime = 0.0;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Set connection state */
	void SetState(EDeskillzWebSocketState NewState);
	
	/** Handle connection opened */
	void HandleConnected();
	
	/** Handle connection closed */
	void HandleDisconnected(int32 StatusCode, const FString& Reason, bool bWasClean);
	
	/** Handle connection error */
	void HandleError(const FString& Error);
	
	/** Handle message received */
	void HandleMessage(const FString& Message);
	
	/** Handle binary message */
	void HandleBinaryMessage(const void* Data, SIZE_T Size, bool bIsLastFragment);
	
	/** Parse message JSON */
	bool ParseMessageJson(const FString& Message, FDeskillzWebSocketMessage& OutMessage);
	
	/** Route message to subscribers */
	void RouteMessage(const FDeskillzWebSocketMessage& Message);
	
	/** Start heartbeat */
	void StartHeartbeat();
	
	/** Stop heartbeat */
	void StopHeartbeat();
	
	/** Send heartbeat ping */
	void SendHeartbeat();
	
	/** Attempt reconnection */
	void AttemptReconnect();
	
	/** Flush message queue */
	void FlushMessageQueue();
	
	/** Rejoin rooms after reconnect */
	void RejoinRooms();
	
	/** Create WebSocket with headers */
	void CreateWebSocket();
};
