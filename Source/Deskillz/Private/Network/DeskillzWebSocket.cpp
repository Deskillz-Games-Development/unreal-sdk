// Copyright Deskillz Games. All Rights Reserved.

#include "Network/DeskillzWebSocket.h"
#include "Deskillz.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Static singleton
static UDeskillzWebSocket* GWebSocket = nullptr;

UDeskillzWebSocket::UDeskillzWebSocket()
{
}

UDeskillzWebSocket::~UDeskillzWebSocket()
{
	Disconnect();
}

UDeskillzWebSocket* UDeskillzWebSocket::Get()
{
	if (!GWebSocket)
	{
		GWebSocket = NewObject<UDeskillzWebSocket>();
		GWebSocket->AddToRoot();
	}
	return GWebSocket;
}

// ============================================================================
// Connection
// ============================================================================

void UDeskillzWebSocket::Connect(const FString& Url)
{
	if (CurrentState == EDeskillzWebSocketState::Connected ||
		CurrentState == EDeskillzWebSocketState::Connecting)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("WebSocket already connected or connecting"));
		return;
	}
	
	ServerUrl = Url;
	CurrentReconnectAttempt = 0;
	
	SetState(EDeskillzWebSocketState::Connecting);
	CreateWebSocket();
	
	if (WebSocket.IsValid())
	{
		WebSocket->Connect();
		UE_LOG(LogDeskillz, Log, TEXT("WebSocket connecting to: %s"), *Url);
	}
}

void UDeskillzWebSocket::ConnectWithAuth(const FString& Url, const FString& Token)
{
	AuthToken = Token;
	Connect(Url);
}

void UDeskillzWebSocket::Disconnect()
{
	StopHeartbeat();
	
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(ReconnectTimerHandle);
	}
	
	if (WebSocket.IsValid())
	{
		WebSocket->Close();
		WebSocket.Reset();
	}
	
	SetState(EDeskillzWebSocketState::Disconnected);
	UE_LOG(LogDeskillz, Log, TEXT("WebSocket disconnected"));
}

bool UDeskillzWebSocket::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

// ============================================================================
// Messaging
// ============================================================================

bool UDeskillzWebSocket::Send(const FString& Message)
{
	if (!IsConnected())
	{
		// Queue message for later
		MessageQueue.Add(Message);
		UE_LOG(LogDeskillz, Verbose, TEXT("WebSocket not connected, message queued"));
		return false;
	}
	
	WebSocket->Send(Message);
	UE_LOG(LogDeskillz, Verbose, TEXT("WebSocket sent: %s"), *Message.Left(100));
	return true;
}

bool UDeskillzWebSocket::SendJson(const FString& EventType, const TMap<FString, FString>& Data)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("event"), EventType);
	JsonObject->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp() * 1000);
	
	// Add data fields
	TSharedPtr<FJsonObject> DataObject = MakeShareable(new FJsonObject());
	for (const auto& Pair : Data)
	{
		DataObject->SetStringField(Pair.Key, Pair.Value);
	}
	JsonObject->SetObjectField(TEXT("data"), DataObject);
	
	return SendJsonObject(JsonObject);
}

bool UDeskillzWebSocket::SendBinary(const TArray<uint8>& Data)
{
	if (!IsConnected())
	{
		return false;
	}
	
	WebSocket->Send(Data.GetData(), Data.Num(), true);
	return true;
}

bool UDeskillzWebSocket::SendJsonObject(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}
	
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	return Send(JsonString);
}

// ============================================================================
// Event Subscription
// ============================================================================

void UDeskillzWebSocket::SubscribeToEvent(const FString& EventType, const FOnDeskillzWSMessageNative& Callback)
{
	EventCallbacks.Add(EventType, Callback);
	UE_LOG(LogDeskillz, Verbose, TEXT("Subscribed to WebSocket event: %s"), *EventType);
}

void UDeskillzWebSocket::UnsubscribeFromEvent(const FString& EventType)
{
	EventCallbacks.Remove(EventType);
	UE_LOG(LogDeskillz, Verbose, TEXT("Unsubscribed from WebSocket event: %s"), *EventType);
}

void UDeskillzWebSocket::K2_SubscribeToEvent(const FString& EventType)
{
	// Blueprint version - events will be received through OnMessage delegate
	// Just log for tracking
	UE_LOG(LogDeskillz, Verbose, TEXT("Blueprint subscribed to event: %s"), *EventType);
}

// ============================================================================
// Room/Channel
// ============================================================================

void UDeskillzWebSocket::JoinRoom(const FString& RoomId)
{
	TMap<FString, FString> Data;
	Data.Add(TEXT("room_id"), RoomId);
	
	if (SendJson(TEXT("room:join"), Data))
	{
		JoinedRooms.Add(RoomId);
		UE_LOG(LogDeskillz, Log, TEXT("Joining room: %s"), *RoomId);
	}
}

void UDeskillzWebSocket::LeaveRoom(const FString& RoomId)
{
	TMap<FString, FString> Data;
	Data.Add(TEXT("room_id"), RoomId);
	
	if (SendJson(TEXT("room:leave"), Data))
	{
		JoinedRooms.Remove(RoomId);
		UE_LOG(LogDeskillz, Log, TEXT("Leaving room: %s"), *RoomId);
	}
}

void UDeskillzWebSocket::SendToRoom(const FString& RoomId, const FString& Message)
{
	TMap<FString, FString> Data;
	Data.Add(TEXT("room_id"), RoomId);
	Data.Add(TEXT("message"), Message);
	
	SendJson(TEXT("room:message"), Data);
}

// ============================================================================
// Configuration
// ============================================================================

void UDeskillzWebSocket::SetAutoReconnect(bool bEnable, float DelaySeconds, int32 MaxAttempts)
{
	bAutoReconnect = bEnable;
	ReconnectDelay = FMath::Max(1.0f, DelaySeconds);
	MaxReconnectAttempts = FMath::Max(1, MaxAttempts);
}

void UDeskillzWebSocket::SetHeartbeatInterval(float Seconds)
{
	HeartbeatInterval = FMath::Max(5.0f, Seconds);
	
	// Restart heartbeat if connected
	if (IsConnected())
	{
		StopHeartbeat();
		StartHeartbeat();
	}
}

void UDeskillzWebSocket::SetAuthToken(const FString& Token)
{
	AuthToken = Token;
	
	// If already connected, send auth message
	if (IsConnected())
	{
		TMap<FString, FString> Data;
		Data.Add(TEXT("token"), Token);
		SendJson(TEXT("auth"), Data);
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzWebSocket::SetState(EDeskillzWebSocketState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnStateChanged.Broadcast(NewState);
		
		UE_LOG(LogDeskillz, Verbose, TEXT("WebSocket state: %d"), static_cast<int32>(NewState));
	}
}

void UDeskillzWebSocket::HandleConnected()
{
	SetState(EDeskillzWebSocketState::Connected);
	CurrentReconnectAttempt = 0;
	
	// Start heartbeat
	StartHeartbeat();
	
	// Send auth if we have token
	if (!AuthToken.IsEmpty())
	{
		TMap<FString, FString> Data;
		Data.Add(TEXT("token"), AuthToken);
		SendJson(TEXT("auth"), Data);
	}
	
	// Flush queued messages
	FlushMessageQueue();
	
	// Rejoin rooms
	RejoinRooms();
	
	OnConnected.Broadcast();
	UE_LOG(LogDeskillz, Log, TEXT("WebSocket connected"));
}

void UDeskillzWebSocket::HandleDisconnected(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	StopHeartbeat();
	
	SetState(EDeskillzWebSocketState::Disconnected);
	OnDisconnected.Broadcast(Reason);
	
	UE_LOG(LogDeskillz, Log, TEXT("WebSocket disconnected: %s (Code: %d, Clean: %d)"), 
		*Reason, StatusCode, bWasClean);
	
	// Attempt reconnection if enabled
	if (bAutoReconnect && CurrentReconnectAttempt < MaxReconnectAttempts)
	{
		AttemptReconnect();
	}
}

void UDeskillzWebSocket::HandleError(const FString& Error)
{
	SetState(EDeskillzWebSocketState::Error);
	OnError.Broadcast(Error);
	
	UE_LOG(LogDeskillz, Error, TEXT("WebSocket error: %s"), *Error);
}

void UDeskillzWebSocket::HandleMessage(const FString& Message)
{
	FDeskillzWebSocketMessage WSMessage;
	WSMessage.Type = EDeskillzMessageType::Text;
	WSMessage.Data = Message;
	WSMessage.Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	
	// Try to parse as JSON
	ParseMessageJson(Message, WSMessage);
	
	// Handle ping/pong
	if (WSMessage.EventType == TEXT("pong"))
	{
		LastPongTime = FPlatformTime::Seconds();
		return;
	}
	
	// Route to subscribers
	RouteMessage(WSMessage);
	
	// Broadcast to all listeners
	OnMessage.Broadcast(WSMessage);
	
	UE_LOG(LogDeskillz, Verbose, TEXT("WebSocket received: %s"), *Message.Left(100));
}

void UDeskillzWebSocket::HandleBinaryMessage(const void* Data, SIZE_T Size, bool bIsLastFragment)
{
	FDeskillzWebSocketMessage WSMessage;
	WSMessage.Type = EDeskillzMessageType::Binary;
	WSMessage.BinaryData.Append(static_cast<const uint8*>(Data), Size);
	WSMessage.Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	
	OnMessage.Broadcast(WSMessage);
}

bool UDeskillzWebSocket::ParseMessageJson(const FString& Message, FDeskillzWebSocketMessage& OutMessage)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}
	
	// Extract event type
	if (JsonObject->HasField(TEXT("event")))
	{
		OutMessage.EventType = JsonObject->GetStringField(TEXT("event"));
	}
	else if (JsonObject->HasField(TEXT("type")))
	{
		OutMessage.EventType = JsonObject->GetStringField(TEXT("type"));
	}
	
	// Extract timestamp
	if (JsonObject->HasField(TEXT("timestamp")))
	{
		OutMessage.Timestamp = static_cast<int64>(JsonObject->GetNumberField(TEXT("timestamp")));
	}
	
	return true;
}

void UDeskillzWebSocket::RouteMessage(const FDeskillzWebSocketMessage& Message)
{
	if (Message.EventType.IsEmpty())
	{
		return;
	}
	
	// Find specific event callback
	if (FOnDeskillzWSMessageNative* Callback = EventCallbacks.Find(Message.EventType))
	{
		Callback->ExecuteIfBound(Message);
	}
	
	// Also check for wildcard patterns (e.g., "match:*")
	FString Prefix = Message.EventType;
	int32 ColonIndex;
	if (Prefix.FindChar(':', ColonIndex))
	{
		Prefix = Prefix.Left(ColonIndex) + TEXT(":*");
		if (FOnDeskillzWSMessageNative* WildcardCallback = EventCallbacks.Find(Prefix))
		{
			WildcardCallback->ExecuteIfBound(Message);
		}
	}
}

void UDeskillzWebSocket::StartHeartbeat()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(
			HeartbeatTimerHandle,
			this,
			&UDeskillzWebSocket::SendHeartbeat,
			HeartbeatInterval,
			true
		);
	}
	
	LastPingTime = FPlatformTime::Seconds();
	LastPongTime = LastPingTime;
}

void UDeskillzWebSocket::StopHeartbeat()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	}
}

void UDeskillzWebSocket::SendHeartbeat()
{
	if (!IsConnected())
	{
		return;
	}
	
	// Check if we missed pongs
	double Now = FPlatformTime::Seconds();
	if (Now - LastPongTime > HeartbeatInterval * 3)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("WebSocket heartbeat timeout"));
		Disconnect();
		return;
	}
	
	// Send ping
	TMap<FString, FString> Data;
	Data.Add(TEXT("timestamp"), FString::Printf(TEXT("%lld"), FDateTime::UtcNow().ToUnixTimestamp() * 1000));
	SendJson(TEXT("ping"), Data);
	
	LastPingTime = Now;
}

void UDeskillzWebSocket::AttemptReconnect()
{
	CurrentReconnectAttempt++;
	SetState(EDeskillzWebSocketState::Reconnecting);
	
	// Calculate delay with exponential backoff
	float Delay = ReconnectDelay * FMath::Pow(1.5f, static_cast<float>(CurrentReconnectAttempt - 1));
	Delay = FMath::Min(Delay, 60.0f); // Max 60 seconds
	
	UE_LOG(LogDeskillz, Log, TEXT("WebSocket reconnecting in %.1fs (attempt %d/%d)"), 
		Delay, CurrentReconnectAttempt, MaxReconnectAttempts);
	
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(
			ReconnectTimerHandle,
			[this]()
			{
				if (!ServerUrl.IsEmpty())
				{
					SetState(EDeskillzWebSocketState::Connecting);
					CreateWebSocket();
					if (WebSocket.IsValid())
					{
						WebSocket->Connect();
					}
				}
			},
			Delay,
			false
		);
	}
}

void UDeskillzWebSocket::FlushMessageQueue()
{
	if (MessageQueue.Num() == 0)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Flushing %d queued messages"), MessageQueue.Num());
	
	for (const FString& Message : MessageQueue)
	{
		Send(Message);
	}
	
	MessageQueue.Empty();
}

void UDeskillzWebSocket::RejoinRooms()
{
	for (const FString& RoomId : JoinedRooms)
	{
		TMap<FString, FString> Data;
		Data.Add(TEXT("room_id"), RoomId);
		SendJson(TEXT("room:join"), Data);
	}
}

void UDeskillzWebSocket::CreateWebSocket()
{
	// Ensure WebSockets module is loaded
	FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
	
	// Create headers
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("X-Client-Platform"), TEXT("Unreal"));
	Headers.Add(TEXT("X-Client-Version"), TEXT("1.0.0"));
	
	if (!AuthToken.IsEmpty())
	{
		Headers.Add(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}
	
	// Create WebSocket
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerUrl, TEXT(""), Headers);
	
	// Bind events
	WebSocket->OnConnected().AddUObject(this, &UDeskillzWebSocket::HandleConnected);
	WebSocket->OnConnectionError().AddUObject(this, &UDeskillzWebSocket::HandleError);
	WebSocket->OnClosed().AddUObject(this, &UDeskillzWebSocket::HandleDisconnected);
	WebSocket->OnMessage().AddUObject(this, &UDeskillzWebSocket::HandleMessage);
	WebSocket->OnBinaryMessage().AddUObject(this, &UDeskillzWebSocket::HandleBinaryMessage);
}
