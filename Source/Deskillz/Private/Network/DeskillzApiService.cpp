// Copyright Deskillz Games. All Rights Reserved.

#include "Network/DeskillzApiService.h"
#include "Network/DeskillzHttpClient.h"
#include "Network/DeskillzApiEndpoints.h"
#include "Deskillz.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// Static singleton
static UDeskillzApiService* GApiService = nullptr;

UDeskillzApiService::UDeskillzApiService()
{
}

UDeskillzApiService* UDeskillzApiService::Get()
{
	if (!GApiService)
	{
		GApiService = NewObject<UDeskillzApiService>();
		GApiService->AddToRoot();
		GApiService->Http = UDeskillzHttpClient::Get();
	}
	return GApiService;
}

// ============================================================================
// Authentication
// ============================================================================

void UDeskillzApiService::Login(const FString& Email, const FString& Password, const FOnDeskillzAuthResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("email"), Email);
	Body->SetStringField(TEXT("password"), Password);
	
	Http->PostJson(DeskillzApi::Auth::Login, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FString Token = Json->GetStringField(TEXT("access_token"));
					OnComplete.ExecuteIfBound(true, Token, TEXT(""));
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, TEXT(""), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::Register(const FString& Email, const FString& Password, const FString& Username,
	const FOnDeskillzAuthResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("email"), Email);
	Body->SetStringField(TEXT("password"), Password);
	Body->SetStringField(TEXT("username"), Username);
	
	Http->PostJson(DeskillzApi::Auth::Register, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FString Token = Json->GetStringField(TEXT("access_token"));
					OnComplete.ExecuteIfBound(true, Token, TEXT(""));
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, TEXT(""), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::Logout(const FOnDeskillzApiResult& OnComplete)
{
	Http->Post(DeskillzApi::Auth::Logout, TEXT(""),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::ConnectWallet(const FString& WalletAddress, const FString& Signature,
	const FOnDeskillzAuthResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("wallet_address"), WalletAddress);
	Body->SetStringField(TEXT("signature"), Signature);
	
	Http->PostJson(DeskillzApi::Auth::WalletVerify, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FString Token = Json->GetStringField(TEXT("access_token"));
					OnComplete.ExecuteIfBound(true, Token, TEXT(""));
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, TEXT(""), Response.ErrorMessage);
		})
	);
}

// ============================================================================
// User
// ============================================================================

void UDeskillzApiService::GetCurrentUser(const FOnDeskillzUserLoaded& OnComplete)
{
	Http->Get(DeskillzApi::User::Me,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FDeskillzPlayerInfo User = ParseUser(Json);
					OnComplete.ExecuteIfBound(true, User);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, FDeskillzPlayerInfo());
		})
	);
}

void UDeskillzApiService::GetUser(const FString& UserId, const FOnDeskillzUserLoaded& OnComplete)
{
	Http->Get(DeskillzApi::User::GetById(UserId),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FDeskillzPlayerInfo User = ParseUser(Json);
					OnComplete.ExecuteIfBound(true, User);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, FDeskillzPlayerInfo());
		})
	);
}

void UDeskillzApiService::UpdateProfile(const FString& Username, const FString& AvatarUrl,
	const FOnDeskillzApiResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("username"), Username);
	if (!AvatarUrl.IsEmpty())
	{
		Body->SetStringField(TEXT("avatar_url"), AvatarUrl);
	}
	
	Http->PostJson(DeskillzApi::User::UpdateMe, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

// ============================================================================
// Tournaments
// ============================================================================

void UDeskillzApiService::GetTournaments(const FOnDeskillzTournamentsLoaded& OnComplete,
	const FString& GameId, bool bFeaturedOnly, int32 Limit)
{
	FString Endpoint = bFeaturedOnly ? DeskillzApi::Tournament::Featured : DeskillzApi::Tournament::List;
	
	TMap<FString, FString> QueryParams;
	if (!GameId.IsEmpty())
	{
		QueryParams.Add(TEXT("game_id"), GameId);
	}
	QueryParams.Add(TEXT("limit"), FString::FromInt(Limit));
	
	Http->Get(Endpoint,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			TArray<FDeskillzTournament> Tournaments;
			
			if (Response.IsOk())
			{
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				if (UDeskillzHttpClient::ParseJsonArrayResponse(Response, JsonArray))
				{
					for (const TSharedPtr<FJsonValue>& Value : JsonArray)
					{
						if (TSharedPtr<FJsonObject> JsonObj = Value->AsObject())
						{
							Tournaments.Add(ParseTournament(JsonObj));
						}
					}
				}
				
				OnComplete.ExecuteIfBound(true, Tournaments);
				return;
			}
			
			OnComplete.ExecuteIfBound(false, Tournaments);
		}),
		QueryParams
	);
}

void UDeskillzApiService::GetTournament(const FString& TournamentId, const FOnDeskillzTournamentLoaded& OnComplete)
{
	Http->Get(DeskillzApi::Tournament::GetById(TournamentId),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FDeskillzTournament Tournament = ParseTournament(Json);
					OnComplete.ExecuteIfBound(true, Tournament);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, FDeskillzTournament());
		})
	);
}

void UDeskillzApiService::EnterTournament(const FString& TournamentId, const FString& Currency,
	const FOnDeskillzApiResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("currency"), Currency);
	
	Http->PostJson(DeskillzApi::Tournament::Enter(TournamentId), Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::LeaveTournament(const FString& TournamentId, const FOnDeskillzApiResult& OnComplete)
{
	Http->Post(DeskillzApi::Tournament::Leave(TournamentId), TEXT(""),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::GetMyTournaments(const FOnDeskillzTournamentsLoaded& OnComplete)
{
	Http->Get(DeskillzApi::Tournament::MyActive,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			TArray<FDeskillzTournament> Tournaments;
			
			if (Response.IsOk())
			{
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				if (UDeskillzHttpClient::ParseJsonArrayResponse(Response, JsonArray))
				{
					for (const TSharedPtr<FJsonValue>& Value : JsonArray)
					{
						if (TSharedPtr<FJsonObject> JsonObj = Value->AsObject())
						{
							Tournaments.Add(ParseTournament(JsonObj));
						}
					}
				}
				
				OnComplete.ExecuteIfBound(true, Tournaments);
				return;
			}
			
			OnComplete.ExecuteIfBound(false, Tournaments);
		})
	);
}

// ============================================================================
// Matches
// ============================================================================

void UDeskillzApiService::FindMatch(const FString& TournamentId, const FOnDeskillzMatchLoaded& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("tournament_id"), TournamentId);
	
	Http->PostJson(DeskillzApi::Match::Find, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FDeskillzMatch Match = ParseMatch(Json);
					OnComplete.ExecuteIfBound(true, Match);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, FDeskillzMatch());
		})
	);
}

void UDeskillzApiService::CancelMatchmaking(const FOnDeskillzApiResult& OnComplete)
{
	Http->Post(DeskillzApi::Match::CancelFind, TEXT(""),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::GetMatch(const FString& MatchId, const FOnDeskillzMatchLoaded& OnComplete)
{
	Http->Get(DeskillzApi::Match::GetById(MatchId),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FDeskillzMatch Match = ParseMatch(Json);
					OnComplete.ExecuteIfBound(true, Match);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, FDeskillzMatch());
		})
	);
}

void UDeskillzApiService::StartMatch(const FString& MatchId, const FOnDeskillzApiResult& OnComplete)
{
	Http->Post(DeskillzApi::Match::Start(MatchId), TEXT(""),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::SubmitScore(const FString& MatchId, int64 Score, const FString& ScoreProof,
	const FOnDeskillzApiResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetNumberField(TEXT("score"), static_cast<double>(Score));
	Body->SetStringField(TEXT("proof"), ScoreProof);
	Body->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp() * 1000);
	
	Http->PostJson(DeskillzApi::Match::SubmitScore(MatchId), Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::CompleteMatch(const FString& MatchId, const FOnDeskillzMatchResultLoaded& OnComplete)
{
	Http->Post(DeskillzApi::Match::Complete(MatchId), TEXT(""),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FDeskillzMatchResult Result = ParseMatchResult(Json);
					OnComplete.ExecuteIfBound(true, Result);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, FDeskillzMatchResult());
		})
	);
}

void UDeskillzApiService::AbortMatch(const FString& MatchId, const FString& Reason,
	const FOnDeskillzApiResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("reason"), Reason);
	
	Http->PostJson(DeskillzApi::Match::Abort(MatchId), Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::GetMatchResult(const FString& MatchId, const FOnDeskillzMatchResultLoaded& OnComplete)
{
	Http->Get(DeskillzApi::Match::Result(MatchId),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FDeskillzMatchResult Result = ParseMatchResult(Json);
					OnComplete.ExecuteIfBound(true, Result);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, FDeskillzMatchResult());
		})
	);
}

// ============================================================================
// Wallet
// ============================================================================

void UDeskillzApiService::GetWalletBalances(const FOnDeskillzBalancesLoaded& OnComplete)
{
	Http->Get(DeskillzApi::Wallet::Balances,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			TMap<FString, double> Balances;
			
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					Balances = ParseBalances(Json);
					OnComplete.ExecuteIfBound(true, Balances);
					return;
				}
			}
			
			OnComplete.ExecuteIfBound(false, Balances);
		})
	);
}

void UDeskillzApiService::GetDepositAddress(const FString& Currency,
	TFunction<void(bool, const FString&)> OnComplete)
{
	Http->Get(DeskillzApi::Wallet::DepositAddress(Currency),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					FString Address = Json->GetStringField(TEXT("address"));
					OnComplete(true, Address);
					return;
				}
			}
			
			OnComplete(false, TEXT(""));
		})
	);
}

void UDeskillzApiService::RequestWithdrawal(const FString& Currency, double Amount, const FString& ToAddress,
	const FOnDeskillzApiResult& OnComplete)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("currency"), Currency);
	Body->SetNumberField(TEXT("amount"), Amount);
	Body->SetStringField(TEXT("to_address"), ToAddress);
	
	Http->PostJson(DeskillzApi::Wallet::Withdraw, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			OnComplete.ExecuteIfBound(Response.IsOk(), Response.ErrorMessage);
		})
	);
}

void UDeskillzApiService::GetTransactions(int32 Page, int32 Limit,
	TFunction<void(bool, const TArray<TSharedPtr<FJsonValue>>&)> OnComplete)
{
	FString Endpoint = DeskillzApi::WithPagination(DeskillzApi::Wallet::Transactions, Page, Limit);
	
	Http->Get(Endpoint,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			TArray<TSharedPtr<FJsonValue>> Transactions;
			
			if (Response.IsOk())
			{
				UDeskillzHttpClient::ParseJsonArrayResponse(Response, Transactions);
				OnComplete(true, Transactions);
				return;
			}
			
			OnComplete(false, Transactions);
		})
	);
}

// ============================================================================
// Leaderboard
// ============================================================================

void UDeskillzApiService::GetGlobalLeaderboard(int32 Limit, const FOnDeskillzLeaderboardLoaded& OnComplete)
{
	TMap<FString, FString> QueryParams;
	QueryParams.Add(TEXT("limit"), FString::FromInt(Limit));
	
	Http->Get(DeskillzApi::Leaderboard::Global,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			TArray<FDeskillzLeaderboardEntry> Entries;
			
			if (Response.IsOk())
			{
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				if (UDeskillzHttpClient::ParseJsonArrayResponse(Response, JsonArray))
				{
					for (const TSharedPtr<FJsonValue>& Value : JsonArray)
					{
						if (TSharedPtr<FJsonObject> JsonObj = Value->AsObject())
						{
							Entries.Add(ParseLeaderboardEntry(JsonObj));
						}
					}
				}
				
				OnComplete.ExecuteIfBound(true, Entries);
				return;
			}
			
			OnComplete.ExecuteIfBound(false, Entries);
		}),
		QueryParams
	);
}

void UDeskillzApiService::GetTournamentLeaderboard(const FString& TournamentId, int32 Limit,
	const FOnDeskillzLeaderboardLoaded& OnComplete)
{
	TMap<FString, FString> QueryParams;
	QueryParams.Add(TEXT("limit"), FString::FromInt(Limit));
	
	Http->Get(DeskillzApi::Leaderboard::ByTournament(TournamentId),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			TArray<FDeskillzLeaderboardEntry> Entries;
			
			if (Response.IsOk())
			{
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				if (UDeskillzHttpClient::ParseJsonArrayResponse(Response, JsonArray))
				{
					for (const TSharedPtr<FJsonValue>& Value : JsonArray)
					{
						if (TSharedPtr<FJsonObject> JsonObj = Value->AsObject())
						{
							Entries.Add(ParseLeaderboardEntry(JsonObj));
						}
					}
				}
				
				OnComplete.ExecuteIfBound(true, Entries);
				return;
			}
			
			OnComplete.ExecuteIfBound(false, Entries);
		}),
		QueryParams
	);
}

void UDeskillzApiService::GetNearbyRanks(int32 Range, const FOnDeskillzLeaderboardLoaded& OnComplete)
{
	TMap<FString, FString> QueryParams;
	QueryParams.Add(TEXT("range"), FString::FromInt(Range));
	
	Http->Get(DeskillzApi::Leaderboard::Nearby,
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			TArray<FDeskillzLeaderboardEntry> Entries;
			
			if (Response.IsOk())
			{
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				if (UDeskillzHttpClient::ParseJsonArrayResponse(Response, JsonArray))
				{
					for (const TSharedPtr<FJsonValue>& Value : JsonArray)
					{
						if (TSharedPtr<FJsonObject> JsonObj = Value->AsObject())
						{
							Entries.Add(ParseLeaderboardEntry(JsonObj));
						}
					}
				}
				
				OnComplete.ExecuteIfBound(true, Entries);
				return;
			}
			
			OnComplete.ExecuteIfBound(false, Entries);
		}),
		QueryParams
	);
}

// ============================================================================
// Games
// ============================================================================

void UDeskillzApiService::GetGameConfig(const FString& GameId,
	TFunction<void(bool, TSharedPtr<FJsonObject>)> OnComplete)
{
	Http->Get(DeskillzApi::Game::Config(GameId),
		FOnDeskillzHttpResponse::CreateLambda([OnComplete](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				TSharedPtr<FJsonObject> Json;
				if (UDeskillzHttpClient::ParseJsonResponse(Response, Json))
				{
					OnComplete(true, Json);
					return;
				}
			}
			
			OnComplete(false, nullptr);
		})
	);
}

// ============================================================================
// Parsing Helpers
// ============================================================================

FDeskillzPlayerInfo UDeskillzApiService::ParseUser(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzPlayerInfo User;
	
	if (Json.IsValid())
	{
		User.Id = Json->GetStringField(TEXT("id"));
		User.Username = Json->GetStringField(TEXT("username"));
		User.AvatarUrl = Json->GetStringField(TEXT("avatar_url"));
		User.SkillRating = static_cast<int32>(Json->GetNumberField(TEXT("skill_rating")));
		User.Level = static_cast<int32>(Json->GetNumberField(TEXT("level")));
		User.TotalWins = static_cast<int32>(Json->GetNumberField(TEXT("total_wins")));
		User.TotalLosses = static_cast<int32>(Json->GetNumberField(TEXT("total_losses")));
	}
	
	return User;
}

FDeskillzTournament UDeskillzApiService::ParseTournament(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzTournament Tournament;
	
	if (Json.IsValid())
	{
		Tournament.Id = Json->GetStringField(TEXT("id"));
		Tournament.Name = Json->GetStringField(TEXT("name"));
		Tournament.Description = Json->GetStringField(TEXT("description"));
		Tournament.GameId = Json->GetStringField(TEXT("game_id"));
		Tournament.EntryFee = Json->GetNumberField(TEXT("entry_fee"));
		Tournament.EntryCurrency = Json->GetStringField(TEXT("entry_currency"));
		Tournament.PrizePool = Json->GetNumberField(TEXT("prize_pool"));
		Tournament.PrizeCurrency = Json->GetStringField(TEXT("prize_currency"));
		Tournament.MaxPlayers = static_cast<int32>(Json->GetNumberField(TEXT("max_players")));
		Tournament.CurrentPlayers = static_cast<int32>(Json->GetNumberField(TEXT("current_players")));
		Tournament.bIsFeatured = Json->GetBoolField(TEXT("is_featured"));
		Tournament.StartTimeMs = static_cast<int64>(Json->GetNumberField(TEXT("start_time")));
		Tournament.EndTimeMs = static_cast<int64>(Json->GetNumberField(TEXT("end_time")));
		
		FString StatusStr = Json->GetStringField(TEXT("status"));
		if (StatusStr == TEXT("active")) Tournament.Status = EDeskillzTournamentStatus::Active;
		else if (StatusStr == TEXT("upcoming")) Tournament.Status = EDeskillzTournamentStatus::Upcoming;
		else if (StatusStr == TEXT("completed")) Tournament.Status = EDeskillzTournamentStatus::Completed;
	}
	
	return Tournament;
}

FDeskillzMatch UDeskillzApiService::ParseMatch(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzMatch Match;
	
	if (Json.IsValid())
	{
		Match.Id = Json->GetStringField(TEXT("id"));
		Match.TournamentId = Json->GetStringField(TEXT("tournament_id"));
		Match.EntryFee = Json->GetNumberField(TEXT("entry_fee"));
		Match.EntryCurrency = Json->GetStringField(TEXT("entry_currency"));
		Match.PrizeAmount = Json->GetNumberField(TEXT("prize_amount"));
		Match.PrizeCurrency = Json->GetStringField(TEXT("prize_currency"));
		Match.StartTimeMs = static_cast<int64>(Json->GetNumberField(TEXT("start_time")));
		
		FString StatusStr = Json->GetStringField(TEXT("status"));
		if (StatusStr == TEXT("pending")) Match.Status = EDeskillzMatchStatus::Pending;
		else if (StatusStr == TEXT("ready")) Match.Status = EDeskillzMatchStatus::Ready;
		else if (StatusStr == TEXT("in_progress")) Match.Status = EDeskillzMatchStatus::InProgress;
		else if (StatusStr == TEXT("completed")) Match.Status = EDeskillzMatchStatus::Completed;
		
		// Parse players
		const TArray<TSharedPtr<FJsonValue>>* PlayersArray;
		if (Json->TryGetArrayField(TEXT("players"), PlayersArray))
		{
			for (const TSharedPtr<FJsonValue>& PlayerValue : *PlayersArray)
			{
				if (TSharedPtr<FJsonObject> PlayerJson = PlayerValue->AsObject())
				{
					Match.Players.Add(ParseUser(PlayerJson));
				}
			}
		}
	}
	
	return Match;
}

FDeskillzMatchResult UDeskillzApiService::ParseMatchResult(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzMatchResult Result;
	
	if (Json.IsValid())
	{
		Result.MatchId = Json->GetStringField(TEXT("match_id"));
		Result.TournamentId = Json->GetStringField(TEXT("tournament_id"));
		Result.PlayerScore = static_cast<int64>(Json->GetNumberField(TEXT("player_score")));
		Result.OpponentScore = static_cast<int64>(Json->GetNumberField(TEXT("opponent_score")));
		Result.PlayerName = Json->GetStringField(TEXT("player_name"));
		Result.OpponentName = Json->GetStringField(TEXT("opponent_name"));
		Result.PrizeWon = Json->GetNumberField(TEXT("prize_won"));
		Result.PrizeCurrency = Json->GetStringField(TEXT("prize_currency"));
		Result.OldRating = static_cast<int32>(Json->GetNumberField(TEXT("old_rating")));
		Result.NewRating = static_cast<int32>(Json->GetNumberField(TEXT("new_rating")));
		
		FString OutcomeStr = Json->GetStringField(TEXT("outcome"));
		if (OutcomeStr == TEXT("win")) Result.Outcome = EDeskillzMatchOutcome::Win;
		else if (OutcomeStr == TEXT("loss")) Result.Outcome = EDeskillzMatchOutcome::Loss;
		else if (OutcomeStr == TEXT("draw")) Result.Outcome = EDeskillzMatchOutcome::Draw;
	}
	
	return Result;
}

FDeskillzLeaderboardEntry UDeskillzApiService::ParseLeaderboardEntry(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzLeaderboardEntry Entry;
	
	if (Json.IsValid())
	{
		Entry.Rank = static_cast<int32>(Json->GetNumberField(TEXT("rank")));
		Entry.PlayerId = Json->GetStringField(TEXT("player_id"));
		Entry.Username = Json->GetStringField(TEXT("username"));
		Entry.Score = static_cast<int64>(Json->GetNumberField(TEXT("score")));
		Entry.Wins = static_cast<int32>(Json->GetNumberField(TEXT("wins")));
		Entry.WinRate = static_cast<float>(Json->GetNumberField(TEXT("win_rate")));
		Entry.bIsCurrentPlayer = Json->GetBoolField(TEXT("is_current_player"));
	}
	
	return Entry;
}

TMap<FString, double> UDeskillzApiService::ParseBalances(const TSharedPtr<FJsonObject>& Json)
{
	TMap<FString, double> Balances;
	
	if (Json.IsValid())
	{
		const TSharedPtr<FJsonObject>* BalancesObj;
		if (Json->TryGetObjectField(TEXT("balances"), BalancesObj))
		{
			for (const auto& Pair : (*BalancesObj)->Values)
			{
				Balances.Add(Pair.Key, Pair.Value->AsNumber());
			}
		}
	}
	
	return Balances;
}
