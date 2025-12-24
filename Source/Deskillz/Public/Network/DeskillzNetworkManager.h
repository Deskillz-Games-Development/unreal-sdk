// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Network/DeskillzHttpClient.h"
#include "Network/DeskillzWebSocket.h"
#include "DeskillzNetworkManager.generated.h"

class UDeskillzHttpClient;
class UDeskillzWebSocket;

/**
 * Network connectivity state
 */
UENUM(BlueprintType)
enum class EDeskillzNetworkState : uint8
{
	/** Fully online (HTTP + WebSocket) */
	Online,
	
	/** HTTP only (WebSocket disconnected) */
	PartialOnline,
	
	/** Completely offline */
	Offline,
	
	/** Connecting */
	Connecting
};

/**
 * Server region
 */
UENUM(BlueprintType)
enum class EDeskillzServerRegion : uint8
{
	Auto,
	NorthAmerica,
	Europe,
	AsiaPacific,
	SouthAmerica
};

/**
 * Network configuration
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzNetworkConfig
{
	GENERATED_BODY()
	
	/** API Base URL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString ApiBaseUrl = TEXT("https://api.deskillz.games");
	
	/** WebSocket URL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	FString WebSocketUrl = TEXT("wss://ws.deskillz.games");
	
	/** Server region */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	EDeskillzServerRegion Region = EDeskillzServerRegion::Auto;
	
	/** Enable SSL/TLS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableSSL = true;
	
	/** Request timeout */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float RequestTimeout = 30.0f;
	
	/** WebSocket heartbeat interval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float HeartbeatInterval = 30.0f;
	
	/** Enable auto-reconnect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bAutoReconnect = true;
	
	/** Max reconnect attempts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 MaxReconnectAttempts = 10;
	
	/** Enable request caching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableCaching = true;
	
	/** Cache TTL (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float CacheTTL = 60.0f;
	
	/** Enable offline queue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableOfflineQueue = true;
	
	/** Max offline queue size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 MaxOfflineQueueSize = 100;
};

/** Network state delegate */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkStateChanged, EDeskillzNetworkState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthTokenRefreshed, const FString&, NewToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAuthTokenExpired);

/**
 * Deskillz Network Manager
 * 
 * High-level network management combining HTTP and WebSocket:
 * - Unified configuration
 * - Connection state management
 * - Auto-reconnection handling
 * - Token refresh management
 * - Offline queue processing
 * - Region selection
 * 
 * Usage:
 *   UDeskillzNetworkManager* Network = UDeskillzNetworkManager::Get();
 *   Network->Initialize(Config);
 *   Network->Connect(AuthToken);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzNetworkManager : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzNetworkManager();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Network Manager instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network", meta = (DisplayName = "Get Deskillz Network"))
	static UDeskillzNetworkManager* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize network with configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void Initialize(const FDeskillzNetworkConfig& Config);
	
	/**
	 * Initialize with default configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void InitializeDefault();
	
	/**
	 * Shutdown network connections
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void Shutdown();
	
	// ========================================================================
	// Connection
	// ========================================================================
	
	/**
	 * Connect with authentication token
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void Connect(const FString& AuthToken);
	
	/**
	 * Disconnect all connections
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void Disconnect();
	
	/**
	 * Reconnect (useful after network change)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void Reconnect();
	
	/**
	 * Get current network state
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	EDeskillzNetworkState GetNetworkState() const { return CurrentState; }
	
	/**
	 * Is fully connected (HTTP + WebSocket)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	bool IsFullyConnected() const;
	
	/**
	 * Is HTTP available
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	bool IsHttpAvailable() const;
	
	/**
	 * Is WebSocket connected
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	bool IsWebSocketConnected() const;
	
	// ========================================================================
	// Authentication
	// ========================================================================
	
	/**
	 * Set authentication token
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetAuthToken(const FString& Token);
	
	/**
	 * Clear authentication token
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void ClearAuthToken();
	
	/**
	 * Refresh authentication token
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void RefreshAuthToken();
	
	/**
	 * Get current auth token
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	FString GetAuthToken() const { return AuthToken; }
	
	/**
	 * Is authenticated
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	bool IsAuthenticated() const { return !AuthToken.IsEmpty(); }
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/**
	 * Get current configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	FDeskillzNetworkConfig GetConfig() const { return Config; }
	
	/**
	 * Set server region
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetRegion(EDeskillzServerRegion Region);
	
	/**
	 * Get optimal server region
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void DetectOptimalRegion();
	
	// ========================================================================
	// Accessors
	// ========================================================================
	
	/**
	 * Get HTTP client
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	UDeskillzHttpClient* GetHttpClient() const { return HttpClient; }
	
	/**
	 * Get WebSocket client
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	UDeskillzWebSocket* GetWebSocket() const { return WebSocketClient; }
	
	// ========================================================================
	// Offline Queue
	// ========================================================================
	
	/**
	 * Get offline queue size
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	int32 GetOfflineQueueSize() const;
	
	/**
	 * Process offline queue (called automatically on reconnect)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void ProcessOfflineQueue();
	
	/**
	 * Clear offline queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void ClearOfflineQueue();
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when network state changes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnNetworkStateChanged OnNetworkStateChanged;
	
	/** Called when auth token is refreshed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnAuthTokenRefreshed OnAuthTokenRefreshed;
	
	/** Called when auth token expires */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Network")
	FOnAuthTokenExpired OnAuthTokenExpired;
	
protected:
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Network configuration */
	UPROPERTY()
	FDeskillzNetworkConfig Config;
	
	/** Auth token */
	UPROPERTY()
	FString AuthToken;
	
	/** Refresh token */
	UPROPERTY()
	FString RefreshToken;
	
	/** Current network state */
	UPROPERTY()
	EDeskillzNetworkState CurrentState = EDeskillzNetworkState::Offline;
	
	/** Is initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	// ========================================================================
	// Clients
	// ========================================================================
	
	/** HTTP Client */
	UPROPERTY()
	UDeskillzHttpClient* HttpClient;
	
	/** WebSocket Client */
	UPROPERTY()
	UDeskillzWebSocket* WebSocketClient;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Offline request queue */
	TArray<FDeskillzHttpRequest> OfflineQueue;
	
	/** Token refresh timer */
	FTimerHandle TokenRefreshTimerHandle;
	
	/** Network check timer */
	FTimerHandle NetworkCheckTimerHandle;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Set network state */
	void SetNetworkState(EDeskillzNetworkState NewState);
	
	/** Update network state based on connections */
	void UpdateNetworkState();
	
	/** Handle WebSocket connected */
	UFUNCTION()
	void OnWebSocketConnected();
	
	/** Handle WebSocket disconnected */
	UFUNCTION()
	void OnWebSocketDisconnected(const FString& Reason);
	
	/** Handle HTTP unauthorized response */
	void OnHttpUnauthorized();
	
	/** Schedule token refresh */
	void ScheduleTokenRefresh(float DelaySeconds);
	
	/** Perform token refresh */
	void DoTokenRefresh();
	
	/** Get region URL */
	FString GetRegionApiUrl(EDeskillzServerRegion Region) const;
	
	/** Get region WebSocket URL */
	FString GetRegionWebSocketUrl(EDeskillzServerRegion Region) const;
	
	/** Add request to offline queue */
	void QueueOfflineRequest(const FDeskillzHttpRequest& Request);
	
	/** Start network monitoring */
	void StartNetworkMonitoring();
	
	/** Stop network monitoring */
	void StopNetworkMonitoring();
	
	/** Check network connectivity */
	void CheckNetworkConnectivity();
};
