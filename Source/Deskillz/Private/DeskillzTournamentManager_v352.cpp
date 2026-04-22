// Copyright Deskillz Games. All Rights Reserved.
// DeskillzTournamentManager_v352.cpp
// Path: Source/Deskillz/Private/DeskillzTournamentManager_v352.cpp

#include "DeskillzTournamentManager_v352.h"
#include "Network/DeskillzHttpClient.h"
#include "Core/DeskillzTypes_v352.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// Singleton
static UDeskillzTournamentManager_v352* GInstance = nullptr;

UDeskillzTournamentManager_v352* UDeskillzTournamentManager_v352::Get()
{
	if (!GInstance)
	{
		GInstance = NewObject<UDeskillzTournamentManager_v352>();
		GInstance->AddToRoot(); // Prevent GC
	}
	return GInstance;
}

// ============================================================================
// TOURNAMENT LISTING (3.1)
// ============================================================================

void UDeskillzTournamentManager_v352::GetTournaments(
	const TMap<FString, FString>& Filters,
	const FOnTournamentListings& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi::Tournament::List, FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			TArray<FDeskillzTournamentListing> Listings;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* TournamentsArray;
				if (Json->TryGetArrayField(TEXT("tournaments"), TournamentsArray))
				{
					for (const auto& Val : *TournamentsArray)
					{
						if (Val->Type == EJson::Object)
						{
							Listings.Add(ParseTournamentListing(Val->AsObject()));
						}
					}
				}
			}
			OnSuccess.ExecuteIfBound(Listings);
		}
	), Filters);
}

void UDeskillzTournamentManager_v352::GetActiveTournaments(
	const FString& GameId,
	const FOnTournamentListings& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi_v352::Tournament::ActiveByGame(GameId), FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			TArray<FDeskillzTournamentListing> Listings;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* Arr;
				if (Json->TryGetArrayField(TEXT("tournaments"), Arr))
				{
					for (const auto& Val : *Arr)
					{
						if (Val->Type == EJson::Object)
							Listings.Add(ParseTournamentListing(Val->AsObject()));
					}
				}
			}
			OnSuccess.ExecuteIfBound(Listings);
		}
	));
}

// ============================================================================
// REGISTRATION FLOW (3.2 - 3.5)
// ============================================================================

void UDeskillzTournamentManager_v352::Register(
	const FString& TournamentId,
	const FOnTournamentRegistration& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Post(DeskillzApi_v352::Tournament::Register(TournamentId), TEXT("{}"),
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
			{
				OnSuccess.ExecuteIfBound(ParseRegistration(Json));
			}
		}
	));
}

void UDeskillzTournamentManager_v352::CheckIn(
	const FString& TournamentId,
	const FOnTournamentRegistration& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Post(DeskillzApi_v352::Tournament::CheckIn(TournamentId), TEXT("{}"),
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
			{
				OnSuccess.ExecuteIfBound(ParseRegistration(Json));
			}
		}
	));
}

void UDeskillzTournamentManager_v352::Leave(
	const FString& TournamentId,
	const FOnSuccess& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Delete(DeskillzApi_v352::Tournament::Leave(TournamentId),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError, TournamentId, this](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			OnTournamentLeft.Broadcast(TournamentId);
			OnSuccess.ExecuteIfBound();
		}
	));
}

// ============================================================================
// STATUS & SCHEDULE (3.6 - 3.9)
// ============================================================================

void UDeskillzTournamentManager_v352::GetEnrollmentStatus(
	const FString& TournamentId,
	const FOnEnrollmentState& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi_v352::Tournament::MyStatus(TournamentId),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				// Return NOT_REGISTERED on error (mirrors web SDK)
				FDeskillzEnrollmentState EmptyState;
				EmptyState.Status = TEXT("NOT_REGISTERED");
				OnSuccess.ExecuteIfBound(EmptyState);
				return;
			}
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				OnSuccess.ExecuteIfBound(ParseEnrollmentState(Json));
			}
		}
	));
}

void UDeskillzTournamentManager_v352::GetMyRegistrations(
	const FOnTournamentRegistrations& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi_v352::Tournament::MyRegistrations,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			TArray<FDeskillzTournamentRegistration> Regs;
			TArray<TSharedPtr<FJsonValue>> JsonArray;
			if (UDeskillzHttpClient::ParseJsonArrayResponse(Response, JsonArray))
			{
				for (const auto& Val : JsonArray)
				{
					if (Val->Type == EJson::Object)
						Regs.Add(ParseRegistration(Val->AsObject()));
				}
			}
			OnSuccess.ExecuteIfBound(Regs);
		}
	));
}

void UDeskillzTournamentManager_v352::GetSchedule(
	const FString& TournamentId,
	const FOnTournamentSchedule& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi_v352::Tournament::Schedule(TournamentId),
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
			{
				OnSuccess.ExecuteIfBound(ParseSchedule(Json));
			}
		}
	));
}

void UDeskillzTournamentManager_v352::GetMyTableAssignment(
	const FString& TournamentId,
	const FOnTableAssignment& OnSuccess,
	const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(DeskillzApi_v352::Tournament::MySeat(TournamentId),
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
			{
				OnSuccess.ExecuteIfBound(ParseTableAssignment(Json));
			}
		}
	));
}

// ============================================================================
// SOCKET EVENT HANDLERS (3.10)
// ============================================================================

void UDeskillzTournamentManager_v352::HandleTournamentStarted(const FString& TournamentId)
{
	UE_LOG(LogTemp, Log, TEXT("[TournamentManager] Tournament started: %s"), *TournamentId);
	OnTournamentStarted.Broadcast(TournamentId);
}

void UDeskillzTournamentManager_v352::HandleTournamentLeft(const FString& TournamentId)
{
	UE_LOG(LogTemp, Log, TEXT("[TournamentManager] Tournament left: %s"), *TournamentId);
	OnTournamentLeft.Broadcast(TournamentId);
}

// ============================================================================
// JSON PARSERS
// ============================================================================

FDeskillzTournamentListing UDeskillzTournamentManager_v352::ParseTournamentListing(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzTournamentListing T;
	if (!Json.IsValid()) return T;

	Json->TryGetStringField(TEXT("id"), T.Id);
	Json->TryGetStringField(TEXT("name"), T.Name);
	Json->TryGetStringField(TEXT("description"), T.Description);
	Json->TryGetStringField(TEXT("gameId"), T.GameId);
	Json->TryGetStringField(TEXT("gameName"), T.GameName);
	Json->TryGetStringField(TEXT("status"), T.Status);
	Json->TryGetNumberField(TEXT("entryFee"), T.EntryFee);
	Json->TryGetStringField(TEXT("currency"), T.Currency);
	Json->TryGetNumberField(TEXT("prizePool"), T.PrizePool);
	Json->TryGetNumberField(TEXT("maxPlayers"), T.MaxPlayers);
	Json->TryGetNumberField(TEXT("currentPlayers"), T.CurrentPlayers);
	Json->TryGetNumberField(TEXT("minPlayersPerTable"), T.MinPlayersPerTable);
	Json->TryGetStringField(TEXT("socialGameType"), T.SocialGameType);
	Json->TryGetBoolField(TEXT("isFeatured"), T.bIsFeatured);

	return T;
}

FDeskillzTournamentRegistration UDeskillzTournamentManager_v352::ParseRegistration(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzTournamentRegistration R;
	if (!Json.IsValid()) return R;

	Json->TryGetStringField(TEXT("id"), R.Id);
	Json->TryGetStringField(TEXT("tournamentId"), R.TournamentId);
	Json->TryGetStringField(TEXT("tournamentName"), R.TournamentName);
	Json->TryGetStringField(TEXT("status"), R.Status);
	Json->TryGetStringField(TEXT("bookingStatus"), R.BookingStatus);
	Json->TryGetNumberField(TEXT("entryFee"), R.EntryFee);
	Json->TryGetStringField(TEXT("currency"), R.Currency);

	return R;
}

FDeskillzEnrollmentState UDeskillzTournamentManager_v352::ParseEnrollmentState(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzEnrollmentState S;
	if (!Json.IsValid()) return S;

	Json->TryGetStringField(TEXT("status"), S.Status);
	Json->TryGetStringField(TEXT("bookingStatus"), S.BookingStatus);
	Json->TryGetStringField(TEXT("tournamentId"), S.TournamentId);
	Json->TryGetBoolField(TEXT("isRegistered"), S.bIsRegistered);
	Json->TryGetBoolField(TEXT("isCheckedIn"), S.bIsCheckedIn);
	Json->TryGetBoolField(TEXT("canCheckIn"), S.bCanCheckIn);
	Json->TryGetBoolField(TEXT("canLeave"), S.bCanLeave);

	return S;
}

FDeskillzTournamentSchedule UDeskillzTournamentManager_v352::ParseSchedule(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzTournamentSchedule Sched;
	if (!Json.IsValid()) return Sched;

	Json->TryGetStringField(TEXT("tournamentId"), Sched.TournamentId);
	Json->TryGetNumberField(TEXT("totalRounds"), Sched.TotalRounds);
	Json->TryGetNumberField(TEXT("currentRound"), Sched.CurrentRound);
	Json->TryGetNumberField(TEXT("totalPlayers"), Sched.TotalPlayers);
	Json->TryGetNumberField(TEXT("playersRemaining"), Sched.PlayersRemaining);

	const TArray<TSharedPtr<FJsonValue>>* RoundsArray;
	if (Json->TryGetArrayField(TEXT("rounds"), RoundsArray))
	{
		for (const auto& RVal : *RoundsArray)
		{
			if (RVal->Type != EJson::Object) continue;
			auto RObj = RVal->AsObject();

			FDeskillzTournamentRound Round;
			RObj->TryGetStringField(TEXT("id"), Round.Id);
			RObj->TryGetNumberField(TEXT("roundNumber"), Round.RoundNumber);
			RObj->TryGetStringField(TEXT("status"), Round.Status);
			RObj->TryGetNumberField(TEXT("totalTables"), Round.TotalTables);
			RObj->TryGetNumberField(TEXT("playersRemaining"), Round.PlayersRemaining);

			const TArray<TSharedPtr<FJsonValue>>* TablesArray;
			if (RObj->TryGetArrayField(TEXT("tables"), TablesArray))
			{
				for (const auto& TVal : *TablesArray)
				{
					if (TVal->Type != EJson::Object) continue;
					auto TObj = TVal->AsObject();

					FDeskillzTournamentTable Table;
					TObj->TryGetStringField(TEXT("id"), Table.Id);
					TObj->TryGetNumberField(TEXT("tableNumber"), Table.TableNumber);
					TObj->TryGetStringField(TEXT("status"), Table.Status);
					TObj->TryGetStringField(TEXT("matchId"), Table.MatchId);

					const TArray<TSharedPtr<FJsonValue>>* SeatsArray;
					if (TObj->TryGetArrayField(TEXT("seats"), SeatsArray))
					{
						for (const auto& SVal : *SeatsArray)
						{
							if (SVal->Type != EJson::Object) continue;
							auto SObj = SVal->AsObject();

							FDeskillzTableSeat Seat;
							SObj->TryGetNumberField(TEXT("seatNumber"), Seat.SeatNumber);
							SObj->TryGetStringField(TEXT("playerId"), Seat.PlayerId);
							SObj->TryGetStringField(TEXT("username"), Seat.Username);
							SObj->TryGetBoolField(TEXT("isNPC"), Seat.bIsNPC);
							SObj->TryGetNumberField(TEXT("finalScore"), Seat.FinalScore);
							SObj->TryGetBoolField(TEXT("isWinner"), Seat.bIsWinner);
							Table.Seats.Add(Seat);
						}
					}
					Round.Tables.Add(Table);
				}
			}
			Sched.Rounds.Add(Round);
		}
	}

	return Sched;
}

FDeskillzTableAssignment UDeskillzTournamentManager_v352::ParseTableAssignment(const TSharedPtr<FJsonObject>& Json)
{
	FDeskillzTableAssignment A;
	if (!Json.IsValid()) return A;

	Json->TryGetStringField(TEXT("tournamentId"), A.TournamentId);
	Json->TryGetNumberField(TEXT("roundNumber"), A.RoundNumber);
	Json->TryGetStringField(TEXT("tableId"), A.TableId);
	Json->TryGetNumberField(TEXT("tableNumber"), A.TableNumber);
	Json->TryGetNumberField(TEXT("seatNumber"), A.SeatNumber);
	Json->TryGetStringField(TEXT("matchId"), A.MatchId);

	const TArray<TSharedPtr<FJsonValue>>* OppsArray;
	if (Json->TryGetArrayField(TEXT("opponents"), OppsArray))
	{
		for (const auto& Val : *OppsArray)
		{
			if (Val->Type != EJson::Object) continue;
			auto Obj = Val->AsObject();
			FDeskillzTableSeat Seat;
			Obj->TryGetNumberField(TEXT("seatNumber"), Seat.SeatNumber);
			Obj->TryGetStringField(TEXT("playerId"), Seat.PlayerId);
			Obj->TryGetStringField(TEXT("username"), Seat.Username);
			Obj->TryGetBoolField(TEXT("isNPC"), Seat.bIsNPC);
			A.Opponents.Add(Seat);
		}
	}

	return A;
}

FDeskillzError UDeskillzTournamentManager_v352::ParseError(const FString& ResponseBody, int32 StatusCode)
{
	FDeskillzError Err;
	Err.HttpStatusCode = StatusCode;

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
	{
		Json->TryGetStringField(TEXT("message"), Err.Message);
	}

	if (Err.Message.IsEmpty())
	{
		switch (StatusCode)
		{
			case 401: Err.Message = TEXT("Authentication required"); break;
			case 404: Err.Message = TEXT("Tournament not found"); break;
			case 409: Err.Message = TEXT("Already registered"); break;
			default:  Err.Message = TEXT("Unknown error"); break;
		}
	}

	Err.Code = EDeskillzErrorCode::ServerError;
	return Err;
}