// Copyright Deskillz Games. All Rights Reserved.
// DeskillzRoomExtensions_v352.cpp
// Path: Source/Deskillz/Private/DeskillzRoomExtensions_v352.cpp

#include "DeskillzRoomExtensions_v352.h"
#include "Network/DeskillzHttpClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

static UDeskillzRoomExtensions_v352* GRoomExtInstance = nullptr;

UDeskillzRoomExtensions_v352* UDeskillzRoomExtensions_v352::Get()
{
	if (!GRoomExtInstance)
	{
		GRoomExtInstance = NewObject<UDeskillzRoomExtensions_v352>();
		GRoomExtInstance->AddToRoot();
	}
	return GRoomExtInstance;
}

// ============================================================================
// CREATE WITH HOST ROLE (5.1, 5.2)
// ============================================================================

void UDeskillzRoomExtensions_v352::CreateEsportRoom(
	const FDeskillzCreateEsportRoomOpts& Opts,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("name"), Opts.Name);
	Body->SetNumberField(TEXT("entryFee"), Opts.EntryFee);
	Body->SetStringField(TEXT("currency"), Opts.Currency);
	Body->SetNumberField(TEXT("minPlayers"), Opts.MinPlayers);
	Body->SetNumberField(TEXT("maxPlayers"), Opts.MaxPlayers);
	Body->SetStringField(TEXT("hostRole"), Opts.HostRole == EDeskillzHostRole::SPECTATOR ? TEXT("SPECTATOR") : TEXT("PLAYER"));
	Body->SetNumberField(TEXT("matchDurationSeconds"), Opts.MatchDurationSeconds);
	Body->SetStringField(TEXT("visibility"), Opts.Visibility);

	FString MatchModeStr;
	switch (Opts.MatchMode)
	{
		case EDeskillzEsportMatchMode::BEST_OF_3: MatchModeStr = TEXT("BEST_OF_3"); break;
		case EDeskillzEsportMatchMode::BEST_OF_5: MatchModeStr = TEXT("BEST_OF_5"); break;
		default: MatchModeStr = TEXT("SINGLE_MATCH"); break;
	}
	Body->SetStringField(TEXT("matchMode"), MatchModeStr);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::Room::Create, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzRoomExtensions_v352::CreateSocialRoom(
	const FDeskillzCreateSocialRoomOpts& Opts,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("name"), Opts.Name);
	Body->SetStringField(TEXT("socialGameType"), Opts.SocialGameType);
	Body->SetNumberField(TEXT("tableStakes"), Opts.TableStakes);
	Body->SetStringField(TEXT("currency"), Opts.Currency);
	Body->SetNumberField(TEXT("playersPerTable"), Opts.PlayersPerTable);
	Body->SetNumberField(TEXT("maxTables"), Opts.MaxTables);
	Body->SetStringField(TEXT("hostRole"), Opts.HostRole == EDeskillzHostRole::SPECTATOR ? TEXT("SPECTATOR") : TEXT("PLAYER"));
	Body->SetNumberField(TEXT("rakePercent"), Opts.RakePercent);
	Body->SetStringField(TEXT("visibility"), Opts.Visibility);

	FString WinCondStr;
	switch (Opts.WinCondition)
	{
		case EDeskillzSocialWinCondition::FIRST_TO_POINTS: WinCondStr = TEXT("FIRST_TO_POINTS"); break;
		case EDeskillzSocialWinCondition::FIXED_ROUNDS: WinCondStr = TEXT("FIXED_ROUNDS"); break;
		case EDeskillzSocialWinCondition::TIMED_SESSION: WinCondStr = TEXT("TIMED_SESSION"); break;
		case EDeskillzSocialWinCondition::SINGLE_GAME: WinCondStr = TEXT("SINGLE_GAME"); break;
		default: WinCondStr = TEXT("OPEN_ENDED"); break;
	}
	Body->SetStringField(TEXT("winCondition"), WinCondStr);
	if (Opts.WinConditionTarget > 0)
		Body->SetNumberField(TEXT("winConditionTarget"), Opts.WinConditionTarget);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::Room::Create, Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

// ============================================================================
// FINANCIAL (5.3 - 5.5)
// ============================================================================

void UDeskillzRoomExtensions_v352::BuyIn(
	const FString& RoomId, double Amount, const FString& Currency,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	FString Body = FString::Printf(TEXT("{\"amount\":%.2f,\"currency\":\"%s\"}"), Amount, *Currency);
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Room::BuyIn(RoomId), Body,
		FOnDeskillzHttpResponse::CreateLambda([this, OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{
			if (!R.IsOk()) { OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); return; }
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(R, Json) && Json.IsValid())
			{
				double Balance = 0; Json->TryGetNumberField(TEXT("chipBalance"), Balance);
				OnBuyInComplete.Broadcast(Balance);
			}
			OnSuccess.ExecuteIfBound();
		}
	));
}

void UDeskillzRoomExtensions_v352::CashOut(
	const FString& RoomId,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Room::CashOut(RoomId), TEXT("{}"),
		FOnDeskillzHttpResponse::CreateLambda([this, OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{
			if (!R.IsOk()) { OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); return; }
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(R, Json) && Json.IsValid())
			{
				double Amt = 0; Json->TryGetNumberField(TEXT("amount"), Amt);
				OnCashOutComplete.Broadcast(Amt);
			}
			OnSuccess.ExecuteIfBound();
		}
	));
}

void UDeskillzRoomExtensions_v352::Rebuy(
	const FString& RoomId, double Amount, const FString& Currency,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	FString Body = FString::Printf(TEXT("{\"amount\":%.2f,\"currency\":\"%s\"}"), Amount, *Currency);
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Room::Rebuy(RoomId), Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

// ============================================================================
// ROUND & SETTLEMENT (5.6, 5.7)
// ============================================================================

void UDeskillzRoomExtensions_v352::SubmitRound(
	const FString& RoomId, const FString& PayloadJson,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Room::Round(RoomId), PayloadJson,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzRoomExtensions_v352::TriggerSettlement(
	const FString& RoomId,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Room::Settle(RoomId), TEXT("{}"),
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

// ============================================================================
// INVITES (5.8 - 5.10)
// ============================================================================

void UDeskillzRoomExtensions_v352::InvitePlayer(
	const FString& RoomId, const FString& TargetUsernameOrId,
	const FString& Message,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("targetUsernameOrId"), TargetUsernameOrId);
	Body->SetStringField(TEXT("message"), Message);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::Room::Invite(RoomId), Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzRoomExtensions_v352::GetMyInvites(
	const TFunction<void(const TArray<FDeskillzRoomInvite>&)>& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Room::MyInvites,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			TArray<FDeskillzRoomInvite> Invites;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* Arr;
				if (Json->TryGetArrayField(TEXT("invites"), Arr))
				{
					for (const auto& Val : *Arr)
					{
						if (Val->Type != EJson::Object) continue;
						auto Obj = Val->AsObject();
						FDeskillzRoomInvite Inv;
						Obj->TryGetStringField(TEXT("id"), Inv.Id);
						Obj->TryGetStringField(TEXT("roomId"), Inv.RoomId);
						Obj->TryGetStringField(TEXT("roomCode"), Inv.RoomCode);
						Obj->TryGetStringField(TEXT("roomName"), Inv.RoomName);
						Obj->TryGetStringField(TEXT("senderUsername"), Inv.SenderUsername);
						Obj->TryGetStringField(TEXT("message"), Inv.Message);
						Obj->TryGetNumberField(TEXT("entryFee"), Inv.EntryFee);
						Obj->TryGetStringField(TEXT("currency"), Inv.Currency);
						Obj->TryGetNumberField(TEXT("currentPlayers"), Inv.CurrentPlayers);
						Obj->TryGetNumberField(TEXT("maxPlayers"), Inv.MaxPlayers);
						Obj->TryGetStringField(TEXT("status"), Inv.Status);
						Invites.Add(Inv);
					}
				}
			}
			OnSuccess(Invites);
		}
	));
}

void UDeskillzRoomExtensions_v352::RespondToInvite(
	const FString& InviteId, bool bAccept,
	const FOnSuccess& OnSuccess, const FOnDeskillzApiError& OnError)
{
	FString Body = FString::Printf(TEXT("{\"accept\":%s}"), bAccept ? TEXT("true") : TEXT("false"));
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Room::RespondInvite(InviteId), Body,
		FOnDeskillzHttpResponse::CreateLambda([OnSuccess, OnError](const FDeskillzHttpResponse& R)
		{ R.IsOk() ? OnSuccess.ExecuteIfBound() : OnError.ExecuteIfBound(ParseError(R.Body, R.StatusCode)); }
	));
}

void UDeskillzRoomExtensions_v352::HandleInviteReceived(const FDeskillzRoomInvite& Invite)
{
	UE_LOG(LogTemp, Log, TEXT("[RoomExtensions] Invite received: %s from %s"), *Invite.RoomName, *Invite.SenderUsername);
	OnInviteReceived.Broadcast(Invite);
}

FDeskillzError UDeskillzRoomExtensions_v352::ParseError(const FString& Body, int32 StatusCode)
{
	FDeskillzError Err;
	Err.HttpStatusCode = StatusCode;
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		Json->TryGetStringField(TEXT("message"), Err.Message);
	if (Err.Message.IsEmpty()) Err.Message = TEXT("Room operation failed");
	Err.Code = EDeskillzErrorCode::ServerError;
	return Err;
}