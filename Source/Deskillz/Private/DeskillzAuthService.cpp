// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#include "DeskillzAuthService.h"
#include "DeskillzConfig.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// Static instance
UDeskillzAuthService* UDeskillzAuthService::Instance = nullptr;

// API Endpoints
static const FString REGISTER_ENDPOINT = TEXT("/api/v1/auth/register");
static const FString LOGIN_ENDPOINT = TEXT("/api/v1/auth/login");
static const FString SOCIAL_AUTH_ENDPOINT = TEXT("/api/v1/auth/social");
static const FString REFRESH_ENDPOINT = TEXT("/api/v1/auth/refresh");
static const FString ME_ENDPOINT = TEXT("/api/v1/auth/me");
static const FString FORGOT_PASSWORD_ENDPOINT = TEXT("/api/v1/auth/forgot-password");
static const FString RESET_PASSWORD_ENDPOINT = TEXT("/api/v1/auth/reset-password");
static const FString WALLET_LINK_ENDPOINT = TEXT("/api/v1/auth/wallet/link");
static const FString WALLET_DISCONNECT_ENDPOINT = TEXT("/api/v1/auth/wallet/disconnect");
static const FString WALLET_NONCE_ENDPOINT = TEXT("/api/v1/auth/nonce");

UDeskillzAuthService::UDeskillzAuthService()
{
}

UDeskillzAuthService* UDeskillzAuthService::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzAuthService>();
        Instance->AddToRoot(); // Prevent garbage collection
    }
    return Instance;
}

FString UDeskillzAuthService::GetBaseUrl() const
{
    UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (Config)
    {
        return Config->GetApiBaseUrl();
    }
    return TEXT("https://api.deskillz.games");
}

// ============================================================================
// Public API Methods
// ============================================================================

void UDeskillzAuthService::SignUp(const FDeskillzSignUpRequest& Request, FOnAuthResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Registering user: %s"), *Request.Email);
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), REGISTER_ENDPOINT);
    
    // Build JSON body
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("email"), Request.Email);
    JsonBody->SetStringField(TEXT("password"), Request.Password);
    JsonBody->SetStringField(TEXT("username"), Request.Username);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
    
    HttpRequest->SetContentAsString(RequestBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleAuthResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::Login(const FDeskillzLoginRequest& Request, FOnAuthResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Logging in user: %s"), *Request.Email);
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), LOGIN_ENDPOINT);
    
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("email"), Request.Email);
    JsonBody->SetStringField(TEXT("password"), Request.Password);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
    
    HttpRequest->SetContentAsString(RequestBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleAuthResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::SocialLogin(const FDeskillzSocialAuthRequest& Request, FOnAuthResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Social login via: %s"), *Request.GetProviderName());
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), SOCIAL_AUTH_ENDPOINT);
    
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("provider"), Request.GetProviderName());
    JsonBody->SetStringField(TEXT("idToken"), Request.IdToken);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
    
    HttpRequest->SetContentAsString(RequestBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleAuthResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::RefreshToken(const FString& RefreshToken, FOnAuthResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Refreshing access token"));
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), REFRESH_ENDPOINT);
    
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("refreshToken"), RefreshToken);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
    
    HttpRequest->SetContentAsString(RequestBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleAuthResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::GetMe(const FString& AccessToken, FOnUserResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Fetching current user"));
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("GET"), ME_ENDPOINT);
    AddAuthHeader(HttpRequest, AccessToken);
    
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleUserResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::ForgotPassword(const FString& Email, FOnSimpleResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Requesting password reset for: %s"), *Email);
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), FORGOT_PASSWORD_ENDPOINT);
    
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("email"), Email);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
    
    HttpRequest->SetContentAsString(RequestBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleSimpleResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::ResetPassword(const FString& Token, const FString& NewPassword, FOnSimpleResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Resetting password"));
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), RESET_PASSWORD_ENDPOINT);
    
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("token"), Token);
    JsonBody->SetStringField(TEXT("password"), NewPassword);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
    
    HttpRequest->SetContentAsString(RequestBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleSimpleResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::LinkWallet(const FDeskillzWalletLinkRequest& Request, const FString& AccessToken, FOnUserResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Linking wallet: %s"), *Request.WalletAddress);
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), WALLET_LINK_ENDPOINT);
    AddAuthHeader(HttpRequest, AccessToken);
    
    TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
    JsonBody->SetStringField(TEXT("walletAddress"), Request.WalletAddress);
    JsonBody->SetStringField(TEXT("signature"), Request.Signature);
    JsonBody->SetStringField(TEXT("message"), Request.Message);
    JsonBody->SetStringField(TEXT("nonce"), Request.Nonce);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
    
    HttpRequest->SetContentAsString(RequestBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleUserResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::DisconnectWallet(const FString& AccessToken, FOnUserResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Disconnecting wallet"));
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("POST"), WALLET_DISCONNECT_ENDPOINT);
    AddAuthHeader(HttpRequest, AccessToken);
    HttpRequest->SetContentAsString(TEXT("{}"));
    
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleUserResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

void UDeskillzAuthService::GetWalletNonce(const FString& WalletAddress, FOnNonceResponse OnComplete)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Getting nonce for wallet: %s"), *WalletAddress);
    
    FString Endpoint = FString::Printf(TEXT("%s?walletAddress=%s"), *WALLET_NONCE_ENDPOINT, *FGenericPlatformHttp::UrlEncode(WalletAddress));
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = CreateRequest(TEXT("GET"), Endpoint);
    
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDeskillzAuthService::HandleNonceResponse, OnComplete);
    HttpRequest->ProcessRequest();
}

// ============================================================================
// Internal Helpers
// ============================================================================

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UDeskillzAuthService::CreateRequest(const FString& Verb, const FString& Endpoint)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    
    Request->SetVerb(Verb);
    Request->SetURL(GetBaseUrl() + Endpoint);
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
    
    // Add game ID header
    UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (Config)
    {
        Request->SetHeader(TEXT("X-Game-Id"), Config->GameId);
    }
    
    return Request;
}

void UDeskillzAuthService::AddAuthHeader(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request, const FString& AccessToken)
{
    if (!AccessToken.IsEmpty())
    {
        Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AccessToken));
    }
}

bool UDeskillzAuthService::ParseAuthResponse(const FString& ResponseContent, FDeskillzAuthResponse& OutResponse)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        return false;
    }
    
    OutResponse.AccessToken = JsonObject->GetStringField(TEXT("accessToken"));
    OutResponse.RefreshToken = JsonObject->GetStringField(TEXT("refreshToken"));
    OutResponse.bIsNewUser = JsonObject->GetBoolField(TEXT("isNewUser"));
    OutResponse.ExpiresIn = JsonObject->GetIntegerField(TEXT("expiresIn"));
    
    // Parse user object
    const TSharedPtr<FJsonObject>* UserObject;
    if (JsonObject->TryGetObjectField(TEXT("user"), UserObject))
    {
        ParseUser(*UserObject, OutResponse.User);
    }
    
    return true;
}

bool UDeskillzAuthService::ParseUser(const TSharedPtr<FJsonObject>& JsonObject, FDeskillzAuthUser& OutUser)
{
    if (!JsonObject.IsValid())
    {
        return false;
    }
    
    OutUser.Id = JsonObject->GetStringField(TEXT("id"));
    OutUser.Email = JsonObject->GetStringField(TEXT("email"));
    OutUser.Username = JsonObject->GetStringField(TEXT("username"));
    OutUser.DisplayName = JsonObject->GetStringField(TEXT("displayName"));
    OutUser.AvatarUrl = JsonObject->GetStringField(TEXT("avatarUrl"));
    OutUser.Role = ParseUserRole(JsonObject->GetStringField(TEXT("role")));
    OutUser.bEmailVerified = JsonObject->GetBoolField(TEXT("emailVerified"));
    OutUser.bHasWallet = JsonObject->GetBoolField(TEXT("hasWallet"));
    OutUser.WalletAddress = JsonObject->GetStringField(TEXT("walletAddress"));
    OutUser.bTwoFactorEnabled = JsonObject->GetBoolField(TEXT("twoFactorEnabled"));
    
    // Parse dates
    FString CreatedAtStr, LastLoginAtStr;
    if (JsonObject->TryGetStringField(TEXT("createdAt"), CreatedAtStr))
    {
        FDateTime::ParseIso8601(*CreatedAtStr, OutUser.CreatedAt);
    }
    if (JsonObject->TryGetStringField(TEXT("lastLoginAt"), LastLoginAtStr))
    {
        FDateTime::ParseIso8601(*LastLoginAtStr, OutUser.LastLoginAt);
    }
    
    return true;
}

EDeskillzUserRole UDeskillzAuthService::ParseUserRole(const FString& RoleString)
{
    if (RoleString.Equals(TEXT("ADMIN"), ESearchCase::IgnoreCase))
    {
        return EDeskillzUserRole::Admin;
    }
    else if (RoleString.Equals(TEXT("DEVELOPER"), ESearchCase::IgnoreCase))
    {
        return EDeskillzUserRole::Developer;
    }
    return EDeskillzUserRole::Player;
}

// ============================================================================
// Response Handlers
// ============================================================================

void UDeskillzAuthService::HandleAuthResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnAuthResponse OnComplete)
{
    FDeskillzAuthResponse AuthResponse;
    
    if (!bSuccess || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[DeskillzAuthService] Request failed - no response"));
        OnComplete.ExecuteIfBound(false, AuthResponse);
        return;
    }
    
    int32 ResponseCode = Response->GetResponseCode();
    FString ResponseContent = Response->GetContentAsString();
    
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuthService] Response (%d): %s"), ResponseCode, *ResponseContent);
    
    if (ResponseCode >= 200 && ResponseCode < 300)
    {
        if (ParseAuthResponse(ResponseContent, AuthResponse))
        {
            OnComplete.ExecuteIfBound(true, AuthResponse);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[DeskillzAuthService] Failed to parse response"));
            OnComplete.ExecuteIfBound(false, AuthResponse);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[DeskillzAuthService] Request failed (%d): %s"), ResponseCode, *ResponseContent);
        OnComplete.ExecuteIfBound(false, AuthResponse);
    }
}

void UDeskillzAuthService::HandleUserResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnUserResponse OnComplete)
{
    FDeskillzAuthUser User;
    
    if (!bSuccess || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[DeskillzAuthService] Request failed - no response"));
        OnComplete.ExecuteIfBound(false, User);
        return;
    }
    
    int32 ResponseCode = Response->GetResponseCode();
    FString ResponseContent = Response->GetContentAsString();
    
    if (ResponseCode >= 200 && ResponseCode < 300)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && ParseUser(JsonObject, User))
        {
            OnComplete.ExecuteIfBound(true, User);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[DeskillzAuthService] Failed to parse user response"));
            OnComplete.ExecuteIfBound(false, User);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[DeskillzAuthService] Request failed (%d): %s"), ResponseCode, *ResponseContent);
        OnComplete.ExecuteIfBound(false, User);
    }
}

void UDeskillzAuthService::HandleSimpleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnSimpleResponse OnComplete)
{
    if (!bSuccess || !Response.IsValid())
    {
        OnComplete.ExecuteIfBound(false, TEXT("Network error"));
        return;
    }
    
    int32 ResponseCode = Response->GetResponseCode();
    FString ResponseContent = Response->GetContentAsString();
    
    if (ResponseCode >= 200 && ResponseCode < 300)
    {
        OnComplete.ExecuteIfBound(true, TEXT(""));
    }
    else
    {
        // Try to extract error message
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
        
        FString ErrorMessage = ResponseContent;
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            JsonObject->TryGetStringField(TEXT("message"), ErrorMessage);
        }
        
        OnComplete.ExecuteIfBound(false, ErrorMessage);
    }
}

void UDeskillzAuthService::HandleNonceResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnNonceResponse OnComplete)
{
    if (!bSuccess || !Response.IsValid())
    {
        OnComplete.ExecuteIfBound(false, TEXT(""), TEXT(""));
        return;
    }
    
    int32 ResponseCode = Response->GetResponseCode();
    FString ResponseContent = Response->GetContentAsString();
    
    if (ResponseCode >= 200 && ResponseCode < 300)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FString Nonce = JsonObject->GetStringField(TEXT("nonce"));
            FString Message = JsonObject->GetStringField(TEXT("message"));
            OnComplete.ExecuteIfBound(true, Nonce, Message);
        }
        else
        {
            OnComplete.ExecuteIfBound(false, TEXT(""), TEXT(""));
        }
    }
    else
    {
        OnComplete.ExecuteIfBound(false, TEXT(""), TEXT(""));
    }
}