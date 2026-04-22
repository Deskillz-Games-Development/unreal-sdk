// Copyright Deskillz Games. All Rights Reserved.
// DeskillzWalletManager_v352.cpp
// Path: Source/Deskillz/Private/DeskillzWalletManager_v352.cpp

#include "DeskillzWalletManager_v352.h"
#include "Network/DeskillzHttpClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

static UDeskillzWalletManager_v352* GWalletInstance = nullptr;

UDeskillzWalletManager_v352* UDeskillzWalletManager_v352::Get()
{
	if (!GWalletInstance)
	{
		GWalletInstance = NewObject<UDeskillzWalletManager_v352>();
		GWalletInstance->AddToRoot();
	}
	return GWalletInstance;
}

void UDeskillzWalletManager_v352::GetBalance(
	const FOnWalletBalances& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Wallet::Balance,
		FOnDeskillzHttpResponse::CreateLambda(
		[this, OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk()) { OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode)); return; }

			TArray<FDeskillzWalletBalanceEntry> Balances;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* Arr;
				if (Json->TryGetArrayField(TEXT("balances"), Arr))
				{
					for (const auto& Val : *Arr)
					{
						if (Val->Type != EJson::Object) continue;
						auto Obj = Val->AsObject();
						FDeskillzWalletBalanceEntry B;
						Obj->TryGetStringField(TEXT("currency"), B.Currency);
						Obj->TryGetStringField(TEXT("symbol"), B.Symbol);
						Obj->TryGetNumberField(TEXT("amount"), B.Amount);
						Obj->TryGetNumberField(TEXT("usdValue"), B.UsdValue);
						Obj->TryGetStringField(TEXT("color"), B.Color);
						Obj->TryGetStringField(TEXT("network"), B.Network);
						Balances.Add(B);
					}
				}
			}
			CachedBalances = Balances;
			OnSuccess.ExecuteIfBound(Balances);
		}
	));
}

void UDeskillzWalletManager_v352::GetBalanceForCurrency(
	const FString& Currency,
	const FOnWalletBalance& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Wallet::BalanceForCurrency(Currency),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk()) { OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode)); return; }
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				FDeskillzWalletBalanceEntry B;
				Json->TryGetStringField(TEXT("currency"), B.Currency);
				Json->TryGetStringField(TEXT("symbol"), B.Symbol);
				Json->TryGetNumberField(TEXT("amount"), B.Amount);
				Json->TryGetNumberField(TEXT("usdValue"), B.UsdValue);
				OnSuccess.ExecuteIfBound(B);
			}
		}
	));
}

void UDeskillzWalletManager_v352::Deposit(
	const FString& Currency, double Amount,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	FString Body = FString::Printf(TEXT("{\"currency\":\"%s\",\"amount\":%.2f}"), *Currency, Amount);
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Wallet::Deposit, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzWalletManager_v352::Withdraw(
	const FString& Currency, double Amount, const FString& WalletAddress,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("currency"), Currency);
	Body->SetNumberField(TEXT("amount"), Amount);
	Body->SetStringField(TEXT("walletAddress"), WalletAddress);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::Wallet::Withdraw, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzWalletManager_v352::GetPlayerStats(
	const FOnPlayerStats& OnSuccess, const FOnDeskillzApiError& OnError)
{
	// Need user ID - get from auth
	UDeskillzHttpClient::Get()->Get(TEXT("/api/v1/users/me"),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk()) { OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode)); return; }

			FDeskillzPlayerStats Stats;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				// Stats may be nested under "stats" field
				const TSharedPtr<FJsonObject>* StatsObj;
				TSharedPtr<FJsonObject> S = Json;
				if (Json->TryGetObjectField(TEXT("stats"), StatsObj))
					S = *StatsObj;

				S->TryGetNumberField(TEXT("totalMatches"), Stats.TotalMatches);
				S->TryGetNumberField(TEXT("totalWins"), Stats.Wins);
				S->TryGetNumberField(TEXT("totalLosses"), Stats.Losses);
				S->TryGetNumberField(TEXT("totalEarnings"), Stats.TotalEarnings);
				S->TryGetNumberField(TEXT("tournamentsPlayed"), Stats.TournamentsPlayed);
				S->TryGetNumberField(TEXT("tournamentsWon"), Stats.TournamentsWon);
				S->TryGetNumberField(TEXT("currentStreak"), Stats.CurrentStreak);
				S->TryGetNumberField(TEXT("bestStreak"), Stats.BestStreak);
				Stats.WinRate = Stats.TotalMatches > 0 ? (float)Stats.Wins / Stats.TotalMatches * 100.f : 0.f;
			}
			OnSuccess.ExecuteIfBound(Stats);
		}
	));
}

void UDeskillzWalletManager_v352::GetMatchHistory(
	int32 Page, int32 Limit,
	const FOnMatchRecords& OnSuccess, const FOnDeskillzApiError& OnError)
{
	int32 Offset = (Page - 1) * Limit;
	FString Endpoint = FString::Printf(TEXT("/api/v1/matches/history/me?limit=%d&offset=%d"), Limit, Offset);

	UDeskillzHttpClient::Get()->Get(Endpoint,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk()) { OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode)); return; }

			TArray<FDeskillzMatchRecord> Records;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* Arr;
				if (Json->TryGetArrayField(TEXT("matches"), Arr))
				{
					for (const auto& Val : *Arr)
					{
						if (Val->Type != EJson::Object) continue;
						auto Obj = Val->AsObject();
						FDeskillzMatchRecord M;
						Obj->TryGetStringField(TEXT("matchId"), M.MatchId);
						Obj->TryGetStringField(TEXT("tournamentId"), M.TournamentId);
						Obj->TryGetStringField(TEXT("gameName"), M.GameName);
						Obj->TryGetNumberField(TEXT("score"), M.Score);
						Obj->TryGetNumberField(TEXT("rank"), M.Rank);
						Obj->TryGetNumberField(TEXT("prizeWon"), M.PrizeWon);
						Obj->TryGetStringField(TEXT("outcome"), M.Outcome);
						Obj->TryGetStringField(TEXT("opponentName"), M.OpponentName);
						Records.Add(M);
					}
				}
			}
			OnSuccess.ExecuteIfBound(Records);
		}
	));
}

void UDeskillzWalletManager_v352::GetGameLeaderboard(
	const FString& GameId, const FString& Period, int32 Limit,
	const FOnDeskillzLeaderboardReceived& OnSuccess, const FOnDeskillzApiError& OnError)
{
	FString Endpoint = FString::Printf(TEXT("/api/v1/leaderboard/%s?period=%s&limit=%d"), *GameId, *Period, Limit);

	UDeskillzHttpClient::Get()->Get(Endpoint,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				FDeskillzError Err = ParseError(Response.Body, Response.StatusCode);
				OnSuccess.Broadcast({}, Err);
				return;
			}

			TArray<FDeskillzLeaderboardEntry> Entries;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* Arr;
				if (Json->TryGetArrayField(TEXT("entries"), Arr))
				{
					for (const auto& Val : *Arr)
					{
						if (Val->Type != EJson::Object) continue;
						auto Obj = Val->AsObject();
						FDeskillzLeaderboardEntry E;
						Obj->TryGetNumberField(TEXT("rank"), E.Rank);
						Obj->TryGetNumberField(TEXT("score"), E.Score);
						Obj->TryGetNumberField(TEXT("earnings"), E.Earnings);
						Obj->TryGetNumberField(TEXT("matchesWon"), E.MatchesWon);
						Obj->TryGetNumberField(TEXT("matchesPlayed"), E.MatchesPlayed);

						const TSharedPtr<FJsonObject>* PlayerObj;
						if (Obj->TryGetObjectField(TEXT("player"), PlayerObj))
						{
							(*PlayerObj)->TryGetStringField(TEXT("playerId"), E.Player.PlayerId);
							(*PlayerObj)->TryGetStringField(TEXT("username"), E.Player.Username);
							(*PlayerObj)->TryGetStringField(TEXT("avatarUrl"), E.Player.AvatarUrl);
						}
						Entries.Add(E);
					}
				}
			}
			OnSuccess.Broadcast(Entries, FDeskillzError::None());
		}
	));
}

void UDeskillzWalletManager_v352::GetTransactions(
	int32 Limit, int32 Offset,
	const FString& Type, const FString& Currency,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	FString Endpoint = FString::Printf(TEXT("/api/v1/wallet/transactions?limit=%d&offset=%d"), Limit, Offset);
	if (!Type.IsEmpty()) Endpoint += FString::Printf(TEXT("&type=%s"), *Type);
	if (!Currency.IsEmpty()) Endpoint += FString::Printf(TEXT("&currency=%s"), *Currency);

	UDeskillzHttpClient::Get()->Get(Endpoint,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

FDeskillzError UDeskillzWalletManager_v352::ParseError(const FString& Body, int32 StatusCode)
{
	FDeskillzError Err;
	Err.HttpStatusCode = StatusCode;
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		Json->TryGetStringField(TEXT("message"), Err.Message);
	if (Err.Message.IsEmpty()) Err.Message = TEXT("Wallet operation failed");
	Err.Code = EDeskillzErrorCode::ServerError;
	return Err;
}