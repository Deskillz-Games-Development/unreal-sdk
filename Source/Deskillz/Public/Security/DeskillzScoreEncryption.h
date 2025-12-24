// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzScoreEncryption.generated.h"

/**
 * Encrypted score payload structure
 * Contains all data needed to verify and decrypt a score
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzEncryptedScore
{
	GENERATED_BODY()
	
	/** Base64 encoded encrypted score data */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString EncryptedData;
	
	/** Base64 encoded initialization vector (IV) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString IV;
	
	/** Base64 encoded authentication tag (GCM) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString AuthTag;
	
	/** HMAC-SHA256 signature of the payload */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString Signature;
	
	/** Timestamp when score was encrypted (Unix milliseconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int64 Timestamp = 0;
	
	/** Unique nonce to prevent replay attacks */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString Nonce;
	
	/** Match ID this score belongs to */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString MatchId;
	
	/** Device fingerprint for verification */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString DeviceFingerprint;
	
	/** Algorithm version for forward compatibility */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 Version = 1;
	
	/** Check if this is a valid encrypted score */
	bool IsValid() const
	{
		return !EncryptedData.IsEmpty() && 
			   !IV.IsEmpty() && 
			   !AuthTag.IsEmpty() && 
			   !Signature.IsEmpty() &&
			   Timestamp > 0;
	}
	
	/** Convert to JSON for transmission */
	FString ToJson() const;
	
	/** Parse from JSON response */
	static FDeskillzEncryptedScore FromJson(const FString& JsonString);
};

/**
 * Score integrity data for anti-cheat verification
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzScoreIntegrity
{
	GENERATED_BODY()
	
	/** The actual score value */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int64 Score = 0;
	
	/** Match random seed (must match server) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int64 RandomSeed = 0;
	
	/** Game session duration in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int64 SessionDurationMs = 0;
	
	/** Number of score updates during match */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 ScoreUpdateCount = 0;
	
	/** Hash of game state at submission */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString GameStateHash;
	
	/** Input event count for validation */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int32 InputEventCount = 0;
	
	/** Frame count during gameplay */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	int64 FrameCount = 0;
	
	/** Average FPS during gameplay */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	float AverageFPS = 0.0f;
	
	/** Checksum of all score updates */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Security")
	FString ScoreTrailHash;
};

/**
 * Deskillz Score Encryption System
 * 
 * Provides military-grade encryption for score submissions:
 * - AES-256-GCM for data encryption
 * - HMAC-SHA256 for signature verification
 * - Replay attack prevention via nonces
 * - Device fingerprinting
 * - Score trail hashing
 * 
 * All encryption keys are derived server-side and rotated regularly.
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzScoreEncryption : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzScoreEncryption();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize encryption with server-provided keys
	 * Called automatically when match starts
	 * @param EncryptionKey Base64 encoded AES-256 key
	 * @param SigningKey Base64 encoded HMAC key
	 * @param MatchId Current match identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void Initialize(const FString& EncryptionKey, const FString& SigningKey, const FString& MatchId);
	
	/**
	 * Clear all keys and reset state
	 * Called when match ends
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void Reset();
	
	/**
	 * Check if encryption is initialized and ready
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool IsInitialized() const { return bIsInitialized; }
	
	// ========================================================================
	// Score Encryption
	// ========================================================================
	
	/**
	 * Encrypt a score for secure transmission
	 * @param Score The score value to encrypt
	 * @param Integrity Additional integrity data for verification
	 * @return Encrypted score payload ready for submission
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	FDeskillzEncryptedScore EncryptScore(int64 Score, const FDeskillzScoreIntegrity& Integrity);
	
	/**
	 * Encrypt score with automatic integrity collection
	 * Simpler version that gathers integrity data automatically
	 * @param Score The score value to encrypt
	 * @return Encrypted score payload
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	FDeskillzEncryptedScore EncryptScoreSimple(int64 Score);
	
	/**
	 * Verify an encrypted score's signature locally
	 * @param EncryptedScore The score to verify
	 * @return True if signature is valid
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	bool VerifySignature(const FDeskillzEncryptedScore& EncryptedScore) const;
	
	// ========================================================================
	// Score Trail (Anti-Cheat)
	// ========================================================================
	
	/**
	 * Record a score update in the trail
	 * Call this every time the score changes
	 * @param NewScore The updated score value
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void RecordScoreUpdate(int64 NewScore);
	
	/**
	 * Record an input event for validation
	 * @param InputType Type of input (e.g., "tap", "swipe", "button")
	 * @param InputData Optional additional data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Security")
	void RecordInputEvent(const FString& InputType, const FString& InputData = TEXT(""));
	
	/**
	 * Get the current score trail hash
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	FString GetScoreTrailHash() const;
	
	/**
	 * Get number of score updates recorded
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	int32 GetScoreUpdateCount() const { return ScoreTrail.Num(); }
	
	/**
	 * Get number of input events recorded
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	int32 GetInputEventCount() const { return InputEventCount; }
	
	// ========================================================================
	// Device Fingerprinting
	// ========================================================================
	
	/**
	 * Generate device fingerprint for anti-cheat
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	FString GetDeviceFingerprint() const;
	
	/**
	 * Get unique device identifier
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	static FString GetDeviceId();
	
	// ========================================================================
	// Utility
	// ========================================================================
	
	/**
	 * Generate a cryptographically secure random nonce
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	static FString GenerateNonce();
	
	/**
	 * Get current Unix timestamp in milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	static int64 GetTimestampMs();
	
	/**
	 * Hash arbitrary data using SHA-256
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	static FString HashSHA256(const FString& Data);
	
	/**
	 * Encode bytes to Base64
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	static FString ToBase64(const TArray<uint8>& Bytes);
	
	/**
	 * Decode Base64 to bytes
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Security")
	static TArray<uint8> FromBase64(const FString& Base64String);
	
private:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Is encryption initialized with valid keys */
	bool bIsInitialized = false;
	
	/** AES-256 encryption key (32 bytes) */
	TArray<uint8> EncryptionKey;
	
	/** HMAC signing key */
	TArray<uint8> SigningKey;
	
	/** Current match ID */
	FString CurrentMatchId;
	
	/** Score update trail for integrity verification */
	TArray<TPair<int64, int64>> ScoreTrail; // Timestamp, Score
	
	/** Running hash of score trail */
	FString RunningScoreHash;
	
	/** Input event count */
	int32 InputEventCount = 0;
	
	/** Session start time */
	int64 SessionStartTime = 0;
	
	/** Frame counter */
	int64 FrameCount = 0;
	
	/** FPS samples for average */
	TArray<float> FPSSamples;
	
	/** Used nonces to prevent replay */
	TSet<FString> UsedNonces;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** AES-256-GCM encryption */
	bool AES256GCMEncrypt(
		const TArray<uint8>& Plaintext,
		const TArray<uint8>& Key,
		const TArray<uint8>& IV,
		const TArray<uint8>& AAD,
		TArray<uint8>& Ciphertext,
		TArray<uint8>& AuthTag
	);
	
	/** HMAC-SHA256 signature */
	TArray<uint8> HMACSHA256(const TArray<uint8>& Data, const TArray<uint8>& Key);
	
	/** Generate random IV for AES-GCM (12 bytes) */
	TArray<uint8> GenerateIV();
	
	/** Build payload for signing */
	FString BuildSigningPayload(const FString& EncryptedData, const FString& IV, int64 Timestamp, const FString& Nonce);
	
	/** Update running score hash */
	void UpdateScoreHash(int64 Score, int64 Timestamp);
	
	/** Collect current integrity data */
	FDeskillzScoreIntegrity CollectIntegrityData(int64 FinalScore);
};
