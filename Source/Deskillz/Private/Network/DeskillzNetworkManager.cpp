// Copyright Deskillz Games. All Rights Reserved.

#include "Network/DeskillzNetworkManager.h"
#include "Network/DeskillzHttpClient.h"
#include "Network/DeskillzWebSocket.h"
#include "Network/DeskillzApiEndpoints.h"
#include "Deskillz.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// Static singleton
static UDeskillzNetworkManager* GNetworkManager = nullptr;

UDeskillzNetworkManager::UDeskillzNetworkManager()
{
}

UDeskillzNetworkManager* UDeskillzNetworkManager::Get()
{
	if (!GNetworkManager)
	{
		GNetworkManager = NewObject<UDeskillzNetworkManager>();
		GNetworkManager->AddToRoot();
	}
	return GNetworkManager;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzNetworkManager::Initialize(const FDeskillzNetworkConfig& InConfig)
{
	if (bIsInitialized)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Network already initialized"));
		return;
	}
	
	Config = InConfig;
	
	// Get or create HTTP client
	HttpClient = UDeskillzHttpClient::Get();
	HttpClient->SetBaseUrl(Config.ApiBaseUrl);
	HttpClient->SetDefaultTimeout(Config.RequestTimeout);
	
	// Get or create WebSocket client
	WebSocketClient = UDeskillzWebSocket::Get();
	WebSocketClient->SetAutoReconnect(Config.bAutoReconnect, 5.0f, Config.MaxReconnectAttempts);
	WebSocketClient->SetHeartbeatInterval(Config.HeartbeatInterval);
	
	// Bind WebSocket events
	WebSocketClient->OnConnected.AddDynamic(this, &UDeskillzNetworkManager::OnWebSocketConnected);
	WebSocketClient->OnDisconnected.AddDynamic(this, &UDeskillzNetworkManager::OnWebSocketDisconnected);
	
	bIsInitialized = true;
	SetNetworkState(EDeskillzNetworkState::Offline);
	
	// Start network monitoring
	StartNetworkMonitoring();
	
	UE_LOG(LogDeskillz, Log, TEXT("Network initialized - API: %s, WS: %s"), 
		*Config.ApiBaseUrl, *Config.WebSocketUrl);
}

void UDeskillzNetworkManager::InitializeDefault()
{
	FDeskillzNetworkConfig DefaultConfig;
	Initialize(DefaultConfig);
}

void UDeskillzNetworkManager::Shutdown()
{
	StopNetworkMonitoring();
	
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(TokenRefreshTimerHandle);
	}
	
	Disconnect();
	
	bIsInitialized = false;
	UE_LOG(LogDeskillz, Log, TEXT("Network shutdown"));
}

// ============================================================================
// Connection
// ============================================================================

void UDeskillzNetworkManager::Connect(const FString& Token)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Network not initialized"));
		return;
	}
	
	SetAuthToken(Token);
	SetNetworkState(EDeskillzNetworkState::Connecting);
	
	// Connect WebSocket
	FString WsUrl = Config.WebSocketUrl;
	if (!Token.IsEmpty())
	{
		WsUrl = DeskillzApi::WithQuery(WsUrl, TEXT("token"), Token);
	}
	
	WebSocketClient->ConnectWithAuth(Config.WebSocketUrl, Token);
	
	UE_LOG(LogDeskillz, Log, TEXT("Connecting to network..."));
}

void UDeskillzNetworkManager::Disconnect()
{
	if (WebSocketClient)
	{
		WebSocketClient->Disconnect();
	}
	
	if (HttpClient)
	{
		HttpClient->CancelAllRequests();
	}
	
	SetNetworkState(EDeskillzNetworkState::Offline);
	UE_LOG(LogDeskillz, Log, TEXT("Disconnected from network"));
}

void UDeskillzNetworkManager::Reconnect()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Reconnecting..."));
	
	// Disconnect first
	if (WebSocketClient)
	{
		WebSocketClient->Disconnect();
	}
	
	// Reconnect with current token
	if (!AuthToken.IsEmpty())
	{
		Connect(AuthToken);
	}
}

bool UDeskillzNetworkManager::IsFullyConnected() const
{
	return CurrentState == EDeskillzNetworkState::Online;
}

bool UDeskillzNetworkManager::IsHttpAvailable() const
{
	return HttpClient && HttpClient->IsOnline();
}

bool UDeskillzNetworkManager::IsWebSocketConnected() const
{
	return WebSocketClient && WebSocketClient->IsConnected();
}

// ============================================================================
// Authentication
// ============================================================================

void UDeskillzNetworkManager::SetAuthToken(const FString& Token)
{
	AuthToken = Token;
	
	if (HttpClient)
	{
		HttpClient->SetAuthToken(Token);
	}
	
	if (WebSocketClient)
	{
		WebSocketClient->SetAuthToken(Token);
	}
	
	// Schedule token refresh (refresh at 80% of token lifetime)
	// Assuming token is valid for 1 hour
	if (!Token.IsEmpty())
	{
		ScheduleTokenRefresh(2880.0f); // 48 minutes
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Auth token set"));
}

void UDeskillzNetworkManager::ClearAuthToken()
{
	AuthToken.Empty();
	RefreshToken.Empty();
	
	if (HttpClient)
	{
		HttpClient->ClearAuthToken();
	}
	
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(TokenRefreshTimerHandle);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Auth token cleared"));
}

void UDeskillzNetworkManager::RefreshAuthToken()
{
	if (RefreshToken.IsEmpty())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No refresh token available"));
		OnAuthTokenExpired.Broadcast();
		return;
	}
	
	DoTokenRefresh();
}

// ============================================================================
// Configuration
// ============================================================================

void UDeskillzNetworkManager::SetRegion(EDeskillzServerRegion Region)
{
	if (Config.Region != Region)
	{
		Config.Region = Region;
		
		// Update URLs
		Config.ApiBaseUrl = GetRegionApiUrl(Region);
		Config.WebSocketUrl = GetRegionWebSocketUrl(Region);
		
		if (HttpClient)
		{
			HttpClient->SetBaseUrl(Config.ApiBaseUrl);
		}
		
		// Reconnect if connected
		if (CurrentState != EDeskillzNetworkState::Offline)
		{
			Reconnect();
		}
		
		UE_LOG(LogDeskillz, Log, TEXT("Region set to: %d"), static_cast<int32>(Region));
	}
}

void UDeskillzNetworkManager::DetectOptimalRegion()
{
	// In full implementation, ping each region and select lowest latency
	// For now, default to Auto (which uses main server)
	UE_LOG(LogDeskillz, Log, TEXT("Detecting optimal region..."));
	
	// Simulate region detection
	SetRegion(EDeskillzServerRegion::Auto);
}

// ============================================================================
// Offline Queue
// ============================================================================

int32 UDeskillzNetworkManager::GetOfflineQueueSize() const
{
	return OfflineQueue.Num();
}

void UDeskillzNetworkManager::ProcessOfflineQueue()
{
	if (OfflineQueue.Num() == 0)
	{
		return;
	}
	
	if (!IsHttpAvailable())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot process offline queue - not connected"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Processing %d queued requests"), OfflineQueue.Num());
	
	// Process each queued request
	for (const FDeskillzHttpRequest& Request : OfflineQueue)
	{
		HttpClient->SendRequest(Request, FOnDeskillzHttpResponse::CreateLambda([](const FDeskillzHttpResponse& Response)
		{
			// Log result but don't handle callbacks (original callbacks are lost)
			UE_LOG(LogDeskillz, Verbose, TEXT("Queued request completed: %d"), Response.StatusCode);
		}));
	}
	
	OfflineQueue.Empty();
}

void UDeskillzNetworkManager::ClearOfflineQueue()
{
	int32 Count = OfflineQueue.Num();
	OfflineQueue.Empty();
	UE_LOG(LogDeskillz, Log, TEXT("Cleared %d queued requests"), Count);
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzNetworkManager::SetNetworkState(EDeskillzNetworkState NewState)
{
	if (CurrentState != NewState)
	{
		EDeskillzNetworkState OldState = CurrentState;
		CurrentState = NewState;
		
		OnNetworkStateChanged.Broadcast(NewState);
		
		UE_LOG(LogDeskillz, Log, TEXT("Network state: %d -> %d"), 
			static_cast<int32>(OldState), static_cast<int32>(NewState));
		
		// Process offline queue when coming online
		if (NewState == EDeskillzNetworkState::Online && Config.bEnableOfflineQueue)
		{
			ProcessOfflineQueue();
		}
	}
}

void UDeskillzNetworkManager::UpdateNetworkState()
{
	bool bHttpOnline = IsHttpAvailable();
	bool bWsConnected = IsWebSocketConnected();
	
	if (bHttpOnline && bWsConnected)
	{
		SetNetworkState(EDeskillzNetworkState::Online);
	}
	else if (bHttpOnline)
	{
		SetNetworkState(EDeskillzNetworkState::PartialOnline);
	}
	else
	{
		SetNetworkState(EDeskillzNetworkState::Offline);
	}
}

void UDeskillzNetworkManager::OnWebSocketConnected()
{
	UE_LOG(LogDeskillz, Log, TEXT("WebSocket connected"));
	UpdateNetworkState();
}

void UDeskillzNetworkManager::OnWebSocketDisconnected(const FString& Reason)
{
	UE_LOG(LogDeskillz, Log, TEXT("WebSocket disconnected: %s"), *Reason);
	UpdateNetworkState();
}

void UDeskillzNetworkManager::OnHttpUnauthorized()
{
	UE_LOG(LogDeskillz, Warning, TEXT("HTTP 401 Unauthorized - attempting token refresh"));
	RefreshAuthToken();
}

void UDeskillzNetworkManager::ScheduleTokenRefresh(float DelaySeconds)
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(
			TokenRefreshTimerHandle,
			this,
			&UDeskillzNetworkManager::DoTokenRefresh,
			DelaySeconds,
			false
		);
		
		UE_LOG(LogDeskillz, Verbose, TEXT("Token refresh scheduled in %.0f seconds"), DelaySeconds);
	}
}

void UDeskillzNetworkManager::DoTokenRefresh()
{
	if (!HttpClient || RefreshToken.IsEmpty())
	{
		OnAuthTokenExpired.Broadcast();
		return;
	}
	
	// Build refresh request
	TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject());
	JsonBody->SetStringField(TEXT("refresh_token"), RefreshToken);
	
	HttpClient->PostJson(DeskillzApi::Auth::RefreshToken, JsonBody,
		FOnDeskillzHttpResponse::CreateLambda([this](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> JsonResponse;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, JsonResponse))
				{
					FString NewToken = JsonResponse->GetStringField(TEXT("access_token"));
					FString NewRefreshToken = JsonResponse->GetStringField(TEXT("refresh_token"));
					
					AuthToken = NewToken;
					RefreshToken = NewRefreshToken;
					
					SetAuthToken(NewToken);
					OnAuthTokenRefreshed.Broadcast(NewToken);
					
					UE_LOG(LogDeskillz, Log, TEXT("Auth token refreshed"));
				}
			}
			else if (Response.IsUnauthorized())
			{
				ClearAuthToken();
				OnAuthTokenExpired.Broadcast();
				UE_LOG(LogDeskillz, Warning, TEXT("Token refresh failed - token expired"));
			}
			else
			{
				// Retry later
				ScheduleTokenRefresh(60.0f);
				UE_LOG(LogDeskillz, Warning, TEXT("Token refresh failed - will retry"));
			}
		})
	);
}

FString UDeskillzNetworkManager::GetRegionApiUrl(EDeskillzServerRegion Region) const
{
	switch (Region)
	{
		case EDeskillzServerRegion::NorthAmerica:
			return TEXT("https://us.api.deskillz.games");
		case EDeskillzServerRegion::Europe:
			return TEXT("https://eu.api.deskillz.games");
		case EDeskillzServerRegion::AsiaPacific:
			return TEXT("https://ap.api.deskillz.games");
		case EDeskillzServerRegion::SouthAmerica:
			return TEXT("https://sa.api.deskillz.games");
		default:
			return TEXT("https://api.deskillz.games");
	}
}

FString UDeskillzNetworkManager::GetRegionWebSocketUrl(EDeskillzServerRegion Region) const
{
	switch (Region)
	{
		case EDeskillzServerRegion::NorthAmerica:
			return TEXT("wss://us.ws.deskillz.games");
		case EDeskillzServerRegion::Europe:
			return TEXT("wss://eu.ws.deskillz.games");
		case EDeskillzServerRegion::AsiaPacific:
			return TEXT("wss://ap.ws.deskillz.games");
		case EDeskillzServerRegion::SouthAmerica:
			return TEXT("wss://sa.ws.deskillz.games");
		default:
			return TEXT("wss://ws.deskillz.games");
	}
}

void UDeskillzNetworkManager::QueueOfflineRequest(const FDeskillzHttpRequest& Request)
{
	if (!Config.bEnableOfflineQueue)
	{
		return;
	}
	
	if (OfflineQueue.Num() >= Config.MaxOfflineQueueSize)
	{
		// Remove oldest request
		OfflineQueue.RemoveAt(0);
	}
	
	OfflineQueue.Add(Request);
	UE_LOG(LogDeskillz, Verbose, TEXT("Request queued for offline: %s"), *Request.Endpoint);
}

void UDeskillzNetworkManager::StartNetworkMonitoring()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(
			NetworkCheckTimerHandle,
			this,
			&UDeskillzNetworkManager::CheckNetworkConnectivity,
			30.0f, // Check every 30 seconds
			true
		);
	}
}

void UDeskillzNetworkManager::StopNetworkMonitoring()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(NetworkCheckTimerHandle);
	}
}

void UDeskillzNetworkManager::CheckNetworkConnectivity()
{
	// Simple connectivity check via HTTP ping
	if (HttpClient)
	{
		HttpClient->Get(TEXT("/health"),
			FOnDeskillzHttpResponse::CreateLambda([this](const FDeskillzHttpResponse& Response)
			{
				UpdateNetworkState();
			})
		);
	}
}
