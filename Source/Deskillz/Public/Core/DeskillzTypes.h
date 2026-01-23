// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "DeskillzTypes.generated.h"

// ============================================================================
// SDK State & Environment
// ============================================================================

/**
 * SDK initialization states
 */
UENUM(BlueprintType)
enum class EDeskillzSDKState : uint8
{
	Uninitialized    UMETA(DisplayName = "Uninitialized"),
	Initializing     UMETA(DisplayName = "Initializing"),
	Initialized      UMETA(DisplayName = "Initialized"),
	Error            UMETA(DisplayName = "Error"),
	Disabled         UMETA(DisplayName = "Disabled")
};

/**
 * SDK environment (production vs sandbox)
 */
UENUM(BlueprintType)
enum class EDeskillzEnvironment : uint8
{
	Production       UMETA(DisplayName = "Production"),
	Sandbox          UMETA(DisplayName = "Sandbox"),
	Development      UMETA(DisplayName = "Development")
};

// ============================================================================
// Cryptocurrency & Wallet
// ============================================================================

/**
 * Supported cryptocurrencies for entry fees and prizes
 */
UENUM(BlueprintType)
enum class EDeskillzCurrency : uint8
{
	BNB              UMETA(DisplayName = "BNB (BSC)"),
	USDT_BSC         UMETA(DisplayName = "USDT (BSC)"),
	USDT_TRON        UMETA(DisplayName = "USDT (TRON)"),
	USDC_BSC         UMETA(DisplayName = "USDC (BSC)"),
	USDC_TRON        UMETA(DisplayName = "USDC (TRON)"),
	Free             UMETA(DisplayName = "Free Entry")
};

/**
 * Wallet connection status
 */
UENUM(BlueprintType)
enum class EDeskillzWalletStatus : uint8
{
	Disconnected     UMETA(DisplayName = "Disconnected"),
	Connecting       UMETA(DisplayName = "Connecting"),
	Connected        UMETA(DisplayName = "Connected"),
	Error            UMETA(DisplayName = "Error")
};

/**
 * Wallet balance information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzWalletBalance
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	EDeskillzCurrency Currency = EDeskillzCurrency::USDT;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	double Amount = 0.0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	double PendingAmount = 0.0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Wallet")
	FString FormattedAmount;
	
	FDeskillzWalletBalance() = default;
	
	FDeskillzWalletBalance(EDeskillzCurrency InCurrency, double InAmount)
		: Currency(InCurrency), Amount(InAmount)
	{
		UpdateFormattedAmount();
	}
	
	void UpdateFormattedAmount()
	{
		FormattedAmount = FString::Printf(TEXT("%.6f"), Amount);
	}
};

// ============================================================================
// Tournament & Match Types
// ============================================================================

/**
 * Tournament status
 */
UENUM(BlueprintType)
enum class EDeskillzTournamentStatus : uint8
{
	Upcoming         UMETA(DisplayName = "Upcoming"),
	Open             UMETA(DisplayName = "Open for Entry"),
	InProgress       UMETA(DisplayName = "In Progress"),
	Completed        UMETA(DisplayName = "Completed"),
	Cancelled        UMETA(DisplayName = "Cancelled")
};

/**
 * Match type (sync vs async)
 */
UENUM(BlueprintType)
enum class EDeskillzMatchType : uint8
{
	Synchronous      UMETA(DisplayName = "Synchronous (Real-time)"),
	Asynchronous     UMETA(DisplayName = "Asynchronous (Turn-based)")
};

/**
 * Match status
 */
UENUM(BlueprintType)
enum class EDeskillzMatchStatus : uint8
{
	Pending          UMETA(DisplayName = "Pending"),
	Matchmaking      UMETA(DisplayName = "Finding Opponent"),
	Ready            UMETA(DisplayName = "Ready to Start"),
	InProgress       UMETA(DisplayName = "In Progress"),
	Completed        UMETA(DisplayName = "Completed"),
	Cancelled        UMETA(DisplayName = "Cancelled"),
	Disputed         UMETA(DisplayName = "Disputed"),
	Expired          UMETA(DisplayName = "Expired")
};

/**
 * Match result
 */
UENUM(BlueprintType)
enum class EDeskillzMatchResult : uint8
{
	Pending          UMETA(DisplayName = "Pending"),
	Win              UMETA(DisplayName = "Win"),
	Loss             UMETA(DisplayName = "Loss"),
	Draw             UMETA(DisplayName = "Draw"),
	Forfeit          UMETA(DisplayName = "Forfeit"),
	Disqualified     UMETA(DisplayName = "Disqualified")
};

/**
 * Player information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzPlayer
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	FString PlayerId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	FString Username;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	FString AvatarUrl;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	int32 Rating = 1000;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	int32 GamesPlayed = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	int32 Wins = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	float WinRate = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Player")
	bool bIsCurrentUser = false;
	
	FDeskillzPlayer() = default;
};

/**
 * Tournament entry fee
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzEntryFee
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	EDeskillzCurrency Currency = EDeskillzCurrency::USDT;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	double Amount = 0.0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString FormattedAmount;
	
	FDeskillzEntryFee() = default;
	
	FDeskillzEntryFee(EDeskillzCurrency InCurrency, double InAmount)
		: Currency(InCurrency), Amount(InAmount)
	{
		FormattedAmount = FString::Printf(TEXT("%.2f %s"), Amount, *GetCurrencySymbol());
	}
	
	FString GetCurrencySymbol() const
	{
		switch (Currency)
		{
			case EDeskillzCurrency::BTC:  return TEXT("BTC");
			case EDeskillzCurrency::ETH:  return TEXT("ETH");
			case EDeskillzCurrency::SOL:  return TEXT("SOL");
			case EDeskillzCurrency::XRP:  return TEXT("XRP");
			case EDeskillzCurrency::BNB:  return TEXT("BNB");
			case EDeskillzCurrency::USDT: return TEXT("USDT");
			case EDeskillzCurrency::USDC: return TEXT("USDC");
			default: return TEXT("CREDITS");
		}
	}
};

/**
 * Tournament information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTournament
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString TournamentId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Name;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString Description;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FString GameId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	EDeskillzTournamentStatus Status = EDeskillzTournamentStatus::Upcoming;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	EDeskillzMatchType MatchType = EDeskillzMatchType::Asynchronous;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FDeskillzEntryFee EntryFee;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	double PrizePool = 0.0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 MaxPlayers = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 CurrentPlayers = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FDateTime StartTime;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	FDateTime EndTime;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	int32 MatchDurationSeconds = 180;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Tournament")
	TMap<FString, FString> CustomRules;
	
	FDeskillzTournament() = default;
	
	bool IsOpen() const { return Status == EDeskillzTournamentStatus::Open; }
	bool IsFull() const { return MaxPlayers > 0 && CurrentPlayers >= MaxPlayers; }
	bool CanJoin() const { return IsOpen() && !IsFull(); }
};

/**
 * Match session information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString MatchId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString TournamentId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	EDeskillzMatchType MatchType = EDeskillzMatchType::Asynchronous;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	EDeskillzMatchStatus Status = EDeskillzMatchStatus::Pending;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FDeskillzPlayer LocalPlayer;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FDeskillzPlayer Opponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FDateTime StartTime;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int32 DurationSeconds = 180;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int64 RandomSeed = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	TMap<FString, FString> CustomData;
	
	FDeskillzMatchInfo() = default;
	
	bool IsInProgress() const { return Status == EDeskillzMatchStatus::InProgress; }
	bool IsSynchronous() const { return MatchType == EDeskillzMatchType::Synchronous; }
};

/**
 * Match result with scores
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzMatchResult
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	FString MatchId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	EDeskillzMatchResult Result = EDeskillzMatchResult::Pending;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int64 PlayerScore = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int64 OpponentScore = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	double PrizeWon = 0.0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	EDeskillzCurrency PrizeCurrency = EDeskillzCurrency::USDT;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int32 RatingChange = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int32 NewRating = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int32 Rank = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Match")
	int32 TotalParticipants = 0;
	
	FDeskillzMatchResult() = default;
	
	bool IsWin() const { return Result == EDeskillzMatchResult::Win; }
	bool IsLoss() const { return Result == EDeskillzMatchResult::Loss; }
	bool IsDraw() const { return Result == EDeskillzMatchResult::Draw; }
};

// ============================================================================
// Error & Response Types
// ============================================================================

/**
 * Error codes for SDK operations
 */
UENUM(BlueprintType)
enum class EDeskillzErrorCode : uint8
{
	None                     UMETA(DisplayName = "No Error"),
	NetworkError             UMETA(DisplayName = "Network Error"),
	AuthenticationFailed     UMETA(DisplayName = "Authentication Failed"),
	InvalidAPIKey            UMETA(DisplayName = "Invalid API Key"),
	InvalidGameId            UMETA(DisplayName = "Invalid Game ID"),
	InsufficientFunds        UMETA(DisplayName = "Insufficient Funds"),
	TournamentFull           UMETA(DisplayName = "Tournament Full"),
	TournamentClosed         UMETA(DisplayName = "Tournament Closed"),
	MatchNotFound            UMETA(DisplayName = "Match Not Found"),
	MatchExpired             UMETA(DisplayName = "Match Expired"),
	InvalidScore             UMETA(DisplayName = "Invalid Score"),
	ScoreVerificationFailed  UMETA(DisplayName = "Score Verification Failed"),
	RateLimited              UMETA(DisplayName = "Rate Limited"),
	ServerError              UMETA(DisplayName = "Server Error"),
	Unknown                  UMETA(DisplayName = "Unknown Error")
};

/**
 * SDK error information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzError
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Error")
	EDeskillzErrorCode Code = EDeskillzErrorCode::None;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Error")
	FString Message;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Error")
	FString Details;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Error")
	int32 HttpStatusCode = 0;
	
	FDeskillzError() = default;
	
	FDeskillzError(EDeskillzErrorCode InCode, const FString& InMessage)
		: Code(InCode), Message(InMessage) {}
	
	bool IsError() const { return Code != EDeskillzErrorCode::None; }
	bool IsNetworkError() const { return Code == EDeskillzErrorCode::NetworkError; }
	bool IsAuthError() const { return Code == EDeskillzErrorCode::AuthenticationFailed || Code == EDeskillzErrorCode::InvalidAPIKey; }
	
	static FDeskillzError None() { return FDeskillzError(); }
	static FDeskillzError NetworkError(const FString& Details = TEXT("")) 
	{ 
		FDeskillzError Err(EDeskillzErrorCode::NetworkError, TEXT("Network connection failed"));
		Err.Details = Details;
		return Err;
	}
};

// ============================================================================
// Leaderboard Types
// ============================================================================

/**
 * Leaderboard time period
 */
UENUM(BlueprintType)
enum class EDeskillzLeaderboardPeriod : uint8
{
	Daily            UMETA(DisplayName = "Daily"),
	Weekly           UMETA(DisplayName = "Weekly"),
	Monthly          UMETA(DisplayName = "Monthly"),
	AllTime          UMETA(DisplayName = "All Time")
};

/**
 * Leaderboard entry
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzLeaderboardEntry
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Leaderboard")
	int32 Rank = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Leaderboard")
	FDeskillzPlayer Player;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Leaderboard")
	int64 Score = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Leaderboard")
	double Earnings = 0.0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Leaderboard")
	int32 MatchesWon = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Leaderboard")
	int32 MatchesPlayed = 0;
	
	FDeskillzLeaderboardEntry() = default;
};
// ============================================================================
// Authentication Types (Self-Sufficient Architecture)
// ============================================================================

/**
 * Authentication state
 */
UENUM(BlueprintType)
enum class EDeskillzAuthState : uint8
{
    NotAuthenticated    UMETA(DisplayName = "Not Authenticated"),
    Authenticating      UMETA(DisplayName = "Authenticating"),
    Authenticated       UMETA(DisplayName = "Authenticated"),
    Error               UMETA(DisplayName = "Authentication Error")
};

/**
 * Social auth provider
 */
UENUM(BlueprintType)
enum class EDeskillzAuthProvider : uint8
{
    Email               UMETA(DisplayName = "Email/Password"),
    Google              UMETA(DisplayName = "Google"),
    Apple               UMETA(DisplayName = "Apple"),
    Facebook            UMETA(DisplayName = "Facebook"),
    Wallet              UMETA(DisplayName = "Wallet (SIWE)")
};

/**
 * User role
 */
UENUM(BlueprintType)
enum class EDeskillzUserRole : uint8
{
    Player              UMETA(DisplayName = "Player"),
    Developer           UMETA(DisplayName = "Developer"),
    Admin               UMETA(DisplayName = "Admin")
};

/**
 * Authenticated user information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzAuthUser
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString Id;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString Email;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString Username;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString DisplayName;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString AvatarUrl;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    EDeskillzUserRole Role = EDeskillzUserRole::Player;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    bool bEmailVerified = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    bool bHasWallet = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString WalletAddress;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FDateTime CreatedAt;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FDateTime LastLoginAt;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    bool bTwoFactorEnabled = false;
    
    FDeskillzAuthUser() = default;
    
    bool IsDeveloper() const { return Role == EDeskillzUserRole::Developer || Role == EDeskillzUserRole::Admin; }
    bool IsAdmin() const { return Role == EDeskillzUserRole::Admin; }
    bool IsValid() const { return !Id.IsEmpty(); }
};

/**
 * Login request data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzLoginRequest
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Email;
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Password;
    
    FDeskillzLoginRequest() = default;
    
    FDeskillzLoginRequest(const FString& InEmail, const FString& InPassword)
        : Email(InEmail), Password(InPassword) {}
};

/**
 * Sign up request data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSignUpRequest
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Email;
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Password;
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Username;
    
    FDeskillzSignUpRequest() = default;
    
    FDeskillzSignUpRequest(const FString& InEmail, const FString& InPassword, const FString& InUsername)
        : Email(InEmail), Password(InPassword), Username(InUsername) {}
};

/**
 * Authentication response
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzAuthResponse
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString AccessToken;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FString RefreshToken;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    FDeskillzAuthUser User;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    bool bIsNewUser = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Auth")
    int64 ExpiresIn = 0;
    
    FDeskillzAuthResponse() = default;
    
    bool IsValid() const { return !AccessToken.IsEmpty() && User.IsValid(); }
};

/**
 * Social auth request data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSocialAuthRequest
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    EDeskillzAuthProvider Provider = EDeskillzAuthProvider::Google;
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString IdToken;
    
    FDeskillzSocialAuthRequest() = default;
    
    FDeskillzSocialAuthRequest(EDeskillzAuthProvider InProvider, const FString& InIdToken)
        : Provider(InProvider), IdToken(InIdToken) {}
    
    FString GetProviderName() const
    {
        switch (Provider)
        {
            case EDeskillzAuthProvider::Google: return TEXT("google");
            case EDeskillzAuthProvider::Apple: return TEXT("apple");
            case EDeskillzAuthProvider::Facebook: return TEXT("facebook");
            default: return TEXT("email");
        }
    }
};

/**
 * Wallet link request data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzWalletLinkRequest
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString WalletAddress;
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Signature;
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Message;
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Nonce;
    
    FDeskillzWalletLinkRequest() = default;
    
    FDeskillzWalletLinkRequest(const FString& InAddress, const FString& InSignature, 
                                const FString& InMessage, const FString& InNonce)
        : WalletAddress(InAddress), Signature(InSignature), Message(InMessage), Nonce(InNonce) {}
};

/**
 * Forgot password request
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzForgotPasswordRequest
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "Deskillz|Auth")
    FString Email;
    
    FDeskillzForgotPasswordRequest() = default;
    
    FDeskillzForgotPasswordRequest(const FString& InEmail)
        : Email(InEmail) {}
};

// ============================================================================
// Authentication Delegates
// ============================================================================

/** Called when login succeeds */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzLoginSuccess, const FDeskillzAuthUser&, User);

/** Called when logout occurs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzLogout);

/** Called when auth error occurs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzAuthError, const FString&, ErrorMessage);

/** Called when wallet is connected to account */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzWalletConnected, const FString&, WalletAddress);

/** Called when wallet is disconnected from account */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzWalletDisconnected);

/** Called when sign up succeeds */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzSignUpSuccess, const FDeskillzAuthUser&, User);

/** Called when password reset email is sent */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzPasswordResetSent);

/** Called when auth state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzAuthStateChangedNew, EDeskillzAuthState, NewState, const FDeskillzAuthUser&, User);
// ============================================================================
// Callback Delegates
// ============================================================================

/** Delegate for SDK initialization complete */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzInitialized, bool, bSuccess, const FDeskillzError&, Error);

/** Delegate for tournament list received */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzTournamentsReceived, const TArray<FDeskillzTournament>&, Tournaments, const FDeskillzError&, Error);

/** Delegate for match started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzMatchStarted, const FDeskillzMatchInfo&, MatchInfo, const FDeskillzError&, Error);

/** Delegate for match completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzMatchCompleted, const FDeskillzMatchResult&, Result, const FDeskillzError&, Error);

/** Delegate for score submitted */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzScoreSubmitted, bool, bSuccess, const FDeskillzError&, Error);

/** Delegate for wallet balance updated */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzWalletUpdated, const TArray<FDeskillzWalletBalance>&, Balances, const FDeskillzError&, Error);

/** Delegate for leaderboard received */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzLeaderboardReceived, const TArray<FDeskillzLeaderboardEntry>&, Entries, const FDeskillzError&, Error);

/** Delegate for generic errors */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzError, const FDeskillzError&, Error);
