// Copyright Deskillz Games. All Rights Reserved.
// DeskillzSessionManager_v352.cpp
// Path: Source/Deskillz/Private/DeskillzSessionManager_v352.cpp

#include "DeskillzSessionManager_v352.h"
#include "Network/DeskillzHttpClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/CommandLine.h"

static UDeskillzSessionManager_v352* GSessionInstance = nullptr;

UDeskillzSessionManager_v352* UDeskillzSessionManager_v352::Get()
{
	if (!GSessionInstance)
	{
		GSessionInstance = NewObject<UDeskillzSessionManager_v352>();
		GSessionInstance->AddToRoot();
	}
	return GSessionInstance;
}

// ============================================================================
// SSO TOKEN (1.1)
// ============================================================================

void UDeskillzSessionManager_v352::ConsumeSSOToken()
{
	// Check command line for launch URL with token
	FString CmdLine = FCommandLine::Get();
	FString Token = ExtractQueryParam(CmdLine, TEXT("token"));

	if (Token.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[SessionManager] No SSO token in launch args"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SessionManager] SSO token found, validating..."));

	// Set token on HTTP client
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->SetAuthToken(Token);

	// Validate by fetching user profile
	Http->Get(TEXT("/api/v1/users/me"),
		FOnDeskillzHttpResponse::CreateLambda(
		[this](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				UE_LOG(LogTemp, Warning, TEXT("[SessionManager] SSO token invalid: %s"), *Response.ErrorMessage);
				return;
			}

			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				FDeskillzAuthUser User;
				Json->TryGetStringField(TEXT("id"), User.Id);
				Json->TryGetStringField(TEXT("username"), User.Username);
				Json->TryGetStringField(TEXT("email"), User.Email);
				Json->TryGetStringField(TEXT("avatarUrl"), User.AvatarUrl);
				Json->TryGetStringField(TEXT("walletAddress"), User.WalletAddress);
				Json->TryGetBoolField(TEXT("emailVerified"), User.bEmailVerified);
				User.bHasWallet = !User.WalletAddress.IsEmpty();

				UE_LOG(LogTemp, Log, TEXT("[SessionManager] SSO authenticated: %s"), *User.Username);
				OnSSOAuthenticated.Broadcast(User);
			}
		}
	));
}

// ============================================================================
// ACTIVE SESSION (1.2, 1.3)
// ============================================================================

void UDeskillzSessionManager_v352::CheckForActiveSession(const FOnActiveSession& OnResult)
{
	if (bIsGuest)
	{
		CachedSession = FDeskillzActiveSessionPayload();
		OnResult.ExecuteIfBound(CachedSession);
		return;
	}

	UDeskillzHttpClient::Get()->Get(DeskillzApi_v352::Room::MyActive,
		FOnDeskillzHttpResponse::CreateLambda(
		[this, OnResult](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk() || Response.Body.IsEmpty() || Response.Body == TEXT("null"))
			{
				CachedSession = FDeskillzActiveSessionPayload();
				OnResult.ExecuteIfBound(CachedSession);
				return;
			}

			TSharedPtr<FJsonObject> Json;
			if (UDeskillzHttpClient::ParseJsonResponse(Response, Json) && Json.IsValid())
			{
				FDeskillzActiveSessionPayload Session;
				Json->TryGetStringField(TEXT("type"), Session.Type);
				Json->TryGetStringField(TEXT("roomId"), Session.RoomId);
				Json->TryGetStringField(TEXT("roomCode"), Session.RoomCode);
				Json->TryGetStringField(TEXT("matchId"), Session.MatchId);
				Json->TryGetStringField(TEXT("tournamentId"), Session.TournamentId);
				Json->TryGetStringField(TEXT("gameId"), Session.GameId);
				Json->TryGetStringField(TEXT("status"), Session.Status);

				if (!Session.RoomId.IsEmpty())
				{
					Session.bHasActiveSession = true;
					CachedSession = Session;
					UE_LOG(LogTemp, Log, TEXT("[SessionManager] Active session: %s room=%s"),
						*Session.Type, *Session.RoomCode);
					OnSessionResumed.Broadcast(Session);
				}
				else
				{
					CachedSession = FDeskillzActiveSessionPayload();
				}
			}

			OnResult.ExecuteIfBound(CachedSession);
		}
	));
}

// ============================================================================
// GUEST MODE (1.6)
// ============================================================================

void UDeskillzSessionManager_v352::EnableGuestMode()
{
	bIsGuest = true;
	UE_LOG(LogTemp, Log, TEXT("[SessionManager] Guest mode enabled"));
	OnGuestModeActivated.Broadcast();
}

void UDeskillzSessionManager_v352::DisableGuestMode()
{
	bIsGuest = false;
	UE_LOG(LogTemp, Log, TEXT("[SessionManager] Guest mode disabled"));
}

bool UDeskillzSessionManager_v352::CanPerformAction(const FString& ActionName) const
{
	if (!bIsGuest) return true;
	UE_LOG(LogTemp, Warning, TEXT("[SessionManager] Action '%s' blocked in guest mode"), *ActionName);
	return false;
}

void UDeskillzSessionManager_v352::Reset()
{
	bIsGuest = false;
	CachedSession = FDeskillzActiveSessionPayload();
}

// ============================================================================
// HELPERS
// ============================================================================

FString UDeskillzSessionManager_v352::ExtractQueryParam(const FString& Url, const FString& ParamName)
{
	int32 QueryStart = Url.Find(TEXT("?"));
	if (QueryStart == INDEX_NONE) return FString();

	FString Query = Url.Mid(QueryStart + 1);
	TArray<FString> Pairs;
	Query.ParseIntoArray(Pairs, TEXT("&"));

	for (const FString& Pair : Pairs)
	{
		FString Key, Value;
		if (Pair.Split(TEXT("="), &Key, &Value) && Key == ParamName)
		{
			return FGenericPlatformHttp::UrlDecode(Value);
		}
	}

	return FString();
}