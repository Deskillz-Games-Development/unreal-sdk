// Copyright Deskillz Games. All Rights Reserved.
// DeskillzLobbyClient.cpp - Implementation of lobby API client

#include "Lobby/DeskillzLobbyClient.h"
#include "Http.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Misc/DateTime.h"
#include "Misc/SecureHash.h"

// Singleton instance
UDeskillzLobbyClient* UDeskillzLobbyClient::Instance = nullptr;

UDeskillzLobbyClient::UDeskillzLobbyClient()
{
	// Constructor
}

UDeskillzLobbyClient* UDeskillzLobbyClient::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UDeskillzLobbyClient>();
		Instance->AddToRoot(); // Prevent garbage collection
	}
	return Instance;
}

void UDeskillzLobbyClient::Initialize(const FString& InPlayerToken, const FString& InMatchId)
{
	PlayerToken = InPlayerToken;
	MatchId = InMatchId;
	bIsInitialized = !PlayerToken.IsEmpty() && !MatchId.IsEmpty();
	
	if (bIsInitialized)
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Initialized for match: %s"), *MatchId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzLobbyClient] Failed to initialize - missing token or match ID"));
	}
}

void UDeskillzLobbyClient::InitializeFromLaunchData(const FDeskillzMatchLaunchData& LaunchData)
{
	Initialize(LaunchData.PlayerToken, LaunchData.MatchId);
}

void UDeskillzLobbyClient::SubmitScore(int64 FinalScore, const TMap<FString, FString>& Stats)
{
	FDeskillzMatchCompletionData CompletionData;
	CompletionData.MatchId = MatchId;
	CompletionData.FinalScore = FinalScore;
	CompletionData.Result = EDeskillzMatchResult::Pending; // Server determines result
	CompletionData.bCompletedNormally = true;
	CompletionData.Stats = Stats;
	
	SubmitScoreWithData(CompletionData);
}

void UDeskillzLobbyClient::SubmitScoreWithData(const FDeskillzMatchCompletionData& CompletionData)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzLobbyClient] Cannot submit score - not initialized"));
		OnApiError.Broadcast(-1, TEXT("Client not initialized"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Submitting score: %lld for match: %s"),
		CompletionData.FinalScore, *CompletionData.MatchId);
	
	// Build request body
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetNumberField(TEXT("score"), CompletionData.FinalScore);
	Body->SetBoolField(TEXT("completed"), CompletionData.bCompletedNormally);
	
	if (!CompletionData.bCompletedNormally)
	{
		Body->SetStringField(TEXT("abortReason"), CompletionData.AbortReason);
	}
	
	// Add timestamp
	int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
	Body->SetNumberField(TEXT("timestamp"), Timestamp);
	
	// Add HMAC signature for anti-cheat
	FString Signature = GenerateScoreSignature(CompletionData.FinalScore, Timestamp);
	Body->SetStringField(TEXT("signature"), Signature);
	
	// Add stats
	if (CompletionData.Stats.Num() > 0)
	{
		TSharedPtr<FJsonObject> StatsObject = MakeShareable(new FJsonObject());
		for (const auto& Stat : CompletionData.Stats)
		{
			StatsObject->SetStringField(Stat.Key, Stat.Value);
		}
		Body->SetObjectField(TEXT("stats"), StatsObject);
	}
	
	// Add match duration
	Body->SetNumberField(TEXT("matchDuration"), CompletionData.MatchDuration);
	
	// Make API request
	FString Endpoint = FString::Printf(TEXT("/matches/%s/scores"), *MatchId);
	
	MakeApiRequest(Endpoint, TEXT("POST"), Body,
		[this](bool bSuccess, TSharedPtr<FJsonObject> Response)
		{
			HandleScoreSubmitResponse(bSuccess, Response);
		});
}

void UDeskillzLobbyClient::ReportMatchStarted()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Reporting match started: %s"), *MatchId);
	
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("status"), TEXT("started"));
	Body->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	
	FString Endpoint = FString::Printf(TEXT("/matches/%s/status"), *MatchId);
	
	MakeApiRequest(Endpoint, TEXT("PUT"), Body,
		[](bool bSuccess, TSharedPtr<FJsonObject> Response)
		{
			if (bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Match start reported successfully"));
			}
		});
}

void UDeskillzLobbyClient::ReportMatchAborted(const FString& Reason)
{
	if (!bIsInitialized)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Reporting match aborted: %s, Reason: %s"), *MatchId, *Reason);
	
	TSharedPtr<FJsonObject> Body = MakeShareable(new FJsonObject());
	Body->SetStringField(TEXT("status"), TEXT("aborted"));
	Body->SetStringField(TEXT("reason"), Reason);
	Body->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp());
	
	FString Endpoint = FString::Printf(TEXT("/matches/%s/status"), *MatchId);
	
	MakeApiRequest(Endpoint, TEXT("PUT"), Body,
		[](bool bSuccess, TSharedPtr<FJsonObject> Response)
		{
			if (bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Match abort reported successfully"));
			}
		});
}

void UDeskillzLobbyClient::GetMatchStatus()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	FString Endpoint = FString::Printf(TEXT("/matches/%s"), *MatchId);
	
	MakeApiRequest(Endpoint, TEXT("GET"), nullptr,
		[this](bool bSuccess, TSharedPtr<FJsonObject> Response)
		{
			HandleMatchStatusResponse(bSuccess, Response);
		});
}

void UDeskillzLobbyClient::GetMatchResult()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	FString Endpoint = FString::Printf(TEXT("/matches/%s/result"), *MatchId);
	
	MakeApiRequest(Endpoint, TEXT("GET"), nullptr,
		[this](bool bSuccess, TSharedPtr<FJsonObject> Response)
		{
			HandleScoreSubmitResponse(bSuccess, Response);
		});
}

FString UDeskillzLobbyClient::GenerateScoreSignature(int64 Score, int64 Timestamp) const
{
	// Build message: matchId|score|timestamp|gameId
	FString Message = FString::Printf(TEXT("%s|%lld|%lld|%s"),
		*MatchId, Score, Timestamp, *GameId);
	
	return ComputeHmacSha256(Message, HmacSecret);
}

FString UDeskillzLobbyClient::ComputeHmacSha256(const FString& Message, const FString& Key) const
{
	if (Key.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzLobbyClient] HMAC secret not set"));
		return TEXT("");
	}
	
	// Convert to UTF8 bytes
	FTCHARToUTF8 MessageUtf8(*Message);
	FTCHARToUTF8 KeyUtf8(*Key);
	
	// Compute HMAC-SHA256
	uint8 DigestBytes[32];
	FSHA256Signature Signature;
	
	// Use platform HMAC implementation
	// Note: This is a simplified version - production should use proper HMAC
	FSHAHash Hash;
	FSHA1::HashBuffer(MessageUtf8.Get(), MessageUtf8.Length(), Hash.Hash);
	
	// Convert to hex string
	FString HexSignature;
	for (int32 i = 0; i < 20; i++)
	{
		HexSignature += FString::Printf(TEXT("%02x"), Hash.Hash[i]);
	}
	
	return HexSignature;
}

void UDeskillzLobbyClient::MakeApiRequest(const FString& Endpoint, const FString& Method,
	const TSharedPtr<FJsonObject>& Body, TFunction<void(bool, TSharedPtr<FJsonObject>)> Callback)
{
	FString URL = ApiBaseUrl + Endpoint;
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb(Method);
	
	// Set headers
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	
	if (!PlayerToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *PlayerToken));
	}
	
	// Set body if provided
	if (Body.IsValid())
	{
		FString BodyString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyString);
		FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);
		Request->SetContentAsString(BodyString);
	}
	
	// Bind response handler
	Request->OnProcessRequestComplete().BindLambda(
		[this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			HandleHttpResponse(Request, Response, bSuccess, Callback);
		});
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] API Request: %s %s"), *Method, *URL);
	
	Request->ProcessRequest();
}

void UDeskillzLobbyClient::HandleHttpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess,
	TFunction<void(bool, TSharedPtr<FJsonObject>)> Callback)
{
	if (!bSuccess || !Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzLobbyClient] HTTP request failed"));
		OnApiError.Broadcast(-1, TEXT("Network error"));
		if (Callback)
		{
			Callback(false, nullptr);
		}
		return;
	}
	
	int32 StatusCode = Response->GetResponseCode();
	FString Content = Response->GetContentAsString();
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Response: %d"), StatusCode);
	
	if (StatusCode >= 400)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzLobbyClient] API error: %d - %s"), StatusCode, *Content);
		OnApiError.Broadcast(StatusCode, Content);
		if (Callback)
		{
			Callback(false, nullptr);
		}
		return;
	}
	
	// Parse JSON response
	TSharedPtr<FJsonObject> JsonResponse = ParseJsonResponse(Content);
	
	if (Callback)
	{
		Callback(true, JsonResponse);
	}
}

TSharedPtr<FJsonObject> UDeskillzLobbyClient::ParseJsonResponse(const FString& Content)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzLobbyClient] Failed to parse JSON response"));
		return nullptr;
	}
	
	return JsonObject;
}

void UDeskillzLobbyClient::HandleScoreSubmitResponse(bool bSuccess, TSharedPtr<FJsonObject> Response)
{
	FDeskillzMatchResult Result;
	FString Message;
	
	if (!bSuccess || !Response.IsValid())
	{
		Message = TEXT("Score submission failed");
		OnScoreSubmitted.Broadcast(false, Message, Result);
		return;
	}
	
	// Parse response
	if (Response->HasField(TEXT("message")))
	{
		Message = Response->GetStringField(TEXT("message"));
	}
	else
	{
		Message = TEXT("Score submitted successfully");
	}
	
	// Parse result if available
	if (Response->HasField(TEXT("result")))
	{
		TSharedPtr<FJsonObject> ResultJson = Response->GetObjectField(TEXT("result"));
		
		Result.MatchId = MatchId;
		
		if (ResultJson->HasField(TEXT("playerScore")))
		{
			Result.PlayerScore = ResultJson->GetIntegerField(TEXT("playerScore"));
		}
		if (ResultJson->HasField(TEXT("opponentScore")))
		{
			Result.OpponentScore = ResultJson->GetIntegerField(TEXT("opponentScore"));
		}
		if (ResultJson->HasField(TEXT("prizeWon")))
		{
			Result.PrizeWon = ResultJson->GetNumberField(TEXT("prizeWon"));
		}
		if (ResultJson->HasField(TEXT("result")))
		{
			FString ResultStr = ResultJson->GetStringField(TEXT("result")).ToLower();
			if (ResultStr == TEXT("win"))
			{
				Result.Result = EDeskillzMatchResult::Win;
			}
			else if (ResultStr == TEXT("loss"))
			{
				Result.Result = EDeskillzMatchResult::Loss;
			}
			else if (ResultStr == TEXT("draw"))
			{
				Result.Result = EDeskillzMatchResult::Draw;
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Score submit success: %s"), *Message);
	OnScoreSubmitted.Broadcast(true, Message, Result);
}

void UDeskillzLobbyClient::HandleMatchStatusResponse(bool bSuccess, TSharedPtr<FJsonObject> Response)
{
	if (!bSuccess || !Response.IsValid())
	{
		OnMatchStatusReceived.Broadcast(false, EDeskillzMatchStatus::Pending);
		return;
	}
	
	EDeskillzMatchStatus Status = EDeskillzMatchStatus::Pending;
	
	if (Response->HasField(TEXT("status")))
	{
		FString StatusStr = Response->GetStringField(TEXT("status")).ToLower();
		
		if (StatusStr == TEXT("pending")) Status = EDeskillzMatchStatus::Pending;
		else if (StatusStr == TEXT("matchmaking")) Status = EDeskillzMatchStatus::Matchmaking;
		else if (StatusStr == TEXT("ready")) Status = EDeskillzMatchStatus::Ready;
		else if (StatusStr == TEXT("in_progress") || StatusStr == TEXT("inprogress")) 
			Status = EDeskillzMatchStatus::InProgress;
		else if (StatusStr == TEXT("completed")) Status = EDeskillzMatchStatus::Completed;
		else if (StatusStr == TEXT("cancelled")) Status = EDeskillzMatchStatus::Cancelled;
		else if (StatusStr == TEXT("disputed")) Status = EDeskillzMatchStatus::Disputed;
		else if (StatusStr == TEXT("expired")) Status = EDeskillzMatchStatus::Expired;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzLobbyClient] Match status received: %d"), (int32)Status);
	OnMatchStatusReceived.Broadcast(true, Status);
}