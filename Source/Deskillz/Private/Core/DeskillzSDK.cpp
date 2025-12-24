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
// NEW: Include lobby deep link handler
#include "Lobby/DeskillzDeepLinkHandler.h"

#define SDK_VERSION TEXT("2.0.0")  // Updated for centralized lobby

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
	
	UE_LOG(LogDeskillz, Log, TEXT("Initializing Deskillz SDK v%s (Centralized Lobby Mode)"), SDK_VERSION);
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
// Authentication
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
	
	MakeAPIRequest(TEXT("/auth/wallet"), TEXT("POST"), RequestBody,
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
			bIsAuthenticated = true;
			
			// Parse player data
			if (TSharedPtr<FJsonObject> PlayerData = Response->GetObjectField(TEXT("player")))
			{
				CurrentPlayer.PlayerId = PlayerData->GetStringField(TEXT("id"));
				CurrentPlayer.Username = PlayerData->GetStringField(TEXT("username"));
				CurrentPlayer.AvatarUrl = PlayerData->GetStringField(TEXT("avatarUrl"));
				CurrentPlayer.Rating = PlayerData->GetIntegerField(TEXT("rating"));
				CurrentPlayer.GamesPlayed = PlayerData->GetIntegerField(TEXT("gamesPlayed"));
				CurrentPlayer.Wins = PlayerData->GetIntegerField(TEXT("wins"));
				CurrentPlayer.bIsCurrentUser = true;
				
				if (CurrentPlayer.GamesPlayed > 0)
				{
					CurrentPlayer.WinRate = (float)CurrentPlayer.Wins / (float)CurrentPlayer.GamesPlayed;
				}
			}
			
			UE_LOG(LogDeskillz, Log, TEXT("Authentication successful: %s"), *CurrentPlayer.Username);
			
			// Fetch wallet balances
			GetWalletBalances();
		});
}

void UDeskillzSDK::AuthenticateWithCredentials(const FString& Username, const FString& Password)
{
	if (!IsReady())
	{
		BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("SDK not initialized")));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Authenticating user: %s"), *Username);
	
	TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
	RequestBody->SetStringField(TEXT("username"), Username);
	RequestBody->SetStringField(TEXT("password"), Password);
	RequestBody->SetStringField(TEXT("gameId"), GameId);
	
	MakeAPIRequest(TEXT("/auth/login"), TEXT("POST"), RequestBody,
		[this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
		{
			if (Error.IsError())
			{
				UE_LOG(LogDeskillz, Error, TEXT("Authentication failed: %s"), *Error.Message);
				BroadcastError(Error);
				return;
			}
			
			AuthToken = Response->GetStringField(TEXT("token"));
			bIsAuthenticated = true;
			
			if (TSharedPtr<FJsonObject> PlayerData = Response->GetObjectField(TEXT("player")))
			{
				CurrentPlayer.PlayerId = PlayerData->GetStringField(TEXT("id"));
				CurrentPlayer.Username = PlayerData->GetStringField(TEXT("username"));
				CurrentPlayer.AvatarUrl = PlayerData->GetStringField(TEXT("avatarUrl"));
				CurrentPlayer.Rating = PlayerData->GetIntegerField(TEXT("rating"));
				CurrentPlayer.bIsCurrentUser = true;
			}
			
			UE_LOG(LogDeskillz, Log, TEXT("Authentication successful"));
			GetWalletBalances();
		});
}

void UDeskillzSDK::Logout()
{
	if (!bIsAuthenticated)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Logging out user: %s"), *CurrentPlayer.Username);
	
	// Notify server
	MakeAPIRequest(TEXT("/auth/logout"), TEXT("POST"), nullptr, nullptr);
	
	// Clear local state
	bIsAuthenticated = false;
	AuthToken.Empty();
	CurrentPlayer = FDeskillzPlayer();
	WalletBalances.Empty();
	
	// End any active match
	if (IsInMatch())
	{
		AbortMatch(TEXT("User logged out"));
	}
}

// ============================================================================
// Tournaments - DEPRECATED (Now handled by main Deskillz app)
// ============================================================================

void UDeskillzSDK::GetTournaments()
{
	// DEPRECATED: Tournament browsing now handled by main Deskillz app
	UE_LOG(LogDeskillz, Warning, TEXT("GetTournaments() is DEPRECATED. Browse tournaments at deskillz.games"));
	
	TArray<FDeskillzTournament> EmptyTournaments;
	OnTournamentsReceived.Broadcast(EmptyTournaments, FDeskillzError::None());
}

void UDeskillzSDK::GetTournamentsFiltered(EDeskillzTournamentStatus Status, EDeskillzMatchType MatchType, float MinEntryFee, float MaxEntryFee)
{
	// DEPRECATED: Tournament browsing now handled by main Deskillz app
	UE_LOG(LogDeskillz, Warning, TEXT("GetTournamentsFiltered() is DEPRECATED. Browse tournaments at deskillz.games"));
	
	TArray<FDeskillzTournament> EmptyTournaments;
	OnTournamentsReceived.Broadcast(EmptyTournaments, FDeskillzError::None());
}

void UDeskillzSDK::GetTournamentDetails(const FString& TournamentId)
{
	// DEPRECATED: Tournament details now shown in main Deskillz app
	UE_LOG(LogDeskillz, Warning, TEXT("GetTournamentDetails() is DEPRECATED. View tournament details at deskillz.games"));
}

void UDeskillzSDK::JoinTournament(const FString& TournamentId, EDeskillzCurrency Currency)
{
	// DEPRECATED: Tournament joining now handled by main Deskillz app
	UE_LOG(LogDeskillz, Warning, TEXT("JoinTournament() is DEPRECATED. Join tournaments at deskillz.games"));
	
	BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, 
		TEXT("Tournament joining is now handled by the main Deskillz app. Visit deskillz.games")));
}

void UDeskillzSDK::LeaveTournament(const FString& TournamentId)
{
	// DEPRECATED: Tournament management now handled by main Deskillz app
	UE_LOG(LogDeskillz, Warning, TEXT("LeaveTournament() is DEPRECATED. Manage tournaments at deskillz.games"));
}

// Helper function for currency enum to string
FString GetCurrencyString(EDeskillzCurrency Currency)
{
	switch (Currency)
	{
		case EDeskillzCurrency::BTC: return TEXT("BTC");
		case EDeskillzCurrency::ETH: return TEXT("ETH");
		case EDeskillzCurrency::SOL: return TEXT("SOL");
		case EDeskillzCurrency::XRP: return TEXT("XRP");
		case EDeskillzCurrency::BNB: return TEXT("BNB");
		case EDeskillzCurrency::USDT: return TEXT("USDT");
		case EDeskillzCurrency::USDC: return TEXT("USDC");
		default: return TEXT("USDT");
	}
}

// ============================================================================
// Matchmaking - DEPRECATED (Now handled by main Deskillz app)
// ============================================================================

void UDeskillzSDK::StartMatchmaking(const FString& TournamentId)
{
	// DEPRECATED: Matchmaking now handled by main Deskillz app
	UE_LOG(LogDeskillz, Warning, TEXT("StartMatchmaking() is DEPRECATED. Matchmaking is handled by the main Deskillz app."));
	UE_LOG(LogDeskillz, Warning, TEXT("Games receive matches via deep links from deskillz.games"));
	
	// Do not set bIsMatchmaking - this is now a no-op
}

void UDeskillzSDK::CancelMatchmaking()
{
	// DEPRECATED: Matchmaking now handled by main Deskillz app
	UE_LOG(LogDeskillz, Warning, TEXT("CancelMatchmaking() is DEPRECATED. Cancel matchmaking via the main Deskillz app."));
	
	bIsMatchmaking = false;
}

// ============================================================================
// Match Lifecycle
// ============================================================================

void UDeskillzSDK::StartMatch()
{
	if (!CurrentMatch.IsInProgress())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No active match to start"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Match starting: %s"), *CurrentMatch.MatchId);
	
	MatchStartTime = FDateTime::UtcNow();
	CurrentScore = 0;
	
	// Notify server that match started
	TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
	RequestBody->SetStringField(TEXT("matchId"), CurrentMatch.MatchId);
	
	MakeAPIRequest(TEXT("/matches/start"), TEXT("POST"), RequestBody, nullptr);
	
	OnMatchStarted.Broadcast(CurrentMatch, FDeskillzError::None());
}

void UDeskillzSDK::UpdateScore(int64 Score)
{
	CurrentScore = Score;
	
	// Validate score range
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	if (Config)
	{
		CurrentScore = FMath::Clamp(CurrentScore, Config->MinScore, Config->MaxScore);
	}
	
	// For synchronous matches, send score updates via WebSocket
	if (CurrentMatch.IsSynchronous() && WebSocket.IsValid() && WebSocket->IsConnected())
	{
		TSharedPtr<FJsonObject> ScoreUpdate = MakeShareable(new FJsonObject());
		ScoreUpdate->SetStringField(TEXT("type"), TEXT("scoreUpdate"));
		ScoreUpdate->SetStringField(TEXT("matchId"), CurrentMatch.MatchId);
		ScoreUpdate->SetNumberField(TEXT("score"), (double)CurrentScore);
		
		FString JsonString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(ScoreUpdate.ToSharedRef(), Writer);
		
		WebSocket->Send(JsonString);
	}
}

void UDeskillzSDK::SubmitScore(int64 FinalScore, bool bForceSubmit)
{
	if (!CurrentMatch.MatchId.IsEmpty() || bIsInPractice)
	{
		CurrentScore = FinalScore;
	}
	else
	{
		BroadcastError(FDeskillzError(EDeskillzErrorCode::MatchNotFound, TEXT("No active match")));
		return;
	}
	
	// Validate score
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	if (Config && !bForceSubmit)
	{
		if (FinalScore < Config->MinScore || FinalScore > Config->MaxScore)
		{
			BroadcastError(FDeskillzError(EDeskillzErrorCode::InvalidScore, 
				FString::Printf(TEXT("Score %lld is outside valid range [%lld, %lld]"), 
					FinalScore, Config->MinScore, Config->MaxScore)));
			return;
		}
	}
	
	// Practice mode - no server submission
	if (bIsInPractice)
	{
		UE_LOG(LogDeskillz, Log, TEXT("Practice score: %lld"), FinalScore);
		OnScoreSubmitted.Broadcast(true, FDeskillzError::None());
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Submitting score: %lld for match: %s"), FinalScore, *CurrentMatch.MatchId);
	
	TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
	RequestBody->SetStringField(TEXT("matchId"), CurrentMatch.MatchId);
	RequestBody->SetNumberField(TEXT("score"), (double)FinalScore);
	RequestBody->SetNumberField(TEXT("elapsedTime"), GetElapsedTime());
	RequestBody->SetStringField(TEXT("deviceId"), GetDeviceId());
	
	// Add checksum for basic validation (full encryption in Security module)
	int64 Checksum = FinalScore ^ CurrentMatch.RandomSeed;
	RequestBody->SetNumberField(TEXT("checksum"), (double)Checksum);
	
	MakeAPIRequest(TEXT("/matches/submit-score"), TEXT("POST"), RequestBody,
		[this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
		{
			if (Error.IsError())
			{
				UE_LOG(LogDeskillz, Error, TEXT("Score submission failed: %s"), *Error.Message);
				OnScoreSubmitted.Broadcast(false, Error);
				return;
			}
			
			UE_LOG(LogDeskillz, Log, TEXT("Score submitted successfully"));
			OnScoreSubmitted.Broadcast(true, FDeskillzError::None());
			
			// Parse match result if available
			if (Response->HasField(TEXT("result")))
			{
				FDeskillzMatchResult Result;
				Result.MatchId = CurrentMatch.MatchId;
				Result.PlayerScore = CurrentScore;
				
				TSharedPtr<FJsonObject> ResultObj = Response->GetObjectField(TEXT("result"));
				Result.OpponentScore = (int64)ResultObj->GetNumberField(TEXT("opponentScore"));
				Result.PrizeWon = ResultObj->GetNumberField(TEXT("prizeWon"));
				Result.RatingChange = ResultObj->GetIntegerField(TEXT("ratingChange"));
				Result.NewRating = ResultObj->GetIntegerField(TEXT("newRating"));
				Result.Rank = ResultObj->GetIntegerField(TEXT("rank"));
				
				FString ResultStr = ResultObj->GetStringField(TEXT("result"));
				if (ResultStr == TEXT("win")) Result.Result = EDeskillzMatchResult::Win;
				else if (ResultStr == TEXT("loss")) Result.Result = EDeskillzMatchResult::Loss;
				else if (ResultStr == TEXT("draw")) Result.Result = EDeskillzMatchResult::Draw;
				
				OnMatchCompleted.Broadcast(Result, FDeskillzError::None());
				
				// Update player rating
				CurrentPlayer.Rating = Result.NewRating;
			}
			
			// Clear match state
			CurrentMatch = FDeskillzMatchInfo();
		});
}

void UDeskillzSDK::AbortMatch(const FString& Reason)
{
	if (CurrentMatch.MatchId.IsEmpty())
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Aborting match: %s Reason: %s"), *CurrentMatch.MatchId, *Reason);
	
	TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
	RequestBody->SetStringField(TEXT("matchId"), CurrentMatch.MatchId);
	RequestBody->SetStringField(TEXT("reason"), Reason);
	
	MakeAPIRequest(TEXT("/matches/abort"), TEXT("POST"), RequestBody, nullptr);
	
	// Set result as forfeit
	FDeskillzMatchResult Result;
	Result.MatchId = CurrentMatch.MatchId;
	Result.Result = EDeskillzMatchResult::Forfeit;
	Result.PlayerScore = CurrentScore;
	
	CurrentMatch = FDeskillzMatchInfo();
	
	OnMatchCompleted.Broadcast(Result, FDeskillzError::None());
}

float UDeskillzSDK::GetRemainingTime() const
{
	if (!CurrentMatch.IsInProgress())
	{
		return 0.0f;
	}
	
	float Elapsed = GetElapsedTime();
	return FMath::Max(0.0f, (float)CurrentMatch.DurationSeconds - Elapsed);
}

float UDeskillzSDK::GetElapsedTime() const
{
	if (!CurrentMatch.IsInProgress())
	{
		return 0.0f;
	}
	
	FTimespan Duration = FDateTime::UtcNow() - MatchStartTime;
	return (float)Duration.GetTotalSeconds();
}

// ============================================================================
// Practice Mode
// ============================================================================

void UDeskillzSDK::StartPractice(int32 DurationSeconds)
{
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	if (Config && !Config->bEnablePracticeMode)
	{
		BroadcastError(FDeskillzError(EDeskillzErrorCode::Unknown, TEXT("Practice mode is disabled")));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Starting practice session (%d seconds)"), DurationSeconds);
	
	bIsInPractice = true;
	CurrentScore = 0;
	MatchStartTime = FDateTime::UtcNow();
	
	// Create a practice match info
	CurrentMatch.MatchId = TEXT("practice_") + FGuid::NewGuid().ToString();
	CurrentMatch.DurationSeconds = DurationSeconds;
	CurrentMatch.Status = EDeskillzMatchStatus::InProgress;
	CurrentMatch.MatchType = EDeskillzMatchType::Asynchronous;
	CurrentMatch.LocalPlayer = CurrentPlayer;
	
	// Practice opponent (AI/bot)
	CurrentMatch.Opponent.Username = TEXT("Practice Bot");
	CurrentMatch.Opponent.Rating = CurrentPlayer.Rating;
	
	OnMatchStarted.Broadcast(CurrentMatch, FDeskillzError::None());
}

void UDeskillzSDK::EndPractice()
{
	if (!bIsInPractice)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Ending practice session. Score: %lld"), CurrentScore);
	
	FDeskillzMatchResult Result;
	Result.MatchId = CurrentMatch.MatchId;
	Result.PlayerScore = CurrentScore;
	Result.Result = EDeskillzMatchResult::Win; // Always "win" in practice
	
	bIsInPractice = false;
	CurrentMatch = FDeskillzMatchInfo();
	
	OnMatchCompleted.Broadcast(Result, FDeskillzError::None());
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
	
	MakeAPIRequest(TEXT("/wallet/balances"), TEXT("GET"), nullptr,
		[this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
		{
			TArray<FDeskillzWalletBalance> Balances;
			
			if (Error.IsError())
			{
				OnWalletUpdated.Broadcast(Balances, Error);
				return;
			}
			
			const TArray<TSharedPtr<FJsonValue>>* BalanceArray;
			if (Response->TryGetArrayField(TEXT("balances"), BalanceArray))
			{
				for (const TSharedPtr<FJsonValue>& BalanceValue : *BalanceArray)
				{
					TSharedPtr<FJsonObject> BalanceObj = BalanceValue->AsObject();
					if (!BalanceObj.IsValid()) continue;
					
					FDeskillzWalletBalance Balance;
					Balance.Amount = BalanceObj->GetNumberField(TEXT("amount"));
					Balance.PendingAmount = BalanceObj->GetNumberField(TEXT("pending"));
					
					FString CurrencyStr = BalanceObj->GetStringField(TEXT("currency"));
					if (CurrencyStr == TEXT("BTC")) Balance.Currency = EDeskillzCurrency::BTC;
					else if (CurrencyStr == TEXT("ETH")) Balance.Currency = EDeskillzCurrency::ETH;
					else if (CurrencyStr == TEXT("SOL")) Balance.Currency = EDeskillzCurrency::SOL;
					else if (CurrencyStr == TEXT("XRP")) Balance.Currency = EDeskillzCurrency::XRP;
					else if (CurrencyStr == TEXT("BNB")) Balance.Currency = EDeskillzCurrency::BNB;
					else if (CurrencyStr == TEXT("USDT")) Balance.Currency = EDeskillzCurrency::USDT;
					else if (CurrencyStr == TEXT("USDC")) Balance.Currency = EDeskillzCurrency::USDC;
					
					Balance.UpdateFormattedAmount();
					
					WalletBalances.Add(Balance.Currency, Balance);
					Balances.Add(Balance);
				}
			}
			
			OnWalletUpdated.Broadcast(Balances, FDeskillzError::None());
		});
}

FDeskillzWalletBalance UDeskillzSDK::GetBalance(EDeskillzCurrency Currency) const
{
	if (const FDeskillzWalletBalance* Balance = WalletBalances.Find(Currency))
	{
		return *Balance;
	}
	return FDeskillzWalletBalance(Currency, 0.0);
}

bool UDeskillzSDK::HasSufficientFunds(const FDeskillzEntryFee& EntryFee) const
{
	FDeskillzWalletBalance Balance = GetBalance(EntryFee.Currency);
	return Balance.Amount >= EntryFee.Amount;
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
	
	FString PeriodStr;
	switch (Period)
	{
		case EDeskillzLeaderboardPeriod::Daily: PeriodStr = TEXT("daily"); break;
		case EDeskillzLeaderboardPeriod::Weekly: PeriodStr = TEXT("weekly"); break;
		case EDeskillzLeaderboardPeriod::Monthly: PeriodStr = TEXT("monthly"); break;
		default: PeriodStr = TEXT("allTime"); break;
	}
	
	FString Endpoint = FString::Printf(TEXT("/tournaments/%s/leaderboard?period=%s&offset=%d&limit=%d"),
		*TournamentId, *PeriodStr, Offset, Limit);
	
	MakeAPIRequest(Endpoint, TEXT("GET"), nullptr,
		[this](TSharedPtr<FJsonObject> Response, FDeskillzError Error)
		{
			TArray<FDeskillzLeaderboardEntry> Entries;
			
			if (Error.IsError())
			{
				OnLeaderboardReceived.Broadcast(Entries, Error);
				return;
			}
			
			const TArray<TSharedPtr<FJsonValue>>* EntryArray;
			if (Response->TryGetArrayField(TEXT("entries"), EntryArray))
			{
				for (const TSharedPtr<FJsonValue>& EntryValue : *EntryArray)
				{
					TSharedPtr<FJsonObject> EntryObj = EntryValue->AsObject();
					if (!EntryObj.IsValid()) continue;
					
					FDeskillzLeaderboardEntry Entry;
					Entry.Rank = EntryObj->GetIntegerField(TEXT("rank"));
					Entry.Score = (int64)EntryObj->GetNumberField(TEXT("score"));
					Entry.Earnings = EntryObj->GetNumberField(TEXT("earnings"));
					Entry.MatchesWon = EntryObj->GetIntegerField(TEXT("matchesWon"));
					Entry.MatchesPlayed = EntryObj->GetIntegerField(TEXT("matchesPlayed"));
					
					if (TSharedPtr<FJsonObject> PlayerObj = EntryObj->GetObjectField(TEXT("player")))
					{
						Entry.Player.PlayerId = PlayerObj->GetStringField(TEXT("id"));
						Entry.Player.Username = PlayerObj->GetStringField(TEXT("username"));
						Entry.Player.AvatarUrl = PlayerObj->GetStringField(TEXT("avatarUrl"));
						Entry.Player.Rating = PlayerObj->GetIntegerField(TEXT("rating"));
						Entry.Player.bIsCurrentUser = (Entry.Player.PlayerId == CurrentPlayer.PlayerId);
					}
					
					Entries.Add(Entry);
				}
			}
			
			OnLeaderboardReceived.Broadcast(Entries, FDeskillzError::None());
		});
}

void UDeskillzSDK::GetGlobalLeaderboard(EDeskillzLeaderboardPeriod Period, int32 Offset, int32 Limit)
{
	GetLeaderboard(GameId, Period, Offset, Limit);
}

// ============================================================================
// Network - HTTP
// ============================================================================

void UDeskillzSDK::MakeAPIRequest(const FString& Endpoint, const FString& Method, const TSharedPtr<FJsonObject>& Body, TFunction<void(TSharedPtr<FJsonObject>, FDeskillzError)> Callback)
{
	FString Url = ActiveEndpoints.BaseUrl + Endpoint;
	
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	
	if (Config && Config->bLogAPICalls)
	{
		UE_LOG(LogDeskillz, Log, TEXT("API Request: %s %s"), *Method, *Url);
	}
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(Method);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("X-API-Key"), APIKey);
	Request->SetHeader(TEXT("X-Game-Id"), GameId);
	
	if (!AuthToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}
	
	if (Body.IsValid())
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