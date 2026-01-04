// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzUpdater.generated.h"

/**
 * Update check result
 */
UENUM(BlueprintType)
enum class EDeskillzUpdateResult : uint8
{
	NoUpdateNeeded      UMETA(DisplayName = "No Update Needed"),
	UpdateAvailable     UMETA(DisplayName = "Optional Update Available"),
	ForceUpdateRequired UMETA(DisplayName = "Forced Update Required"),
	CheckFailed         UMETA(DisplayName = "Check Failed")
};

/**
 * Update information
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzUpdateInfo
{
	GENERATED_BODY()
	
	/** Latest available version string (e.g., "1.2.0") */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	FString LatestVersion;
	
	/** Latest version code (integer) */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	int32 VersionCode = 0;
	
	/** Whether an update is available */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	bool bUpdateAvailable = false;
	
	/** Whether this is a forced/required update */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	bool bIsForced = false;
	
	/** Direct download URL for the APK */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	FString DownloadUrl;
	
	/** File size in bytes */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	int64 FileSize = 0;
	
	/** Release notes/changelog */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	FString ReleaseNotes;
	
	/** Error message if check failed */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Update")
	FString ErrorMessage;
	
	/** Get file size formatted for display (e.g., "52.4 MB") */
	FString GetFileSizeFormatted() const;
	
	FDeskillzUpdateInfo() = default;
};

// ============================================================================
// Delegates
// ============================================================================

/** Delegate for update check started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzUpdateCheckStarted);

/** Delegate for update check completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeskillzUpdateCheckCompleted, EDeskillzUpdateResult, Result, const FDeskillzUpdateInfo&, UpdateInfo);

/** Delegate for optional update available */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzUpdateAvailable, const FDeskillzUpdateInfo&, UpdateInfo);

/** Delegate for forced update required */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzForceUpdateRequired, const FDeskillzUpdateInfo&, UpdateInfo);

/** Delegate for no update needed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeskillzNoUpdateNeeded);

/** Delegate for update check failed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzUpdateCheckFailed, const FString&, ErrorMessage);

/** Delegate for user accepted update */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzUpdateAccepted, const FDeskillzUpdateInfo&, UpdateInfo);

/** Delegate for user skipped update */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeskillzUpdateSkipped, const FDeskillzUpdateInfo&, UpdateInfo);

/**
 * Deskillz Auto-Updater
 * 
 * Manages automatic game updates from the Deskillz platform:
 * - Checks for new APK versions
 * - Handles forced vs optional updates
 * - Remembers skipped versions
 * - Provides delegates for custom UI
 * 
 * Usage:
 *   UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
 *   Updater->OnUpdateAvailable.AddDynamic(this, &AMyGameMode::HandleUpdateAvailable);
 *   Updater->CheckForUpdates();
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzUpdater : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzUpdater();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Updater singleton instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update", meta = (DisplayName = "Get Deskillz Updater"))
	static UDeskillzUpdater* Get();
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/**
	 * Set the current app version
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void SetCurrentVersion(const FString& Version, int32 VersionCode);
	
	/**
	 * Get the current app version string
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	FString GetCurrentVersion() const { return CurrentVersion; }
	
	/**
	 * Get the current version code
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	int32 GetCurrentVersionCode() const { return CurrentVersionCode; }
	
	/**
	 * Enable/disable automatic check on startup
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void SetAutoCheckEnabled(bool bEnabled) { bAutoCheckOnStart = bEnabled; }
	
	/**
	 * Enable/disable optional update prompts
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void SetShowOptionalUpdates(bool bShow) { bShowOptionalUpdatePrompt = bShow; }
	
	/**
	 * Enable/disable remembering skipped versions
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void SetRememberSkippedVersion(bool bRemember) { bRememberSkippedVersion = bRemember; }
	
	// ========================================================================
	// Update Operations
	// ========================================================================
	
	/**
	 * Check for available updates
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void CheckForUpdates();
	
	/**
	 * Start the update (opens download URL or app store)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void StartUpdate();
	
	/**
	 * Skip the current optional update
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void SkipUpdate();
	
	/**
	 * Clear skipped version (will prompt again)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Update")
	void ClearSkippedVersion();
	
	// ========================================================================
	// Status
	// ========================================================================
	
	/**
	 * Whether an update check is in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	bool IsCheckingForUpdates() const { return bIsCheckingForUpdates; }
	
	/**
	 * Whether an update is available
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	bool IsUpdateAvailable() const { return LatestUpdateInfo.bUpdateAvailable; }
	
	/**
	 * Whether a forced update is required
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	bool IsForcedUpdateRequired() const { return LatestUpdateInfo.bIsForced; }
	
	/**
	 * Get the latest update info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	FDeskillzUpdateInfo GetLatestUpdateInfo() const { return LatestUpdateInfo; }
	
	// ========================================================================
	// Delegates (Events)
	// ========================================================================
	
	/** Fired when update check starts */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzUpdateCheckStarted OnUpdateCheckStarted;
	
	/** Fired when update check completes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzUpdateCheckCompleted OnUpdateCheckCompleted;
	
	/** Fired when optional update is available */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzUpdateAvailable OnUpdateAvailable;
	
	/** Fired when forced update is required */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzForceUpdateRequired OnForceUpdateRequired;
	
	/** Fired when no update is needed */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzNoUpdateNeeded OnNoUpdateNeeded;
	
	/** Fired when update check fails */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzUpdateCheckFailed OnUpdateCheckFailed;
	
	/** Fired when user accepts update */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzUpdateAccepted OnUpdateAccepted;
	
	/** Fired when user skips update */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Update")
	FOnDeskillzUpdateSkipped OnUpdateSkipped;
	
	// ========================================================================
	// Utility
	// ========================================================================
	
	/**
	 * Compare two semantic version strings
	 * Returns: -1 if V1 < V2, 0 if equal, 1 if V1 > V2
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	static int32 CompareVersions(const FString& V1, const FString& V2);
	
	/**
	 * Format file size for display
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Update")
	static FString FormatFileSize(int64 Bytes);
	
protected:
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Current app version string */
	FString CurrentVersion = TEXT("1.0.0");
	
	/** Current version code */
	int32 CurrentVersionCode = 1;
	
	/** Check for updates automatically on startup */
	bool bAutoCheckOnStart = true;
	
	/** Delay before auto-check (seconds) */
	float AutoCheckDelay = 2.0f;
	
	/** Show prompts for optional updates */
	bool bShowOptionalUpdatePrompt = true;
	
	/** Allow skipping optional updates */
	bool bAllowSkipOptionalUpdate = true;
	
	/** Remember skipped versions */
	bool bRememberSkippedVersion = true;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Whether currently checking for updates */
	bool bIsCheckingForUpdates = false;
	
	/** Latest update information */
	FDeskillzUpdateInfo LatestUpdateInfo;
	
	/** Skipped version code (stored in config) */
	int32 SkippedVersionCode = 0;
	
	// ========================================================================
	// Internal
	// ========================================================================
	
	/** Handle HTTP response for version check */
	void HandleVersionCheckResponse(const struct FDeskillzHttpResponse& Response);
	
	/** Handle update availability */
	void HandleUpdateAvailable();
	
	/** Load skipped version from config */
	void LoadSkippedVersion();
	
	/** Save skipped version to config */
	void SaveSkippedVersion(int32 VersionCode);
	
private:
	/** Config section for storing update preferences */
	static const FString ConfigSection;
	static const FString SkippedVersionKey;
};