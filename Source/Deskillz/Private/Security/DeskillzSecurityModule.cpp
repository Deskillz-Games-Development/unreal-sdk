// Copyright Deskillz Games. All Rights Reserved.

#include "Security/DeskillzSecurityModule.h"
#include "Deskillz.h"

// Static singleton
static UDeskillzSecurityModule* GSecurityModule = nullptr;

UDeskillzSecurityModule::UDeskillzSecurityModule()
{
	// Create sub-components
	Encryption = NewObject<UDeskillzScoreEncryption>(this);
	Submitter = NewObject<UDeskillzSecureSubmitter>(this);
	AntiCheat = NewObject<UDeskillzAntiCheat>(this);
}

UDeskillzSecurityModule* UDeskillzSecurityModule::Get(const UObject* WorldContextObject)
{
	if (!GSecurityModule)
	{
		GSecurityModule = NewObject<UDeskillzSecurityModule>();
		GSecurityModule->AddToRoot(); // Prevent garbage collection
		
		// Bind events
		if (GSecurityModule->Submitter)
		{
			GSecurityModule->Submitter->OnSubmissionComplete.AddDynamic(
				GSecurityModule, &UDeskillzSecurityModule::HandleSubmissionComplete);
		}
		
		if (GSecurityModule->AntiCheat)
		{
			GSecurityModule->AntiCheat->OnViolationDetected.AddDynamic(
				GSecurityModule, &UDeskillzSecurityModule::HandleViolationDetected);
		}
	}
	return GSecurityModule;
}

void UDeskillzSecurityModule::InitializeForMatch(const FString& MatchId, const FString& EncryptionKey, const FString& SigningKey)
{
	CurrentMatchId = MatchId;
	LastScoreUpdateTime = FPlatformTime::Seconds();
	
	// Initialize encryption
	if (Encryption)
	{
		Encryption->Initialize(EncryptionKey, SigningKey, MatchId);
	}
	
	// Initialize anti-cheat
	if (AntiCheat)
	{
		AntiCheat->Initialize(MatchId);
	}
	
	bIsInitialized = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("Security module initialized for match: %s"), *MatchId);
	
	// Run initial security check
	if (!RunSecurityChecks())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Initial security checks found issues"));
	}
}

void UDeskillzSecurityModule::EndMatch()
{
	// Reset all components
	if (Encryption)
	{
		Encryption->Reset();
	}
	
	if (AntiCheat)
	{
		AntiCheat->Reset();
	}
	
	if (Submitter)
	{
		Submitter->ClearQueue();
	}
	
	CurrentMatchId.Empty();
	bIsInitialized = false;
	
	UE_LOG(LogDeskillz, Log, TEXT("Security module reset"));
}

FDeskillzSecurityStatus UDeskillzSecurityModule::GetSecurityStatus() const
{
	FDeskillzSecurityStatus Status;
	
	Status.bEncryptionReady = Encryption && Encryption->IsInitialized();
	Status.bAntiCheatActive = bIsInitialized;
	Status.TrustLevel = AntiCheat ? AntiCheat->GetDeviceTrustLevel() : 100;
	Status.ViolationCount = AntiCheat ? AntiCheat->GetViolations().Num() : 0;
	Status.bHasCriticalViolations = AntiCheat ? AntiCheat->HasCriticalViolations() : false;
	Status.bScoreIntegrityValid = AntiCheat ? AntiCheat->VerifyScoreIntegrity() : true;
	Status.QueuedSubmissions = Submitter ? Submitter->GetQueuedCount() : 0;
	Status.bIsSubmitting = Submitter ? Submitter->IsSubmitting() : false;
	
	return Status;
}

bool UDeskillzSecurityModule::UpdateScore(int64 Score)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Security not initialized, cannot update score"));
		return false;
	}
	
	double CurrentTime = FPlatformTime::Seconds();
	float DeltaTime = (float)(CurrentTime - LastScoreUpdateTime);
	LastScoreUpdateTime = CurrentTime;
	
	// Validate with anti-cheat
	bool bValid = true;
	if (AntiCheat)
	{
		bValid = AntiCheat->ValidateScoreUpdate(Score, DeltaTime);
	}
	
	// Record in encryption trail
	if (Encryption)
	{
		Encryption->RecordScoreUpdate(Score);
	}
	
	return bValid;
}

int64 UDeskillzSecurityModule::GetCurrentScore() const
{
	if (AntiCheat)
	{
		return AntiCheat->GetProtectedScore();
	}
	return 0;
}

void UDeskillzSecurityModule::SubmitFinalScore(int64 FinalScore)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Security not initialized, cannot submit score"));
		
		FDeskillzSubmissionResult FailResult;
		FailResult.Status = EDeskillzSubmissionStatus::Failed;
		FailResult.ErrorMessage = TEXT("Security module not initialized");
		
		OnScoreSubmissionComplete.Broadcast(FailResult, FinalScore);
		return;
	}
	
	// Run final security checks
	if (!RunSecurityChecks())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Security checks failed before score submission"));
	}
	
	// Check if we should block submission
	if (!ShouldAllowGameplay())
	{
		UE_LOG(LogDeskillz, Error, TEXT("Critical security violations detected, blocking submission"));
		
		FDeskillzSubmissionResult FailResult;
		FailResult.Status = EDeskillzSubmissionStatus::Rejected;
		FailResult.ErrorMessage = TEXT("Security violations detected");
		
		OnScoreSubmissionComplete.Broadcast(FailResult, FinalScore);
		return;
	}
	
	// Update final score in anti-cheat
	if (AntiCheat)
	{
		AntiCheat->SetProtectedScore(FinalScore);
	}
	
	// Submit via secure submitter
	if (Submitter)
	{
		Submitter->SubmitScore(FinalScore);
	}
}

void UDeskillzSecurityModule::RecordInput(const FString& InputType, FVector2D Position)
{
	// Record in encryption
	if (Encryption)
	{
		Encryption->RecordInputEvent(InputType);
	}
	
	// Record in anti-cheat
	if (AntiCheat)
	{
		AntiCheat->RecordInput(InputType, Position);
	}
}

bool UDeskillzSecurityModule::RunSecurityChecks()
{
	bool bAllPassed = true;
	
	// Anti-cheat checks
	if (AntiCheat)
	{
		if (!AntiCheat->RunIntegrityCheck())
		{
			bAllPassed = false;
		}
	}
	
	// Verify score integrity
	if (!VerifyScoreIntegrity())
	{
		bAllPassed = false;
	}
	
	return bAllPassed;
}

bool UDeskillzSecurityModule::VerifyScoreIntegrity() const
{
	if (AntiCheat)
	{
		return AntiCheat->VerifyScoreIntegrity();
	}
	return true;
}

int32 UDeskillzSecurityModule::GetDeviceTrustLevel() const
{
	if (AntiCheat)
	{
		return AntiCheat->GetDeviceTrustLevel();
	}
	return 100;
}

bool UDeskillzSecurityModule::ShouldAllowGameplay() const
{
	// Block if critical violations
	if (AntiCheat && AntiCheat->HasCriticalViolations())
	{
		return false;
	}
	
	// Block if trust level too low
	if (GetDeviceTrustLevel() < 20)
	{
		return false;
	}
	
	return true;
}

void UDeskillzSecurityModule::HandleSubmissionComplete(const FDeskillzSubmissionResult& Result, int64 OriginalScore)
{
	UE_LOG(LogDeskillz, Log, TEXT("Score submission complete: %s (Score: %lld)"), 
		Result.IsSuccess() ? TEXT("SUCCESS") : TEXT("FAILED"), OriginalScore);
	
	// Forward event
	OnScoreSubmissionComplete.Broadcast(Result, OriginalScore);
}

void UDeskillzSecurityModule::HandleViolationDetected(const FDeskillzViolation& Violation)
{
	UE_LOG(LogDeskillz, Warning, TEXT("Security violation: %s"), *Violation.Description);
	
	// Forward event
	OnSecurityViolation.Broadcast(Violation);
}
