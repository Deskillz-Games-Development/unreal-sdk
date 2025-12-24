// Copyright Deskillz Games. All Rights Reserved.

#include "Network/DeskillzHttpClient.h"
#include "Deskillz.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "Misc/Base64.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Static singleton
static UDeskillzHttpClient* GHttpClient = nullptr;

UDeskillzHttpClient::UDeskillzHttpClient()
{
	// Default headers
	DefaultHeaders.Add(TEXT("Content-Type"), TEXT("application/json"));
	DefaultHeaders.Add(TEXT("Accept"), TEXT("application/json"));
	DefaultHeaders.Add(TEXT("X-Client-Platform"), TEXT("Unreal"));
	DefaultHeaders.Add(TEXT("X-Client-Version"), TEXT("1.0.0"));
}

UDeskillzHttpClient* UDeskillzHttpClient::Get()
{
	if (!GHttpClient)
	{
		GHttpClient = NewObject<UDeskillzHttpClient>();
		GHttpClient->AddToRoot();
	}
	return GHttpClient;
}

// ============================================================================
// Configuration
// ============================================================================

void UDeskillzHttpClient::SetBaseUrl(const FString& Url)
{
	BaseUrl = Url;
	
	// Remove trailing slash
	if (BaseUrl.EndsWith(TEXT("/")))
	{
		BaseUrl = BaseUrl.LeftChop(1);
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("HTTP Base URL set: %s"), *BaseUrl);
}

void UDeskillzHttpClient::SetAuthToken(const FString& Token)
{
	AuthToken = Token;
	UE_LOG(LogDeskillz, Log, TEXT("Auth token set"));
}

void UDeskillzHttpClient::ClearAuthToken()
{
	AuthToken.Empty();
	UE_LOG(LogDeskillz, Log, TEXT("Auth token cleared"));
}

void UDeskillzHttpClient::SetDefaultTimeout(float TimeoutSeconds)
{
	DefaultTimeout = FMath::Max(1.0f, TimeoutSeconds);
}

void UDeskillzHttpClient::SetDefaultHeader(const FString& Key, const FString& Value)
{
	DefaultHeaders.Add(Key, Value);
}

// ============================================================================
// Request Methods
// ============================================================================

void UDeskillzHttpClient::Get(const FString& Endpoint, const FOnDeskillzHttpResponse& OnComplete,
	const TMap<FString, FString>& QueryParams)
{
	FDeskillzHttpRequest Request;
	Request.Endpoint = Endpoint;
	Request.Method = EDeskillzHttpMethod::GET;
	Request.QueryParams = QueryParams;
	Request.bCacheable = true;
	
	SendRequest(Request, OnComplete);
}

void UDeskillzHttpClient::Post(const FString& Endpoint, const FString& Body, const FOnDeskillzHttpResponse& OnComplete)
{
	FDeskillzHttpRequest Request;
	Request.Endpoint = Endpoint;
	Request.Method = EDeskillzHttpMethod::POST;
	Request.Body = Body;
	
	SendRequest(Request, OnComplete);
}

void UDeskillzHttpClient::PostJson(const FString& Endpoint, const TSharedPtr<FJsonObject>& JsonBody,
	const FOnDeskillzHttpResponse& OnComplete)
{
	FString Body;
	if (JsonBody.IsValid())
	{
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
	}
	
	Post(Endpoint, Body, OnComplete);
}

void UDeskillzHttpClient::Put(const FString& Endpoint, const FString& Body, const FOnDeskillzHttpResponse& OnComplete)
{
	FDeskillzHttpRequest Request;
	Request.Endpoint = Endpoint;
	Request.Method = EDeskillzHttpMethod::PUT;
	Request.Body = Body;
	
	SendRequest(Request, OnComplete);
}

void UDeskillzHttpClient::Patch(const FString& Endpoint, const FString& Body, const FOnDeskillzHttpResponse& OnComplete)
{
	FDeskillzHttpRequest Request;
	Request.Endpoint = Endpoint;
	Request.Method = EDeskillzHttpMethod::PATCH;
	Request.Body = Body;
	
	SendRequest(Request, OnComplete);
}

void UDeskillzHttpClient::Delete(const FString& Endpoint, const FOnDeskillzHttpResponse& OnComplete)
{
	FDeskillzHttpRequest Request;
	Request.Endpoint = Endpoint;
	Request.Method = EDeskillzHttpMethod::DELETE_;
	
	SendRequest(Request, OnComplete);
}

void UDeskillzHttpClient::SendRequest(const FDeskillzHttpRequest& Request, const FOnDeskillzHttpResponse& OnComplete)
{
	// Check cache first for GET requests
	if (Request.bCacheable && Request.Method == EDeskillzHttpMethod::GET)
	{
		FString CacheKey = GenerateCacheKey(Request);
		FDeskillzHttpResponse CachedResponse;
		
		if (GetCachedResponse(CacheKey, CachedResponse))
		{
			UE_LOG(LogDeskillz, Verbose, TEXT("Cache hit for: %s"), *Request.Endpoint);
			OnComplete.ExecuteIfBound(CachedResponse);
			return;
		}
	}
	
	// Generate request ID
	FDeskillzHttpRequest MutableRequest = Request;
	MutableRequest.RequestId = GenerateRequestId();
	
	// Create HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateHttpRequest(MutableRequest);
	
	// Store callback
	RequestCallbacks.Add(MutableRequest.RequestId, OnComplete);
	ActiveRequests.Add(MutableRequest.RequestId, HttpRequest);
	
	// Bind response handler
	HttpRequest->OnProcessRequestComplete().BindUObject(
		this, &UDeskillzHttpClient::HandleHttpResponse, MutableRequest.RequestId);
	
	// Send request
	double StartTime = FPlatformTime::Seconds();
	bool bSent = HttpRequest->ProcessRequest();
	
	if (!bSent)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Failed to send HTTP request: %s"), *MutableRequest.Endpoint);
		
		FDeskillzHttpResponse ErrorResponse;
		ErrorResponse.bSuccess = false;
		ErrorResponse.ErrorMessage = TEXT("Failed to send request");
		ErrorResponse.RequestId = MutableRequest.RequestId;
		
		RequestCallbacks.Remove(MutableRequest.RequestId);
		ActiveRequests.Remove(MutableRequest.RequestId);
		
		OnComplete.ExecuteIfBound(ErrorResponse);
	}
	else
	{
		UE_LOG(LogDeskillz, Verbose, TEXT("HTTP %s: %s"), 
			*GetMethodString(MutableRequest.Method), *MutableRequest.Endpoint);
	}
}

// ============================================================================
// Blueprint Methods
// ============================================================================

void UDeskillzHttpClient::K2_Get(const FString& Endpoint, const FOnDeskillzHttpResponseDynamic& OnComplete)
{
	Get(Endpoint, FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
	{
		OnComplete.ExecuteIfBound(Response);
	}));
}

void UDeskillzHttpClient::K2_Post(const FString& Endpoint, const FString& Body, 
	const FOnDeskillzHttpResponseDynamic& OnComplete)
{
	Post(Endpoint, Body, FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
	{
		OnComplete.ExecuteIfBound(Response);
	}));
}

// ============================================================================
// Utility
// ============================================================================

void UDeskillzHttpClient::CancelAllRequests()
{
	for (auto& Pair : ActiveRequests)
	{
		Pair.Value->CancelRequest();
	}
	
	ActiveRequests.Empty();
	RequestCallbacks.Empty();
	
	UE_LOG(LogDeskillz, Log, TEXT("All HTTP requests cancelled"));
}

bool UDeskillzHttpClient::CancelRequest(const FString& RequestId)
{
	if (TSharedRef<IHttpRequest, ESPMode::ThreadSafe>* Request = ActiveRequests.Find(RequestId))
	{
		(*Request)->CancelRequest();
		ActiveRequests.Remove(RequestId);
		RequestCallbacks.Remove(RequestId);
		
		UE_LOG(LogDeskillz, Log, TEXT("HTTP request cancelled: %s"), *RequestId);
		return true;
	}
	
	return false;
}

int32 UDeskillzHttpClient::GetPendingRequestCount() const
{
	return ActiveRequests.Num();
}

void UDeskillzHttpClient::ClearCache()
{
	ResponseCache.Empty();
	UE_LOG(LogDeskillz, Log, TEXT("HTTP response cache cleared"));
}

bool UDeskillzHttpClient::ParseJsonResponse(const FDeskillzHttpResponse& Response, TSharedPtr<FJsonObject>& OutJson)
{
	if (!Response.IsOk() || Response.Body.IsEmpty())
	{
		return false;
	}
	
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response.Body);
	return FJsonSerializer::Deserialize(Reader, OutJson) && OutJson.IsValid();
}

bool UDeskillzHttpClient::ParseJsonArrayResponse(const FDeskillzHttpResponse& Response, 
	TArray<TSharedPtr<FJsonValue>>& OutArray)
{
	if (!Response.IsOk() || Response.Body.IsEmpty())
	{
		return false;
	}
	
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response.Body);
	return FJsonSerializer::Deserialize(Reader, OutArray);
}

// ============================================================================
// Internal Methods
// ============================================================================

FString UDeskillzHttpClient::BuildUrl(const FString& Endpoint, const TMap<FString, FString>& QueryParams) const
{
	FString Url = BaseUrl;
	
	// Add endpoint
	if (!Endpoint.StartsWith(TEXT("/")))
	{
		Url += TEXT("/");
	}
	Url += Endpoint;
	
	// Add query parameters
	if (QueryParams.Num() > 0)
	{
		Url += TEXT("?");
		bool bFirst = true;
		
		for (const auto& Pair : QueryParams)
		{
			if (!bFirst)
			{
				Url += TEXT("&");
			}
			Url += FGenericPlatformHttp::UrlEncode(Pair.Key);
			Url += TEXT("=");
			Url += FGenericPlatformHttp::UrlEncode(Pair.Value);
			bFirst = false;
		}
	}
	
	return Url;
}

FString UDeskillzHttpClient::GenerateRequestId()
{
	return FString::Printf(TEXT("req_%d_%lld"), ++RequestCounter, FDateTime::UtcNow().GetTicks());
}

FString UDeskillzHttpClient::GetMethodString(EDeskillzHttpMethod Method)
{
	switch (Method)
	{
		case EDeskillzHttpMethod::GET: return TEXT("GET");
		case EDeskillzHttpMethod::POST: return TEXT("POST");
		case EDeskillzHttpMethod::PUT: return TEXT("PUT");
		case EDeskillzHttpMethod::PATCH: return TEXT("PATCH");
		case EDeskillzHttpMethod::DELETE_: return TEXT("DELETE");
		default: return TEXT("GET");
	}
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UDeskillzHttpClient::CreateHttpRequest(const FDeskillzHttpRequest& Request)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	
	// Set URL
	FString Url = BuildUrl(Request.Endpoint, Request.QueryParams);
	HttpRequest->SetURL(Url);
	
	// Set method
	HttpRequest->SetVerb(GetMethodString(Request.Method));
	
	// Set default headers
	for (const auto& Pair : DefaultHeaders)
	{
		HttpRequest->SetHeader(Pair.Key, Pair.Value);
	}
	
	// Set custom headers
	for (const auto& Pair : Request.Headers)
	{
		HttpRequest->SetHeader(Pair.Key, Pair.Value);
	}
	
	// Set auth header
	if (Request.bRequiresAuth && !AuthToken.IsEmpty())
	{
		HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}
	
	// Set body
	if (!Request.Body.IsEmpty())
	{
		HttpRequest->SetContentAsString(Request.Body);
	}
	
	// Set timeout
	float Timeout = Request.Timeout > 0.0f ? Request.Timeout : DefaultTimeout;
	HttpRequest->SetTimeout(Timeout);
	
	return HttpRequest;
}

void UDeskillzHttpClient::HandleHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, 
	bool bSuccess, FString RequestId)
{
	// Build response struct
	FDeskillzHttpResponse DeskillzResponse;
	DeskillzResponse.RequestId = RequestId;
	DeskillzResponse.bSuccess = bSuccess && Response.IsValid();
	
	if (Response.IsValid())
	{
		DeskillzResponse.StatusCode = Response->GetResponseCode();
		DeskillzResponse.Body = Response->GetContentAsString();
		
		// Get headers
		for (const FString& Header : Response->GetAllHeaders())
		{
			FString Key, Value;
			if (Header.Split(TEXT(": "), &Key, &Value))
			{
				DeskillzResponse.Headers.Add(Key, Value);
			}
		}
	}
	else
	{
		DeskillzResponse.ErrorMessage = TEXT("No response received");
	}
	
	// Log response
	UE_LOG(LogDeskillz, Verbose, TEXT("HTTP Response [%d]: %s"), 
		DeskillzResponse.StatusCode, *RequestId);
	
	// Update online status
	UpdateOnlineStatus(bSuccess);
	
	// Get callback and clean up
	FOnDeskillzHttpResponse Callback;
	if (RequestCallbacks.RemoveAndCopyValue(RequestId, Callback))
	{
		ActiveRequests.Remove(RequestId);
		
		// Execute callback
		Callback.ExecuteIfBound(DeskillzResponse);
	}
}

bool UDeskillzHttpClient::GetCachedResponse(const FString& CacheKey, FDeskillzHttpResponse& OutResponse)
{
	if (TPair<FDeskillzHttpResponse, double>* Cached = ResponseCache.Find(CacheKey))
	{
		double Now = FPlatformTime::Seconds();
		if (Now < Cached->Value)
		{
			OutResponse = Cached->Key;
			OutResponse.bFromCache = true;
			return true;
		}
		else
		{
			// Expired
			ResponseCache.Remove(CacheKey);
		}
	}
	
	return false;
}

void UDeskillzHttpClient::CacheResponse(const FString& CacheKey, const FDeskillzHttpResponse& Response, float TTL)
{
	double ExpireTime = FPlatformTime::Seconds() + TTL;
	ResponseCache.Add(CacheKey, TPair<FDeskillzHttpResponse, double>(Response, ExpireTime));
}

FString UDeskillzHttpClient::GenerateCacheKey(const FDeskillzHttpRequest& Request) const
{
	FString Key = GetMethodString(Request.Method) + TEXT(":") + Request.Endpoint;
	
	// Add query params to key
	for (const auto& Pair : Request.QueryParams)
	{
		Key += FString::Printf(TEXT(":%s=%s"), *Pair.Key, *Pair.Value);
	}
	
	return Key;
}

void UDeskillzHttpClient::ScheduleRetry(const FDeskillzHttpRequest& Request, 
	const FOnDeskillzHttpResponse& OnComplete, int32 AttemptNumber)
{
	if (AttemptNumber >= Request.MaxRetries)
	{
		FDeskillzHttpResponse FailResponse;
		FailResponse.bSuccess = false;
		FailResponse.ErrorMessage = TEXT("Max retries exceeded");
		OnComplete.ExecuteIfBound(FailResponse);
		return;
	}
	
	float Delay = CalculateRetryDelay(AttemptNumber);
	
	UE_LOG(LogDeskillz, Log, TEXT("Scheduling retry %d/%d in %.1fs: %s"), 
		AttemptNumber + 1, Request.MaxRetries, Delay, *Request.Endpoint);
	
	// Schedule retry using timer
	// In full implementation, use FTimerManager
}

float UDeskillzHttpClient::CalculateRetryDelay(int32 AttemptNumber) const
{
	// Exponential backoff: 1s, 2s, 4s, 8s...
	float BaseDelay = 1.0f;
	float MaxDelay = 30.0f;
	float Delay = BaseDelay * FMath::Pow(2.0f, static_cast<float>(AttemptNumber));
	
	// Add jitter
	Delay += FMath::FRandRange(0.0f, 1.0f);
	
	return FMath::Min(Delay, MaxDelay);
}

void UDeskillzHttpClient::UpdateOnlineStatus(bool bOnline)
{
	if (bIsOnline != bOnline)
	{
		bIsOnline = bOnline;
		UE_LOG(LogDeskillz, Log, TEXT("Online status changed: %s"), bOnline ? TEXT("Online") : TEXT("Offline"));
	}
}
