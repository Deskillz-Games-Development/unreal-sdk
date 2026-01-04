// Copyright Deskillz Games. All Rights Reserved.

#include "Core/DeskillzUpdater.h"
#include "Network/DeskillzHttpClient.h"
#include "Core/DeskillzConfig.h"
#include "Deskillz.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/ConfigCacheIni.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformProcess.h"

// ============================================================================
// Constants
// ============================================================================

const FString UDeskillzUpdater::ConfigSection = TEXT("Deskillz.Updater");
const FString UDeskillzUpdater::SkippedVersionKey = TEXT("SkippedVersionCode");

// Static singleton
static UDeskillzUpdater* GUpdater = nullptr;

// ============================================================================
// FDeskillzUpdateInfo Implementation
// ============================================================================

FString FDeskillzUpdateInfo::GetFileSizeFormatted() const
{
	return UDeskillzUpdater::FormatFileSize(FileSize);
}

// ============================================================================
// Constructor
// ============================================================================

UDeskillzUpdater::UDeskillzUpdater()
{
	// Load skipped version from config
	LoadSkippedVersion();
}

// ============================================================================
// Singleton Access
// ============================================================================

UDeskillzUpdater* UDeskillzUpdater::Get()
{
	if (!GUpdater)
	{
		GUpdater = NewObject<UDeskillzUpdater>();
		GUpdater->AddToRoot(); // Prevent garbage collection
		
		UE_LOG(LogDeskillz, Log, TEXT("DeskillzUpdater initialized"));
	}
	return GUpdater;
}

// ============================================================================
// Configuration
// ============================================================================

void UDeskillzUpdater::SetCurrentVersion(const FString& Version, int32 VersionCode)
{
	CurrentVersion = Version;
	CurrentVersionCode = VersionCode;
	
	UE_LOG(LogDeskillz, Log, TEXT("Updater version set: %s (%d)"), *CurrentVersion, CurrentVersionCode);
}

// ============================================================================
// Update Operations
// ============================================================================

void UDeskillzUpdater::CheckForUpdates()
{
	if (bIsCheckingForUpdates)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Update check already in progress"));
		return;
	}
	
	bIsCheckingForUpdates = true;
	OnUpdateCheckStarted.Broadcast();
	
	UE_LOG(LogDeskillz, Log, TEXT("Checking for updates..."));
	
	// Get configuration
	const UDeskillzConfig* Config = UDeskillzConfig::Get();
	if (!Config)
	{
		UE_LOG(LogDeskillz, Error, TEXT("DeskillzConfig not found!"));
		
		FDeskillzUpdateInfo ErrorInfo;
		ErrorInfo.ErrorMessage = TEXT("Configuration not found");
		
		bIsCheckingForUpdates = false;
		OnUpdateCheckFailed.Broadcast(ErrorInfo.ErrorMessage);
		OnUpdateCheckCompleted.Broadcast(EDeskillzUpdateResult::CheckFailed, ErrorInfo);
		return;
	}
	
	// Build endpoint URL
	FString GameId = Config->GameId;
	FString BaseUrl = Config->GetBaseUrl();
	
	// Determine platform
	FString Platform = TEXT("ANDROID");
#if PLATFORM_IOS
	Platform = TEXT("IOS");
#endif
	
	// Build query parameters
	TMap<FString, FString> QueryParams;
	QueryParams.Add(TEXT("gameId"), GameId);
	QueryParams.Add(TEXT("currentVersion"), CurrentVersion);
	QueryParams.Add(TEXT("versionCode"), FString::FromInt(CurrentVersionCode));
	QueryParams.Add(TEXT("platform"), Platform);
	
	// Make request
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->Get(TEXT("/api/v1/sdk/version-check"),
		FOnDeskillzHttpResponse::CreateUObject(this, &UDeskillzUpdater::HandleVersionCheckResponse),
		QueryParams
	);
}

void UDeskillzUpdater::HandleVersionCheckResponse(const FDeskillzHttpResponse& Response)
{
	bIsCheckingForUpdates = false;
	
	if (!Response.IsOk())
	{
		UE_LOG(LogDeskillz, Error, TEXT("Update check failed: %s"), *Response.ErrorMessage);
		
		FDeskillzUpdateInfo ErrorInfo;
		ErrorInfo.ErrorMessage = Response.ErrorMessage;
		
		OnUpdateCheckFailed.Broadcast(Response.ErrorMessage);
		OnUpdateCheckCompleted.Broadcast(EDeskillzUpdateResult::CheckFailed, ErrorInfo);
		return;
	}
	
	// Parse JSON response
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response.Body);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogDeskillz, Error, TEXT("Failed to parse update response JSON"));
		
		FDeskillzUpdateInfo ErrorInfo;
		ErrorInfo.ErrorMessage = TEXT("Failed to parse response");
		
		OnUpdateCheckFailed.Broadcast(ErrorInfo.ErrorMessage);
		OnUpdateCheckCompleted.Broadcast(EDeskillzUpdateResult::CheckFailed, ErrorInfo);
		return;
	}
	
	// Extract update info
	LatestUpdateInfo = FDeskillzUpdateInfo();
	LatestUpdateInfo.LatestVersion = JsonObject->GetStringField(TEXT("latestVersion"));
	LatestUpdateInfo.VersionCode = JsonObject->GetIntegerField(TEXT("versionCode"));
	LatestUpdateInfo.bUpdateAvailable = JsonObject->GetBoolField(TEXT("updateAvailable"));
	LatestUpdateInfo.bIsForced = JsonObject->GetBoolField(TEXT("isForced"));
	LatestUpdateInfo.DownloadUrl = JsonObject->GetStringField(TEXT("downloadUrl"));
	LatestUpdateInfo.FileSize = JsonObject->GetNumberField(TEXT("fileSize"));
	LatestUpdateInfo.ReleaseNotes = JsonObject->GetStringField(TEXT("releaseNotes"));
	
	UE_LOG(LogDeskillz, Log, TEXT("Update check complete. Available: %s, Forced: %s, Version: %s"),
		LatestUpdateInfo.bUpdateAvailable ? TEXT("Yes") : TEXT("No"),
		LatestUpdateInfo.bIsForced ? TEXT("Yes") : TEXT("No"),
		*LatestUpdateInfo.LatestVersion
	);
	
	// Handle result
	if (LatestUpdateInfo.bUpdateAvailable)
	{
		HandleUpdateAvailable();
	}
	else
	{
		OnNoUpdateNeeded.Broadcast();
		OnUpdateCheckCompleted.Broadcast(EDeskillzUpdateResult::NoUpdateNeeded, LatestUpdateInfo);
	}
}

void UDeskillzUpdater::HandleUpdateAvailable()
{
	// Check if this version was previously skipped
	if (bRememberSkippedVersion && !LatestUpdateInfo.bIsForced)
	{
		if (SkippedVersionCode >= LatestUpdateInfo.VersionCode)
		{
			UE_LOG(LogDeskillz, Log, TEXT("Version %d was previously skipped"), LatestUpdateInfo.VersionCode);
			OnNoUpdateNeeded.Broadcast();
			OnUpdateCheckCompleted.Broadcast(EDeskillzUpdateResult::NoUpdateNeeded, LatestUpdateInfo);
			return;
		}
	}
	
	if (LatestUpdateInfo.bIsForced)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Forced update required! Version: %s"), *LatestUpdateInfo.LatestVersion);
		OnForceUpdateRequired.Broadcast(LatestUpdateInfo);
		OnUpdateCheckCompleted.Broadcast(EDeskillzUpdateResult::ForceUpdateRequired, LatestUpdateInfo);
	}
	else
	{
		UE_LOG(LogDeskillz, Log, TEXT("Optional update available: %s"), *LatestUpdateInfo.LatestVersion);
		
		if (bShowOptionalUpdatePrompt)
		{
			OnUpdateAvailable.Broadcast(LatestUpdateInfo);
		}
		OnUpdateCheckCompleted.Broadcast(EDeskillzUpdateResult::UpdateAvailable, LatestUpdateInfo);
	}
}

void UDeskillzUpdater::StartUpdate()
{
	if (!LatestUpdateInfo.bUpdateAvailable)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No update available to start"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Starting update to version %s"), *LatestUpdateInfo.LatestVersion);
	
	// Broadcast event
	OnUpdateAccepted.Broadcast(LatestUpdateInfo);
	
	// Open download URL
	FString UrlToOpen = LatestUpdateInfo.DownloadUrl;
	
	if (UrlToOpen.IsEmpty())
	{
		// Fallback to game page on website
		const UDeskillzConfig* Config = UDeskillzConfig::Get();
		FString GameId = Config ? Config->GameId : TEXT("");
		UrlToOpen = FString::Printf(TEXT("https://deskillz.games/games/%s/download"), *GameId);
	}
	
	// Open URL in browser
	FPlatformProcess::LaunchURL(*UrlToOpen, nullptr, nullptr);
}

void UDeskillzUpdater::SkipUpdate()
{
	if (!LatestUpdateInfo.bUpdateAvailable)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("No update to skip"));
		return;
	}
	
	if (LatestUpdateInfo.bIsForced)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot skip forced update"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("User skipped update to version %s"), *LatestUpdateInfo.LatestVersion);
	
	// Remember skipped version
	if (bRememberSkippedVersion)
	{
		SaveSkippedVersion(LatestUpdateInfo.VersionCode);
	}
	
	// Broadcast event
	OnUpdateSkipped.Broadcast(LatestUpdateInfo);
}

void UDeskillzUpdater::ClearSkippedVersion()
{
	SkippedVersionCode = 0;
	
	// Clear from config
	GConfig->SetInt(*ConfigSection, *SkippedVersionKey, 0, GGameIni);
	GConfig->Flush(false, GGameIni);
	
	UE_LOG(LogDeskillz, Log, TEXT("Skipped version cleared"));
}

// ============================================================================
// Config Persistence
// ============================================================================

void UDeskillzUpdater::LoadSkippedVersion()
{
	GConfig->GetInt(*ConfigSection, *SkippedVersionKey, SkippedVersionCode, GGameIni);
	
	if (SkippedVersionCode > 0)
	{
		UE_LOG(LogDeskillz, Log, TEXT("Loaded skipped version code: %d"), SkippedVersionCode);
	}
}

void UDeskillzUpdater::SaveSkippedVersion(int32 VersionCode)
{
	SkippedVersionCode = VersionCode;
	
	GConfig->SetInt(*ConfigSection, *SkippedVersionKey, VersionCode, GGameIni);
	GConfig->Flush(false, GGameIni);
	
	UE_LOG(LogDeskillz, Log, TEXT("Saved skipped version code: %d"), VersionCode);
}

// ============================================================================
// Utility Functions
// ============================================================================

int32 UDeskillzUpdater::CompareVersions(const FString& V1, const FString& V2)
{
	if (V1.IsEmpty()) return -1;
	if (V2.IsEmpty()) return 1;
	
	TArray<FString> Parts1, Parts2;
	V1.ParseIntoArray(Parts1, TEXT("."));
	V2.ParseIntoArray(Parts2, TEXT("."));
	
	int32 MaxLength = FMath::Max(Parts1.Num(), Parts2.Num());
	
	for (int32 i = 0; i < MaxLength; i++)
	{
		int32 Num1 = (i < Parts1.Num()) ? FCString::Atoi(*Parts1[i]) : 0;
		int32 Num2 = (i < Parts2.Num()) ? FCString::Atoi(*Parts2[i]) : 0;
		
		if (Num1 < Num2) return -1;
		if (Num1 > Num2) return 1;
	}
	
	return 0; // Equal
}

FString UDeskillzUpdater::FormatFileSize(int64 Bytes)
{
	if (Bytes < 1024)
	{
		return FString::Printf(TEXT("%lld B"), Bytes);
	}
	else if (Bytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f KB"), Bytes / 1024.0);
	}
	else if (Bytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f MB"), Bytes / (1024.0 * 1024.0));
	}
	else
	{
		return FString::Printf(TEXT("%.2f GB"), Bytes / (1024.0 * 1024.0 * 1024.0));
	}
}