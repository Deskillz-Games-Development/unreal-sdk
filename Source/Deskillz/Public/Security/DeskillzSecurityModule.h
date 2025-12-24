// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Security/DeskillzScoreEncryption.h"
#include "Security/DeskillzSecureSubmitter.h"
#include "Security/DeskillzAntiCheat.h"
#include "DeskillzSecurityModule.generated.h"

/**
 * Security status summary
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzSecurityStatus
{
	GENERATED_BODY()
	
	/** Is encryption initialized */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	bool bEncryptionReady = false;
	
	/** Is anti-cheat active */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	bool bAntiCheatActive = false;
	
	/** Device trust level (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 TrustLevel = 100;
	
	/** Number of violations detected */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 ViolationCount = 0;
	
	/** Has any critical violations */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	bool bHasCriticalViolations = false;
	
	/** Score integrity verified */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	bool bScoreIntegrityValid = true;
	
	/** Submission queue count */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 QueuedSubmissions = 0;
	
	/** Currently submitting */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	bool bIsSubmitting = false;
};

/**
 * Deskillz Security Module
 * 
 * Unified interface to all security features:
 * - Score encryption (AES-256-GCM + HMAC)
 * - Secure score submission with retries
 * - Anti-cheat detection and prevention
 * - Device integrity validation
 * 
 * This is the recommended way to interact with security features.
 * 
 * Usage:
 *   UDeskillzSecurityModule* Security = UDeskillzSecurityModule::Get(this);
 *   Security->InitializeForMatch(MatchId, EncryptionKey, SigningKey);
 *   ...
 *   Security->UpdateScore(NewScore);
 *   ...
 *   Security->SubmitFinalScore(FinalScore);
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSecurityModule : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzSecurityModule();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the security module instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Deskillz Security"))
	static UDeskillzSecurityModule* Get(const UObject* WorldContextObject);
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize security for a match
	 * @param MatchId The match identifier
	 * @param EncryptionKey Base64 encoded AES-256 key from server
	 * @param SigningKey Base64 encoded HMAC key from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void InitializeForMatch(const FString& MatchId, const FString& EncryptionKey, const FString& SigningKey);
	
	/**
	 * End match and reset security state
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void EndMatch();
	
	/**
	 * Get current security status
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	FDeskillzSecurityStatus GetSecurityStatus() const;
	
	/**
	 * Is security module initialized for a match
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool IsInitialized() const { return bIsInitialized; }
	
	// ========================================================================
	// Score Management
	// ========================================================================
	
	/**
	 * Update the current score
	 * Records the update for anti-cheat and prepares for encryption
	 * @param Score The current score
	 * @return True if score update is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	bool UpdateScore(int64 Score);
	
	/**
	 * Get the current protected score
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	int64 GetCurrentScore() const;
	
	/**
	 * Submit the final score
	 * Encrypts and securely submits to server
	 * @param FinalScore The final score to submit
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void SubmitFinalScore(int64 FinalScore);
	
	/**
	 * Record an input event
	 * Used for anti-cheat input validation
	 * @param InputType Type of input (tap, swipe, button, etc.)
	 * @param Position Screen position (optional)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void RecordInput(const FString& InputType, FVector2D Position = FVector2D::ZeroVector);
	
	// ========================================================================
	// Integrity Checks
	// ========================================================================
	
	/**
	 * Run all security checks
	 * @return True if all checks pass
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	bool RunSecurityChecks();
	
	/**
	 * Verify score hasn't been tampered
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool VerifyScoreIntegrity() const;
	
	/**
	 * Get device trust level (0-100)
	 * Higher is more trustworthy
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	int32 GetDeviceTrustLevel() const;
	
	/**
	 * Check if we should allow gameplay
	 * Returns false if critical security issues detected
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool ShouldAllowGameplay() const;
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when score submission completes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Security")
	FOnScoreSubmissionComplete OnScoreSubmissionComplete;
	
	/** Called when a security violation is detected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Security")
	FOnViolationDetected OnSecurityViolation;
	
	// ========================================================================
	// Component Access
	// ========================================================================
	
	/**
	 * Get encryption component (for advanced usage)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	UDeskillzScoreEncryption* GetEncryption() const { return Encryption; }
	
	/**
	 * Get submitter component (for advanced usage)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	UDeskillzSecureSubmitter* GetSubmitter() const { return Submitter; }
	
	/**
	 * Get anti-cheat component (for advanced usage)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	UDeskillzAntiCheat* GetAntiCheat() const { return AntiCheat; }
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Is initialized for a match */
	bool bIsInitialized = false;
	
	/** Current match ID */
	FString CurrentMatchId;
	
	/** Last score update time */
	double LastScoreUpdateTime = 0;
	
	/** Encryption component */
	UPROPERTY()
	UDeskillzScoreEncryption* Encryption;
	
	/** Submitter component */
	UPROPERTY()
	UDeskillzSecureSubmitter* Submitter;
	
	/** Anti-cheat component */
	UPROPERTY()
	UDeskillzAntiCheat* AntiCheat;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Handle submission complete */
	UFUNCTION()
	void HandleSubmissionComplete(const FDeskillzSubmissionResult& Result, int64 OriginalScore);
	
	/** Handle violation detected */
	UFUNCTION()
	void HandleViolationDetected(const FDeskillzViolation& Violation);
};
