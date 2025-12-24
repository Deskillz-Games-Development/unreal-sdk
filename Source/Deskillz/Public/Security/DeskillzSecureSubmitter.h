// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Security/DeskillzScoreEncryption.h"
#include "DeskillzSecureSubmitter.generated.h"

class UDeskillzSDK;

/**
 * Score submission status
 */
UENUM(BlueprintType)
enum class EDeskillzSubmissionStatus : uint8
{
	/** Not yet submitted */
	Pending,
	
	/** Currently being submitted */
	Submitting,
	
	/** Successfully submitted and verified */
	Verified,
	
	/** Submission failed (will retry) */
	Failed,
	
	/** Submission rejected by server */
	Rejected,
	
	/** Submission timed out */
	TimedOut,
	
	/** Score flagged for review */
	Flagged
};

/**
 * Result of a score submission attempt
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSubmissionResult
{
	GENERATED_BODY()
	
	/** Submission status */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	EDeskillzSubmissionStatus Status = EDeskillzSubmissionStatus::Pending;
	
	/** Server-assigned submission ID */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString SubmissionId;
	
	/** Server-verified score (may differ if adjusted) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int64 VerifiedScore = 0;
	
	/** Error message if failed */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString ErrorMessage;
	
	/** Error code */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 ErrorCode = 0;
	
	/** Number of retry attempts made */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 RetryCount = 0;
	
	/** Server timestamp of submission */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int64 ServerTimestamp = 0;
	
	/** Integrity check passed */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	bool bIntegrityVerified = false;
	
	/** Is this a successful submission */
	bool IsSuccess() const 
	{ 
		return Status == EDeskillzSubmissionStatus::Verified; 
	}
	
	/** Is this submission still in progress */
	bool IsInProgress() const
	{
		return Status == EDeskillzSubmissionStatus::Pending || 
			   Status == EDeskillzSubmissionStatus::Submitting;
	}
};

/**
 * Queued score submission entry
 */
USTRUCT()
struct FDeskillzQueuedSubmission
{
	GENERATED_BODY()
	
	/** Encrypted score payload */
	FDeskillzEncryptedScore EncryptedScore;
	
	/** Original score value */
	int64 OriginalScore = 0;
	
	/** Match ID */
	FString MatchId;
	
	/** Queue timestamp */
	int64 QueuedAt = 0;
	
	/** Number of attempts */
	int32 Attempts = 0;
	
	/** Last attempt timestamp */
	int64 LastAttemptAt = 0;
	
	/** Current status */
	EDeskillzSubmissionStatus Status = EDeskillzSubmissionStatus::Pending;
};

/** Delegate for submission complete */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreSubmissionComplete, const FDeskillzSubmissionResult&, Result, int64, OriginalScore);

/** Delegate for submission progress */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreSubmissionProgress, int64, Score, int32, AttemptNumber, int32, MaxAttempts);

/**
 * Deskillz Secure Score Submitter
 * 
 * Handles the secure submission of encrypted scores:
 * - Encryption via UDeskillzScoreEncryption
 * - Automatic retries with exponential backoff
 * - Offline queueing for poor connectivity
 * - Server-side verification
 * - Integrity validation
 * 
 * Usage:
 *   UDeskillzSecureSubmitter* Submitter = UDeskillzSecureSubmitter::Get(this);
 *   Submitter->SubmitScore(1500);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSecureSubmitter : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzSecureSubmitter();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the secure submitter instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security", meta = (WorldContext = "WorldContextObject"))
	static UDeskillzSecureSubmitter* Get(const UObject* WorldContextObject);
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Maximum retry attempts before giving up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Security")
	int32 MaxRetryAttempts = 5;
	
	/** Base delay between retries in seconds (exponential backoff) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Security")
	float RetryBaseDelay = 1.0f;
	
	/** Maximum delay between retries in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Security")
	float RetryMaxDelay = 30.0f;
	
	/** Submission timeout in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Security")
	float SubmissionTimeout = 15.0f;
	
	/** Enable offline queueing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Security")
	bool bEnableOfflineQueue = true;
	
	/** Maximum queued submissions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Security")
	int32 MaxQueuedSubmissions = 10;
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when score submission completes (success or final failure) */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Security")
	FOnScoreSubmissionComplete OnSubmissionComplete;
	
	/** Called on each submission attempt */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Security")
	FOnScoreSubmissionProgress OnSubmissionProgress;
	
	// ========================================================================
	// Score Submission
	// ========================================================================
	
	/**
	 * Submit a score securely
	 * Encrypts the score and handles transmission with retries
	 * @param Score The final score to submit
	 * @param bForceSubmit Submit even if a submission is in progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void SubmitScore(int64 Score, bool bForceSubmit = false);
	
	/**
	 * Submit with custom integrity data
	 * @param Score The score to submit
	 * @param Integrity Custom integrity data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void SubmitScoreWithIntegrity(int64 Score, const FDeskillzScoreIntegrity& Integrity);
	
	/**
	 * Cancel pending submission
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void CancelSubmission();
	
	/**
	 * Retry failed submissions in queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void RetryQueuedSubmissions();
	
	/**
	 * Clear the submission queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void ClearQueue();
	
	// ========================================================================
	// Status
	// ========================================================================
	
	/**
	 * Check if a submission is currently in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool IsSubmitting() const { return bIsSubmitting; }
	
	/**
	 * Get the current submission status
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	EDeskillzSubmissionStatus GetSubmissionStatus() const { return CurrentStatus; }
	
	/**
	 * Get the last submission result
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	FDeskillzSubmissionResult GetLastResult() const { return LastResult; }
	
	/**
	 * Get number of queued submissions
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	int32 GetQueuedCount() const { return SubmissionQueue.Num(); }
	
	/**
	 * Get current retry attempt number
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	int32 GetCurrentRetryAttempt() const { return CurrentRetryAttempt; }
	
	// ========================================================================
	// Validation
	// ========================================================================
	
	/**
	 * Validate score before submission
	 * @param Score Score to validate
	 * @param OutError Error message if invalid
	 * @return True if score is valid
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool ValidateScore(int64 Score, FString& OutError) const;
	
	/**
	 * Check if score is within expected range
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool IsScoreInRange(int64 Score) const;
	
	/**
	 * Check if score progression is suspicious
	 * @param PreviousScore Previous score
	 * @param NewScore New score
	 * @param ElapsedTimeSeconds Time between scores
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool IsScoreProgressionValid(int64 PreviousScore, int64 NewScore, float ElapsedTimeSeconds) const;
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Is currently submitting */
	bool bIsSubmitting = false;
	
	/** Current submission status */
	EDeskillzSubmissionStatus CurrentStatus = EDeskillzSubmissionStatus::Pending;
	
	/** Last submission result */
	FDeskillzSubmissionResult LastResult;
	
	/** Current retry attempt */
	int32 CurrentRetryAttempt = 0;
	
	/** Current submission being processed */
	FDeskillzQueuedSubmission CurrentSubmission;
	
	/** Queue of pending submissions */
	TArray<FDeskillzQueuedSubmission> SubmissionQueue;
	
	/** Encryption instance */
	UPROPERTY()
	UDeskillzScoreEncryption* Encryption;
	
	/** SDK reference */
	UPROPERTY()
	UDeskillzSDK* SDK;
	
	/** Retry timer handle */
	FTimerHandle RetryTimerHandle;
	
	/** Timeout timer handle */
	FTimerHandle TimeoutTimerHandle;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Initialize with SDK */
	void Initialize(UDeskillzSDK* InSDK);
	
	/** Process the submission queue */
	void ProcessQueue();
	
	/** Send submission to server */
	void SendSubmission(const FDeskillzQueuedSubmission& Submission);
	
	/** Handle server response */
	void HandleSubmissionResponse(int32 HttpStatus, const FString& Response);
	
	/** Handle submission timeout */
	void HandleSubmissionTimeout();
	
	/** Schedule retry with exponential backoff */
	void ScheduleRetry();
	
	/** Calculate retry delay */
	float GetRetryDelay() const;
	
	/** Complete the current submission */
	void CompleteSubmission(const FDeskillzSubmissionResult& Result);
	
	/** Add to offline queue */
	void QueueSubmission(const FDeskillzQueuedSubmission& Submission);
	
	friend class UDeskillzSDK;
};
