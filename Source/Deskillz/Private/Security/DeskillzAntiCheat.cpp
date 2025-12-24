// Copyright Deskillz Games. All Rights Reserved.

#include "Security/DeskillzAntiCheat.h"
#include "Deskillz.h"
#include "HAL/PlatformMisc.h"
#include "Misc/DateTime.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

// Static singleton
static UDeskillzAntiCheat* GAntiCheat = nullptr;

UDeskillzAntiCheat::UDeskillzAntiCheat()
{
	ProtectedScore.Set(0);
}

UDeskillzAntiCheat* UDeskillzAntiCheat::Get(const UObject* WorldContextObject)
{
	if (!GAntiCheat)
	{
		GAntiCheat = NewObject<UDeskillzAntiCheat>();
		GAntiCheat->AddToRoot(); // Prevent garbage collection
	}
	return GAntiCheat;
}

void UDeskillzAntiCheat::Initialize(const FString& MatchId)
{
	CurrentMatchId = MatchId;
	
	// Reset state
	ProtectedScore.Set(0);
	LastScore = 0;
	LastScoreUpdateTime = FPlatformTime::Seconds();
	LastTimeCheck = FPlatformTime::Seconds();
	DetectedViolations.Empty();
	InputHistory.Empty();
	FrameTimeSamples.Empty();
	
	// Run initial integrity check
	if (!RunIntegrityCheck())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Anti-cheat: Initial integrity check found issues"));
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Anti-cheat initialized for match: %s"), *MatchId);
}

void UDeskillzAntiCheat::Reset()
{
	CurrentMatchId.Empty();
	ProtectedScore.Set(0);
	LastScore = 0;
	DetectedViolations.Empty();
	InputHistory.Empty();
	FrameTimeSamples.Empty();
	
	UE_LOG(LogDeskillz, Log, TEXT("Anti-cheat reset"));
}

bool UDeskillzAntiCheat::RunIntegrityCheck()
{
	bool bAllPassed = true;
	
	// Memory integrity
	if (bEnableMemoryProtection && !CheckMemoryIntegrity())
	{
		ReportViolation(EDeskillzViolationType::MemoryTamper, EDeskillzViolationSeverity::High, TEXT("Memory integrity check failed"));
		bAllPassed = false;
	}
	
	// Speed hack check
	if (!CheckSpeedHack())
	{
		ReportViolation(EDeskillzViolationType::SpeedHack, EDeskillzViolationSeverity::High, TEXT("Speed manipulation detected"));
		bAllPassed = false;
	}
	
	// Time tamper check
	if (bEnableTimeValidation && !CheckTimeTamper())
	{
		ReportViolation(EDeskillzViolationType::TimeTamper, EDeskillzViolationSeverity::Medium, TEXT("Time manipulation suspected"));
		bAllPassed = false;
	}
	
	// Debugger check
	if (bEnableDebuggerDetection && IsDebuggerPresent())
	{
		ReportViolation(EDeskillzViolationType::DebuggerDetected, EDeskillzViolationSeverity::Medium, TEXT("Debugger detected"));
		bAllPassed = false;
	}
	
	// Device integrity
	if (bEnableDeviceIntegrity && !CheckDeviceIntegrity())
	{
		bAllPassed = false;
	}
	
	return bAllPassed;
}

// ============================================================================
// Score Protection
// ============================================================================

void UDeskillzAntiCheat::SetProtectedScore(int64 Score)
{
	ProtectedScore.Set(Score);
	LastScore = Score;
	LastScoreUpdateTime = FPlatformTime::Seconds();
}

int64 UDeskillzAntiCheat::GetProtectedScore() const
{
	return ProtectedScore.Get();
}

bool UDeskillzAntiCheat::VerifyScoreIntegrity() const
{
	if (!bEnableMemoryProtection)
	{
		return true;
	}
	
	return ProtectedScore.Verify();
}

bool UDeskillzAntiCheat::ValidateScoreUpdate(int64 NewScore, float DeltaTime)
{
	if (!bEnableScoreAnomalyDetection)
	{
		SetProtectedScore(NewScore);
		return true;
	}
	
	// Check for score decrease (usually invalid)
	if (NewScore < LastScore)
	{
		// Allow small decreases for games with penalties
		int64 Decrease = LastScore - NewScore;
		if (Decrease > 1000)
		{
			ReportViolation(
				EDeskillzViolationType::ScoreAnomaly, 
				EDeskillzViolationSeverity::Medium,
				FString::Printf(TEXT("Suspicious score decrease: %lld -> %lld"), LastScore, NewScore)
			);
		}
	}
	
	// Check score rate
	if (DeltaTime > 0)
	{
		float ScoreRate = (float)(NewScore - LastScore) / DeltaTime;
		
		if (ScoreRate > MaxScorePerSecond)
		{
			ReportViolation(
				EDeskillzViolationType::ScoreAnomaly,
				EDeskillzViolationSeverity::High,
				FString::Printf(TEXT("Score rate too high: %.2f/sec (max: %.2f)"), ScoreRate, MaxScorePerSecond)
			);
			return false;
		}
	}
	
	// Verify previous score wasn't tampered
	if (!VerifyScoreIntegrity())
	{
		ReportViolation(
			EDeskillzViolationType::MemoryTamper,
			EDeskillzViolationSeverity::Critical,
			TEXT("Score memory tampering detected")
		);
		return false;
	}
	
	SetProtectedScore(NewScore);
	return true;
}

// ============================================================================
// Time Validation
// ============================================================================

void UDeskillzAntiCheat::SyncServerTime(int64 ServerTimestamp)
{
	int64 LocalTimestamp = GetTimestamp();
	ServerTimeOffset = ServerTimestamp - LocalTimestamp;
	LastTimeCheck = FPlatformTime::Seconds();
	
	UE_LOG(LogDeskillz, Log, TEXT("Server time synced, offset: %lld ms"), ServerTimeOffset);
}

int64 UDeskillzAntiCheat::GetValidatedTimestamp() const
{
	return GetTimestamp() + ServerTimeOffset;
}

bool UDeskillzAntiCheat::IsTimeValid() const
{
	if (!bEnableTimeValidation)
	{
		return true;
	}
	
	// Check if time is moving forward at expected rate
	double CurrentTime = FPlatformTime::Seconds();
	double TimeDelta = CurrentTime - LastTimeCheck;
	
	// Time shouldn't jump forward or backward significantly
	if (FMath::Abs(TimeDelta) > TimeDriftTolerance)
	{
		return false;
	}
	
	return true;
}

// ============================================================================
// Environment Checks
// ============================================================================

bool UDeskillzAntiCheat::IsDebuggerPresent() const
{
	if (!bEnableDebuggerDetection)
	{
		return false;
	}
	
#if PLATFORM_WINDOWS
	return ::IsDebuggerPresent() != 0;
#elif PLATFORM_MAC || PLATFORM_IOS
	// Check for ptrace
	int mib[4];
	struct kinfo_proc info;
	size_t size = sizeof(info);
	
	info.kp_proc.p_flag = 0;
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = getpid();
	
	if (sysctl(mib, 4, &info, &size, NULL, 0) == -1)
	{
		return false;
	}
	
	return (info.kp_proc.p_flag & P_TRACED) != 0;
#elif PLATFORM_ANDROID
	// Check /proc/self/status for TracerPid
	// Simplified check
	return false;
#else
	return false;
#endif
}

bool UDeskillzAntiCheat::IsEmulatorDetected() const
{
#if PLATFORM_ANDROID
	// Android emulator detection
	// Check for common emulator properties
	FString Hardware = FAndroidMisc::GetDeviceModel();
	FString Brand = FString(); // Would need JNI to get Build.BRAND
	
	// Common emulator indicators
	if (Hardware.Contains(TEXT("goldfish")) || 
		Hardware.Contains(TEXT("sdk")) ||
		Hardware.Contains(TEXT("emulator")))
	{
		return true;
	}
#endif
	
#if PLATFORM_WINDOWS
	// VM detection on Windows
	// Check for common VM indicators
	FString CPUBrand = FPlatformMisc::GetCPUBrand();
	if (CPUBrand.Contains(TEXT("Virtual")) ||
		CPUBrand.Contains(TEXT("VMware")) ||
		CPUBrand.Contains(TEXT("QEMU")))
	{
		return true;
	}
#endif
	
	return false;
}

bool UDeskillzAntiCheat::IsDeviceRooted() const
{
#if PLATFORM_ANDROID
	// Check for root indicators on Android
	// This is a simplified check
	
	// Check for common root binaries
	TArray<FString> RootPaths = {
		TEXT("/system/app/Superuser.apk"),
		TEXT("/system/xbin/su"),
		TEXT("/system/bin/su"),
		TEXT("/sbin/su"),
		TEXT("/data/local/xbin/su"),
		TEXT("/data/local/bin/su"),
		TEXT("/data/local/su")
	};
	
	for (const FString& Path : RootPaths)
	{
		if (FPaths::FileExists(Path))
		{
			return true;
		}
	}
#endif

#if PLATFORM_IOS
	// Check for jailbreak indicators on iOS
	TArray<FString> JailbreakPaths = {
		TEXT("/Applications/Cydia.app"),
		TEXT("/Library/MobileSubstrate/MobileSubstrate.dylib"),
		TEXT("/bin/bash"),
		TEXT("/usr/sbin/sshd"),
		TEXT("/etc/apt"),
		TEXT("/private/var/lib/apt/")
	};
	
	for (const FString& Path : JailbreakPaths)
	{
		if (FPaths::FileExists(Path))
		{
			return true;
		}
	}
#endif
	
	return false;
}

int32 UDeskillzAntiCheat::GetDeviceTrustLevel() const
{
	int32 TrustLevel = 100;
	
	// Deduct for debugger
	if (IsDebuggerPresent())
	{
		TrustLevel -= 30;
	}
	
	// Deduct for emulator
	if (IsEmulatorDetected())
	{
		TrustLevel -= 25;
	}
	
	// Deduct for root/jailbreak
	if (IsDeviceRooted())
	{
		TrustLevel -= 40;
	}
	
	// Deduct for violations
	TrustLevel -= GetViolationCount(EDeskillzViolationSeverity::High) * 15;
	TrustLevel -= GetViolationCount(EDeskillzViolationSeverity::Medium) * 8;
	TrustLevel -= GetViolationCount(EDeskillzViolationSeverity::Low) * 3;
	
	return FMath::Clamp(TrustLevel, 0, 100);
}

// ============================================================================
// Violation Reporting
// ============================================================================

void UDeskillzAntiCheat::ReportViolation(EDeskillzViolationType Type, EDeskillzViolationSeverity Severity, const FString& Description)
{
	FDeskillzViolation Violation;
	Violation.Type = Type;
	Violation.Severity = Severity;
	Violation.Description = Description;
	Violation.Timestamp = GetTimestamp();
	
	// Build metadata
	TSharedPtr<FJsonObject> MetaJson = MakeShared<FJsonObject>();
	MetaJson->SetStringField(TEXT("matchId"), CurrentMatchId);
	MetaJson->SetNumberField(TEXT("score"), GetProtectedScore());
	MetaJson->SetNumberField(TEXT("trustLevel"), GetDeviceTrustLevel());
	
	FString MetaString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MetaString);
	FJsonSerializer::Serialize(MetaJson.ToSharedRef(), Writer);
	Violation.Metadata = MetaString;
	
	// Get stack trace for debugging
#if !UE_BUILD_SHIPPING
	Violation.StackTrace = FPlatformStackWalk::GetBackTraceString();
#endif
	
	DetectedViolations.Add(Violation);
	
	UE_LOG(LogDeskillz, Warning, TEXT("Anti-cheat violation: [%d] %s (Severity: %d)"), 
		(int32)Type, *Description, (int32)Severity);
	
	// Broadcast event
	OnViolationDetected.Broadcast(Violation);
}

int32 UDeskillzAntiCheat::GetViolationCount(EDeskillzViolationSeverity MinSeverity) const
{
	int32 Count = 0;
	for (const FDeskillzViolation& V : DetectedViolations)
	{
		if ((int32)V.Severity >= (int32)MinSeverity)
		{
			Count++;
		}
	}
	return Count;
}

void UDeskillzAntiCheat::ClearViolations()
{
	DetectedViolations.Empty();
}

bool UDeskillzAntiCheat::HasCriticalViolations() const
{
	for (const FDeskillzViolation& V : DetectedViolations)
	{
		if (V.Severity == EDeskillzViolationSeverity::Critical)
		{
			return true;
		}
	}
	return false;
}

FString UDeskillzAntiCheat::GetAntiCheatReport() const
{
	TSharedPtr<FJsonObject> ReportJson = MakeShared<FJsonObject>();
	
	ReportJson->SetStringField(TEXT("matchId"), CurrentMatchId);
	ReportJson->SetNumberField(TEXT("timestamp"), GetTimestamp());
	ReportJson->SetNumberField(TEXT("trustLevel"), GetDeviceTrustLevel());
	ReportJson->SetBoolField(TEXT("debuggerPresent"), IsDebuggerPresent());
	ReportJson->SetBoolField(TEXT("emulatorDetected"), IsEmulatorDetected());
	ReportJson->SetBoolField(TEXT("deviceRooted"), IsDeviceRooted());
	ReportJson->SetBoolField(TEXT("scoreIntegrity"), VerifyScoreIntegrity());
	ReportJson->SetNumberField(TEXT("violationCount"), DetectedViolations.Num());
	
	// Add violations array
	TArray<TSharedPtr<FJsonValue>> ViolationsArray;
	for (const FDeskillzViolation& V : DetectedViolations)
	{
		TSharedPtr<FJsonObject> VObj = MakeShared<FJsonObject>();
		VObj->SetNumberField(TEXT("type"), (int32)V.Type);
		VObj->SetNumberField(TEXT("severity"), (int32)V.Severity);
		VObj->SetStringField(TEXT("description"), V.Description);
		VObj->SetNumberField(TEXT("timestamp"), V.Timestamp);
		ViolationsArray.Add(MakeShared<FJsonValueObject>(VObj));
	}
	ReportJson->SetArrayField(TEXT("violations"), ViolationsArray);
	
	FString ReportString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ReportString);
	FJsonSerializer::Serialize(ReportJson.ToSharedRef(), Writer);
	
	return ReportString;
}

// ============================================================================
// Input Validation
// ============================================================================

void UDeskillzAntiCheat::RecordInput(const FString& InputType, FVector2D Position)
{
	InputHistory.Add(TPair<int64, FVector2D>(GetTimestamp(), Position));
	
	// Keep only recent inputs (last 1000)
	if (InputHistory.Num() > 1000)
	{
		InputHistory.RemoveAt(0, InputHistory.Num() - 1000);
	}
}

bool UDeskillzAntiCheat::ValidateInputPatterns() const
{
	if (InputHistory.Num() < 10)
	{
		return true; // Not enough data
	}
	
	return !DetectInputAutomation();
}

// ============================================================================
// Internal Methods
// ============================================================================

bool UDeskillzAntiCheat::CheckMemoryIntegrity()
{
	return VerifyScoreIntegrity();
}

bool UDeskillzAntiCheat::CheckSpeedHack()
{
	// Sample frame time
	static double LastFrameTime = 0;
	double CurrentTime = FPlatformTime::Seconds();
	
	if (LastFrameTime > 0)
	{
		float FrameTime = (float)(CurrentTime - LastFrameTime);
		FrameTimeSamples.Add(FrameTime);
		
		// Keep last 60 samples
		if (FrameTimeSamples.Num() > 60)
		{
			FrameTimeSamples.RemoveAt(0);
		}
		
		// Check for speed manipulation
		if (FrameTimeSamples.Num() >= 30)
		{
			// Calculate average frame time
			float TotalTime = 0;
			for (float FT : FrameTimeSamples)
			{
				TotalTime += FT;
			}
			float AvgFrameTime = TotalTime / FrameTimeSamples.Num();
			
			// If average frame time is much lower than expected (game running too fast)
			if (AvgFrameTime < ExpectedFrameTime * 0.5f)
			{
				return false; // Possible speed hack
			}
		}
	}
	
	LastFrameTime = CurrentTime;
	return true;
}

bool UDeskillzAntiCheat::CheckTimeTamper()
{
	static int64 LastTimestamp = 0;
	int64 CurrentTimestamp = GetTimestamp();
	
	if (LastTimestamp > 0)
	{
		// Time shouldn't go backward
		if (CurrentTimestamp < LastTimestamp)
		{
			return false;
		}
		
		// Time shouldn't jump forward too much
		int64 TimeDelta = CurrentTimestamp - LastTimestamp;
		if (TimeDelta > 5000) // More than 5 seconds jump
		{
			return false;
		}
	}
	
	LastTimestamp = CurrentTimestamp;
	return true;
}

bool UDeskillzAntiCheat::CheckDeviceIntegrity()
{
	bool bPassed = true;
	
	if (IsEmulatorDetected())
	{
		ReportViolation(EDeskillzViolationType::EmulatorDetected, EDeskillzViolationSeverity::Low, TEXT("Running in emulator"));
		// Note: Don't fail for emulator, just flag it
	}
	
	if (IsDeviceRooted())
	{
		ReportViolation(EDeskillzViolationType::RootDetected, EDeskillzViolationSeverity::Medium, TEXT("Device is rooted/jailbroken"));
		// Note: Don't fail for root, just flag it
	}
	
	return bPassed;
}

bool UDeskillzAntiCheat::DetectInputAutomation() const
{
	if (InputHistory.Num() < 50)
	{
		return false;
	}
	
	// Check for perfectly timed inputs (humans have variance)
	TArray<int64> InputDeltas;
	for (int32 i = 1; i < InputHistory.Num(); i++)
	{
		InputDeltas.Add(InputHistory[i].Key - InputHistory[i-1].Key);
	}
	
	// Calculate standard deviation
	float Mean = 0;
	for (int64 Delta : InputDeltas)
	{
		Mean += Delta;
	}
	Mean /= InputDeltas.Num();
	
	float Variance = 0;
	for (int64 Delta : InputDeltas)
	{
		Variance += FMath::Square((float)Delta - Mean);
	}
	Variance /= InputDeltas.Num();
	float StdDev = FMath::Sqrt(Variance);
	
	// If standard deviation is very low, inputs are suspiciously regular
	if (StdDev < 5.0f && Mean < 100.0f) // Less than 5ms variance at 100ms average
	{
		return true; // Likely automation
	}
	
	// Check for identical positions (humans have position variance)
	int32 IdenticalPositions = 0;
	for (int32 i = 1; i < InputHistory.Num(); i++)
	{
		if (InputHistory[i].Value.Equals(InputHistory[i-1].Value, 0.5f))
		{
			IdenticalPositions++;
		}
	}
	
	// If more than 80% of positions are identical, suspicious
	if ((float)IdenticalPositions / InputHistory.Num() > 0.8f)
	{
		return true;
	}
	
	return false;
}

int64 UDeskillzAntiCheat::GetTimestamp()
{
	return FDateTime::UtcNow().ToUnixTimestamp() * 1000 + FDateTime::UtcNow().GetMillisecond();
}
