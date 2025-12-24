// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "DeskillzHttpClient.generated.h"

/**
 * HTTP Method types
 */
UENUM(BlueprintType)
enum class EDeskillzHttpMethod : uint8
{
	GET,
	POST,
	PUT,
	PATCH,
	DELETE_
};

/**
 * HTTP Request priority
 */
UENUM(BlueprintType)
enum class EDeskillzRequestPriority : uint8
{
	Low,
	Normal,
	High,
	Critical
};

/**
 * HTTP Response data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHttpResponse
{
	GENERATED_BODY()
	
	/** Was request successful */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	bool bSuccess = false;
	
	/** HTTP status code */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	int32 StatusCode = 0;
	
	/** Response body as string */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	FString Body;
	
	/** Response headers */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	TMap<FString, FString> Headers;
	
	/** Error message if failed */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	FString ErrorMessage;
	
	/** Request duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	float Duration = 0.0f;
	
	/** Was this from cache */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	bool bFromCache = false;
	
	/** Request ID for tracking */
	UPROPERTY(BlueprintReadOnly, Category = "HTTP")
	FString RequestId;
	
	/** Check if response is OK (2xx) */
	bool IsOk() const { return bSuccess && StatusCode >= 200 && StatusCode < 300; }
	
	/** Check if unauthorized (401) */
	bool IsUnauthorized() const { return StatusCode == 401; }
	
	/** Check if rate limited (429) */
	bool IsRateLimited() const { return StatusCode == 429; }
	
	/** Check if server error (5xx) */
	bool IsServerError() const { return StatusCode >= 500 && StatusCode < 600; }
};

/**
 * HTTP Request configuration
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzHttpRequest
{
	GENERATED_BODY()
	
	/** Request URL (relative to base URL) */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	FString Endpoint;
	
	/** HTTP Method */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	EDeskillzHttpMethod Method = EDeskillzHttpMethod::GET;
	
	/** Request body (for POST/PUT/PATCH) */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	FString Body;
	
	/** Additional headers */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	TMap<FString, FString> Headers;
	
	/** Query parameters */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	TMap<FString, FString> QueryParams;
	
	/** Request timeout in seconds */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	float Timeout = 30.0f;
	
	/** Number of retry attempts */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	int32 MaxRetries = 3;
	
	/** Request priority */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	EDeskillzRequestPriority Priority = EDeskillzRequestPriority::Normal;
	
	/** Should authenticate this request */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	bool bRequiresAuth = true;
	
	/** Cache response */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	bool bCacheable = false;
	
	/** Cache TTL in seconds */
	UPROPERTY(BlueprintReadWrite, Category = "HTTP")
	float CacheTTL = 60.0f;
	
	/** Unique request ID */
	FString RequestId;
};

/** Delegate for HTTP response */
DECLARE_DELEGATE_OneParam(FOnDeskillzHttpResponse, const FDeskillzHttpResponse&);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDeskillzHttpResponseDynamic, const FDeskillzHttpResponse&, Response);

/** Delegate for progress updates */
DECLARE_DELEGATE_TwoParams(FOnDeskillzHttpProgress, int32 /*BytesSent*/, int32 /*BytesReceived*/);

/**
 * Deskillz HTTP Client
 * 
 * Core HTTP client for all API communication:
 * - REST API calls with automatic auth
 * - Request queuing and prioritization
 * - Retry logic with exponential backoff
 * - Response caching
 * - Progress tracking
 * 
 * Usage:
 *   UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
 *   Http->Get("/api/v1/tournaments", FOnDeskillzHttpResponse::CreateLambda([](const FDeskillzHttpResponse& Response) {
 *       if (Response.IsOk()) { ... }
 *   }));
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzHttpClient : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzHttpClient();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the HTTP Client instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network", meta = (DisplayName = "Get Deskillz HTTP"))
	static UDeskillzHttpClient* Get();
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/**
	 * Set the base URL for all requests
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetBaseUrl(const FString& Url);
	
	/**
	 * Get the base URL
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	FString GetBaseUrl() const { return BaseUrl; }
	
	/**
	 * Set authorization token
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetAuthToken(const FString& Token);
	
	/**
	 * Clear authorization token
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void ClearAuthToken();
	
	/**
	 * Set default timeout
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetDefaultTimeout(float TimeoutSeconds);
	
	/**
	 * Set default headers for all requests
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void SetDefaultHeader(const FString& Key, const FString& Value);
	
	// ========================================================================
	// Request Methods
	// ========================================================================
	
	/**
	 * Send a GET request
	 */
	void Get(const FString& Endpoint, const FOnDeskillzHttpResponse& OnComplete, 
		const TMap<FString, FString>& QueryParams = TMap<FString, FString>());
	
	/**
	 * Send a POST request
	 */
	void Post(const FString& Endpoint, const FString& Body, const FOnDeskillzHttpResponse& OnComplete);
	
	/**
	 * Send a POST request with JSON object
	 */
	void PostJson(const FString& Endpoint, const TSharedPtr<FJsonObject>& JsonBody, 
		const FOnDeskillzHttpResponse& OnComplete);
	
	/**
	 * Send a PUT request
	 */
	void Put(const FString& Endpoint, const FString& Body, const FOnDeskillzHttpResponse& OnComplete);
	
	/**
	 * Send a PATCH request
	 */
	void Patch(const FString& Endpoint, const FString& Body, const FOnDeskillzHttpResponse& OnComplete);
	
	/**
	 * Send a DELETE request
	 */
	void Delete(const FString& Endpoint, const FOnDeskillzHttpResponse& OnComplete);
	
	/**
	 * Send a custom request
	 */
	void SendRequest(const FDeskillzHttpRequest& Request, const FOnDeskillzHttpResponse& OnComplete);
	
	// ========================================================================
	// Blueprint Methods
	// ========================================================================
	
	/**
	 * Send GET request (Blueprint)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network", meta = (DisplayName = "HTTP GET"))
	void K2_Get(const FString& Endpoint, const FOnDeskillzHttpResponseDynamic& OnComplete);
	
	/**
	 * Send POST request (Blueprint)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network", meta = (DisplayName = "HTTP POST"))
	void K2_Post(const FString& Endpoint, const FString& Body, const FOnDeskillzHttpResponseDynamic& OnComplete);
	
	// ========================================================================
	// Utility
	// ========================================================================
	
	/**
	 * Cancel all pending requests
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void CancelAllRequests();
	
	/**
	 * Cancel specific request by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	bool CancelRequest(const FString& RequestId);
	
	/**
	 * Get number of pending requests
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	int32 GetPendingRequestCount() const;
	
	/**
	 * Check if currently online
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Network")
	bool IsOnline() const { return bIsOnline; }
	
	/**
	 * Clear response cache
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Network")
	void ClearCache();
	
	/**
	 * Parse JSON response
	 */
	static bool ParseJsonResponse(const FDeskillzHttpResponse& Response, TSharedPtr<FJsonObject>& OutJson);
	
	/**
	 * Parse JSON array response
	 */
	static bool ParseJsonArrayResponse(const FDeskillzHttpResponse& Response, TArray<TSharedPtr<FJsonValue>>& OutArray);
	
protected:
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Base URL for API */
	UPROPERTY()
	FString BaseUrl;
	
	/** Auth token */
	UPROPERTY()
	FString AuthToken;
	
	/** Default timeout */
	UPROPERTY()
	float DefaultTimeout = 30.0f;
	
	/** Default headers */
	UPROPERTY()
	TMap<FString, FString> DefaultHeaders;
	
	/** Is online */
	UPROPERTY()
	bool bIsOnline = true;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Active requests */
	TMap<FString, TSharedRef<IHttpRequest, ESPMode::ThreadSafe>> ActiveRequests;
	
	/** Request callbacks */
	TMap<FString, FOnDeskillzHttpResponse> RequestCallbacks;
	
	/** Response cache */
	TMap<FString, TPair<FDeskillzHttpResponse, double>> ResponseCache;
	
	/** Request counter for IDs */
	int32 RequestCounter = 0;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Build full URL with query params */
	FString BuildUrl(const FString& Endpoint, const TMap<FString, FString>& QueryParams) const;
	
	/** Generate unique request ID */
	FString GenerateRequestId();
	
	/** Get method string */
	static FString GetMethodString(EDeskillzHttpMethod Method);
	
	/** Create HTTP request */
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateHttpRequest(const FDeskillzHttpRequest& Request);
	
	/** Handle HTTP response */
	void HandleHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FString RequestId);
	
	/** Check cache for response */
	bool GetCachedResponse(const FString& CacheKey, FDeskillzHttpResponse& OutResponse);
	
	/** Store response in cache */
	void CacheResponse(const FString& CacheKey, const FDeskillzHttpResponse& Response, float TTL);
	
	/** Generate cache key */
	FString GenerateCacheKey(const FDeskillzHttpRequest& Request) const;
	
	/** Schedule retry */
	void ScheduleRetry(const FDeskillzHttpRequest& Request, const FOnDeskillzHttpResponse& OnComplete, int32 AttemptNumber);
	
	/** Calculate retry delay */
	float CalculateRetryDelay(int32 AttemptNumber) const;
	
	/** Update online status */
	void UpdateOnlineStatus(bool bOnline);
};
