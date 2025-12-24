// Copyright Deskillz Games. All Rights Reserved.

#include "DeskillzMockServer.h"
#include "Misc/Guid.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "HAL/PlatformProcess.h"
#include "Math/UnrealMathUtility.h"

// Static instance
FDeskillzMockServer* FDeskillzMockServer::Instance = nullptr;
bool FDeskillzMockHttpInterceptor::bEnabled = false;

// ============================================================================
// MOCK SERVER IMPLEMENTATION
// ============================================================================

FDeskillzMockServer::FDeskillzMockServer()
	: bIsRunning(false)
	, ServerUrl(TEXT("http://localhost:8080"))
	, ResponseDelay(0.0f)
	, FailureRate(0.0f)
	, bLoggingEnabled(true)
{
}

FDeskillzMockServer::~FDeskillzMockServer()
{
	Stop();
}

FDeskillzMockServer* FDeskillzMockServer::Get()
{
	if (!Instance)
	{
		Instance = new FDeskillzMockServer();
	}
	return Instance;
}

void FDeskillzMockServer::Start()
{
	FScopeLock Lock(&CriticalSection);

	if (bIsRunning)
	{
		return;
	}

	bIsRunning = true;
	RegisterDefaultHandlers();
	FDeskillzMockHttpInterceptor::Enable();

	UE_LOG(LogTemp, Log, TEXT("[MockServer] Started at %s"), *ServerUrl);
}

void FDeskillzMockServer::Stop()
{
	FScopeLock Lock(&CriticalSection);

	if (!bIsRunning)
	{
		return;
	}

	bIsRunning = false;
	FDeskillzMockHttpInterceptor::Disable();
	ClearHandlers();
	ClearHistory();

	UE_LOG(LogTemp, Log, TEXT("[MockServer] Stopped"));
}

void FDeskillzMockServer::SetResponseDelay(float Seconds)
{
	ResponseDelay = FMath::Max(0.0f, Seconds);
}

void FDeskillzMockServer::SetFailureRate(float Rate)
{
	FailureRate = FMath::Clamp(Rate, 0.0f, 1.0f);
}

void FDeskillzMockServer::SetLoggingEnabled(bool bEnabled)
{
	bLoggingEnabled = bEnabled;
}

void FDeskillzMockServer::Reset()
{
	FScopeLock Lock(&CriticalSection);

	ResponseDelay = 0.0f;
	FailureRate = 0.0f;
	ClearHandlers();
	ClearHistory();
	Expectations.Empty();
	RegisterDefaultHandlers();
}

void FDeskillzMockServer::RegisterHandler(const FString& Method, const FString& Endpoint, FDeskillzMockHandler Handler)
{
	FScopeLock Lock(&CriticalSection);
	FString Key = MakeHandlerKey(Method, Endpoint);
	Handlers.Add(Key, Handler);
}

void FDeskillzMockServer::RegisterStaticResponse(const FString& Method, const FString& Endpoint, const FDeskillzMockResponse& Response)
{
	FScopeLock Lock(&CriticalSection);
	FString Key = MakeHandlerKey(Method, Endpoint);
	StaticResponses.Add(Key, Response);
}

void FDeskillzMockServer::UnregisterHandler(const FString& Method, const FString& Endpoint)
{
	FScopeLock Lock(&CriticalSection);
	FString Key = MakeHandlerKey(Method, Endpoint);
	Handlers.Remove(Key);
	StaticResponses.Remove(Key);
}

void FDeskillzMockServer::ClearHandlers()
{
	FScopeLock Lock(&CriticalSection);
	Handlers.Empty();
	StaticResponses.Empty();
}

FDeskillzMockResponse FDeskillzMockServer::ProcessRequest(const FString& Method, const FString& Endpoint, 
	const FString& Body, const TMap<FString, FString>& Headers)
{
	FScopeLock Lock(&CriticalSection);

	// Record request
	FDeskillzMockRequest Request;
	Request.Method = Method;
	Request.Endpoint = Endpoint;
	Request.Body = Body;
	Request.Headers = Headers;
	RequestHistory.Add(Request);

	if (bLoggingEnabled)
	{
		UE_LOG(LogTemp, Log, TEXT("[MockServer] %s %s"), *Method, *Endpoint);
	}

	// Update expectations
	for (FExpectation& Exp : Expectations)
	{
		if (Exp.Method == Method && Endpoint.Contains(Exp.Endpoint))
		{
			Exp.ActualCount++;
		}
	}

	// Apply delay
	ApplyDelay();

	// Check failure rate
	if (ShouldFail())
	{
		return FDeskillzMockResponse::Error(500, TEXT("Simulated server error"));
	}

	FString Key = MakeHandlerKey(Method, Endpoint);

	// Check static responses first
	if (const FDeskillzMockResponse* StaticResponse = StaticResponses.Find(Key))
	{
		return *StaticResponse;
	}

	// Check dynamic handlers
	if (const FDeskillzMockHandler* Handler = Handlers.Find(Key))
	{
		return Handler->Execute(Method, Body);
	}

	// Try wildcard handlers
	for (const auto& Pair : Handlers)
	{
		if (Pair.Key.Contains(TEXT("*")) && Endpoint.Contains(Pair.Key.Replace(TEXT("*"), TEXT(""))))
		{
			return Pair.Value.Execute(Method, Body);
		}
	}

	// Default 404
	return FDeskillzMockResponse::Error(404, FString::Printf(TEXT("Endpoint not found: %s %s"), *Method, *Endpoint));
}

void FDeskillzMockServer::ExpectCall(const FString& Method, const FString& Endpoint, int32 Times)
{
	FScopeLock Lock(&CriticalSection);

	FExpectation Exp;
	Exp.Method = Method;
	Exp.Endpoint = Endpoint;
	Exp.ExpectedCount = Times;
	Exp.ActualCount = 0;
	Expectations.Add(Exp);
}

bool FDeskillzMockServer::VerifyExpectations(FString& OutErrors)
{
	FScopeLock Lock(&CriticalSection);

	TArray<FString> Errors;
	bool bAllMet = true;

	for (const FExpectation& Exp : Expectations)
	{
		if (Exp.ActualCount < Exp.ExpectedCount)
		{
			Errors.Add(FString::Printf(TEXT("%s %s: expected %d calls, got %d"),
				*Exp.Method, *Exp.Endpoint, Exp.ExpectedCount, Exp.ActualCount));
			bAllMet = false;
		}
	}

	OutErrors = FString::Join(Errors, TEXT("\n"));
	Expectations.Empty();
	return bAllMet;
}

TArray<FDeskillzMockRequest> FDeskillzMockServer::GetRequestsForEndpoint(const FString& Endpoint) const
{
	TArray<FDeskillzMockRequest> Results;
	for (const FDeskillzMockRequest& Request : RequestHistory)
	{
		if (Request.Endpoint.Contains(Endpoint))
		{
			Results.Add(Request);
		}
	}
	return Results;
}

int32 FDeskillzMockServer::GetRequestCount(const FString& Method, const FString& Endpoint) const
{
	int32 Count = 0;
	for (const FDeskillzMockRequest& Request : RequestHistory)
	{
		if (Request.Method == Method && Request.Endpoint.Contains(Endpoint))
		{
			Count++;
		}
	}
	return Count;
}

void FDeskillzMockServer::ClearHistory()
{
	FScopeLock Lock(&CriticalSection);
	RequestHistory.Empty();
}

const FDeskillzMockRequest* FDeskillzMockServer::GetLastRequest() const
{
	if (RequestHistory.Num() > 0)
	{
		return &RequestHistory.Last();
	}
	return nullptr;
}

FString FDeskillzMockServer::MakeHandlerKey(const FString& Method, const FString& Endpoint) const
{
	return Method.ToUpper() + TEXT(":") + Endpoint;
}

bool FDeskillzMockServer::ShouldFail() const
{
	if (FailureRate <= 0.0f)
	{
		return false;
	}
	return FMath::FRand() < FailureRate;
}

void FDeskillzMockServer::ApplyDelay() const
{
	if (ResponseDelay > 0.0f)
	{
		FPlatformProcess::Sleep(ResponseDelay);
	}
}

void FDeskillzMockServer::RegisterDefaultHandlers()
{
	// Auth endpoints
	RegisterHandler(TEXT("POST"), TEXT("/api/v1/auth/login"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleAuth(Method, Body);
		}));

	RegisterHandler(TEXT("POST"), TEXT("/api/v1/auth/register"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleAuth(Method, Body);
		}));

	RegisterHandler(TEXT("POST"), TEXT("/api/v1/auth/refresh"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleAuth(Method, Body);
		}));

	// Tournament endpoints
	RegisterHandler(TEXT("GET"), TEXT("/api/v1/tournaments"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleTournaments(Method, Body);
		}));

	RegisterHandler(TEXT("POST"), TEXT("/api/v1/tournaments/*/enter"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleTournaments(Method, Body);
		}));

	// Match endpoints
	RegisterHandler(TEXT("GET"), TEXT("/api/v1/matches/*"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleMatches(Method, Body);
		}));

	RegisterHandler(TEXT("POST"), TEXT("/api/v1/matches/*/score"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleScore(Method, Body);
		}));

	// Wallet endpoints
	RegisterHandler(TEXT("GET"), TEXT("/api/v1/wallet/balance"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleWallet(Method, Body);
		}));

	RegisterHandler(TEXT("POST"), TEXT("/api/v1/wallet/deposit"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleWallet(Method, Body);
		}));

	RegisterHandler(TEXT("POST"), TEXT("/api/v1/wallet/withdraw"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleWallet(Method, Body);
		}));

	// Leaderboard endpoints
	RegisterHandler(TEXT("GET"), TEXT("/api/v1/leaderboard/*"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleLeaderboard(Method, Body);
		}));

	// Analytics endpoints
	RegisterHandler(TEXT("POST"), TEXT("/api/v1/analytics/events"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleAnalytics(Method, Body);
		}));

	RegisterHandler(TEXT("POST"), TEXT("/api/v1/analytics/telemetry"),
		FDeskillzMockHandler::CreateLambda([this](const FString& Method, const FString& Body) {
			return HandleAnalytics(Method, Body);
		}));
}

FDeskillzMockResponse FDeskillzMockServer::HandleAuth(const FString& Method, const FString& Body)
{
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetStringField(TEXT("token"), GenerateAuthToken());
	Response->SetStringField(TEXT("refreshToken"), GenerateAuthToken());
	Response->SetNumberField(TEXT("expiresIn"), 3600);

	TSharedPtr<FJsonObject> User = MakeShareable(new FJsonObject());
	User->SetStringField(TEXT("userId"), TEXT("user_") + FGuid::NewGuid().ToString().Left(8));
	User->SetStringField(TEXT("username"), TEXT("TestUser"));
	User->SetNumberField(TEXT("skillRating"), 1500);
	Response->SetObjectField(TEXT("user"), User);

	FString ResponseString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	return FDeskillzMockResponse::Success(ResponseString);
}

FDeskillzMockResponse FDeskillzMockServer::HandleTournaments(const FString& Method, const FString& Body)
{
	if (Method == TEXT("GET"))
	{
		return FDeskillzMockResponse::Success(GenerateTournamentListJson(10));
	}
	else if (Method == TEXT("POST"))
	{
		// Tournament entry
		TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
		Response->SetBoolField(TEXT("success"), true);
		Response->SetStringField(TEXT("entryId"), TEXT("entry_") + FGuid::NewGuid().ToString().Left(8));
		Response->SetStringField(TEXT("matchId"), TEXT("match_") + FGuid::NewGuid().ToString().Left(8));

		FString ResponseString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
		FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

		return FDeskillzMockResponse::Success(ResponseString);
	}

	return FDeskillzMockResponse::Error(400, TEXT("Invalid request"));
}

FDeskillzMockResponse FDeskillzMockServer::HandleMatches(const FString& Method, const FString& Body)
{
	return FDeskillzMockResponse::Success(GenerateMatchJson());
}

FDeskillzMockResponse FDeskillzMockServer::HandleWallet(const FString& Method, const FString& Body)
{
	if (Method == TEXT("GET"))
	{
		TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
		
		TArray<TSharedPtr<FJsonValue>> Balances;
		
		// Add multiple currency balances
		TArray<FString> Currencies = { TEXT("USDT"), TEXT("BTC"), TEXT("ETH"), TEXT("SOL") };
		TArray<double> Amounts = { 100.0, 0.005, 0.1, 2.5 };
		
		for (int32 i = 0; i < Currencies.Num(); i++)
		{
			TSharedPtr<FJsonObject> Balance = MakeShareable(new FJsonObject());
			Balance->SetStringField(TEXT("currency"), Currencies[i]);
			Balance->SetNumberField(TEXT("available"), Amounts[i]);
			Balance->SetNumberField(TEXT("pending"), 0.0);
			Balance->SetNumberField(TEXT("total"), Amounts[i]);
			Balances.Add(MakeShareable(new FJsonValueObject(Balance)));
		}
		
		Response->SetArrayField(TEXT("balances"), Balances);

		FString ResponseString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
		FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

		return FDeskillzMockResponse::Success(ResponseString);
	}
	else if (Method == TEXT("POST"))
	{
		TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
		Response->SetBoolField(TEXT("success"), true);
		Response->SetStringField(TEXT("transactionId"), TEXT("tx_") + FGuid::NewGuid().ToString().Left(12));

		FString ResponseString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
		FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

		return FDeskillzMockResponse::Success(ResponseString);
	}

	return FDeskillzMockResponse::Error(400, TEXT("Invalid request"));
}

FDeskillzMockResponse FDeskillzMockServer::HandleLeaderboard(const FString& Method, const FString& Body)
{
	return FDeskillzMockResponse::Success(GenerateLeaderboardJson(50));
}

FDeskillzMockResponse FDeskillzMockServer::HandleAnalytics(const FString& Method, const FString& Body)
{
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetNumberField(TEXT("eventsReceived"), 1);

	FString ResponseString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	return FDeskillzMockResponse::Success(ResponseString);
}

FDeskillzMockResponse FDeskillzMockServer::HandleScore(const FString& Method, const FString& Body)
{
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	Response->SetBoolField(TEXT("success"), true);
	Response->SetBoolField(TEXT("validated"), true);
	Response->SetNumberField(TEXT("rank"), FMath::RandRange(1, 100));

	FString ResponseString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResponseString);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);

	return FDeskillzMockResponse::Success(ResponseString);
}

// ============================================================================
// MOCK DATA GENERATORS
// ============================================================================

FString FDeskillzMockServer::GeneratePlayerJson(const FString& UserId)
{
	FString Id = UserId.IsEmpty() ? TEXT("user_") + FGuid::NewGuid().ToString().Left(8) : UserId;

	TSharedPtr<FJsonObject> Player = MakeShareable(new FJsonObject());
	Player->SetStringField(TEXT("userId"), Id);
	Player->SetStringField(TEXT("username"), TEXT("Player_") + Id.Right(4));
	Player->SetNumberField(TEXT("skillRating"), FMath::RandRange(1000, 2000));
	Player->SetNumberField(TEXT("totalMatches"), FMath::RandRange(10, 500));
	Player->SetNumberField(TEXT("wins"), FMath::RandRange(5, 250));
	Player->SetNumberField(TEXT("losses"), FMath::RandRange(5, 250));
	Player->SetStringField(TEXT("avatarUrl"), TEXT("https://api.deskillz.games/avatars/default.png"));
	Player->SetStringField(TEXT("country"), TEXT("US"));
	Player->SetBoolField(TEXT("isPremium"), FMath::RandRange(0, 10) > 7);

	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(Player.ToSharedRef(), Writer);
	return Result;
}

FString FDeskillzMockServer::GenerateTournamentJson(const FString& TournamentId)
{
	FString Id = TournamentId.IsEmpty() ? TEXT("tournament_") + FGuid::NewGuid().ToString().Left(8) : TournamentId;

	TSharedPtr<FJsonObject> Tournament = MakeShareable(new FJsonObject());
	Tournament->SetStringField(TEXT("tournamentId"), Id);
	Tournament->SetStringField(TEXT("name"), TEXT("Daily Challenge #") + FString::FromInt(FMath::RandRange(1, 999)));
	Tournament->SetStringField(TEXT("gameId"), TEXT("game_puzzle_blast"));
	Tournament->SetNumberField(TEXT("entryFee"), FMath::RandRange(1, 50) * 1.0);
	Tournament->SetStringField(TEXT("currency"), TEXT("USDT"));
	Tournament->SetNumberField(TEXT("prizePool"), FMath::RandRange(100, 5000) * 1.0);
	Tournament->SetNumberField(TEXT("currentParticipants"), FMath::RandRange(10, 90));
	Tournament->SetNumberField(TEXT("maxParticipants"), 100);
	Tournament->SetStringField(TEXT("status"), TEXT("active"));
	Tournament->SetStringField(TEXT("startTime"), FDateTime::Now().ToIso8601());
	Tournament->SetStringField(TEXT("endTime"), (FDateTime::Now() + FTimespan::FromHours(24)).ToIso8601());

	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(Tournament.ToSharedRef(), Writer);
	return Result;
}

FString FDeskillzMockServer::GenerateTournamentListJson(int32 Count)
{
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> Tournaments;

	for (int32 i = 0; i < Count; i++)
	{
		FString TournamentJson = GenerateTournamentJson();
		TSharedPtr<FJsonObject> Tournament;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(TournamentJson);
		FJsonSerializer::Deserialize(Reader, Tournament);
		Tournaments.Add(MakeShareable(new FJsonValueObject(Tournament)));
	}

	Response->SetArrayField(TEXT("tournaments"), Tournaments);
	Response->SetNumberField(TEXT("total"), Count);
	Response->SetNumberField(TEXT("page"), 1);
	Response->SetNumberField(TEXT("pageSize"), Count);

	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);
	return Result;
}

FString FDeskillzMockServer::GenerateMatchJson(const FString& MatchId)
{
	FString Id = MatchId.IsEmpty() ? TEXT("match_") + FGuid::NewGuid().ToString().Left(8) : MatchId;

	TSharedPtr<FJsonObject> Match = MakeShareable(new FJsonObject());
	Match->SetStringField(TEXT("matchId"), Id);
	Match->SetStringField(TEXT("tournamentId"), TEXT("tournament_") + FGuid::NewGuid().ToString().Left(8));
	Match->SetStringField(TEXT("status"), TEXT("ready"));
	Match->SetNumberField(TEXT("entryFee"), FMath::RandRange(1, 20) * 1.0);
	Match->SetNumberField(TEXT("maxPlayers"), 2);
	Match->SetStringField(TEXT("gameMode"), TEXT("ranked"));
	Match->SetBoolField(TEXT("isSynchronous"), FMath::RandBool());

	// Add players
	TArray<TSharedPtr<FJsonValue>> Players;
	for (int32 i = 0; i < 2; i++)
	{
		FString PlayerJson = GeneratePlayerJson();
		TSharedPtr<FJsonObject> Player;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PlayerJson);
		FJsonSerializer::Deserialize(Reader, Player);
		Players.Add(MakeShareable(new FJsonValueObject(Player)));
	}
	Match->SetArrayField(TEXT("players"), Players);

	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(Match.ToSharedRef(), Writer);
	return Result;
}

FString FDeskillzMockServer::GenerateWalletJson(const FString& Currency, double Amount)
{
	TSharedPtr<FJsonObject> Balance = MakeShareable(new FJsonObject());
	Balance->SetStringField(TEXT("currency"), Currency);
	Balance->SetNumberField(TEXT("available"), Amount);
	Balance->SetNumberField(TEXT("pending"), 0.0);
	Balance->SetNumberField(TEXT("total"), Amount);

	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(Balance.ToSharedRef(), Writer);
	return Result;
}

FString FDeskillzMockServer::GenerateLeaderboardJson(int32 Count)
{
	TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> Entries;

	for (int32 i = 0; i < Count; i++)
	{
		TSharedPtr<FJsonObject> Entry = MakeShareable(new FJsonObject());
		Entry->SetNumberField(TEXT("rank"), i + 1);
		Entry->SetStringField(TEXT("userId"), TEXT("user_") + FGuid::NewGuid().ToString().Left(8));
		Entry->SetStringField(TEXT("username"), TEXT("Player") + FString::FromInt(FMath::RandRange(1000, 9999)));
		Entry->SetNumberField(TEXT("score"), (Count - i) * 1000 + FMath::RandRange(0, 999));
		Entry->SetNumberField(TEXT("wins"), FMath::RandRange(10, 100));
		Entry->SetStringField(TEXT("avatarUrl"), TEXT("https://api.deskillz.games/avatars/default.png"));
		Entries.Add(MakeShareable(new FJsonValueObject(Entry)));
	}

	Response->SetArrayField(TEXT("entries"), Entries);
	Response->SetNumberField(TEXT("totalEntries"), Count);

	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);
	return Result;
}

FString FDeskillzMockServer::GenerateErrorJson(const FString& Code, const FString& Message)
{
	TSharedPtr<FJsonObject> Error = MakeShareable(new FJsonObject());
	Error->SetStringField(TEXT("error"), Code);
	Error->SetStringField(TEXT("message"), Message);

	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(Error.ToSharedRef(), Writer);
	return Result;
}

FString FDeskillzMockServer::GenerateAuthToken()
{
	// Generate mock JWT-like token
	return TEXT("eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.") + 
		   FGuid::NewGuid().ToString().Replace(TEXT("-"), TEXT("")) + TEXT(".") +
		   FGuid::NewGuid().ToString().Left(16);
}

// ============================================================================
// HTTP INTERCEPTOR
// ============================================================================

void FDeskillzMockHttpInterceptor::Enable()
{
	bEnabled = true;
}

void FDeskillzMockHttpInterceptor::Disable()
{
	bEnabled = false;
}

bool FDeskillzMockHttpInterceptor::IsEnabled()
{
	return bEnabled;
}

bool FDeskillzMockHttpInterceptor::TryIntercept(const FString& URL, const FString& Method, const FString& Body,
	const TMap<FString, FString>& Headers, FDeskillzMockResponse& OutResponse)
{
	if (!bEnabled)
	{
		return false;
	}

	// Extract endpoint from URL
	FString Endpoint = URL;
	int32 ProtocolIndex;
	if (Endpoint.FindChar('/', ProtocolIndex))
	{
		// Skip protocol
		Endpoint = Endpoint.RightChop(ProtocolIndex);
		if (Endpoint.FindChar('/', ProtocolIndex))
		{
			Endpoint = Endpoint.RightChop(ProtocolIndex);
			if (Endpoint.FindChar('/', ProtocolIndex))
			{
				Endpoint = Endpoint.RightChop(ProtocolIndex);
			}
		}
	}

	// Process through mock server
	OutResponse = FDeskillzMockServer::Get()->ProcessRequest(Method, TEXT("/") + Endpoint, Body, Headers);
	return true;
}

// ============================================================================
// MOCK PLAYER BUILDER
// ============================================================================

FMockPlayerBuilder::FMockPlayerBuilder()
{
	Player.UserId = TEXT("user_") + FGuid::NewGuid().ToString().Left(8);
	Player.Username = TEXT("TestPlayer");
	Player.SkillRating = 1500;
	Player.TotalMatches = 100;
	Player.Wins = 50;
	Player.Losses = 50;
}

FMockPlayerBuilder& FMockPlayerBuilder::WithUserId(const FString& Id)
{
	Player.UserId = Id;
	return *this;
}

FMockPlayerBuilder& FMockPlayerBuilder::WithUsername(const FString& Name)
{
	Player.Username = Name;
	return *this;
}

FMockPlayerBuilder& FMockPlayerBuilder::WithSkillRating(int32 Rating)
{
	Player.SkillRating = Rating;
	return *this;
}

FMockPlayerBuilder& FMockPlayerBuilder::WithWins(int32 Wins)
{
	Player.Wins = Wins;
	return *this;
}

FMockPlayerBuilder& FMockPlayerBuilder::WithLosses(int32 Losses)
{
	Player.Losses = Losses;
	return *this;
}

FMockPlayerBuilder& FMockPlayerBuilder::WithAvatar(const FString& AvatarUrl)
{
	Player.AvatarUrl = AvatarUrl;
	return *this;
}

FMockPlayerBuilder& FMockPlayerBuilder::WithCountry(const FString& Country)
{
	Player.Country = Country;
	return *this;
}

FMockPlayerBuilder& FMockPlayerBuilder::AsPremium()
{
	Player.bIsPremium = true;
	return *this;
}

FDeskillzPlayerInfo FMockPlayerBuilder::Build()
{
	Player.TotalMatches = Player.Wins + Player.Losses;
	return Player;
}

FString FMockPlayerBuilder::BuildJson()
{
	return FDeskillzMockServer::GeneratePlayerJson(Player.UserId);
}

// ============================================================================
// MOCK TOURNAMENT BUILDER
// ============================================================================

FMockTournamentBuilder::FMockTournamentBuilder()
{
	Tournament.TournamentId = TEXT("tournament_") + FGuid::NewGuid().ToString().Left(8);
	Tournament.Name = TEXT("Test Tournament");
	Tournament.EntryFee = 10.0;
	Tournament.Currency = TEXT("USDT");
	Tournament.PrizePool = 1000.0;
	Tournament.CurrentParticipants = 50;
	Tournament.MaxParticipants = 100;
	Tournament.Status = EDeskillzTournamentStatus::Active;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithId(const FString& Id)
{
	Tournament.TournamentId = Id;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithName(const FString& Name)
{
	Tournament.Name = Name;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithEntryFee(double Fee, const FString& Currency)
{
	Tournament.EntryFee = Fee;
	Tournament.Currency = Currency;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithPrizePool(double Prize)
{
	Tournament.PrizePool = Prize;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithParticipants(int32 Current, int32 Max)
{
	Tournament.CurrentParticipants = Current;
	Tournament.MaxParticipants = Max;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithStatus(EDeskillzTournamentStatus Status)
{
	Tournament.Status = Status;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithStartTime(const FDateTime& StartTime)
{
	Tournament.StartTime = StartTime;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithEndTime(const FDateTime& EndTime)
{
	Tournament.EndTime = EndTime;
	return *this;
}

FMockTournamentBuilder& FMockTournamentBuilder::WithGameId(const FString& GameId)
{
	Tournament.GameId = GameId;
	return *this;
}

FDeskillzTournament FMockTournamentBuilder::Build()
{
	return Tournament;
}

FString FMockTournamentBuilder::BuildJson()
{
	return FDeskillzMockServer::GenerateTournamentJson(Tournament.TournamentId);
}

// ============================================================================
// MOCK MATCH BUILDER
// ============================================================================

FMockMatchBuilder::FMockMatchBuilder()
{
	Match.MatchId = TEXT("match_") + FGuid::NewGuid().ToString().Left(8);
	Match.Status = EDeskillzMatchStatus::Pending;
	Match.EntryFee = 10.0;
	Match.MaxPlayers = 2;
	Match.GameMode = TEXT("ranked");
	Match.bIsSynchronous = false;
}

FMockMatchBuilder& FMockMatchBuilder::WithId(const FString& Id)
{
	Match.MatchId = Id;
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::WithTournamentId(const FString& TournamentId)
{
	Match.TournamentId = TournamentId;
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::WithStatus(EDeskillzMatchStatus Status)
{
	Match.Status = Status;
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::WithPlayers(const TArray<FDeskillzPlayerInfo>& Players)
{
	Match.Players = Players;
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::AddPlayer(const FDeskillzPlayerInfo& Player)
{
	Match.Players.Add(Player);
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::WithEntryFee(double Fee)
{
	Match.EntryFee = Fee;
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::WithGameMode(const FString& Mode)
{
	Match.GameMode = Mode;
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::Synchronous()
{
	Match.bIsSynchronous = true;
	return *this;
}

FMockMatchBuilder& FMockMatchBuilder::Asynchronous()
{
	Match.bIsSynchronous = false;
	return *this;
}

FDeskillzMatch FMockMatchBuilder::Build()
{
	return Match;
}

FString FMockMatchBuilder::BuildJson()
{
	return FDeskillzMockServer::GenerateMatchJson(Match.MatchId);
}

// ============================================================================
// MOCK WALLET BUILDER
// ============================================================================

FMockWalletBuilder::FMockWalletBuilder()
{
	Balance.Currency = TEXT("USDT");
	Balance.Available = 100.0;
	Balance.Pending = 0.0;
	Balance.Total = 100.0;
}

FMockWalletBuilder& FMockWalletBuilder::WithCurrency(const FString& Currency)
{
	Balance.Currency = Currency;
	return *this;
}

FMockWalletBuilder& FMockWalletBuilder::WithAvailable(double Amount)
{
	Balance.Available = Amount;
	Balance.Total = Balance.Available + Balance.Pending;
	return *this;
}

FMockWalletBuilder& FMockWalletBuilder::WithPending(double Amount)
{
	Balance.Pending = Amount;
	Balance.Total = Balance.Available + Balance.Pending;
	return *this;
}

FMockWalletBuilder& FMockWalletBuilder::WithAddress(const FString& Address)
{
	WalletAddress = Address;
	return *this;
}

FDeskillzWalletBalance FMockWalletBuilder::Build()
{
	return Balance;
}

FString FMockWalletBuilder::BuildJson()
{
	return FDeskillzMockServer::GenerateWalletJson(Balance.Currency, Balance.Available);
}

// ============================================================================
// MOCK TRANSACTION BUILDER
// ============================================================================

FMockTransactionBuilder::FMockTransactionBuilder()
{
	TransactionData = MakeShareable(new FJsonObject());
	TransactionData->SetStringField(TEXT("transactionId"), TEXT("tx_") + FGuid::NewGuid().ToString().Left(12));
	TransactionData->SetStringField(TEXT("status"), TEXT("pending"));
	TransactionData->SetStringField(TEXT("timestamp"), FDateTime::Now().ToIso8601());
}

FMockTransactionBuilder& FMockTransactionBuilder::WithId(const FString& Id)
{
	TransactionData->SetStringField(TEXT("transactionId"), Id);
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::AsDeposit(double Amount, const FString& Currency)
{
	TransactionData->SetStringField(TEXT("type"), TEXT("deposit"));
	TransactionData->SetNumberField(TEXT("amount"), Amount);
	TransactionData->SetStringField(TEXT("currency"), Currency);
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::AsWithdrawal(double Amount, const FString& Currency)
{
	TransactionData->SetStringField(TEXT("type"), TEXT("withdrawal"));
	TransactionData->SetNumberField(TEXT("amount"), -Amount);
	TransactionData->SetStringField(TEXT("currency"), Currency);
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::AsEntryFee(double Amount, const FString& TournamentId)
{
	TransactionData->SetStringField(TEXT("type"), TEXT("entry_fee"));
	TransactionData->SetNumberField(TEXT("amount"), -Amount);
	TransactionData->SetStringField(TEXT("tournamentId"), TournamentId);
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::AsPrize(double Amount, const FString& TournamentId)
{
	TransactionData->SetStringField(TEXT("type"), TEXT("prize"));
	TransactionData->SetNumberField(TEXT("amount"), Amount);
	TransactionData->SetStringField(TEXT("tournamentId"), TournamentId);
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::WithTimestamp(const FDateTime& Time)
{
	TransactionData->SetStringField(TEXT("timestamp"), Time.ToIso8601());
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::Pending()
{
	TransactionData->SetStringField(TEXT("status"), TEXT("pending"));
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::Completed()
{
	TransactionData->SetStringField(TEXT("status"), TEXT("completed"));
	return *this;
}

FMockTransactionBuilder& FMockTransactionBuilder::Failed()
{
	TransactionData->SetStringField(TEXT("status"), TEXT("failed"));
	return *this;
}

FString FMockTransactionBuilder::BuildJson()
{
	FString Result;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Result);
	FJsonSerializer::Serialize(TransactionData.ToSharedRef(), Writer);
	return Result;
}

// ============================================================================
// TEST SCENARIOS
// ============================================================================

void FDeskillzTestScenarios::SetupHappyPath()
{
	FDeskillzMockServer::Get()->Reset();
	FDeskillzMockServer::Get()->SetResponseDelay(0.0f);
	FDeskillzMockServer::Get()->SetFailureRate(0.0f);
}

void FDeskillzTestScenarios::SetupNewUserScenario()
{
	SetupHappyPath();
	// New users start with zero balance
	FDeskillzMockServer::Get()->RegisterStaticResponse(TEXT("GET"), TEXT("/api/v1/wallet/balance"),
		FDeskillzMockResponse::Success(FDeskillzMockServer::GenerateWalletJson(TEXT("USDT"), 0.0)));
}

void FDeskillzTestScenarios::SetupReturningUserScenario()
{
	SetupHappyPath();
	// Returning users have balance and history
	FDeskillzMockServer::Get()->RegisterStaticResponse(TEXT("GET"), TEXT("/api/v1/wallet/balance"),
		FDeskillzMockResponse::Success(FDeskillzMockServer::GenerateWalletJson(TEXT("USDT"), 150.0)));
}

void FDeskillzTestScenarios::SetupNetworkErrorScenario()
{
	FDeskillzMockServer::Get()->Reset();
	FDeskillzMockServer::Get()->SetFailureRate(1.0f); // All requests fail
}

void FDeskillzTestScenarios::SetupAuthFailureScenario()
{
	SetupHappyPath();
	FDeskillzMockServer::Get()->RegisterStaticResponse(TEXT("POST"), TEXT("/api/v1/auth/login"),
		FDeskillzMockResponse::Error(401, TEXT("Invalid credentials")));
}

void FDeskillzTestScenarios::SetupInsufficientFundsScenario()
{
	SetupHappyPath();
	FDeskillzMockServer::Get()->RegisterStaticResponse(TEXT("POST"), TEXT("/api/v1/tournaments/*/enter"),
		FDeskillzMockResponse::Error(402, TEXT("Insufficient funds")));
}

void FDeskillzTestScenarios::SetupTournamentFullScenario()
{
	SetupHappyPath();
	FDeskillzMockServer::Get()->RegisterStaticResponse(TEXT("POST"), TEXT("/api/v1/tournaments/*/enter"),
		FDeskillzMockResponse::Error(409, TEXT("Tournament is full")));
}

void FDeskillzTestScenarios::SetupHighLatencyScenario(float LatencySeconds)
{
	SetupHappyPath();
	FDeskillzMockServer::Get()->SetResponseDelay(LatencySeconds);
}

void FDeskillzTestScenarios::SetupIntermittentFailureScenario(float FailureRate)
{
	SetupHappyPath();
	FDeskillzMockServer::Get()->SetFailureRate(FailureRate);
}
