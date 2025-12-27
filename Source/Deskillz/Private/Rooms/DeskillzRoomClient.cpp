// Copyright Deskillz Games. All Rights Reserved.

#include "DeskillzRoomClient.h"
#include "DeskillzSDK.h"
#include "DeskillzConfig.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"
#include "Engine/World.h"

// =============================================================================
// Constants
// =============================================================================

const int32 UDeskillzRoomClient::MaxReconnectAttempts = 5;
const float UDeskillzRoomClient::ReconnectDelay = 2.0f;
const FString UDeskillzRoomClient::RoomsEndpoint = TEXT("/api/v1/private-rooms");

// =============================================================================
// Constructor
// =============================================================================

UDeskillzRoomClient::UDeskillzRoomClient()
	: bIsInitialized(false)
	, bShouldReconnect(false)
	, ReconnectAttempts(0)
{
}

// =============================================================================
// Lifecycle
// =============================================================================

void UDeskillzRoomClient::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}

	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] Initialized"));
}

void UDeskillzRoomClient::Shutdown()
{
	Disconnect();
	bIsInitialized = false;
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] Shutdown"));
}

// =============================================================================
// WebSocket Connection
// =============================================================================

void UDeskillzRoomClient::Connect(const FString& RoomId)
{
	if (RoomId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzRoomClient] Cannot connect: empty room ID"));
		return;
	}

	CurrentRoomId = RoomId;
	bShouldReconnect = true;
	ReconnectAttempts = 0;

	// Close existing connection
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		// Already connected, just subscribe to new room
		SubscribeToRoom();
		return;
	}

	// Create new WebSocket
	FString WsUrl = GetWebSocketUrl();
	FString AuthToken = GetAuthToken();

	TMap<FString, FString> Headers;
	if (!AuthToken.IsEmpty())
	{
		Headers.Add(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(WsUrl, TEXT(""), Headers);

	// Bind events
	WebSocket->OnConnected().AddUObject(this, &UDeskillzRoomClient::HandleWebSocketConnected);
	WebSocket->OnConnectionError().AddUObject(this, &UDeskillzRoomClient::HandleWebSocketError);
	WebSocket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		HandleWebSocketDisconnected();
	});
	WebSocket->OnMessage().AddUObject(this, &UDeskillzRoomClient::HandleWebSocketMessage);

	// Connect
	WebSocket->Connect();
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] Connecting to %s"), *WsUrl);
}

void UDeskillzRoomClient::Disconnect()
{
	bShouldReconnect = false;
	CurrentRoomId.Empty();

	if (WebSocket.IsValid())
	{
		UnsubscribeFromRoom();
		WebSocket->Close();
		WebSocket.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] Disconnected"));
}

bool UDeskillzRoomClient::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

// =============================================================================
// HTTP API - Room Creation
// =============================================================================

void UDeskillzRoomClient::CreateRoom(
	const FCreateRoomConfig& Config,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(GetOuter());
	if (!SDK)
	{
		OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("SDK not initialized")));
		return;
	}

	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("POST"), RoomsEndpoint);

	// Build JSON body
	TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
	JsonBody->SetStringField(TEXT("name"), Config.Name);
	JsonBody->SetStringField(TEXT("description"), Config.Description);
	JsonBody->SetStringField(TEXT("gameId"), SDK->GetGameId());
	JsonBody->SetNumberField(TEXT("entryFee"), Config.EntryFee);
	JsonBody->SetStringField(TEXT("entryCurrency"), Config.EntryCurrency);
	JsonBody->SetNumberField(TEXT("minPlayers"), Config.MinPlayers);
	JsonBody->SetNumberField(TEXT("maxPlayers"), Config.MaxPlayers);
	JsonBody->SetStringField(TEXT("visibility"), StaticEnum<ERoomVisibility>()->GetNameStringByValue(static_cast<int64>(Config.Visibility)).ToUpper());
	JsonBody->SetStringField(TEXT("mode"), Config.Mode == ERoomMode::Sync ? TEXT("SYNC") : TEXT("ASYNC"));
	JsonBody->SetNumberField(TEXT("roundsCount"), Config.RoundsCount);
	JsonBody->SetBoolField(TEXT("inviteRequired"), Config.bInviteRequired);

	if (Config.MatchDuration > 0)
	{
		JsonBody->SetNumberField(TEXT("matchDuration"), Config.MatchDuration);
	}

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonString);

	Request->OnProcessRequestComplete().BindLambda([this, OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				FPrivateRoom Room = ParseRoom(JsonObject);
				OnSuccess.ExecuteIfBound(Room);
			}
			else
			{
				OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Invalid response format")));
			}
		}
		else
		{
			OnError.ExecuteIfBound(ParseError(Res));
		}
	});

	Request->ProcessRequest();
}

// =============================================================================
// HTTP API - Room Discovery
// =============================================================================

void UDeskillzRoomClient::GetPublicRooms(
	const FString& GameId,
	FOnRoomListSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s?gameId=%s"), *RoomsEndpoint, *GameId);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("GET"), Endpoint);

	Request->OnProcessRequestComplete().BindLambda([this, OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			TArray<TSharedPtr<FJsonValue>> JsonArray;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonArray))
			{
				TArray<FPrivateRoom> Rooms;
				for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
				{
					if (JsonValue->Type == EJson::Object)
					{
						Rooms.Add(ParseRoom(JsonValue->AsObject()));
					}
				}
				OnSuccess.ExecuteIfBound(Rooms);
			}
			else
			{
				OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Invalid response format")));
			}
		}
		else
		{
			OnError.ExecuteIfBound(ParseError(Res));
		}
	});

	Request->ProcessRequest();
}

void UDeskillzRoomClient::GetMyRooms(
	FOnRoomListSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s/my-rooms"), *RoomsEndpoint);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("GET"), Endpoint);

	Request->OnProcessRequestComplete().BindLambda([this, OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			TArray<TSharedPtr<FJsonValue>> JsonArray;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonArray))
			{
				TArray<FPrivateRoom> Rooms;
				for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
				{
					if (JsonValue->Type == EJson::Object)
					{
						Rooms.Add(ParseRoom(JsonValue->AsObject()));
					}
				}
				OnSuccess.ExecuteIfBound(Rooms);
			}
			else
			{
				OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Invalid response format")));
			}
		}
		else
		{
			OnError.ExecuteIfBound(ParseError(Res));
		}
	});

	Request->ProcessRequest();
}

void UDeskillzRoomClient::GetRoomByCode(
	const FString& RoomCode,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s/code/%s"), *RoomsEndpoint, *RoomCode);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("GET"), Endpoint);

	Request->OnProcessRequestComplete().BindLambda([this, OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				OnSuccess.ExecuteIfBound(ParseRoom(JsonObject));
			}
			else
			{
				OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Invalid response format")));
			}
		}
		else
		{
			OnError.ExecuteIfBound(ParseError(Res));
		}
	});

	Request->ProcessRequest();
}

void UDeskillzRoomClient::GetRoomById(
	const FString& RoomId,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s/%s"), *RoomsEndpoint, *RoomId);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("GET"), Endpoint);

	Request->OnProcessRequestComplete().BindLambda([this, OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				OnSuccess.ExecuteIfBound(ParseRoom(JsonObject));
			}
			else
			{
				OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Invalid response format")));
			}
		}
		else
		{
			OnError.ExecuteIfBound(ParseError(Res));
		}
	});

	Request->ProcessRequest();
}

// =============================================================================
// HTTP API - Join/Leave
// =============================================================================

void UDeskillzRoomClient::JoinRoom(
	const FString& RoomCode,
	FOnRoomSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s/join"), *RoomsEndpoint);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("POST"), Endpoint);

	TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
	JsonBody->SetStringField(TEXT("roomCode"), RoomCode);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonString);

	Request->OnProcessRequestComplete().BindLambda([this, OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Res->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				OnSuccess.ExecuteIfBound(ParseRoom(JsonObject));
			}
			else
			{
				OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Invalid response format")));
			}
		}
		else
		{
			OnError.ExecuteIfBound(ParseError(Res));
		}
	});

	Request->ProcessRequest();
}

void UDeskillzRoomClient::LeaveRoom(
	const FString& RoomId,
	FOnRoomActionSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s/%s/leave"), *RoomsEndpoint, *RoomId);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("POST"), Endpoint);

	Request->OnProcessRequestComplete().BindLambda([OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			OnSuccess.ExecuteIfBound();
		}
		else
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Failed to leave room")));
		}
	});

	Request->ProcessRequest();
}

// =============================================================================
// HTTP API - Host Actions
// =============================================================================

void UDeskillzRoomClient::KickPlayer(
	const FString& RoomId,
	const FString& PlayerId,
	FOnRoomActionSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s/%s/kick"), *RoomsEndpoint, *RoomId);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("POST"), Endpoint);

	TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
	JsonBody->SetStringField(TEXT("playerId"), PlayerId);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);

	Request->SetContentAsString(JsonString);

	Request->OnProcessRequestComplete().BindLambda([OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			OnSuccess.ExecuteIfBound();
		}
		else
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Failed to kick player")));
		}
	});

	Request->ProcessRequest();
}

void UDeskillzRoomClient::CancelRoom(
	const FString& RoomId,
	FOnRoomActionSuccess OnSuccess,
	FOnRoomError OnError)
{
	FString Endpoint = FString::Printf(TEXT("%s/%s"), *RoomsEndpoint, *RoomId);
	TSharedRef<IHttpRequest> Request = CreateRequest(TEXT("DELETE"), Endpoint);

	Request->OnProcessRequestComplete().BindLambda([OnSuccess, OnError](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::NetworkError, TEXT("Network error")));
			return;
		}

		if (Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300)
		{
			OnSuccess.ExecuteIfBound();
		}
		else
		{
			OnError.ExecuteIfBound(FRoomError(FRoomError::ServerError, TEXT("Failed to cancel room")));
		}
	});

	Request->ProcessRequest();
}

// =============================================================================
// WebSocket Messages
// =============================================================================

void UDeskillzRoomClient::SendReady(const FString& RoomId, bool bIsReady)
{
	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("roomId"), RoomId);
	Data->SetBoolField(TEXT("isReady"), bIsReady);

	SendWebSocketMessage(TEXT("room:ready"), Data);
}

void UDeskillzRoomClient::SendChat(const FString& RoomId, const FString& Message)
{
	if (Message.IsEmpty() || Message.Len() > 500)
	{
		return;
	}

	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("roomId"), RoomId);
	Data->SetStringField(TEXT("message"), Message);

	SendWebSocketMessage(TEXT("room:chat"), Data);
}

void UDeskillzRoomClient::SendStart(const FString& RoomId)
{
	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("roomId"), RoomId);

	SendWebSocketMessage(TEXT("room:start"), Data);
}

void UDeskillzRoomClient::SendKick(const FString& RoomId, const FString& TargetUserId)
{
	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("roomId"), RoomId);
	Data->SetStringField(TEXT("targetUserId"), TargetUserId);

	SendWebSocketMessage(TEXT("room:kick"), Data);
}

void UDeskillzRoomClient::SendCancel(const FString& RoomId)
{
	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("roomId"), RoomId);

	SendWebSocketMessage(TEXT("room:cancel"), Data);
}

// =============================================================================
// HTTP Helpers
// =============================================================================

FString UDeskillzRoomClient::GetBaseUrl() const
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(GetOuter());
	if (SDK)
	{
		return SDK->GetApiBaseUrl();
	}
	return TEXT("https://api.deskillz.games");
}

FString UDeskillzRoomClient::GetWebSocketUrl() const
{
	FString BaseUrl = GetBaseUrl();
	BaseUrl = BaseUrl.Replace(TEXT("https://"), TEXT("wss://"));
	BaseUrl = BaseUrl.Replace(TEXT("http://"), TEXT("ws://"));
	return BaseUrl + TEXT("/lobby");
}

FString UDeskillzRoomClient::GetAuthToken() const
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(GetOuter());
	if (SDK)
	{
		return SDK->GetAuthToken();
	}
	return FString();
}

TSharedRef<IHttpRequest> UDeskillzRoomClient::CreateRequest(const FString& Verb, const FString& Endpoint)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();

	Request->SetURL(GetBaseUrl() + Endpoint);
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	FString AuthToken = GetAuthToken();
	if (!AuthToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}

	return Request;
}

FRoomError UDeskillzRoomClient::ParseError(FHttpResponsePtr Response) const
{
	if (!Response.IsValid())
	{
		return FRoomError(FRoomError::NetworkError, TEXT("Network error"));
	}

	int32 StatusCode = Response->GetResponseCode();
	FString Content = Response->GetContentAsString();

	// Try to parse error from JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		FString Code = JsonObject->GetStringField(TEXT("code"));
		FString Message = JsonObject->GetStringField(TEXT("message"));
		if (!Message.IsEmpty())
		{
			return FRoomError(Code.IsEmpty() ? FRoomError::ServerError : Code, Message);
		}
	}

	// Default error based on status code
	switch (StatusCode)
	{
		case 401: return FRoomError(FRoomError::NotAuthenticated, TEXT("Authentication required"));
		case 403: return FRoomError(FRoomError::NotHost, TEXT("Permission denied"));
		case 404: return FRoomError(FRoomError::RoomNotFound, TEXT("Room not found"));
		case 409: return FRoomError(FRoomError::AlreadyInRoom, TEXT("Already in room"));
		case 422: return FRoomError(FRoomError::InvalidCode, TEXT("Invalid request"));
		default: return FRoomError(FRoomError::ServerError, FString::Printf(TEXT("Server error (%d)"), StatusCode));
	}
}

FPrivateRoom UDeskillzRoomClient::ParseRoom(const TSharedPtr<FJsonObject>& JsonObject) const
{
	FPrivateRoom Room;

	if (!JsonObject.IsValid())
	{
		return Room;
	}

	Room.Id = JsonObject->GetStringField(TEXT("id"));
	Room.RoomCode = JsonObject->GetStringField(TEXT("roomCode"));
	Room.Name = JsonObject->GetStringField(TEXT("name"));
	Room.Description = JsonObject->GetStringField(TEXT("description"));
	Room.EntryFee = JsonObject->GetNumberField(TEXT("entryFee"));
	Room.EntryCurrency = JsonObject->GetStringField(TEXT("entryCurrency"));
	Room.PrizePool = JsonObject->GetNumberField(TEXT("prizePool"));
	Room.MinPlayers = JsonObject->GetIntegerField(TEXT("minPlayers"));
	Room.MaxPlayers = JsonObject->GetIntegerField(TEXT("maxPlayers"));
	Room.CurrentPlayers = JsonObject->GetIntegerField(TEXT("currentPlayers"));
	Room.bInviteRequired = JsonObject->GetBoolField(TEXT("inviteRequired"));

	// Parse host
	const TSharedPtr<FJsonObject>* HostObj;
	if (JsonObject->TryGetObjectField(TEXT("host"), HostObj))
	{
		Room.Host.Id = (*HostObj)->GetStringField(TEXT("id"));
		Room.Host.Username = (*HostObj)->GetStringField(TEXT("username"));
		Room.Host.AvatarUrl = (*HostObj)->GetStringField(TEXT("avatarUrl"));
	}

	// Parse game
	const TSharedPtr<FJsonObject>* GameObj;
	if (JsonObject->TryGetObjectField(TEXT("game"), GameObj))
	{
		Room.Game.Id = (*GameObj)->GetStringField(TEXT("id"));
		Room.Game.Name = (*GameObj)->GetStringField(TEXT("name"));
		Room.Game.IconUrl = (*GameObj)->GetStringField(TEXT("iconUrl"));
	}

	// Parse players
	const TArray<TSharedPtr<FJsonValue>>* PlayersArray;
	if (JsonObject->TryGetArrayField(TEXT("players"), PlayersArray))
	{
		for (const TSharedPtr<FJsonValue>& PlayerValue : *PlayersArray)
		{
			if (PlayerValue->Type == EJson::Object)
			{
				Room.Players.Add(ParsePlayer(PlayerValue->AsObject()));
			}
		}
	}

	return Room;
}

FRoomPlayer UDeskillzRoomClient::ParsePlayer(const TSharedPtr<FJsonObject>& JsonObject) const
{
	FRoomPlayer Player;

	if (!JsonObject.IsValid())
	{
		return Player;
	}

	Player.Id = JsonObject->GetStringField(TEXT("id"));
	Player.Username = JsonObject->GetStringField(TEXT("username"));
	Player.AvatarUrl = JsonObject->GetStringField(TEXT("avatarUrl"));
	Player.bIsReady = JsonObject->GetBoolField(TEXT("isReady"));
	Player.bIsAdmin = JsonObject->GetBoolField(TEXT("isAdmin"));

	return Player;
}

// =============================================================================
// WebSocket Helpers
// =============================================================================

void UDeskillzRoomClient::SendWebSocketMessage(const FString& Event, const TSharedPtr<FJsonObject>& Data)
{
	if (!IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzRoomClient] Cannot send message: not connected"));
		return;
	}

	TSharedPtr<FJsonObject> Envelope = MakeShareable(new FJsonObject);
	Envelope->SetStringField(TEXT("event"), Event);
	Envelope->SetObjectField(TEXT("data"), Data);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Envelope.ToSharedRef(), Writer);

	WebSocket->Send(JsonString);
}

void UDeskillzRoomClient::HandleWebSocketConnected()
{
	ReconnectAttempts = 0;
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] WebSocket connected"));

	SubscribeToRoom();
}

void UDeskillzRoomClient::HandleWebSocketDisconnected()
{
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] WebSocket disconnected"));

	if (bShouldReconnect && ReconnectAttempts < MaxReconnectAttempts)
	{
		AttemptReconnect();
	}
}

void UDeskillzRoomClient::HandleWebSocketMessage(const FString& Message)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzRoomClient] Failed to parse WebSocket message"));
		return;
	}

	FString EventType = JsonObject->GetStringField(TEXT("event"));
	const TSharedPtr<FJsonObject>* DataObj;
	TSharedPtr<FJsonObject> Data;

	if (JsonObject->TryGetObjectField(TEXT("data"), DataObj))
	{
		Data = *DataObj;
	}

	ProcessWebSocketEvent(EventType, Data);
}

void UDeskillzRoomClient::HandleWebSocketError(const FString& Error)
{
	UE_LOG(LogTemp, Error, TEXT("[DeskillzRoomClient] WebSocket error: %s"), *Error);
	OnError.Broadcast(Error);
}

void UDeskillzRoomClient::ProcessWebSocketEvent(const FString& EventType, const TSharedPtr<FJsonObject>& Data)
{
	UE_LOG(LogTemp, Verbose, TEXT("[DeskillzRoomClient] Event: %s"), *EventType);

	if (EventType == TEXT("room:state"))
	{
		OnStateUpdate.Broadcast(ParseRoom(Data));
	}
	else if (EventType == TEXT("private-room:player-joined"))
	{
		OnPlayerJoined.Broadcast(ParsePlayer(Data));
	}
	else if (EventType == TEXT("private-room:player-left"))
	{
		OnPlayerLeft.Broadcast(Data->GetStringField(TEXT("id")));
	}
	else if (EventType == TEXT("private-room:player-kicked"))
	{
		OnPlayerKicked.Broadcast(Data->GetStringField(TEXT("id")));
	}
	else if (EventType == TEXT("private-room:player-ready"))
	{
		OnPlayerReady.Broadcast(
			Data->GetStringField(TEXT("id")),
			Data->GetBoolField(TEXT("isReady")),
			Data->GetBoolField(TEXT("allReady"))
		);
	}
	else if (EventType == TEXT("private-room:all-ready"))
	{
		OnAllReady.Broadcast(Data->GetIntegerField(TEXT("playerCount")));
	}
	else if (EventType == TEXT("private-room:countdown-started"))
	{
		OnCountdownStarted.Broadcast(Data->GetIntegerField(TEXT("countdownSeconds")));
	}
	else if (EventType == TEXT("private-room:countdown-tick"))
	{
		OnCountdownTick.Broadcast(Data->GetIntegerField(TEXT("seconds")));
	}
	else if (EventType == TEXT("private-room:launching"))
	{
		FMatchLaunchData LaunchData;
		LaunchData.MatchId = Data->GetStringField(TEXT("matchId"));
		LaunchData.DeepLink = Data->GetStringField(TEXT("deepLink"));
		LaunchData.Token = Data->GetStringField(TEXT("token"));
		LaunchData.GameSessionId = Data->GetStringField(TEXT("gameSessionId"));
		OnLaunching.Broadcast(LaunchData);
	}
	else if (EventType == TEXT("private-room:cancelled"))
	{
		OnCancelled.Broadcast(Data->GetStringField(TEXT("reason")));
	}
	else if (EventType == TEXT("private-room:kicked"))
	{
		OnKicked.Broadcast(Data->GetStringField(TEXT("reason")));
	}
	else if (EventType == TEXT("private-room:chat"))
	{
		OnChat.Broadcast(
			Data->GetStringField(TEXT("id")),
			Data->GetStringField(TEXT("username")),
			Data->GetStringField(TEXT("message"))
		);
	}
	else if (EventType == TEXT("error"))
	{
		OnError.Broadcast(Data->GetStringField(TEXT("message")));
	}
}

void UDeskillzRoomClient::SubscribeToRoom()
{
	if (CurrentRoomId.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("roomId"), CurrentRoomId);

	SendWebSocketMessage(TEXT("room:subscribe"), Data);
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] Subscribed to room %s"), *CurrentRoomId);
}

void UDeskillzRoomClient::UnsubscribeFromRoom()
{
	if (CurrentRoomId.IsEmpty() || !IsConnected())
	{
		return;
	}

	TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
	Data->SetStringField(TEXT("roomId"), CurrentRoomId);

	SendWebSocketMessage(TEXT("room:unsubscribe"), Data);
}

void UDeskillzRoomClient::AttemptReconnect()
{
	ReconnectAttempts++;
	UE_LOG(LogTemp, Log, TEXT("[DeskillzRoomClient] Reconnecting (attempt %d/%d)..."), ReconnectAttempts, MaxReconnectAttempts);

	// Use timer for delayed reconnect
	if (UWorld* World = GEngine->GetWorldFromContextObject(GetOuter(), EGetWorldErrorMode::ReturnNull))
	{
		World->GetTimerManager().SetTimer(
			ReconnectTimerHandle,
			[this]() { Connect(CurrentRoomId); },
			ReconnectDelay * ReconnectAttempts,
			false
		);
	}
}