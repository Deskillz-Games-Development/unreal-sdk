// Copyright Deskillz Games. All Rights Reserved.

#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Misc/Guid.h"
#include "Misc/App.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
// Lobby deep link handler
#include "Lobby/DeskillzDeepLinkHandler.h"
// Self-Sufficient Architecture Auth
#include "Auth/DeskillzAuth.h"
#include "Auth/DeskillzAuthController.h"

#define SDK_VERSION TEXT("2.1.0")  // Updated for Self-Sufficient Architecture

// ============================================================================
// Constructor & Lifecycle
// ============================================================================

UDeskillzSDK::UDeskillzSDK()
{
    SDKState = EDeskillzSDKState::Uninitialized;
}

void UDeskillzSDK::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Subsystem Initializing..."));

    // Initialize auth system (Self-Sufficient Architecture)
    InitializeAuth();
    
    // Auto-initialize if config is valid
    const UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (Config && Config->bEnableSDK && Config->IsValid())
    {
        InitializeSDK();
    }
}

void UDeskillzSDK::Deinitialize()
{
    UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Subsystem Deinitializing..."));

    // Shutdown auth (Self-Sufficient Architecture)
    ShutdownAuth();
    
    Shutdown();
    
    Super::Deinitialize();
}

bool UDeskillzSDK::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create if SDK is enabled
    const UDeskillzConfig* Config = UDeskillzConfig::Get();
    return Config && Config->bEnableSDK;
}

// ============================================================================
// Static Accessors
// ============================================================================

UDeskillzSDK* UDeskillzSDK::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }
    
    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }
    
    return GameInstance->GetSubsystem<UDeskillzSDK>();
}

bool UDeskillzSDK::IsInitialized(const UObject* WorldContextObject)
{
    UDeskillzSDK* SDK = Get(WorldContextObject);
    return SDK && SDK->IsReady();
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzSDK::InitializeSDK()
{
    const UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (!Config)
    {
        UE_LOG(LogDeskillz, Error, TEXT("Deskillz Config not found!"));
        SDKState = EDeskillzSDKState::Error;
        BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("Configuration not found")));
        return;
    }
    
    InitializeWithCredentials(Config->APIKey, Config->GameId, Config->Environment);
}

void UDeskillzSDK::InitializeWithCredentials(const FString& InAPIKey, const FString& InGameId, EDeskillzEnvironment InEnvironment)
{
    if (SDKState == EDeskillzSDKState::Initializing)
    {
        UE_LOG(LogDeskillz, Warning, TEXT("SDK is already initializing"));
        return;
    }
    
    if (SDKState == EDeskillzSDKState::Initialized)
    {
        UE_LOG(LogDeskillz, Warning, TEXT("SDK is already initialized"));
        return;
    }
    
    UE_LOG(LogDeskillz, Log, TEXT("Initializing Deskillz SDK v%s (Self-Sufficient Architecture)"), SDK_VERSION);
    UE_LOG(LogDeskillz, Log, TEXT("Environment: %s"), 
        InEnvironment == EDeskillzEnvironment::Production ? TEXT("Production") :
        InEnvironment == EDeskillzEnvironment::Sandbox ? TEXT("Sandbox") : TEXT("Development"));
    
    SDKState = EDeskillzSDKState::Initializing;
    
    // Store credentials
    APIKey = InAPIKey;
    GameId = InGameId;
    ActiveEnvironment = InEnvironment;
    
    // Get endpoints
    const UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (Config && Config->bUseCustomEndpoints)
    {
        ActiveEndpoints = Config->CustomEndpoints;
    }
    else
    {
        ActiveEndpoints = FDeskillzEndpoints::ForEnvironment(InEnvironment);
    }
    
    // Initialize Lobby Deep Link Handler
    UDeskillzDeepLinkHandler* DeepLinkHandler = UDeskillzDeepLinkHandler::Get();
    if (DeepLinkHandler)
    {
        DeepLinkHandler->Initialize();
        UE_LOG(LogDeskillz, Log, TEXT("Lobby Deep Link Handler initialized"));
    }
    
    // Validate credentials with server
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("gameId"), GameId);
    RequestBody->SetStringField(TEXT("deviceId"), GetDeviceId());
    RequestBody->SetStringField(TEXT("platform"), FPlatformMisc::GetUBTPlatform());
    RequestBody->SetStringField(TEXT("sdkVersion"), SDK_VERSION);

    MakeAPIRequest(TEXT("/sdk/initialize"), TEXT("POST"), RequestBody, 
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                UE_LOG(LogDeskillz, Error, TEXT("SDK Initialization failed: %s"), *Error.Message);
                SDKState = EDeskillzSDKState::Error;
                OnInitialized.Broadcast(false, Error);
                return;
            }
            
            // Successfully initialized
            SDKState = EDeskillzSDKState::Initialized;
            
            UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Initialized Successfully"));
            
            // Connect WebSocket for real-time features
            const UDeskillzConfig* Config = UDeskillzConfig::Get();
            if (Config && Config->bEnableWebSocket)
            {
                ConnectWebSocket();
            }
            
            // Process pending deep link after initialization
            UDeskillzDeepLinkHandler* DeepLinkHandler = UDeskillzDeepLinkHandler::Get();
            if (DeepLinkHandler && DeepLinkHandler->HasPendingLaunch())
            {
                DeepLinkHandler->ProcessPendingLaunch();
            }

            OnInitialized.Broadcast(true, FDeskillzError::None());
        });
}

void UDeskillzSDK::Shutdown()
{
    UE_LOG(LogDeskillz, Log, TEXT("Shutting down Deskillz SDK..."));
    
    // Disconnect WebSocket
    DisconnectWebSocket();
    
    // Clear match state
    if (IsInMatch())
    {
        AbortMatch(TEXT("SDK Shutdown"));
    }
    
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MatchTimerHandle);
        World->GetTimerManager().ClearTimer(WebSocketReconnectHandle);
    }
    
    // Shutdown deep link handler
    UDeskillzDeepLinkHandler* DeepLinkHandler = UDeskillzDeepLinkHandler::Get();
    if (DeepLinkHandler)
    {
        DeepLinkHandler->Shutdown();
    }

    // Reset state
    SDKState = EDeskillzSDKState::Uninitialized;
    bIsAuthenticated = false;
    bIsMatchmaking = false;
    bIsInPractice = false;
    CurrentMatch = FDeskillzMatchInfo();
    CurrentPlayer = FDeskillzPlayer();
    CurrentScore = 0;
    AuthToken.Empty();
    WalletBalances.Empty();
    
    UE_LOG(LogDeskillz, Log, TEXT("Deskillz SDK Shutdown Complete"));
}

// ============================================================================
// Auth System (Self-Sufficient Architecture)
// ============================================================================

void UDeskillzSDK::InitializeAuth()
{
    if (bAuthInitialized)
    {
        return;
    }

    UE_LOG(LogDeskillz, Log, TEXT("Initializing auth system (Self-Sufficient Architecture)..."));

    // Get auth singleton
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->Initialize();

        // Bind to auth events
        Auth->OnLoginSuccess.AddDynamic(this, &UDeskillzSDK::HandleAuthLoginSuccess);
        Auth->OnLogout.AddDynamic(this, &UDeskillzSDK::HandleAuthLogout);
        Auth->OnAuthError.AddDynamic(this, &UDeskillzSDK::HandleAuthError);
        Auth->OnWalletConnected.AddDynamic(this, &UDeskillzSDK::HandleWalletConnected);
        Auth->OnWalletDisconnected.AddDynamic(this, &UDeskillzSDK::HandleWalletDisconnected);
    }

    // Get or create auth controller
    AuthController = UDeskillzAuthController::Get(this);
    if (AuthController)
    {
        AuthController->Initialize();
    }

    bAuthInitialized = true;
    UE_LOG(LogDeskillz, Log, TEXT("Auth system initialized"));
}

void UDeskillzSDK::ShutdownAuth()
{
    if (!bAuthInitialized)
    {
        return;
    }

    UE_LOG(LogDeskillz, Log, TEXT("Shutting down auth system..."));

    // Unbind from auth events
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->OnLoginSuccess.RemoveDynamic(this, &UDeskillzSDK::HandleAuthLoginSuccess);
        Auth->OnLogout.RemoveDynamic(this, &UDeskillzSDK::HandleAuthLogout);
        Auth->OnAuthError.RemoveDynamic(this, &UDeskillzSDK::HandleAuthError);
        Auth->OnWalletConnected.RemoveDynamic(this, &UDeskillzSDK::HandleWalletConnected);
        Auth->OnWalletDisconnected.RemoveDynamic(this, &UDeskillzSDK::HandleWalletDisconnected);
    }

    // Shutdown auth controller
    if (AuthController)
    {
        AuthController->Shutdown();
        AuthController = nullptr;
    }

    bAuthInitialized = false;
    UE_LOG(LogDeskillz, Log, TEXT("Auth system shutdown complete"));
}

void UDeskillzSDK::LoginWithEmail(const FString& Email, const FString& Password, bool bRememberMe)
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->Login(Email, Password, bRememberMe);
    }
}

void UDeskillzSDK::SignUpWithEmail(const FString& Email, const FString& Password, const FString& Username)
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->SignUp(Email, Password, Username);
    }
}

void UDeskillzSDK::SocialLogin(const FString& Provider, const FString& IdToken)
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->SocialLogin(Provider, IdToken);
    }
}

void UDeskillzSDK::LinkWallet(const FString& WalletAddress, const FString& Signature,
                              const FString& Message, const FString& Nonce)
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->LinkWallet(WalletAddress, Signature, Message, Nonce);
    }
}

void UDeskillzSDK::DisconnectWallet()
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->DisconnectWallet();
    }
}

void UDeskillzSDK::ForgotPassword(const FString& Email)
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->ForgotPassword(Email);
    }
}

bool UDeskillzSDK::RequireWallet(const FString& Reason)
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        return Auth->RequireWallet(Reason);
    }
    return true; // Require wallet if auth not available
}

FDeskillzAuthUser UDeskillzSDK::GetCurrentAuthUser() const
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        return Auth->GetCurrentUser();
    }
    return FDeskillzAuthUser();
}

void UDeskillzSDK::HandleAuthLoginSuccess(const FDeskillzAuthUser& User)
{
    UE_LOG(LogDeskillz, Log, TEXT("Auth login success: %s"), *User.Username);

    // Update current player from auth user
    CurrentPlayer.PlayerId = User.Id;
    CurrentPlayer.Username = User.Username;
    CurrentPlayer.AvatarUrl = User.AvatarUrl;

    bIsAuthenticated = true;
}

void UDeskillzSDK::HandleAuthLogout()
{
    UE_LOG(LogDeskillz, Log, TEXT("Auth logout"));

    CurrentPlayer = FDeskillzPlayer();
    bIsAuthenticated = false;
}

void UDeskillzSDK::HandleAuthError(const FString& Error)
{
    UE_LOG(LogDeskillz, Error, TEXT("Auth error: %s"), *Error);

    FDeskillzError DeskillzError;
    DeskillzError.Code = EDeskillzErrorCode::AuthenticationFailed;
    DeskillzError.Message = Error;

    OnError.Broadcast(DeskillzError);
}

void UDeskillzSDK::HandleWalletConnected(const FString& WalletAddress)
{
    UE_LOG(LogDeskillz, Log, TEXT("Wallet connected: %s"), *WalletAddress);
    // Update player wallet info if needed
}

void UDeskillzSDK::HandleWalletDisconnected()
{
    UE_LOG(LogDeskillz, Log, TEXT("Wallet disconnected"));
    // Clear player wallet info if needed
}

// ============================================================================
// Authentication (Legacy)
// ============================================================================

void UDeskillzSDK::AuthenticateWithWallet(const FString& WalletAddress, const FString& Signature)
{
    if (!IsReady())
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("SDK not initialized")));
        return;
    }
    
    UE_LOG(LogDeskillz, Log, TEXT("Authenticating with wallet: %s"), *WalletAddress.Left(10));
    
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("walletAddress"), WalletAddress);
    RequestBody->SetStringField(TEXT("signature"), Signature);
    RequestBody->SetStringField(TEXT("gameId"), GameId);
    
    MakeAPIRequest(TEXT("/api/v1/auth/wallet"), TEXT("POST"), RequestBody,
        [this, WalletAddress](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                UE_LOG(LogDeskillz, Error, TEXT("Wallet authentication failed: %s"), *Error.Message);
                BroadcastError(Error);
                return;
            }
            
            // Extract auth token and player info
            AuthToken = Response->GetStringField(TEXT("token"));
            
            if (TSharedPtr<FJsonObject> UserObj = Response->GetObjectField(TEXT("user")))
            {
                CurrentPlayer.PlayerId = UserObj->GetStringField(TEXT("id"));
                CurrentPlayer.Username = UserObj->GetStringField(TEXT("username"));
                CurrentPlayer.AvatarUrl = UserObj->GetStringField(TEXT("avatarUrl"));
                CurrentPlayer.Rating = UserObj->GetIntegerField(TEXT("rating"));
                CurrentPlayer.WalletAddress = WalletAddress;
            }
            
            bIsAuthenticated = true;
            
            UE_LOG(LogDeskillz, Log, TEXT("Wallet authentication successful: %s"), *CurrentPlayer.Username);
        });
}

void UDeskillzSDK::AuthenticateWithCredentials(const FString& Username, const FString& Password)
{
    if (!IsReady())
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("SDK not initialized")));
        return;
    }
    
    UE_LOG(LogDeskillz, Log, TEXT("Authenticating with credentials: %s"), *Username);
    
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("email"), Username);
    RequestBody->SetStringField(TEXT("password"), Password);
    RequestBody->SetStringField(TEXT("gameId"), GameId);
    
    MakeAPIRequest(TEXT("/api/v1/auth/login"), TEXT("POST"), RequestBody,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                UE_LOG(LogDeskillz, Error, TEXT("Credential authentication failed: %s"), *Error.Message);
                BroadcastError(Error);
                return;
            }
            
            // Extract auth token and player info
            AuthToken = Response->GetStringField(TEXT("accessToken"));
            
            if (TSharedPtr<FJsonObject> UserObj = Response->GetObjectField(TEXT("user")))
            {
                CurrentPlayer.PlayerId = UserObj->GetStringField(TEXT("id"));
                CurrentPlayer.Username = UserObj->GetStringField(TEXT("username"));
                CurrentPlayer.AvatarUrl = UserObj->GetStringField(TEXT("avatarUrl"));
                CurrentPlayer.Rating = UserObj->GetIntegerField(TEXT("rating"));
            }
            
            bIsAuthenticated = true;
            
            UE_LOG(LogDeskillz, Log, TEXT("Credential authentication successful: %s"), *CurrentPlayer.Username);
        });
}

void UDeskillzSDK::Logout()
{
    UE_LOG(LogDeskillz, Log, TEXT("Logging out..."));
    
    // Also logout from Self-Sufficient Auth
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->Logout();
    }
    
    // Clear local state
    AuthToken.Empty();
    CurrentPlayer = FDeskillzPlayer();
    bIsAuthenticated = false;
    
    UE_LOG(LogDeskillz, Log, TEXT("Logout complete"));
}

// ============================================================================
// Tournaments
// ============================================================================

void UDeskillzSDK::GetTournaments()
{
    if (!IsReady())
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("SDK not initialized")));
        return;
    }
    
    MakeAPIRequest(TEXT("/api/v1/tournaments"), TEXT("GET"), nullptr,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                OnTournamentsReceived.Broadcast(TArray<FDeskillzTournament>(), Error);
                return;
            }
            
            TArray<FDeskillzTournament> Tournaments;
            // Parse tournaments from response
            const TArray<TSharedPtr<FJsonValue>>* TournamentsArray;
            if (Response->TryGetArrayField(TEXT("tournaments"), TournamentsArray))
            {
                for (const TSharedPtr<FJsonValue>& Value : *TournamentsArray)
                {
                    TSharedPtr<FJsonObject> TournamentObj = Value->AsObject();
                    if (TournamentObj.IsValid())
                    {
                        FDeskillzTournament Tournament;
                        Tournament.TournamentId = TournamentObj->GetStringField(TEXT("id"));
                        Tournament.Name = TournamentObj->GetStringField(TEXT("name"));
                        Tournament.Description = TournamentObj->GetStringField(TEXT("description"));
                        // Parse other fields...
                        Tournaments.Add(Tournament);
                    }
                }
            }
            
            OnTournamentsReceived.Broadcast(Tournaments, FDeskillzError::None());
        });
}

void UDeskillzSDK::GetTournamentsFiltered(EDeskillzTournamentStatus Status, EDeskillzMatchType MatchType, float MinEntryFee, float MaxEntryFee)
{
    // Implementation with filters
    GetTournaments(); // Simplified for now
}

void UDeskillzSDK::GetTournamentDetails(const FString& TournamentId)
{
    if (!IsReady())
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("SDK not initialized")));
        return;
    }
    
    FString Endpoint = FString::Printf(TEXT("/api/v1/tournaments/%s"), *TournamentId);
    MakeAPIRequest(Endpoint, TEXT("GET"), nullptr,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            // Handle response
        });
}

void UDeskillzSDK::JoinTournament(const FString& TournamentId, EDeskillzCurrency Currency)
{
    if (!IsReady() || !bIsAuthenticated)
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::AuthenticationFailed, TEXT("Must be authenticated to join tournament")));
        return;
    }
    
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("tournamentId"), TournamentId);
    RequestBody->SetStringField(TEXT("currency"), UEnum::GetValueAsString(Currency));
    
    MakeAPIRequest(TEXT("/api/v1/tournaments/join"), TEXT("POST"), RequestBody,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            // Handle response
        });
}

void UDeskillzSDK::LeaveTournament(const FString& TournamentId)
{
    if (!IsReady() || !bIsAuthenticated)
    {
        return;
    }
    
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("tournamentId"), TournamentId);
    
    MakeAPIRequest(TEXT("/api/v1/tournaments/leave"), TEXT("POST"), RequestBody,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            // Handle response
        });
}

// ============================================================================
// Matchmaking
// ============================================================================

void UDeskillzSDK::StartMatchmaking(const FString& TournamentId)
{
    if (!IsReady() || !bIsAuthenticated)
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::AuthenticationFailed, TEXT("Must be authenticated for matchmaking")));
        return;
    }
    
    if (bIsMatchmaking)
    {
        return;
    }
    
    bIsMatchmaking = true;
    
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("tournamentId"), TournamentId);
    
    MakeAPIRequest(TEXT("/api/v1/matchmaking/start"), TEXT("POST"), RequestBody,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                bIsMatchmaking = false;
                BroadcastError(Error);
            }
        });
}

void UDeskillzSDK::CancelMatchmaking()
{
    if (!bIsMatchmaking)
    {
        return;
    }
    
    MakeAPIRequest(TEXT("/api/v1/matchmaking/cancel"), TEXT("POST"), nullptr,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            bIsMatchmaking = false;
        });
}

// ============================================================================
// Match Lifecycle
// ============================================================================

void UDeskillzSDK::StartMatch()
{
    if (!CurrentMatch.IsValid())
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::MatchNotFound, TEXT("No active match")));
        return;
    }
    
    CurrentMatch.Status = EDeskillzMatchStatus::InProgress;
    MatchStartTime = FDateTime::UtcNow();
    CurrentScore = 0;
    
    UE_LOG(LogDeskillz, Log, TEXT("Match started: %s"), *CurrentMatch.MatchId);
}

void UDeskillzSDK::UpdateScore(int64 Score)
{
    if (!IsInMatch())
    {
        return;
    }
    
    CurrentScore = Score;
    
    // Send to server for real-time sync matches
    if (CurrentMatch.MatchType == EDeskillzMatchType::Synchronous && WebSocket.IsValid() && WebSocket->IsConnected())
    {
        TSharedPtr<FJsonObject> ScoreUpdate = MakeShareable(new FJsonObject());
        ScoreUpdate->SetStringField(TEXT("type"), TEXT("scoreUpdate"));
        ScoreUpdate->SetStringField(TEXT("matchId"), CurrentMatch.MatchId);
        ScoreUpdate->SetNumberField(TEXT("score"), Score);
        
        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(ScoreUpdate.ToSharedRef(), Writer);
        
        WebSocket->Send(JsonString);
    }
}

void UDeskillzSDK::SubmitScore(int64 FinalScore, bool bForceSubmit)
{
    if (!IsInMatch() && !bForceSubmit)
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::MatchNotFound, TEXT("No active match")));
        return;
    }
    
    CurrentScore = FinalScore;
    
    UE_LOG(LogDeskillz, Log, TEXT("Submitting score: %lld for match: %s"), FinalScore, *CurrentMatch.MatchId);
    
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("matchId"), CurrentMatch.MatchId);
    RequestBody->SetNumberField(TEXT("score"), FinalScore);
    RequestBody->SetNumberField(TEXT("duration"), GetElapsedTime());
    
    MakeAPIRequest(TEXT("/api/v1/matches/submit-score"), TEXT("POST"), RequestBody,
        [this, FinalScore](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                OnScoreSubmitted.Broadcast(false, Error);
                return;
            }
            
            OnScoreSubmitted.Broadcast(true, FDeskillzError::None());
            
            // Match will be completed via WebSocket or polling
        });
}

void UDeskillzSDK::AbortMatch(const FString& Reason)
{
    if (!IsInMatch())
    {
        return;
    }
    
    UE_LOG(LogDeskillz, Log, TEXT("Aborting match: %s, Reason: %s"), *CurrentMatch.MatchId, *Reason);
    
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
    RequestBody->SetStringField(TEXT("matchId"), CurrentMatch.MatchId);
    RequestBody->SetStringField(TEXT("reason"), Reason);
    
    MakeAPIRequest(TEXT("/api/v1/matches/abort"), TEXT("POST"), RequestBody,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            CurrentMatch = FDeskillzMatchInfo();
            CurrentScore = 0;
        });
}

float UDeskillzSDK::GetRemainingTime() const
{
    if (!IsInMatch())
    {
        return 0.0f;
    }
    
    float Elapsed = GetElapsedTime();
    return FMath::Max(0.0f, (float)CurrentMatch.DurationSeconds - Elapsed);
}

float UDeskillzSDK::GetElapsedTime() const
{
    if (!IsInMatch())
    {
        return 0.0f;
    }
    
    return (FDateTime::UtcNow() - MatchStartTime).GetTotalSeconds();
}

// ============================================================================
// Practice Mode
// ============================================================================

void UDeskillzSDK::StartPractice(int32 DurationSeconds)
{
    if (IsInMatch())
    {
        BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("Cannot start practice while in match")));
        return;
    }
    
    bIsInPractice = true;
    CurrentScore = 0;
    MatchStartTime = FDateTime::UtcNow();
    
    // Set up practice match
    CurrentMatch = FDeskillzMatchInfo();
    CurrentMatch.MatchId = TEXT("practice_") + FGuid::NewGuid().ToString();
    CurrentMatch.MatchType = EDeskillzMatchType::Practice;
    CurrentMatch.DurationSeconds = DurationSeconds;
    CurrentMatch.Status = EDeskillzMatchStatus::InProgress;
    
    UE_LOG(LogDeskillz, Log, TEXT("Practice session started: %d seconds"), DurationSeconds);
}

void UDeskillzSDK::EndPractice()
{
    if (!bIsInPractice)
    {
        return;
    }
    
    bIsInPractice = false;
    CurrentMatch = FDeskillzMatchInfo();
    
    UE_LOG(LogDeskillz, Log, TEXT("Practice session ended. Final score: %lld"), CurrentScore);
}

// ============================================================================
// Wallet & Currency
// ============================================================================

void UDeskillzSDK::GetWalletBalances()
{
    if (!IsReady() || !bIsAuthenticated)
    {
        return;
    }
    
    MakeAPIRequest(TEXT("/api/v1/wallet/balances"), TEXT("GET"), nullptr,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                OnWalletUpdated.Broadcast(TMap<EDeskillzCurrency, FDeskillzWalletBalance>(), Error);
                return;
            }
            
            WalletBalances.Empty();
            // Parse balances from response
            
            OnWalletUpdated.Broadcast(WalletBalances, FDeskillzError::None());
        });
}

FDeskillzWalletBalance UDeskillzSDK::GetBalance(EDeskillzCurrency Currency) const
{
    if (const FDeskillzWalletBalance* Balance = WalletBalances.Find(Currency))
    {
        return *Balance;
    }
    return FDeskillzWalletBalance();
}

bool UDeskillzSDK::HasSufficientFunds(const FDeskillzEntryFee& EntryFee) const
{
    FDeskillzWalletBalance Balance = GetBalance(EntryFee.Currency);
    return Balance.Available >= EntryFee.Amount;
}

// ============================================================================
// Leaderboards
// ============================================================================

void UDeskillzSDK::GetLeaderboard(const FString& TournamentId, EDeskillzLeaderboardPeriod Period, int32 Offset, int32 Limit)
{
    if (!IsReady())
    {
        return;
    }
    
    FString Endpoint = FString::Printf(TEXT("/api/v1/leaderboards/%s?offset=%d&limit=%d"), *TournamentId, Offset, Limit);
    MakeAPIRequest(Endpoint, TEXT("GET"), nullptr,
        [this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
        {
            if (Error.IsError())
            {
                OnLeaderboardReceived.Broadcast(TArray<FDeskillzLeaderboardEntry>(), Error);
                return;
            }
            
            TArray<FDeskillzLeaderboardEntry> Entries;
            // Parse leaderboard entries
            
            OnLeaderboardReceived.Broadcast(Entries, FDeskillzError::None());
        });
}

void UDeskillzSDK::GetGlobalLeaderboard(EDeskillzLeaderboardPeriod Period, int32 Offset, int32 Limit)
{
    GetLeaderboard(TEXT("global"), Period, Offset, Limit);
}

// ============================================================================
// Network - HTTP
// ============================================================================

void UDeskillzSDK::MakeAPIRequest(const FString& Endpoint, const FString& Method, const TSharedPtr<FJsonObject>& Body, TFunction<void(TSharedPtr<FJsonObject>, FDeskillzError)> Callback)
{
    FHttpModule& HttpModule = FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();
    
    FString Url = ActiveEndpoints.BaseUrl + Endpoint;
    Request->SetURL(Url);
    Request->SetVerb(Method);
    
    // Set headers
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("X-API-Key"), APIKey);
    Request->SetHeader(TEXT("X-Game-Id"), GameId);
    
    if (!AuthToken.IsEmpty())
    {
        Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
    }
    
    const UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (Config && Config->bLogAPICalls)
    {
        UE_LOG(LogDeskillz, Log, TEXT("API Request: %s %s"), *Method, *Url);
    }
    
    // Set body for POST/PUT
    if (Body.IsValid() && (Method == TEXT("POST") || Method == TEXT("PUT") || Method == TEXT("PATCH")))
    {
        FString JsonString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);
        Request->SetContentAsString(JsonString);
    }
    
    if (Config)
    {
        Request->SetTimeout(Config->RequestTimeout);
    }
    
    Request->OnProcessRequestComplete().BindLambda(
        [this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
        {
            HandleHttpResponse(Request, Response, bSuccess, Callback);
        });
    
    Request->ProcessRequest();
}

void UDeskillzSDK::HandleHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, TFunction<void(TSharedPtr<FJsonObject>, FDeskillzError)> Callback)
{
    if (!Callback)
    {
        return;
    }
    
    if (!bSuccess || !Response.IsValid())
    {
        Callback(nullptr, FDeskillzError::NetworkError(TEXT("Request failed")));
        return;
    }
    
    int32 StatusCode = Response->GetResponseCode();
    FString Content = Response->GetContentAsString();
    
    const UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (Config && Config->bLogAPICalls)
    {
        UE_LOG(LogDeskillz, Log, TEXT("API Response (%d): %s"), StatusCode, *Content.Left(500));
    }
    
    // Parse JSON
    TSharedPtr<FJsonObject> JsonResponse = ParseJsonResponse(Content);
    
    // Check for HTTP errors
    if (StatusCode >= 400)
    {
        FDeskillzError Error;
        Error.HttpStatusCode = StatusCode;
        
        if (JsonResponse.IsValid() && JsonResponse->HasField(TEXT("message")))
        {
            Error.Message = JsonResponse->GetStringField(TEXT("message"));
        }
        else
        {
            Error.Message = FString::Printf(TEXT("HTTP Error %d"), StatusCode);
        }
        
        switch (StatusCode)
        {
            case 401: Error.Code = EDeskillzErrorCode::AuthenticationFailed; break;
            case 403: Error.Code = EDeskillzErrorCode::InvalidAPIKey; break;
            case 404: Error.Code = EDeskillzErrorCode::MatchNotFound; break;
            case 429: Error.Code = EDeskillzErrorCode::RateLimited; break;
            default: Error.Code = (StatusCode >= 500) ? EDeskillzErrorCode::ServerError : EDeskillzErrorCode::Unknown;
        }
        
        Callback(nullptr, Error);
        return;
    }
    
    Callback(JsonResponse, FDeskillzError::None());
}

TSharedPtr<FJsonObject> UDeskillzSDK::ParseJsonResponse(const FString& Content)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogDeskillz, Warning, TEXT("Failed to parse JSON response"));
        return nullptr;
    }
    
    return JsonObject;
}

// ============================================================================
// Network - WebSocket
// ============================================================================

void UDeskillzSDK::ConnectWebSocket()
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        return;
    }
    
    FString WebSocketUrl = ActiveEndpoints.WebSocketUrl + TEXT("?token=") + AuthToken + TEXT("&gameId=") + GameId;
    
    UE_LOG(LogDeskillz, Log, TEXT("Connecting WebSocket..."));
    
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(WebSocketUrl, TEXT("wss"));
    
    WebSocket->OnConnected().AddUObject(this, &UDeskillzSDK::OnWebSocketConnected);
    WebSocket->OnConnectionError().AddUObject(this, &UDeskillzSDK::OnWebSocketError);
    WebSocket->OnClosed().AddUObject(this, &UDeskillzSDK::OnWebSocketDisconnected);
    WebSocket->OnMessage().AddUObject(this, &UDeskillzSDK::OnWebSocketMessage);
    
    WebSocket->Connect();
}

void UDeskillzSDK::DisconnectWebSocket()
{
    if (WebSocket.IsValid())
    {
        WebSocket->Close();
        WebSocket.Reset();
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WebSocketReconnectHandle);
    }
}

void UDeskillzSDK::OnWebSocketConnected()
{
    UE_LOG(LogDeskillz, Log, TEXT("WebSocket Connected"));
    WebSocketReconnectAttempts = 0;
}

void UDeskillzSDK::OnWebSocketDisconnected(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogDeskillz, Log, TEXT("WebSocket Disconnected: %d - %s"), StatusCode, *Reason);
    
    // Attempt reconnection
    if (IsReady() && WebSocketReconnectAttempts < 5)
    {
        WebSocketReconnectAttempts++;
        
        if (UWorld* World = GetWorld())
        {
            float Delay = FMath::Min(30.0f, FMath::Pow(2.0f, (float)WebSocketReconnectAttempts));
            
            World->GetTimerManager().SetTimer(WebSocketReconnectHandle, this, &UDeskillzSDK::ConnectWebSocket, Delay, false);
        }
    }
}

void UDeskillzSDK::OnWebSocketError(const FString& Error)
{
    UE_LOG(LogDeskillz, Error, TEXT("WebSocket Error: %s"), *Error);
}

void UDeskillzSDK::OnWebSocketMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> JsonMessage = ParseJsonResponse(Message);
    if (!JsonMessage.IsValid())
    {
        return;
    }
    
    FString MessageType = JsonMessage->GetStringField(TEXT("type"));
    
    if (MessageType == TEXT("matchFound"))
    {
        // Match found through matchmaking
        bIsMatchmaking = false;
        
        CurrentMatch.MatchId = JsonMessage->GetStringField(TEXT("matchId"));
        CurrentMatch.TournamentId = JsonMessage->GetStringField(TEXT("tournamentId"));
        CurrentMatch.DurationSeconds = JsonMessage->GetIntegerField(TEXT("duration"));
        CurrentMatch.RandomSeed = (int64)JsonMessage->GetNumberField(TEXT("randomSeed"));
        CurrentMatch.Status = EDeskillzMatchStatus::Ready;
        
        // Parse opponent
        if (TSharedPtr<FJsonObject> OpponentObj = JsonMessage->GetObjectField(TEXT("opponent")))
        {
            CurrentMatch.Opponent.PlayerId = OpponentObj->GetStringField(TEXT("id"));
            CurrentMatch.Opponent.Username = OpponentObj->GetStringField(TEXT("username"));
            CurrentMatch.Opponent.AvatarUrl = OpponentObj->GetStringField(TEXT("avatarUrl"));
            CurrentMatch.Opponent.Rating = OpponentObj->GetIntegerField(TEXT("rating"));
        }
        
        CurrentMatch.LocalPlayer = CurrentPlayer;
        
        UE_LOG(LogDeskillz, Log, TEXT("Match found! Opponent: %s"), *CurrentMatch.Opponent.Username);
        
        OnMatchStarted.Broadcast(CurrentMatch, FDeskillzError::None());
    }
    else if (MessageType == TEXT("matchStart"))
    {
        // Match officially starting
        CurrentMatch.Status = EDeskillzMatchStatus::InProgress;
        CurrentMatch.StartTime = FDateTime::UtcNow();
    }
    else if (MessageType == TEXT("opponentScore"))
    {
        // Real-time opponent score update (synchronous matches)
        int64 OpponentScore = (int64)JsonMessage->GetNumberField(TEXT("score"));
        // Game can use this to update UI
    }
    else if (MessageType == TEXT("matchComplete"))
    {
        // Match completed
        FDeskillzMatchResult Result;
        Result.MatchId = CurrentMatch.MatchId;
        Result.PlayerScore = CurrentScore;
        Result.OpponentScore = (int64)JsonMessage->GetNumberField(TEXT("opponentScore"));
        Result.PrizeWon = JsonMessage->GetNumberField(TEXT("prizeWon"));
        Result.RatingChange = JsonMessage->GetIntegerField(TEXT("ratingChange"));
        Result.NewRating = JsonMessage->GetIntegerField(TEXT("newRating"));
        Result.Rank = JsonMessage->GetIntegerField(TEXT("rank"));
        
        FString ResultStr = JsonMessage->GetStringField(TEXT("result"));
        if (ResultStr == TEXT("win")) Result.Result = EDeskillzMatchResult::Win;
        else if (ResultStr == TEXT("loss")) Result.Result = EDeskillzMatchResult::Loss;
        else if (ResultStr == TEXT("draw")) Result.Result = EDeskillzMatchResult::Draw;
        
        CurrentPlayer.Rating = Result.NewRating;
        CurrentMatch = FDeskillzMatchInfo();
        
        OnMatchCompleted.Broadcast(Result, FDeskillzError::None());
    }
}

// ============================================================================
// Utility
// ============================================================================

FString UDeskillzSDK::GetSDKVersion()
{
    return SDK_VERSION;
}

FString UDeskillzSDK::GetDeviceId()
{
    // Generate a unique device ID (persisted)
    static FString CachedDeviceId;
    
    if (CachedDeviceId.IsEmpty())
    {
        TArray<uint8> MacAddress = FPlatformMisc::GetMacAddress();
        if (MacAddress.Num() > 0)
        {
            CachedDeviceId = FMD5::HashBytes(MacAddress.GetData(), MacAddress.Num());
        }
        else
        {
            CachedDeviceId = FPlatformMisc::GetDeviceId();
        }
        
        if (CachedDeviceId.IsEmpty())
        {
            CachedDeviceId = FGuid::NewGuid().ToString();
        }
    }
    
    return CachedDeviceId;
}

void UDeskillzSDK::BroadcastError(const FDeskillzError& Error)
{
    UE_LOG(LogDeskillz, Error, TEXT("SDK Error [%d]: %s"), (int32)Error.Code, *Error.Message);
    OnError.Broadcast(Error);
}