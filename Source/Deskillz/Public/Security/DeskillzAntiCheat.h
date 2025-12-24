// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzAntiCheat.generated.h"

/**
 * Types of cheating violations detected
 */
UENUM(BlueprintType)
enum class EDeskillzViolationType : uint8
{
	/** No violation */
	None,
	
	/** Memory was modified externally */
	MemoryTamper,
	
	/** Score changed impossibly fast */
	SpeedHack,
	
	/** Time manipulation detected */
	TimeTamper,
	
	/** Code injection detected */
	CodeInjection,
	
	/** Debugger attached */
	DebuggerDetected,
	
	/** Emulator/VM detected */
	EmulatorDetected,
	
	/** Root/jailbreak detected */
	RootDetected,
	
	/** Score exceeds possible maximum */
	ScoreAnomaly,
	
	/** Duplicate submission attempt */
	ReplayAttempt,
	
	/** Invalid device fingerprint */
	DeviceAnomaly,
	
	/** Network manipulation */
	NetworkTamper,
	
	/** Input automation detected */
	AutomationDetected
};

/**
 * Severity level of a violation
 */
UENUM(BlueprintType)
enum class EDeskillzViolationSeverity : uint8
{
	/** Low severity - warning only */
	Low,
	
	/** Medium severity - flag for review */
	Medium,
	
	/** High severity - reject score */
	High,
	
	/** Critical - ban player */
	Critical
};

/**
 * Details about a detected violation
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzViolation
{
	GENERATED_BODY()
	
	/** Type of violation */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|AntiCheat")
	EDeskillzViolationType Type = EDeskillzViolationType::None;
	
	/** Severity level */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|AntiCheat")
	EDeskillzViolationSeverity Severity = EDeskillzViolationSeverity::Low;
	
	/** Description of the violation */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|AntiCheat")
	FString Description;
	
	/** Timestamp when detected */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|AntiCheat")
	int64 Timestamp = 0;
	
	/** Additional data for server analysis */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|AntiCheat")
	FString Metadata;
	
	/** Stack trace (if available) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|AntiCheat")
	FString StackTrace;
	
	/** Is this a valid violation */
	bool IsValid() const { return Type != EDeskillzViolationType::None; }
};

/**
 * Protected value that detects tampering
 */
template<typename T>
struct TDeskillzProtectedValue
{
	T Get() const
	{
		// XOR decrypt
		return Value ^ Key;
	}
	
	void Set(T NewValue)
	{
		// Generate new key
		Key = FMath::Rand();
		Value = NewValue ^ Key;
		
		// Update checksum
		UpdateChecksum();
	}
	
	bool Verify() const
	{
		uint32 CurrentChecksum = CalculateChecksum();
		return CurrentChecksum == StoredChecksum;
	}
	
private:
	T Value = 0;
	T Key = 0;
	uint32 StoredChecksum = 0;
	
	void UpdateChecksum()
	{
		StoredChecksum = CalculateChecksum();
	}
	
	uint32 CalculateChecksum() const
	{
		// Simple checksum based on value and key
		uint32 Hash = 0x811c9dc5; // FNV offset basis
		Hash ^= (uint32)Value;
		Hash *= 0x01000193; // FNV prime
		Hash ^= (uint32)Key;
		Hash *= 0x01000193;
		return Hash;
	}
};

/** Delegate for violation detection */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViolationDetected, const FDeskillzViolation&, Violation);

/**
 * Deskillz Anti-Cheat Module
 * 
 * Provides comprehensive cheat detection and prevention:
 * - Memory protection (obfuscated score storage)
 * - Time manipulation detection
 * - Debugger detection
 * - Score anomaly detection
 * - Input validation
 * - Device integrity checks
 * 
 * All violations are logged and can be sent to server for analysis.
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzAntiCheat : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzAntiCheat();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the anti-cheat instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat", meta = (WorldContext = "WorldContextObject"))
	static UDeskillzAntiCheat* Get(const UObject* WorldContextObject);
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Enable memory protection (performance impact) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|AntiCheat")
	bool bEnableMemoryProtection = true;
	
	/** Enable time validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|AntiCheat")
	bool bEnableTimeValidation = true;
	
	/** Enable debugger detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|AntiCheat")
	bool bEnableDebuggerDetection = true;
	
	/** Enable score anomaly detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|AntiCheat")
	bool bEnableScoreAnomalyDetection = true;
	
	/** Enable device integrity checks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|AntiCheat")
	bool bEnableDeviceIntegrity = true;
	
	/** Maximum score per second (for anomaly detection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|AntiCheat")
	float MaxScorePerSecond = 1000.0f;
	
	/** Time drift tolerance in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|AntiCheat")
	float TimeDriftTolerance = 2.0f;
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when a violation is detected */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|AntiCheat")
	FOnViolationDetected OnViolationDetected;
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize anti-cheat for a match
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	void Initialize(const FString& MatchId);
	
	/**
	 * Reset anti-cheat state
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	void Reset();
	
	/**
	 * Run all integrity checks
	 * @return True if all checks pass
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	bool RunIntegrityCheck();
	
	// ========================================================================
	// Score Protection
	// ========================================================================
	
	/**
	 * Set protected score value
	 * Stores score with memory obfuscation
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	void SetProtectedScore(int64 Score);
	
	/**
	 * Get protected score value
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	int64 GetProtectedScore() const;
	
	/**
	 * Verify score integrity
	 * @return True if score hasn't been tampered
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	bool VerifyScoreIntegrity() const;
	
	/**
	 * Validate score update
	 * @param NewScore The new score to validate
	 * @param DeltaTime Time since last update
	 * @return True if score change is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	bool ValidateScoreUpdate(int64 NewScore, float DeltaTime);
	
	// ========================================================================
	// Time Validation
	// ========================================================================
	
	/**
	 * Sync with server time
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	void SyncServerTime(int64 ServerTimestamp);
	
	/**
	 * Get validated timestamp
	 * @return Timestamp corrected for any detected drift
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	int64 GetValidatedTimestamp() const;
	
	/**
	 * Check for time manipulation
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	bool IsTimeValid() const;
	
	// ========================================================================
	// Environment Checks
	// ========================================================================
	
	/**
	 * Check if debugger is attached
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	bool IsDebuggerPresent() const;
	
	/**
	 * Check if running in emulator/VM
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	bool IsEmulatorDetected() const;
	
	/**
	 * Check if device is rooted/jailbroken
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	bool IsDeviceRooted() const;
	
	/**
	 * Get device trust level (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	int32 GetDeviceTrustLevel() const;
	
	// ========================================================================
	// Violation Reporting
	// ========================================================================
	
	/**
	 * Report a detected violation
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	void ReportViolation(EDeskillzViolationType Type, EDeskillzViolationSeverity Severity, const FString& Description);
	
	/**
	 * Get all detected violations
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	TArray<FDeskillzViolation> GetViolations() const { return DetectedViolations; }
	
	/**
	 * Get violation count by severity
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	int32 GetViolationCount(EDeskillzViolationSeverity MinSeverity) const;
	
	/**
	 * Clear recorded violations
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	void ClearViolations();
	
	/**
	 * Check if any critical violations exist
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	bool HasCriticalViolations() const;
	
	/**
	 * Get anti-cheat report for server
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	FString GetAntiCheatReport() const;
	
	// ========================================================================
	// Input Validation
	// ========================================================================
	
	/**
	 * Record an input event for validation
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|AntiCheat")
	void RecordInput(const FString& InputType, FVector2D Position = FVector2D::ZeroVector);
	
	/**
	 * Validate input patterns (detect automation)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|AntiCheat")
	bool ValidateInputPatterns() const;
	
protected:
	// ========================================================================
	// Internal State
	// ========================================================================
	
	/** Current match ID */
	FString CurrentMatchId;
	
	/** Protected score storage */
	TDeskillzProtectedValue<int64> ProtectedScore;
	
	/** Last validated score */
	int64 LastScore = 0;
	
	/** Last score update time */
	double LastScoreUpdateTime = 0;
	
	/** Server time offset */
	int64 ServerTimeOffset = 0;
	
	/** Last time check */
	double LastTimeCheck = 0;
	
	/** Recorded violations */
	TArray<FDeskillzViolation> DetectedViolations;
	
	/** Input history for pattern detection */
	TArray<TPair<int64, FVector2D>> InputHistory;
	
	/** Frame time samples for speed hack detection */
	TArray<float> FrameTimeSamples;
	
	/** Expected frame time */
	float ExpectedFrameTime = 1.0f / 60.0f;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Check for memory tampering */
	bool CheckMemoryIntegrity();
	
	/** Check for speed hacks */
	bool CheckSpeedHack();
	
	/** Check for time tampering */
	bool CheckTimeTamper();
	
	/** Check device integrity */
	bool CheckDeviceIntegrity();
	
	/** Detect input automation */
	bool DetectInputAutomation() const;
	
	/** Get current timestamp */
	static int64 GetTimestamp();
};
