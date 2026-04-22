// Copyright Deskillz Games. All Rights Reserved.
// DeskillzHostExtensions_v352.cpp
// Path: Source/Deskillz/Private/DeskillzHostExtensions_v352.cpp

#include "DeskillzHostExtensions_v352.h"
#include "Network/DeskillzHttpClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

static UDeskillzHostExtensions_v352* GHostExtInstance = nullptr;

UDeskillzHostExtensions_v352* UDeskillzHostExtensions_v352::Get()
{
	if (!GHostExtInstance)
	{
		GHostExtInstance = NewObject<UDeskillzHostExtensions_v352>();
		GHostExtInstance->AddToRoot();
	}
	return GHostExtInstance;
}

// ============================================================================
// GET DASHBOARD (8.1)
// ============================================================================

void UDeskillzHostExtensions_v352::GetDashboard(
	const FOnHostDashboard& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Host::Dashboard,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}

			FDeskillzHostDashboard Dashboard;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				// Profile
				const TSharedPtr<FJsonObject>* ProfileObj;
				if (Json->TryGetObjectField(TEXT("profile"), ProfileObj))
				{
					(*ProfileObj)->TryGetNumberField(TEXT("totalPlayersHosted"), Dashboard.TotalPlayersHosted);
					(*ProfileObj)->TryGetNumberField(TEXT("totalRoomsCompleted"), Dashboard.TotalRoomsCompleted);
					(*ProfileObj)->TryGetNumberField(TEXT("activeRoomCount"), Dashboard.ActiveRoomCount);
					(*ProfileObj)->TryGetNumberField(TEXT("hostLevel"), Dashboard.HostLevel);
					(*ProfileObj)->TryGetStringField(TEXT("hostTitle"), Dashboard.HostTitle);
					(*ProfileObj)->TryGetBoolField(TEXT("isVerified"), Dashboard.bIsVerified);
					(*ProfileObj)->TryGetStringField(TEXT("currentEsportsTier"), Dashboard.EsportsTier);
					(*ProfileObj)->TryGetStringField(TEXT("currentSocialTier"), Dashboard.SocialTier);
				}

				// Earnings
				const TSharedPtr<FJsonObject>* EarningsObj;
				if (Json->TryGetObjectField(TEXT("earnings"), EarningsObj))
				{
					(*EarningsObj)->TryGetNumberField(TEXT("totalAllTime"), Dashboard.TotalEarnings);
					(*EarningsObj)->TryGetNumberField(TEXT("totalThisMonth"), Dashboard.MonthlyEarnings);
					(*EarningsObj)->TryGetNumberField(TEXT("pendingSettlement"), Dashboard.PendingSettlement);
					(*EarningsObj)->TryGetNumberField(TEXT("availableWithdrawal"), Dashboard.AvailableWithdrawal);
					(*EarningsObj)->TryGetNumberField(TEXT("esportsEarnings"), Dashboard.EsportsEarnings);
					(*EarningsObj)->TryGetNumberField(TEXT("socialEarnings"), Dashboard.SocialEarnings);
				}

				// Tier info
				const TSharedPtr<FJsonObject>* EsTierObj;
				if (Json->TryGetObjectField(TEXT("esportsTierInfo"), EsTierObj))
					(*EsTierObj)->TryGetNumberField(TEXT("hostShare"), Dashboard.EsportsHostShare);

				const TSharedPtr<FJsonObject>* SocTierObj;
				if (Json->TryGetObjectField(TEXT("socialTierInfo"), SocTierObj))
					(*SocTierObj)->TryGetNumberField(TEXT("hostShare"), Dashboard.SocialHostShare);
			}
			OnSuccess.ExecuteIfBound(Dashboard);
		}
	));
}

// ============================================================================
// WITHDRAW ALL (8.2)
// ============================================================================

void UDeskillzHostExtensions_v352::WithdrawAllEarnings(
	const FOnHostWithdrawal& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Post(DeskillzApi_v352::Host::Withdraw, TEXT("{}"),
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			FDeskillzHostWithdrawalResponse Result;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetStringField(TEXT("transactionId"), Result.TransactionId);
				Json->TryGetStringField(TEXT("estimatedArrival"), Result.EstimatedArrival);
			}
			OnSuccess.ExecuteIfBound(Result);
		}
	));
}

// ============================================================================
// REQUEST WITHDRAWAL WITH PARAMS (8.3)
// ============================================================================

void UDeskillzHostExtensions_v352::RequestWithdrawal(
	double Amount, const FString& Currency, const FString& WalletAddress,
	const FOnHostWithdrawal& OnSuccess, const FOnDeskillzApiError& OnError)
{
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetNumberField(TEXT("amount"), Amount);
	Body->SetStringField(TEXT("currency"), Currency);
	Body->SetStringField(TEXT("walletAddress"), WalletAddress);

	UDeskillzHttpClient::Get()->PostJson(DeskillzApi_v352::Host::Withdraw, Body,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			FDeskillzHostWithdrawalResponse Result;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetStringField(TEXT("transactionId"), Result.TransactionId);
				Json->TryGetStringField(TEXT("estimatedArrival"), Result.EstimatedArrival);
			}
			OnSuccess.ExecuteIfBound(Result);
		}
	));
}

// ============================================================================
// AGE VERIFICATION (8.4)
// ============================================================================

void UDeskillzHostExtensions_v352::CheckAgeVerified(
	const FOnAgeVerification& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Host::VerifyAgeStatus,
		FOnDeskillzHttpResponse::CreateLambda(
		[OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				OnError.ExecuteIfBound(ParseError(Response.Body, Response.StatusCode));
				return;
			}
			FDeskillzAgeVerificationStatus Status;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
				Json->TryGetBoolField(TEXT("isVerified"), Status.bIsVerified);
			OnSuccess.ExecuteIfBound(Status);
		}
	));
}

// ============================================================================
// GAME CAPABILITIES (9.1)
// ============================================================================

void UDeskillzHostExtensions_v352::GetGameCapabilities(
	const FString& GameId,
	const FOnGameCapabilities& OnSuccess, const FOnDeskillzApiError& OnError)
{
	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Capabilities::ForGame(GameId),
		FOnDeskillzHttpResponse::CreateLambda(
		[this, OnSuccess, OnError](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				// Fallback to defaults
				OnSuccess.ExecuteIfBound(FDeskillzGameCapabilities::Default());
				return;
			}

			FDeskillzGameCapabilities Caps;
			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				Json->TryGetBoolField(TEXT("supports1v1"), Caps.bSupports1v1);
				Json->TryGetBoolField(TEXT("supportsFFA"), Caps.bSupportsFFA);
				Json->TryGetBoolField(TEXT("supportsSingleElimination"), Caps.bSupportsSingleElimination);
				Json->TryGetBoolField(TEXT("supportsSync"), Caps.bSupportsSync);
				Json->TryGetBoolField(TEXT("supportsAsync"), Caps.bSupportsAsync);
				Json->TryGetBoolField(TEXT("supportsTurnBased"), Caps.bSupportsTurnBased);
				Json->TryGetNumberField(TEXT("minMatchDurationSeconds"), Caps.MinMatchDurationSeconds);
				Json->TryGetNumberField(TEXT("maxMatchDurationSeconds"), Caps.MaxMatchDurationSeconds);
				Json->TryGetNumberField(TEXT("maxTournamentSize"), Caps.MaxTournamentSize);
				Json->TryGetNumberField(TEXT("minPlayersPerMatch"), Caps.MinPlayersPerMatch);
				Json->TryGetNumberField(TEXT("maxPlayersPerMatch"), Caps.MaxPlayersPerMatch);
			}

			CachedCapabilities = Caps;
			bCapabilitiesFetched = true;
			OnSuccess.ExecuteIfBound(Caps);
		}
	));
}

FDeskillzError UDeskillzHostExtensions_v352::ParseError(const FString& Body, int32 StatusCode)
{
	FDeskillzError Err;
	Err.HttpStatusCode = StatusCode;
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		Json->TryGetStringField(TEXT("message"), Err.Message);
	if (Err.Message.IsEmpty()) Err.Message = TEXT("Host operation failed");
	Err.Code = EDeskillzErrorCode::ServerError;
	return Err;
}