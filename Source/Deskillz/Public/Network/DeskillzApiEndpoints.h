// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Deskillz API Endpoints
 * 
 * Centralized definition of all REST API endpoints.
 * All endpoints are relative to the base URL.
 * 
 * API Version: v1
 * 
 * Categories:
 * - Auth: Authentication and session management
 * - User: User profiles and settings
 * - Tournament: Tournament operations
 * - Match: Match lifecycle
 * - Wallet: Cryptocurrency wallet operations
 * - Leaderboard: Rankings and stats
 * - Developer: SDK/Developer portal
 */
namespace DeskillzApi
{
	/** API Version */
	static const FString Version = TEXT("v1");
	
	/** Base path for all endpoints */
	static const FString BasePath = TEXT("/api/v1");
	
	// ========================================================================
	// Authentication Endpoints
	// ========================================================================
	
	namespace Auth
	{
		/** Login with credentials */
		static const FString Login = TEXT("/api/v1/auth/login");
		
		/** Register new account */
		static const FString Register = TEXT("/api/v1/auth/register");
		
		/** Logout current session */
		static const FString Logout = TEXT("/api/v1/auth/logout");
		
		/** Refresh auth token */
		static const FString RefreshToken = TEXT("/api/v1/auth/refresh");
		
		/** Verify token validity */
		static const FString VerifyToken = TEXT("/api/v1/auth/verify");
		
		/** Request password reset */
		static const FString ForgotPassword = TEXT("/api/v1/auth/forgot-password");
		
		/** Reset password with token */
		static const FString ResetPassword = TEXT("/api/v1/auth/reset-password");
		
		/** OAuth callback */
		static const FString OAuthCallback = TEXT("/api/v1/auth/oauth/callback");
		
		/** Connect wallet for auth */
		static const FString WalletConnect = TEXT("/api/v1/auth/wallet/connect");
		
		/** Verify wallet signature */
		static const FString WalletVerify = TEXT("/api/v1/auth/wallet/verify");
	}
	
	// ========================================================================
	// User Endpoints
	// ========================================================================
	
	namespace User
	{
		/** Get current user profile */
		static const FString Me = TEXT("/api/v1/users/me");
		
		/** Update current user profile */
		static const FString UpdateMe = TEXT("/api/v1/users/me");
		
		/** Get user by ID */
		static FString GetById(const FString& UserId)
		{
			return FString::Printf(TEXT("/api/v1/users/%s"), *UserId);
		}
		
		/** Get user's public profile */
		static FString GetPublicProfile(const FString& UserId)
		{
			return FString::Printf(TEXT("/api/v1/users/%s/profile"), *UserId);
		}
		
		/** Get user stats */
		static FString GetStats(const FString& UserId)
		{
			return FString::Printf(TEXT("/api/v1/users/%s/stats"), *UserId);
		}
		
		/** Get user match history */
		static FString GetMatchHistory(const FString& UserId)
		{
			return FString::Printf(TEXT("/api/v1/users/%s/matches"), *UserId);
		}
		
		/** Update avatar */
		static const FString UpdateAvatar = TEXT("/api/v1/users/me/avatar");
		
		/** Get user settings */
		static const FString Settings = TEXT("/api/v1/users/me/settings");
		
		/** Update notification preferences */
		static const FString Notifications = TEXT("/api/v1/users/me/notifications");
		
		/** Block a user */
		static FString BlockUser(const FString& UserId)
		{
			return FString::Printf(TEXT("/api/v1/users/%s/block"), *UserId);
		}
		
		/** Report a user */
		static FString ReportUser(const FString& UserId)
		{
			return FString::Printf(TEXT("/api/v1/users/%s/report"), *UserId);
		}
	}
	
	// ========================================================================
	// Tournament Endpoints
	// ========================================================================
	
	namespace Tournament
	{
		/** List all tournaments */
		static const FString List = TEXT("/api/v1/tournaments");
		
		/** Get featured tournaments */
		static const FString Featured = TEXT("/api/v1/tournaments/featured");
		
		/** Get tournament by ID */
		static FString GetById(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/api/v1/tournaments/%s"), *TournamentId);
		}
		
		/** Enter tournament */
		static FString Enter(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/api/v1/tournaments/%s/enter"), *TournamentId);
		}
		
		/** Leave tournament */
		static FString Leave(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/api/v1/tournaments/%s/leave"), *TournamentId);
		}
		
		/** Get tournament leaderboard */
		static FString Leaderboard(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/api/v1/tournaments/%s/leaderboard"), *TournamentId);
		}
		
		/** Get tournament participants */
		static FString Participants(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/api/v1/tournaments/%s/participants"), *TournamentId);
		}
		
		/** Get tournament results */
		static FString Results(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/api/v1/tournaments/%s/results"), *TournamentId);
		}
		
		/** Get tournaments by game */
		static FString ByGame(const FString& GameId)
		{
			return FString::Printf(TEXT("/api/v1/games/%s/tournaments"), *GameId);
		}
		
		/** Get my active tournaments */
		static const FString MyActive = TEXT("/api/v1/tournaments/my/active");
		
		/** Get my tournament history */
		static const FString MyHistory = TEXT("/api/v1/tournaments/my/history");
	}
	
	// ========================================================================
	// Match Endpoints
	// ========================================================================
	
	namespace Match
	{
		/** Find match (matchmaking) */
		static const FString Find = TEXT("/api/v1/matches/find");
		
		/** Cancel matchmaking */
		static const FString CancelFind = TEXT("/api/v1/matches/cancel");
		
		/** Get match by ID */
		static FString GetById(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s"), *MatchId);
		}
		
		/** Start match */
		static FString Start(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s/start"), *MatchId);
		}
		
		/** Submit score */
		static FString SubmitScore(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s/score"), *MatchId);
		}
		
		/** Complete match */
		static FString Complete(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s/complete"), *MatchId);
		}
		
		/** Abort match */
		static FString Abort(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s/abort"), *MatchId);
		}
		
		/** Get match result */
		static FString Result(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s/result"), *MatchId);
		}
		
		/** Report match issue */
		static FString Report(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s/report"), *MatchId);
		}
		
		/** Get current active match */
		static const FString Current = TEXT("/api/v1/matches/current");
		
		/** Sync match state (for async) */
		static FString SyncState(const FString& MatchId)
		{
			return FString::Printf(TEXT("/api/v1/matches/%s/sync"), *MatchId);
		}
	}
	
	// ========================================================================
	// Wallet Endpoints
	// ========================================================================
	
	namespace Wallet
	{
		/** Get all balances */
		static const FString Balances = TEXT("/api/v1/wallet/balances");
		
		/** Get balance for currency */
		static FString Balance(const FString& Currency)
		{
			return FString::Printf(TEXT("/api/v1/wallet/balances/%s"), *Currency);
		}
		
		/** Get deposit address */
		static FString DepositAddress(const FString& Currency)
		{
			return FString::Printf(TEXT("/api/v1/wallet/deposit/%s"), *Currency);
		}
		
		/** Request withdrawal */
		static const FString Withdraw = TEXT("/api/v1/wallet/withdraw");
		
		/** Get withdrawal status */
		static FString WithdrawStatus(const FString& WithdrawalId)
		{
			return FString::Printf(TEXT("/api/v1/wallet/withdraw/%s"), *WithdrawalId);
		}
		
		/** Get transaction history */
		static const FString Transactions = TEXT("/api/v1/wallet/transactions");
		
		/** Get transaction by ID */
		static FString Transaction(const FString& TransactionId)
		{
			return FString::Printf(TEXT("/api/v1/wallet/transactions/%s"), *TransactionId);
		}
		
		/** Get supported currencies */
		static const FString Currencies = TEXT("/api/v1/wallet/currencies");
		
		/** Get exchange rates */
		static const FString ExchangeRates = TEXT("/api/v1/wallet/rates");
		
		/** Convert currency */
		static const FString Convert = TEXT("/api/v1/wallet/convert");
	}
	
	// ========================================================================
	// Leaderboard Endpoints
	// ========================================================================
	
	namespace Leaderboard
	{
		/** Get global leaderboard */
		static const FString Global = TEXT("/api/v1/leaderboards/global");
		
		/** Get leaderboard by game */
		static FString ByGame(const FString& GameId)
		{
			return FString::Printf(TEXT("/api/v1/games/%s/leaderboard"), *GameId);
		}
		
		/** Get leaderboard by tournament */
		static FString ByTournament(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/api/v1/tournaments/%s/leaderboard"), *TournamentId);
		}
		
		/** Get user's rank */
		static FString UserRank(const FString& UserId)
		{
			return FString::Printf(TEXT("/api/v1/leaderboards/rank/%s"), *UserId);
		}
		
		/** Get nearby ranks (around user) */
		static const FString Nearby = TEXT("/api/v1/leaderboards/nearby");
		
		/** Get friends leaderboard */
		static const FString Friends = TEXT("/api/v1/leaderboards/friends");
	}
	
	// ========================================================================
	// Game Endpoints
	// ========================================================================
	
	namespace Game
	{
		/** List all games */
		static const FString List = TEXT("/api/v1/games");
		
		/** Get game by ID */
		static FString GetById(const FString& GameId)
		{
			return FString::Printf(TEXT("/api/v1/games/%s"), *GameId);
		}
		
		/** Get game config */
		static FString Config(const FString& GameId)
		{
			return FString::Printf(TEXT("/api/v1/games/%s/config"), *GameId);
		}
		
		/** Get featured games */
		static const FString Featured = TEXT("/api/v1/games/featured");
		
		/** Search games */
		static const FString Search = TEXT("/api/v1/games/search");
	}
	
	// ========================================================================
	// Developer Endpoints
	// ========================================================================
	
	namespace Developer
	{
		/** Register as developer */
		static const FString Register = TEXT("/api/v1/developer/register");
		
		/** Get developer profile */
		static const FString Profile = TEXT("/api/v1/developer/profile");
		
		/** List developer's games */
		static const FString MyGames = TEXT("/api/v1/developer/games");
		
		/** Create new game */
		static const FString CreateGame = TEXT("/api/v1/developer/games");
		
		/** Update game */
		static FString UpdateGame(const FString& GameId)
		{
			return FString::Printf(TEXT("/api/v1/developer/games/%s"), *GameId);
		}
		
		/** Get game analytics */
		static FString Analytics(const FString& GameId)
		{
			return FString::Printf(TEXT("/api/v1/developer/games/%s/analytics"), *GameId);
		}
		
		/** Get API keys */
		static const FString ApiKeys = TEXT("/api/v1/developer/api-keys");
		
		/** Generate new API key */
		static const FString GenerateKey = TEXT("/api/v1/developer/api-keys/generate");
		
		/** Revoke API key */
		static FString RevokeKey(const FString& KeyId)
		{
			return FString::Printf(TEXT("/api/v1/developer/api-keys/%s/revoke"), *KeyId);
		}
		
		/** Get earnings report */
		static const FString Earnings = TEXT("/api/v1/developer/earnings");
		
		/** Get payout history */
		static const FString Payouts = TEXT("/api/v1/developer/payouts");
	}
	
	// ========================================================================
	// WebSocket Endpoints
	// ========================================================================
	
	namespace WebSocket
	{
		/** Main WebSocket endpoint */
		static const FString Main = TEXT("/ws");
		
		/** Match WebSocket */
		static FString Match(const FString& MatchId)
		{
			return FString::Printf(TEXT("/ws/match/%s"), *MatchId);
		}
		
		/** Tournament WebSocket */
		static FString Tournament(const FString& TournamentId)
		{
			return FString::Printf(TEXT("/ws/tournament/%s"), *TournamentId);
		}
	}
	
	// ========================================================================
	// Utility Functions
	// ========================================================================
	
	/** Build full URL from base and endpoint */
	inline FString BuildUrl(const FString& BaseUrl, const FString& Endpoint)
	{
		FString Url = BaseUrl;
		if (Url.EndsWith(TEXT("/")))
		{
			Url = Url.LeftChop(1);
		}
		return Url + Endpoint;
	}
	
	/** Add query parameter */
	inline FString WithQuery(const FString& Endpoint, const FString& Key, const FString& Value)
	{
		FString Separator = Endpoint.Contains(TEXT("?")) ? TEXT("&") : TEXT("?");
		return Endpoint + Separator + Key + TEXT("=") + Value;
	}
	
	/** Add pagination */
	inline FString WithPagination(const FString& Endpoint, int32 Page, int32 Limit)
	{
		FString Result = WithQuery(Endpoint, TEXT("page"), FString::FromInt(Page));
		return WithQuery(Result, TEXT("limit"), FString::FromInt(Limit));
	}
}
