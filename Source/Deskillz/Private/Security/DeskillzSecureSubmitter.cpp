// Copyright Deskillz Games. All Rights Reserved.

#include "Security/DeskillzSecureSubmitter.h"
#include "Core/DeskillzSDK.h"
#include "Core/DeskillzConfig.h"
#include "Deskillz.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

// Static singleton
static UDeskillzSecureSubmitter* GSecureSubmitter = nullptr;

UDeskillzSecureSubmitter::UDeskillzSecureSubmitter()
{
	// Create encryption instance
	Encryption = NewObject<UDeskillzScoreEncryption>(this);
}

UDeskillzSecureSubmitter* UDeskillzSecureSubmitter::Get(const UObject* WorldContextObject)
{
	if (!GSecureSubmitter)
	{
		GSecureSubmitter = NewObject<UDeskillzSecureSubmitter>();
		GSecureSubmitter->AddToRoot(); // Prevent garbage collection
	}
	
	// Ensure SDK reference is set
	if (!GSecureSubmitter->SDK && WorldContextObject)
	{
		GSecureSubmitter->SDK = UDeskillzSDK::Get(WorldContextObject);
	}
	
	return GSecureSubmitter;
}

void UDeskillzSecureSubmitter::Initialize(UDeskillzSDK* InSDK)
{
	SDK = InSDK;
	
	// Reset state
	bIsSubmitting = false;
	CurrentStatus = EDeskillzSubmissionStatus::Pending;
	CurrentRetryAttempt = 0;
	SubmissionQueue.Empty();
}

void UDeskillzSecureSubmitter::SubmitScore(int64 Score, bool bForceSubmit)
{
	// Validate score first
	FString ValidationError;
	if (!ValidateScore(Score, ValidationError))
	{
		UE_LOG(LogDeskillz, Error, TEXT("Score validation failed: %s"), *ValidationError);
		
		FDeskillzSubmissionResult FailResult;
		FailResult.Status = EDeskillzSubmissionStatus::Rejected;
		FailResult.ErrorMessage = ValidationError;
		FailResult.ErrorCode = -1;
		
		OnSubmissionComplete.Broadcast(FailResult, Score);
		return;
	}
	
	// Check if already submitting
	if (bIsSubmitting && !bForceSubmit)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Submission already in progress, queueing score: %lld"), Score);
		
		// Queue for later
		if (bEnableOfflineQueue)
		{
			FDeskillzQueuedSubmission QueuedEntry;
			QueuedEntry.OriginalScore = Score;
			QueuedEntry.QueuedAt = UDeskillzScoreEncryption::GetTimestampMs();
			QueuedEntry.Status = EDeskillzSubmissionStatus::Pending;
			
			if (Encryption->IsInitialized())
			{
				QueuedEntry.EncryptedScore = Encryption->EncryptScoreSimple(Score);
			}
			
			QueueSubmission(QueuedEntry);
		}
		return;
	}
	
	// Collect integrity data and encrypt
	FDeskillzScoreIntegrity Integrity;
	if (SDK)
	{
		// Get match info for integrity data
		FDeskillzMatchInfo Match = SDK->GetCurrentMatch();
		Integrity.RandomSeed = Match.RandomSeed;
	}
	
	SubmitScoreWithIntegrity(Score, Integrity);
}

void UDeskillzSecureSubmitter::SubmitScoreWithIntegrity(int64 Score, const FDeskillzScoreIntegrity& Integrity)
{
	UE_LOG(LogDeskillz, Log, TEXT("Submitting score: %lld"), Score);
	
	// Create submission entry
	FDeskillzQueuedSubmission Submission;
	Submission.OriginalScore = Score;
	Submission.QueuedAt = UDeskillzScoreEncryption::GetTimestampMs();
	Submission.Status = EDeskillzSubmissionStatus::Pending;
	
	// Get match ID
	if (SDK)
	{
		Submission.MatchId = SDK->GetCurrentMatch().MatchId;
		
		// Initialize encryption if not already
		if (!Encryption->IsInitialized())
		{
			// In production, keys come from server when match starts
			// For now, use placeholder keys
			FString TempKey = TEXT("dGVtcG9yYXJ5X2VuY3J5cHRpb25fa2V5XzMyYnl0ZXM="); // 32 bytes base64
			FString TempSigning = TEXT("dGVtcG9yYXJ5X3NpZ25pbmdfa2V5"); // signing key
			Encryption->Initialize(TempKey, TempSigning, Submission.MatchId);
		}
	}
	
	// Encrypt the score
	FDeskillzScoreIntegrity FullIntegrity = Integrity;
	FullIntegrity.Score = Score;
	FullIntegrity.ScoreUpdateCount = Encryption->GetScoreUpdateCount();
	FullIntegrity.InputEventCount = Encryption->GetInputEventCount();
	FullIntegrity.ScoreTrailHash = Encryption->GetScoreTrailHash();
	
	Submission.EncryptedScore = Encryption->EncryptScore(Score, FullIntegrity);
	
	// Check encryption success
	if (!Submission.EncryptedScore.IsValid())
	{
		UE_LOG(LogDeskillz, Error, TEXT("Failed to encrypt score"));
		
		FDeskillzSubmissionResult FailResult;
		FailResult.Status = EDeskillzSubmissionStatus::Failed;
		FailResult.ErrorMessage = TEXT("Encryption failed");
		FailResult.ErrorCode = -2;
		
		OnSubmissionComplete.Broadcast(FailResult, Score);
		return;
	}
	
	// Start submission
	bIsSubmitting = true;
	CurrentStatus = EDeskillzSubmissionStatus::Submitting;
	CurrentRetryAttempt = 0;
	CurrentSubmission = Submission;
	
	SendSubmission(Submission);
}

void UDeskillzSecureSubmitter::CancelSubmission()
{
	if (!bIsSubmitting)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Cancelling submission"));
	
	// Clear timers
	if (SDK && SDK->GetWorld())
	{
		SDK->GetWorld()->GetTimerManager().ClearTimer(RetryTimerHandle);
		SDK->GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}
	
	bIsSubmitting = false;
	CurrentStatus = EDeskillzSubmissionStatus::Pending;
}

void UDeskillzSecureSubmitter::RetryQueuedSubmissions()
{
	if (SubmissionQueue.Num() == 0)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Retrying %d queued submissions"), SubmissionQueue.Num());
	
	ProcessQueue();
}

void UDeskillzSecureSubmitter::ClearQueue()
{
	UE_LOG(LogDeskillz, Log, TEXT("Clearing %d queued submissions"), SubmissionQueue.Num());
	SubmissionQueue.Empty();
}

bool UDeskillzSecureSubmitter::ValidateScore(int64 Score, FString& OutError) const
{
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	
	// Check score range
	if (Config)
	{
		if (Score < Config->MinScore)
		{
			OutError = FString::Printf(TEXT("Score %lld is below minimum %lld"), Score, Config->MinScore);
			return false;
		}
		
		if (Score > Config->MaxScore)
		{
			OutError = FString::Printf(TEXT("Score %lld exceeds maximum %lld"), Score, Config->MaxScore);
			return false;
		}
	}
	
	// Check for negative scores (if not allowed)
	if (Score < 0)
	{
		OutError = TEXT("Negative scores are not allowed");
		return false;
	}
	
	return true;
}

bool UDeskillzSecureSubmitter::IsScoreInRange(int64 Score) const
{
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	
	if (!Config)
	{
		return true; // No config, allow all
	}
	
	return Score >= Config->MinScore && Score <= Config->MaxScore;
}

bool UDeskillzSecureSubmitter::IsScoreProgressionValid(int64 PreviousScore, int64 NewScore, float ElapsedTimeSeconds) const
{
	// Basic validation rules:
	// 1. Score shouldn't decrease (in most games)
	// 2. Score increase rate shouldn't be impossibly fast
	
	int64 ScoreDelta = NewScore - PreviousScore;
	
	// Allow small decreases (some games have penalties)
	if (ScoreDelta < -1000)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Suspicious score decrease: %lld -> %lld"), PreviousScore, NewScore);
		return false;
	}
	
	// Check for impossibly fast gains
	if (ElapsedTimeSeconds > 0)
	{
		float ScorePerSecond = (float)ScoreDelta / ElapsedTimeSeconds;
		
		// Threshold depends on game (this is just a baseline)
		if (ScorePerSecond > 10000)
		{
			UE_LOG(LogDeskillz, Warning, TEXT("Suspicious score rate: %.2f points/sec"), ScorePerSecond);
			return false;
		}
	}
	
	return true;
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzSecureSubmitter::ProcessQueue()
{
	if (bIsSubmitting || SubmissionQueue.Num() == 0)
	{
		return;
	}
	
	// Get next submission
	FDeskillzQueuedSubmission& NextSubmission = SubmissionQueue[0];
	
	// Check if too old (submissions expire after 1 hour)
	int64 Age = UDeskillzScoreEncryption::GetTimestampMs() - NextSubmission.QueuedAt;
	if (Age > 3600000) // 1 hour in ms
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Queued submission expired, removing"));
		SubmissionQueue.RemoveAt(0);
		ProcessQueue(); // Try next
		return;
	}
	
	// Submit
	bIsSubmitting = true;
	CurrentStatus = EDeskillzSubmissionStatus::Submitting;
	CurrentRetryAttempt = NextSubmission.Attempts;
	CurrentSubmission = NextSubmission;
	
	SendSubmission(NextSubmission);
}

void UDeskillzSecureSubmitter::SendSubmission(const FDeskillzQueuedSubmission& Submission)
{
	CurrentSubmission.Attempts++;
	CurrentSubmission.LastAttemptAt = UDeskillzScoreEncryption::GetTimestampMs();
	
	UE_LOG(LogDeskillz, Log, TEXT("Sending score submission (attempt %d/%d)"), 
		CurrentSubmission.Attempts, MaxRetryAttempts);
	
	// Broadcast progress
	OnSubmissionProgress.Broadcast(Submission.OriginalScore, CurrentSubmission.Attempts, MaxRetryAttempts);
	
	// Build API request
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	FDeskillzEndpoints Endpoints = FDeskillzEndpoints::ForEnvironment(
		Config ? Config->Environment : EDeskillzEnvironment::Sandbox);
	
	FString URL = Endpoints.BaseUrl + TEXT("/api/v1/matches/") + Submission.MatchId + TEXT("/score");
	
	// Create HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	// Add auth headers
	if (SDK && !SDK->GetAuthToken().IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + SDK->GetAuthToken());
	}
	
	if (Config && !Config->APIKey.IsEmpty())
	{
		Request->SetHeader(TEXT("X-API-Key"), Config->APIKey);
	}
	
	// Set body
	Request->SetContentAsString(Submission.EncryptedScore.ToJson());
	
	// Set up response handler
	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr HttpRequest, FHttpResponsePtr Response, bool bSuccess)
	{
		if (!bSuccess || !Response.IsValid())
		{
			HandleSubmissionResponse(-1, TEXT(""));
			return;
		}
		
		HandleSubmissionResponse(Response->GetResponseCode(), Response->GetContentAsString());
	});
	
	// Start timeout timer
	if (SDK && SDK->GetWorld())
	{
		SDK->GetWorld()->GetTimerManager().SetTimer(
			TimeoutTimerHandle,
			this,
			&UDeskillzSecureSubmitter::HandleSubmissionTimeout,
			SubmissionTimeout,
			false
		);
	}
	
	// Send request
	Request->ProcessRequest();
}

void UDeskillzSecureSubmitter::HandleSubmissionResponse(int32 HttpStatus, const FString& Response)
{
	// Clear timeout timer
	if (SDK && SDK->GetWorld())
	{
		SDK->GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}
	
	FDeskillzSubmissionResult Result;
	Result.RetryCount = CurrentSubmission.Attempts;
	
	// Parse response
	if (HttpStatus == 200 || HttpStatus == 201)
	{
		// Success!
		Result.Status = EDeskillzSubmissionStatus::Verified;
		Result.VerifiedScore = CurrentSubmission.OriginalScore;
		Result.bIntegrityVerified = true;
		
		// Parse server response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("submissionId"), Result.SubmissionId);
			JsonObject->TryGetNumberField(TEXT("verifiedScore"), Result.VerifiedScore);
			JsonObject->TryGetNumberField(TEXT("serverTimestamp"), Result.ServerTimestamp);
			
			bool bVerified = false;
			if (JsonObject->TryGetBoolField(TEXT("integrityVerified"), bVerified))
			{
				Result.bIntegrityVerified = bVerified;
			}
		}
		
		UE_LOG(LogDeskillz, Log, TEXT("Score submission verified: %lld (ID: %s)"), 
			Result.VerifiedScore, *Result.SubmissionId);
		
		CompleteSubmission(Result);
	}
	else if (HttpStatus == 422 || HttpStatus == 400)
	{
		// Rejected by server (don't retry)
		Result.Status = EDeskillzSubmissionStatus::Rejected;
		Result.ErrorCode = HttpStatus;
		
		// Parse error message
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("error"), Result.ErrorMessage);
			JsonObject->TryGetStringField(TEXT("message"), Result.ErrorMessage);
		}
		
		if (Result.ErrorMessage.IsEmpty())
		{
			Result.ErrorMessage = TEXT("Score rejected by server");
		}
		
		UE_LOG(LogDeskillz, Warning, TEXT("Score submission rejected: %s"), *Result.ErrorMessage);
		
		CompleteSubmission(Result);
	}
	else if (HttpStatus == 409)
	{
		// Score flagged for review
		Result.Status = EDeskillzSubmissionStatus::Flagged;
		Result.ErrorMessage = TEXT("Score flagged for review");
		Result.ErrorCode = HttpStatus;
		
		UE_LOG(LogDeskillz, Warning, TEXT("Score flagged for anti-cheat review"));
		
		CompleteSubmission(Result);
	}
	else
	{
		// Failed, might retry
		Result.Status = EDeskillzSubmissionStatus::Failed;
		Result.ErrorCode = HttpStatus;
		Result.ErrorMessage = FString::Printf(TEXT("HTTP %d"), HttpStatus);
		
		UE_LOG(LogDeskillz, Warning, TEXT("Score submission failed: HTTP %d (attempt %d/%d)"), 
			HttpStatus, CurrentSubmission.Attempts, MaxRetryAttempts);
		
		// Check if we should retry
		if (CurrentSubmission.Attempts < MaxRetryAttempts)
		{
			ScheduleRetry();
		}
		else
		{
			// Max retries exceeded
			Result.ErrorMessage = TEXT("Max retry attempts exceeded");
			CompleteSubmission(Result);
		}
	}
}

void UDeskillzSecureSubmitter::HandleSubmissionTimeout()
{
	UE_LOG(LogDeskillz, Warning, TEXT("Score submission timed out"));
	
	FDeskillzSubmissionResult Result;
	Result.Status = EDeskillzSubmissionStatus::TimedOut;
	Result.ErrorMessage = TEXT("Submission timed out");
	Result.ErrorCode = -3;
	Result.RetryCount = CurrentSubmission.Attempts;
	
	// Check if we should retry
	if (CurrentSubmission.Attempts < MaxRetryAttempts)
	{
		ScheduleRetry();
	}
	else
	{
		CompleteSubmission(Result);
	}
}

void UDeskillzSecureSubmitter::ScheduleRetry()
{
	float Delay = GetRetryDelay();
	
	UE_LOG(LogDeskillz, Log, TEXT("Scheduling retry in %.1f seconds"), Delay);
	
	if (SDK && SDK->GetWorld())
	{
		SDK->GetWorld()->GetTimerManager().SetTimer(
			RetryTimerHandle,
			[this]()
			{
				SendSubmission(CurrentSubmission);
			},
			Delay,
			false
		);
	}
}

float UDeskillzSecureSubmitter::GetRetryDelay() const
{
	// Exponential backoff with jitter
	float BaseDelay = RetryBaseDelay * FMath::Pow(2.0f, (float)CurrentSubmission.Attempts - 1);
	float JitteredDelay = BaseDelay * (0.5f + FMath::FRand());
	return FMath::Min(JitteredDelay, RetryMaxDelay);
}

void UDeskillzSecureSubmitter::CompleteSubmission(const FDeskillzSubmissionResult& Result)
{
	LastResult = Result;
	bIsSubmitting = false;
	CurrentStatus = Result.Status;
	
	// Remove from queue if it was queued
	if (SubmissionQueue.Num() > 0 && SubmissionQueue[0].OriginalScore == CurrentSubmission.OriginalScore)
	{
		SubmissionQueue.RemoveAt(0);
	}
	
	// Broadcast completion
	OnSubmissionComplete.Broadcast(Result, CurrentSubmission.OriginalScore);
	
	// Process next in queue
	if (SubmissionQueue.Num() > 0)
	{
		ProcessQueue();
	}
}

void UDeskillzSecureSubmitter::QueueSubmission(const FDeskillzQueuedSubmission& Submission)
{
	// Check queue limit
	if (SubmissionQueue.Num() >= MaxQueuedSubmissions)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Submission queue full, dropping oldest"));
		SubmissionQueue.RemoveAt(0);
	}
	
	SubmissionQueue.Add(Submission);
	
	UE_LOG(LogDeskillz, Log, TEXT("Score queued for later submission (queue size: %d)"), SubmissionQueue.Num());
}
