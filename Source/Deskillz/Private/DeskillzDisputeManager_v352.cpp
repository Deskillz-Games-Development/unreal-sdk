// Copyright Deskillz Games. All Rights Reserved.
// DeskillzDisputeManager_v352.cpp
// Path: Source/Deskillz/Private/DeskillzDisputeManager_v352.cpp

#include "DeskillzDisputeManager_v352.h"
#include "Network/DeskillzHttpClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

const FString UDeskillzDisputeManager_v352::LastMatchSaveSlot = TEXT("DeskillzLastMatch");

static UDeskillzDisputeManager_v352* GDisputeInstance = nullptr;

UDeskillzDisputeManager_v352* UDeskillzDisputeManager_v352::Get()
{
	if (!GDisputeInstance)
	{
		GDisputeInstance = NewObject<UDeskillzDisputeManager_v352>();
		GDisputeInstance->AddToRoot();
	}
	return GDisputeInstance;
}

// ============================================================================
// FILE DISPUTE (4.1)
// ============================================================================

void UDeskillzDisputeManager_v352::FileDispute(
	const FDeskillzFileDisputeParams& Params,
	const FOnDisputeRecord& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("disputeType"), Params.DisputeType);
	if (!Params.TournamentId.IsEmpty()) Body->SetStringField(TEXT("tournamentId"), Params.TournamentId);
	if (!Params.MatchId.IsEmpty()) Body->SetStringField(TEXT("matchId"), Params.MatchId);
	if (!Params.RoomCode.IsEmpty()) Body->SetStringField(TEXT("roomCode"), Params.RoomCode);
	Body->SetStringField(TEXT("reason"), Params.Reason);
	Body->SetStringField(TEXT("description"), Params.Description);

	TArray<TSharedPtr<FJsonValue>> EvidenceArr;
	for (const FString& E : Params.Evidence)
		EvidenceArr.Add(MakeShareable(new FJsonValueString(E)));
	Body->SetArrayField(TEXT("evidence"), EvidenceArr);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::Dispute::File, Body,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
				OnSuccess.ExecuteIfBound(ParseDispute(Json));
		}
	));
}

// ============================================================================
// GET MY DISPUTES (4.2)
// ============================================================================

void UDeskillzDisputeManager_v352::GetMyDisputes(
	const FString& StatusFilter,
	const FOnDisputeRecords& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	FString Endpoint = DeskillzApi_v352::Dispute::Mine;
	if (!StatusFilter.IsEmpty())
		Endpoint += FString::Printf(TEXT("?status=%s"), *StatusFilter);

	UDeskillzHttpClient::Get()->Get(Endpoint,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			TArray<FDeskillzDisputeRecord> Records;
			TArray<TSharedPtr<FJsonValue>> JsonArray;
			if (UDeskillzHttpClient::ParseJsonArrayResponse(Response, JsonArray))
			{
				for (const auto& Val : JsonArray)
				{
					if (Val->Type == EJson::Object)
						Records.Add(ParseDispute(Val->AsObject()));
				}
			}
			OnSuccess.ExecuteIfBound(Records);
		}
	));
}

// ============================================================================
// GET DISPUTE DETAILS (4.3)
// ============================================================================

void UDeskillzDisputeManager_v352::GetDisputeDetails(
	const FString& DisputeId,
	const FOnDisputeRecord& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Dispute::Details(DisputeId),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
				OnSuccess.ExecuteIfBound(ParseDispute(Json));
		}
	));
}

// ============================================================================
// ADD EVIDENCE (4.4)
// ============================================================================

void UDeskillzDisputeManager_v352::AddEvidence(
	const FString& DisputeId,
	const TArray<FString>& Evidence,
	const FOnSuccess& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> Arr;
	for (const FString& E : Evidence)
		Arr.Add(MakeShareable(new FJsonValueString(E)));
	Body->SetArrayField(TEXT("evidence"), Arr);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::Dispute::Evidence(DisputeId), Body,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

// ============================================================================
// RECENT MATCHES (4.5)
// ============================================================================

void UDeskillzDisputeManager_v352::GetRecentMatches(
	const FOnMatchRecords& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	FString Endpoint = DeskillzApi_v352::Dispute::RecentMatches + TEXT("?limit=10");

	UDeskillzHttpClient::Get()->Get(Endpoint,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

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

// ============================================================================
// LAST MATCH PERSISTENCE (4.6)
// ============================================================================

void UDeskillzDisputeManager_v352::PersistLastMatch(const FDeskillzLastMatchContext& Context)
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	Json->SetStringField(TEXT("matchId"), Context.MatchId);
	Json->SetStringField(TEXT("tournamentId"), Context.TournamentId);
	Json->SetStringField(TEXT("roomCode"), Context.RoomCode);
	Json->SetStringField(TEXT("disputeType"), Context.DisputeType);
	Json->SetStringField(TEXT("opponentName"), Context.OpponentName);
	Json->SetStringField(TEXT("completedAt"), Context.CompletedAt);

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	FString FilePath = FPaths::ProjectSavedDir() / LastMatchSaveSlot + TEXT(".json");
	FFileHelper::SaveStringToFile(Output, *FilePath);
}

FDeskillzLastMatchContext UDeskillzDisputeManager_v352::GetLastMatch() const
{
	FDeskillzLastMatchContext Ctx;
	FString FilePath = FPaths::ProjectSavedDir() / LastMatchSaveSlot + TEXT(".json");
	FString Content;

	if (!FFileHelper::LoadFileToString(Content, *FilePath))
		return Ctx;

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
		return Ctx;

	Json->TryGetStringField(TEXT("matchId"), Ctx.MatchId);
	Json->TryGetStringField(TEXT("tournamentId"), Ctx.TournamentId);
	Json->TryGetStringField(TEXT("roomCode"), Ctx.RoomCode);
	Json->TryGetStringField(TEXT("disputeType"), Ctx.DisputeType);
	Json->TryGetStringField(TEXT("opponentName"), Ctx.OpponentName);
	Json->TryGetStringField(TEXT("completedAt"), Ctx.CompletedAt);

	// Expire after 7 days
	if (!Ctx.CompletedAt.IsEmpty())
	{
		FDateTime CompletedTime;
		if (FDateTime::ParseIso8601(*Ctx.CompletedAt, CompletedTime))
		{
			if ((FDateTime::UtcNow() - CompletedTime).GetTotalDays() > 7.0)
			{
				IFileManager::Get().Delete(*FilePath);
				return FDeskillzLastMatchContext();
			}
		}
	}

	return Ctx;
}

void UDeskillzDisputeManager_v352::ClearLastMatch()
{
	FString FilePath = FPaths::ProjectSavedDir() / LastMatchSaveSlot + TEXT(".json");
	IFileManager::Get().Delete(*FilePath);
}

// ============================================================================
// PARSERS
// ============================================================================

FDeskillzDisputeRecord UDeskillzDisputeManager_v352::ParseDispute(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzDisputeRecord D;
	if (!Json.IsValid()) return D;

	Json->TryGetStringField(TEXT("id"), D.Id);
	Json->TryGetStringField(TEXT("matchId"), D.MatchId);
	Json->TryGetStringField(TEXT("tournamentId"), D.TournamentId);
	Json->TryGetStringField(TEXT("roomCode"), D.RoomCode);
	Json->TryGetStringField(TEXT("reason"), D.Reason);
	Json->TryGetStringField(TEXT("description"), D.Description);
	Json->TryGetStringField(TEXT("status"), D.Status);
	Json->TryGetStringField(TEXT("resolution"), D.Resolution);

	const TArray<TSharedPtr<FJsonValue>>* EArr;
	if (Json->TryGetArrayField(TEXT("evidence"), EArr))
		for (const auto& V : *EArr) D.Evidence.Add(V->AsString());

	return D;
}

FDeskillzError UDeskillzDisputeManager_v352::ParseError(const FString& Body, int32 StatusCode)
{
	FDeskillzError Err;
	Err.HttpStatusCode = StatusCode;
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		Json->TryGetStringField(TEXT("message"), Err.Message);
	if (Err.Message.IsEmpty()) Err.Message = TEXT("Dispute operation failed");
	Err.Code = EDeskillzErrorCode::ServerError;
	return Err;
}