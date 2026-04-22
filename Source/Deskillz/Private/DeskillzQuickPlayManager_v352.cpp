// Copyright Deskillz Games. All Rights Reserved.
// DeskillzQuickPlayManager_v352.cpp
// Path: Source/Deskillz/Private/DeskillzQuickPlayManager_v352.cpp

#include "DeskillzQuickPlayManager_v352.h"
#include "Network/DeskillzHttpClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

static UDeskillzQuickPlayManager_v352* GQPInstance = nullptr;

UDeskillzQuickPlayManager_v352* UDeskillzQuickPlayManager_v352::Get()
{
	if (!GQPInstance)
	{
		GQPInstance = NewObject<UDeskillzQuickPlayManager_v352>();
		GQPInstance->AddToRoot();
	}
	return GQPInstance;
}

// ============================================================================
// ESPORT QUICK PLAY
// ============================================================================

void UDeskillzQuickPlayManager_v352::JoinQueue(
	const FDeskillzQuickPlayJoinParams& Params,
	const FOnQuickPlayJoinResult& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("gameId"), Params.GameId);
	Body->SetNumberField(TEXT("entryFee"), Params.EntryFee);
	Body->SetStringField(TEXT("currency"), Params.Currency);
	Body->SetNumberField(TEXT("playerCount"), Params.PlayerCount);

	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->PostJson(DeskillzApi_v352::QuickPlay::Join, Body,
		FOnDeskillzHttpResponse::CreateLambda(
		[this, OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			FDeskillzQuickPlayJoinResult Result;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetBoolField(TEXT("success"), Result.bSuccess);
				Json->TryGetStringField(TEXT("queueKey"), Result.QueueKey);
				Json->TryGetStringField(TEXT("gameId"), Result.GameId);
				Json->TryGetNumberField(TEXT("position"), Result.Position);
				Json->TryGetNumberField(TEXT("estimatedWait"), Result.EstimatedWaitSeconds);
				Json->TryGetStringField(TEXT("matchId"), Result.MatchId);
			}

			bIsInQueue = true;
			OnSuccess.ExecuteIfBound(Result);
		}
	));
}

void UDeskillzQuickPlayManager_v352::LeaveQueue(
	const FOnSuccess& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Post(DeskillzApi_v352::QuickPlay::Leave, TEXT("{}"),
		FOnDeskillzHttpResponse::CreateLambda(
		[this, OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			bIsInQueue = false;
			OnSuccess.ExecuteIfBound();
		}
	));
}

void UDeskillzQuickPlayManager_v352::GetConfig(
	const FString& GameId,
	const FOnQuickPlayConfig& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi_v352::QuickPlay::Config(GameId),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			FDeskillzQuickPlayConfig Config;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetStringField(TEXT("id"), Config.Id);
				Json->TryGetStringField(TEXT("gameId"), Config.GameId);
				Json->TryGetBoolField(TEXT("enabled"), Config.bEnabled);
				Json->TryGetNumberField(TEXT("minPlayers"), Config.MinPlayers);
				Json->TryGetNumberField(TEXT("maxPlayers"), Config.MaxPlayers);
				Json->TryGetNumberField(TEXT("entryFee"), Config.EntryFee);
				Json->TryGetStringField(TEXT("currency"), Config.Currency);
				Json->TryGetNumberField(TEXT("matchDurationSeconds"), Config.MatchDurationSeconds);
				Json->TryGetNumberField(TEXT("queueTimeoutSeconds"), Config.QueueTimeoutSeconds);
				Json->TryGetStringField(TEXT("socialWinCondition"), Config.SocialWinCondition);
				Json->TryGetNumberField(TEXT("socialDefaultTarget"), Config.SocialDefaultTarget);
				Json->TryGetBoolField(TEXT("socialAllowFreePlay"), Config.bSocialAllowFreePlay);

				const TArray<TSharedPtr<FJsonValue>>* PtArr;
				if (Json->TryGetArrayField(TEXT("socialPointTargets"), PtArr))
					for (const auto& V : *PtArr) Config.SocialPointTargets.Add(V->AsNumber());

				const TArray<TSharedPtr<FJsonValue>>* RtArr;
				if (Json->TryGetArrayField(TEXT("socialRoundTargets"), RtArr))
					for (const auto& V : *RtArr) Config.SocialRoundTargets.Add(V->AsNumber());
			}
			OnSuccess.ExecuteIfBound(Config);
		}
	));
}

void UDeskillzQuickPlayManager_v352::LaunchMatch(
	const FString& MatchSessionId,
	const FOnQuickPlayLaunchData& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("matchSessionId"), MatchSessionId);

	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->PostJson(DeskillzApi_v352::QuickPlay::MatchLaunch, Body,
		FOnDeskillzHttpResponse::CreateLambda(
		[this, OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			FDeskillzQuickPlayLaunchData Data;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetStringField(TEXT("matchId"), Data.MatchId);
				Json->TryGetStringField(TEXT("matchSessionId"), Data.MatchSessionId);
				Json->TryGetStringField(TEXT("gameId"), Data.GameId);
				Json->TryGetStringField(TEXT("deepLink"), Data.DeepLink);
				Json->TryGetStringField(TEXT("token"), Data.Token);
				Json->TryGetNumberField(TEXT("entryFee"), Data.EntryFee);
				Json->TryGetStringField(TEXT("currency"), Data.Currency);
				Json->TryGetNumberField(TEXT("prizePool"), Data.PrizePool);
				Json->TryGetNumberField(TEXT("matchDurationSecs"), Data.MatchDurationSecs);
			}

			bIsInQueue = false;
			CurrentMatch = Data;
			OnSuccess.ExecuteIfBound(Data);
		}
	));
}

void UDeskillzQuickPlayManager_v352::SubmitScore(
	const FString& MatchId, int64 Score,
	const FOnQuickPlayScoreResult& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetNumberField(TEXT("score"), Score);

	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->PostJson(DeskillzApi_v352::QuickPlay::MatchScore(MatchId), Body,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			FDeskillzQuickPlayScoreResult Result;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetBoolField(TEXT("success"), Result.bSuccess);
				Json->TryGetStringField(TEXT("matchId"), Result.MatchId);
				Json->TryGetNumberField(TEXT("score"), Result.Score);
				Json->TryGetBoolField(TEXT("allScoresSubmitted"), Result.bAllScoresSubmitted);
			}
			OnSuccess.ExecuteIfBound(Result);
		}
	));
}

void UDeskillzQuickPlayManager_v352::GetMatchResults(
	const FString& MatchId,
	const FOnQuickPlayMatchResult& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi_v352::QuickPlay::MatchResults(MatchId),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			FDeskillzQuickPlayMatchResult Result;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetStringField(TEXT("matchId"), Result.MatchId);
				Json->TryGetStringField(TEXT("status"), Result.Status);
				Json->TryGetStringField(TEXT("winnerId"), Result.WinnerId);
				Json->TryGetNumberField(TEXT("prizePool"), Result.PrizePool);
				Json->TryGetStringField(TEXT("currency"), Result.Currency);

				const TArray<TSharedPtr<FJsonValue>>* PlayersArr;
				if (Json->TryGetArrayField(TEXT("players"), PlayersArr))
				{
					for (const auto& PVal : *PlayersArr)
					{
						if (PVal->Type != EJson::Object) continue;
						auto PObj = PVal->AsObject();
						FDeskillzQuickPlayPlayerResult PR;
						PObj->TryGetStringField(TEXT("playerId"), PR.PlayerId);
						PObj->TryGetStringField(TEXT("username"), PR.Username);
						PObj->TryGetNumberField(TEXT("score"), PR.Score);
						PObj->TryGetNumberField(TEXT("rank"), PR.Rank);
						PObj->TryGetNumberField(TEXT("prizeWon"), PR.PrizeWon);
						PObj->TryGetBoolField(TEXT("isWinner"), PR.bIsWinner);
						Result.Players.Add(PR);
					}
				}
			}
			OnSuccess.ExecuteIfBound(Result);
		}
	));
}

void UDeskillzQuickPlayManager_v352::ForceCompleteMatch(
	const FString& MatchId,
	const FOnQuickPlayMatchResult& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Post(DeskillzApi_v352::QuickPlay::MatchComplete(MatchId), TEXT("{}"),
		FOnDeskillzHttpResponse::CreateLambda(
		[this, OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			CurrentMatch = FDeskillzQuickPlayLaunchData();
			// Re-use GetMatchResults parsing
			FDeskillzQuickPlayMatchResult Result;
			Result.MatchId = Response.Body; // simplified
			OnSuccess.ExecuteIfBound(Result);
		}
	));
}

// ============================================================================
// SOCIAL QUICK PLAY (6.9 - 6.11)
// ============================================================================

void UDeskillzQuickPlayManager_v352::CreateSocialRoom(
	const FString& GameId, double PointValueUsd,
	const FString& Currency, int32 SeatsPerTable,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("gameId"), GameId);
	Body->SetNumberField(TEXT("pointValueUsd"), PointValueUsd);
	Body->SetStringField(TEXT("currency"), Currency);
	Body->SetNumberField(TEXT("seatsPerTable"), SeatsPerTable);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::QuickPlay::SocialCreate, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzQuickPlayManager_v352::SubmitSocialRound(
	const FString& RoomId, const FString& PayloadJson,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::QuickPlay::SocialRound(RoomId), PayloadJson,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzQuickPlayManager_v352::SocialRebuy(
	const FString& RoomId, double Amount,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	FString Body = FString::Printf(TEXT("{\"amount\":%.2f}"), Amount);
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::QuickPlay::SocialRebuy(RoomId), Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzQuickPlayManager_v352::SocialCashOut(
	const FString& RoomId,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::QuickPlay::SocialCashOut(RoomId), TEXT("{}"),
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzQuickPlayManager_v352::EndSocialGame(
	const FString& RoomId,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::QuickPlay::SocialEnd(RoomId), TEXT("{}"),
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

// ============================================================================
// SOCKET EVENT HANDLERS (6.12)
// ============================================================================

void UDeskillzQuickPlayManager_v352::HandleQueueMatched(const FString& MatchSessionId)
{
	bIsInQueue = false;
	OnQueueMatched.Broadcast(MatchSessionId);
}

void UDeskillzQuickPlayManager_v352::HandleQueueTimeout()
{
	bIsInQueue = false;
	OnQueueTimeout.Broadcast();
}

void UDeskillzQuickPlayManager_v352::HandleMatchCompleted(const FString& MatchId)
{
	CurrentMatch = FDeskillzQuickPlayLaunchData();
	OnMatchCompleted.Broadcast(MatchId);
}

FDeskillzError UDeskillzQuickPlayManager_v352::ParseError(const FString& Body, int32 StatusCode)
{
	FDeskillzError Err;
	Err.HttpStatusCode = StatusCode;
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		Json->TryGetStringField(TEXT("message"), Err.Message);
	if (Err.Message.IsEmpty()) Err.Message = TEXT("Quick Play operation failed");
	Err.Code = EDeskillzErrorCode::ServerError;
	return Err;
}