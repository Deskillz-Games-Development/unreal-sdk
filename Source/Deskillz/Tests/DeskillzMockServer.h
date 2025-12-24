// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/DeskillzTypes.h"

/**
 * Mock HTTP Response
 */
struct DESKILLZ_API FDeskillzMockResponse
{
	int32 StatusCode;
	FString Body;
	TMap<FString, FString> Headers;
	float Delay;

	FDeskillzMockResponse()
		: StatusCode(200)
		, Delay(0.0f)
	{}

	static FDeskillzMockResponse Success(const FString& ResponseBody)
	{
		FDeskillzMockResponse Response;
		Response.StatusCode = 200;
		Response.Body = ResponseBody;
		return Response;
	}

	static FDeskillzMockResponse Error(int32 Code, const FString& Message)
	{
		FDeskillzMockResponse Response;
		Response.StatusCode = Code;
		Response.Body = FString::Printf(TEXT("{\"error\":\"%s\"}"), *Message);
		return Response;
	}
};

/**
 * Mock Request Record
 */
struct DESKILLZ_API FDeskillzMockRequest
{
	FString Method;
	FString Endpoint;
	FString Body;
	TMap<FString, FString> Headers;
	FDateTime Timestamp;

	FDeskillzMockRequest()
		: Timestamp(FDateTime::Now())
	{}
};

/**
 * Endpoint Handler Delegate
 */
DECLARE_DELEGATE_RetVal_TwoParams(FDeskillzMockResponse, FDeskillzMockHandler, const FString&, const FString&);

/**
 * Mock Server for Deskillz SDK Testing
 * Simulates backend API responses for unit and integration tests
 */
class DESKILLZ_API FDeskillzMockServer
{
public:
	/** Get singleton instance */
	static FDeskillzMockServer* Get();

	/** Start mock server */
	void Start();

	/** Stop mock server */
	void Stop();

	/** Check if server is running */
	bool IsRunning() const { return bIsRunning; }

	/** Get server URL */
	FString GetServerUrl() const { return ServerUrl; }

	// ============================================================================
	// MOCK CONFIGURATION
	// ============================================================================

	/** Set global response delay (simulates network latency) */
	void SetResponseDelay(float Seconds);

	/** Set global failure rate (0.0 - 1.0) */
	void SetFailureRate(float Rate);

	/** Enable/disable request logging */
	void SetLoggingEnabled(bool bEnabled);

	/** Reset all mock configurations */
	void Reset();

	// ============================================================================
	// ENDPOINT REGISTRATION
	// ============================================================================

	/** Register mock handler for endpoint */
	void RegisterHandler(const FString& Method, const FString& Endpoint, FDeskillzMockHandler Handler);

	/** Register static response for endpoint */
	void RegisterStaticResponse(const FString& Method, const FString& Endpoint, const FDeskillzMockResponse& Response);

	/** Unregister handler */
	void UnregisterHandler(const FString& Method, const FString& Endpoint);

	/** Clear all handlers */
	void ClearHandlers();

	// ============================================================================
	// REQUEST HANDLING
	// ============================================================================

	/** Process mock request (called by HTTP interceptor) */
	FDeskillzMockResponse ProcessRequest(const FString& Method, const FString& Endpoint, const FString& Body, const TMap<FString, FString>& Headers);

	/** Expect a specific call (for verification) */
	void ExpectCall(const FString& Method, const FString& Endpoint, int32 Times = 1);

	/** Verify all expectations were met */
	bool VerifyExpectations(FString& OutErrors);

	// ============================================================================
	// REQUEST HISTORY
	// ============================================================================

	/** Get all recorded requests */
	const TArray<FDeskillzMockRequest>& GetRequestHistory() const { return RequestHistory; }

	/** Get requests for specific endpoint */
	TArray<FDeskillzMockRequest> GetRequestsForEndpoint(const FString& Endpoint) const;

	/** Get request count for endpoint */
	int32 GetRequestCount(const FString& Method, const FString& Endpoint) const;

	/** Clear request history */
	void ClearHistory();

	/** Get last request */
	const FDeskillzMockRequest* GetLastRequest() const;

	// ============================================================================
	// MOCK DATA GENERATORS
	// ============================================================================

	/** Generate mock player info JSON */
	static FString GeneratePlayerJson(const FString& UserId = TEXT(""));

	/** Generate mock tournament JSON */
	static FString GenerateTournamentJson(const FString& TournamentId = TEXT(""));

	/** Generate mock tournament list JSON */
	static FString GenerateTournamentListJson(int32 Count = 10);

	/** Generate mock match JSON */
	static FString GenerateMatchJson(const FString& MatchId = TEXT(""));

	/** Generate mock wallet balance JSON */
	static FString GenerateWalletJson(const FString& Currency = TEXT("USDT"), double Amount = 100.0);

	/** Generate mock leaderboard JSON */
	static FString GenerateLeaderboardJson(int32 Count = 50);

	/** Generate mock error response */
	static FString GenerateErrorJson(const FString& Code, const FString& Message);

	/** Generate mock auth token */
	static FString GenerateAuthToken();

private:
	FDeskillzMockServer();
	~FDeskillzMockServer();

	// Prevent copying
	FDeskillzMockServer(const FDeskillzMockServer&) = delete;
	FDeskillzMockServer& operator=(const FDeskillzMockServer&) = delete;

	/** Generate handler key */
	FString MakeHandlerKey(const FString& Method, const FString& Endpoint) const;

	/** Check if request should fail based on failure rate */
	bool ShouldFail() const;

	/** Apply response delay */
	void ApplyDelay() const;

	/** Default handlers */
	void RegisterDefaultHandlers();

	// Default handler implementations
	FDeskillzMockResponse HandleAuth(const FString& Method, const FString& Body);
	FDeskillzMockResponse HandleTournaments(const FString& Method, const FString& Body);
	FDeskillzMockResponse HandleMatches(const FString& Method, const FString& Body);
	FDeskillzMockResponse HandleWallet(const FString& Method, const FString& Body);
	FDeskillzMockResponse HandleLeaderboard(const FString& Method, const FString& Body);
	FDeskillzMockResponse HandleAnalytics(const FString& Method, const FString& Body);
	FDeskillzMockResponse HandleScore(const FString& Method, const FString& Body);

private:
	static FDeskillzMockServer* Instance;

	bool bIsRunning;
	FString ServerUrl;
	float ResponseDelay;
	float FailureRate;
	bool bLoggingEnabled;

	TMap<FString, FDeskillzMockHandler> Handlers;
	TMap<FString, FDeskillzMockResponse> StaticResponses;
	TArray<FDeskillzMockRequest> RequestHistory;

	struct FExpectation
	{
		FString Method;
		FString Endpoint;
		int32 ExpectedCount;
		int32 ActualCount;
	};
	TArray<FExpectation> Expectations;

	mutable FCriticalSection CriticalSection;
};

// ============================================================================
// MOCK HTTP INTERCEPTOR
// ============================================================================

/**
 * HTTP Interceptor that redirects requests to mock server
 */
class DESKILLZ_API FDeskillzMockHttpInterceptor
{
public:
	/** Enable interception */
	static void Enable();

	/** Disable interception */
	static void Disable();

	/** Check if enabled */
	static bool IsEnabled();

	/** Process intercepted request */
	static bool TryIntercept(const FString& URL, const FString& Method, const FString& Body, 
		const TMap<FString, FString>& Headers, FDeskillzMockResponse& OutResponse);

private:
	static bool bEnabled;
};

// ============================================================================
// TEST DATA BUILDERS
// ============================================================================

/**
 * Builder for creating mock player data
 */
class DESKILLZ_API FMockPlayerBuilder
{
public:
	FMockPlayerBuilder();

	FMockPlayerBuilder& WithUserId(const FString& Id);
	FMockPlayerBuilder& WithUsername(const FString& Name);
	FMockPlayerBuilder& WithSkillRating(int32 Rating);
	FMockPlayerBuilder& WithWins(int32 Wins);
	FMockPlayerBuilder& WithLosses(int32 Losses);
	FMockPlayerBuilder& WithAvatar(const FString& AvatarUrl);
	FMockPlayerBuilder& WithCountry(const FString& Country);
	FMockPlayerBuilder& AsPremium();

	FDeskillzPlayerInfo Build();
	FString BuildJson();

private:
	FDeskillzPlayerInfo Player;
};

/**
 * Builder for creating mock tournament data
 */
class DESKILLZ_API FMockTournamentBuilder
{
public:
	FMockTournamentBuilder();

	FMockTournamentBuilder& WithId(const FString& Id);
	FMockTournamentBuilder& WithName(const FString& Name);
	FMockTournamentBuilder& WithEntryFee(double Fee, const FString& Currency = TEXT("USDT"));
	FMockTournamentBuilder& WithPrizePool(double Prize);
	FMockTournamentBuilder& WithParticipants(int32 Current, int32 Max);
	FMockTournamentBuilder& WithStatus(EDeskillzTournamentStatus Status);
	FMockTournamentBuilder& WithStartTime(const FDateTime& StartTime);
	FMockTournamentBuilder& WithEndTime(const FDateTime& EndTime);
	FMockTournamentBuilder& WithGameId(const FString& GameId);

	FDeskillzTournament Build();
	FString BuildJson();

private:
	FDeskillzTournament Tournament;
};

/**
 * Builder for creating mock match data
 */
class DESKILLZ_API FMockMatchBuilder
{
public:
	FMockMatchBuilder();

	FMockMatchBuilder& WithId(const FString& Id);
	FMockMatchBuilder& WithTournamentId(const FString& TournamentId);
	FMockMatchBuilder& WithStatus(EDeskillzMatchStatus Status);
	FMockMatchBuilder& WithPlayers(const TArray<FDeskillzPlayerInfo>& Players);
	FMockMatchBuilder& AddPlayer(const FDeskillzPlayerInfo& Player);
	FMockMatchBuilder& WithEntryFee(double Fee);
	FMockMatchBuilder& WithGameMode(const FString& Mode);
	FMockMatchBuilder& Synchronous();
	FMockMatchBuilder& Asynchronous();

	FDeskillzMatch Build();
	FString BuildJson();

private:
	FDeskillzMatch Match;
};

/**
 * Builder for creating mock wallet data
 */
class DESKILLZ_API FMockWalletBuilder
{
public:
	FMockWalletBuilder();

	FMockWalletBuilder& WithCurrency(const FString& Currency);
	FMockWalletBuilder& WithAvailable(double Amount);
	FMockWalletBuilder& WithPending(double Amount);
	FMockWalletBuilder& WithAddress(const FString& Address);

	FDeskillzWalletBalance Build();
	FString BuildJson();

private:
	FDeskillzWalletBalance Balance;
	FString WalletAddress;
};

/**
 * Builder for creating mock transactions
 */
class DESKILLZ_API FMockTransactionBuilder
{
public:
	FMockTransactionBuilder();

	FMockTransactionBuilder& WithId(const FString& Id);
	FMockTransactionBuilder& AsDeposit(double Amount, const FString& Currency);
	FMockTransactionBuilder& AsWithdrawal(double Amount, const FString& Currency);
	FMockTransactionBuilder& AsEntryFee(double Amount, const FString& TournamentId);
	FMockTransactionBuilder& AsPrize(double Amount, const FString& TournamentId);
	FMockTransactionBuilder& WithTimestamp(const FDateTime& Time);
	FMockTransactionBuilder& Pending();
	FMockTransactionBuilder& Completed();
	FMockTransactionBuilder& Failed();

	FString BuildJson();

private:
	TSharedPtr<FJsonObject> TransactionData;
};

// ============================================================================
// SCENARIO HELPERS
// ============================================================================

/**
 * Pre-configured test scenarios
 */
class DESKILLZ_API FDeskillzTestScenarios
{
public:
	/** Setup happy path scenario (all calls succeed) */
	static void SetupHappyPath();

	/** Setup new user scenario */
	static void SetupNewUserScenario();

	/** Setup returning user scenario */
	static void SetupReturningUserScenario();

	/** Setup network error scenario */
	static void SetupNetworkErrorScenario();

	/** Setup authentication failure scenario */
	static void SetupAuthFailureScenario();

	/** Setup insufficient funds scenario */
	static void SetupInsufficientFundsScenario();

	/** Setup tournament full scenario */
	static void SetupTournamentFullScenario();

	/** Setup high latency scenario */
	static void SetupHighLatencyScenario(float LatencySeconds = 2.0f);

	/** Setup intermittent failure scenario */
	static void SetupIntermittentFailureScenario(float FailureRate = 0.3f);
};
