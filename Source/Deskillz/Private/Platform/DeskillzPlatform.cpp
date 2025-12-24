// Copyright Deskillz Games. All Rights Reserved.

#include "Platform/DeskillzPlatform.h"
#include "Deskillz.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/App.h"
#include "GenericPlatform/GenericApplication.h"
#include "Framework/Application/SlateApplication.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

// Static singleton
static UDeskillzPlatform* GPlatform = nullptr;

UDeskillzPlatform::UDeskillzPlatform()
{
}

UDeskillzPlatform* UDeskillzPlatform::Get()
{
	if (!GPlatform)
	{
		GPlatform = NewObject<UDeskillzPlatform>();
		GPlatform->AddToRoot();
	}
	return GPlatform;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzPlatform::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}
	
	CollectDeviceInfo();
	RefreshNetworkInfo();
	RefreshBatteryInfo();
	
	StartMonitoring();
	
	bIsInitialized = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("Platform initialized: %s (%s)"), 
		*GetPlatformName(), *DeviceInfo.DeviceModel);
}

void UDeskillzPlatform::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	StopMonitoring();
	bIsInitialized = false;
	
	UE_LOG(LogDeskillz, Log, TEXT("Platform shutdown"));
}

// ============================================================================
// Platform Detection
// ============================================================================

FString UDeskillzPlatform::GetPlatformName() const
{
	switch (DeviceInfo.Platform)
	{
		case EDeskillzPlatformType::Windows: return TEXT("Windows");
		case EDeskillzPlatformType::Mac: return TEXT("Mac");
		case EDeskillzPlatformType::Linux: return TEXT("Linux");
		case EDeskillzPlatformType::iOS: return TEXT("iOS");
		case EDeskillzPlatformType::Android: return TEXT("Android");
		case EDeskillzPlatformType::Xbox: return TEXT("Xbox");
		case EDeskillzPlatformType::PlayStation: return TEXT("PlayStation");
		case EDeskillzPlatformType::Switch: return TEXT("Switch");
		default: return TEXT("Unknown");
	}
}

bool UDeskillzPlatform::IsMobile() const
{
	return DeviceInfo.Platform == EDeskillzPlatformType::iOS || 
		   DeviceInfo.Platform == EDeskillzPlatformType::Android;
}

bool UDeskillzPlatform::IsDesktop() const
{
	return DeviceInfo.Platform == EDeskillzPlatformType::Windows ||
		   DeviceInfo.Platform == EDeskillzPlatformType::Mac ||
		   DeviceInfo.Platform == EDeskillzPlatformType::Linux;
}

bool UDeskillzPlatform::IsConsole() const
{
	return DeviceInfo.Platform == EDeskillzPlatformType::Xbox ||
		   DeviceInfo.Platform == EDeskillzPlatformType::PlayStation ||
		   DeviceInfo.Platform == EDeskillzPlatformType::Switch;
}

// ============================================================================
// Device Info
// ============================================================================

void UDeskillzPlatform::RefreshDeviceInfo()
{
	CollectDeviceInfo();
}

void UDeskillzPlatform::CollectDeviceInfo()
{
	// Platform type
	DeviceInfo.Platform = DetectPlatform();
	
	// Device model and manufacturer
	DeviceInfo.DeviceModel = FPlatformMisc::GetDefaultDeviceProfileName();
	DeviceInfo.CPUBrand = FPlatformMisc::GetCPUBrand();
	DeviceInfo.GPUBrand = FPlatformMisc::GetPrimaryGPUBrand();
	
	// OS version
	DeviceInfo.OSVersion = FPlatformMisc::GetOSVersion();
	
	// Device ID
	DeviceInfo.DeviceId = FPlatformMisc::GetDeviceId();
	
	// Memory
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	DeviceInfo.TotalRAM = static_cast<int32>(MemStats.TotalPhysical / (1024 * 1024));
	DeviceInfo.AvailableRAM = static_cast<int32>(MemStats.AvailablePhysical / (1024 * 1024));
	
	// CPU
	DeviceInfo.CPUCores = FPlatformMisc::NumberOfCores();
	
	// Screen
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		DeviceInfo.ScreenWidth = static_cast<int32>(ViewportSize.X);
		DeviceInfo.ScreenHeight = static_cast<int32>(ViewportSize.Y);
	}
	
	// Locale
	DeviceInfo.LanguageCode = FPlatformMisc::GetDefaultLanguage();
	DeviceInfo.CountryCode = FPlatformMisc::GetDefaultLocale();
	
	// App info
	DeviceInfo.AppVersion = FApp::GetProjectVersion();
	DeviceInfo.BuildNumber = FApp::GetBuildVersion();
	
	// Device tier
	DeviceInfo.DeviceTier = DetectDeviceTier();
	
	// Platform-specific detection
#if PLATFORM_IOS
	DeviceInfo.Manufacturer = TEXT("Apple");
	// Check for tablet based on screen size
	DeviceInfo.bIsTablet = FMath::Max(DeviceInfo.ScreenWidth, DeviceInfo.ScreenHeight) >= 1024;
	DeviceInfo.bSupportsHaptics = true;
#elif PLATFORM_ANDROID
	// Get manufacturer from Android API (simplified)
	DeviceInfo.Manufacturer = TEXT("Android OEM");
	DeviceInfo.bIsTablet = FMath::Max(DeviceInfo.ScreenWidth, DeviceInfo.ScreenHeight) >= 1024;
	DeviceInfo.bSupportsHaptics = true;
#else
	DeviceInfo.bIsTablet = false;
	DeviceInfo.bSupportsHaptics = false;
#endif
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Device: %s | OS: %s | RAM: %dMB | Tier: %d"),
		*DeviceInfo.DeviceModel, *DeviceInfo.OSVersion, DeviceInfo.TotalRAM,
		static_cast<int32>(DeviceInfo.DeviceTier));
}

EDeskillzPlatformType UDeskillzPlatform::DetectPlatform() const
{
#if PLATFORM_WINDOWS
	return EDeskillzPlatformType::Windows;
#elif PLATFORM_MAC
	return EDeskillzPlatformType::Mac;
#elif PLATFORM_LINUX
	return EDeskillzPlatformType::Linux;
#elif PLATFORM_IOS
	return EDeskillzPlatformType::iOS;
#elif PLATFORM_ANDROID
	return EDeskillzPlatformType::Android;
#elif PLATFORM_XBOXONE || PLATFORM_XSX
	return EDeskillzPlatformType::Xbox;
#elif PLATFORM_PS4 || PLATFORM_PS5
	return EDeskillzPlatformType::PlayStation;
#elif PLATFORM_SWITCH
	return EDeskillzPlatformType::Switch;
#else
	return EDeskillzPlatformType::Unknown;
#endif
}

EDeskillzDeviceTier UDeskillzPlatform::DetectDeviceTier() const
{
	// Simple tier detection based on RAM and CPU cores
	int32 Score = 0;
	
	// RAM scoring
	if (DeviceInfo.TotalRAM >= 8192) // 8GB+
	{
		Score += 3;
	}
	else if (DeviceInfo.TotalRAM >= 4096) // 4GB+
	{
		Score += 2;
	}
	else if (DeviceInfo.TotalRAM >= 2048) // 2GB+
	{
		Score += 1;
	}
	
	// CPU cores scoring
	if (DeviceInfo.CPUCores >= 8)
	{
		Score += 3;
	}
	else if (DeviceInfo.CPUCores >= 4)
	{
		Score += 2;
	}
	else if (DeviceInfo.CPUCores >= 2)
	{
		Score += 1;
	}
	
	// Determine tier
	if (Score >= 5)
	{
		return EDeskillzDeviceTier::High;
	}
	else if (Score >= 3)
	{
		return EDeskillzDeviceTier::Medium;
	}
	else
	{
		return EDeskillzDeviceTier::Low;
	}
}

// ============================================================================
// Network
// ============================================================================

void UDeskillzPlatform::RefreshNetworkInfo()
{
	// Check connection status using platform-specific methods
#if PLATFORM_IOS || PLATFORM_ANDROID
	NetworkInfo.bIsConnected = FPlatformMisc::HasActiveWiFiConnection() || 
							   FPlatformMisc::HasCellularConnection();
	
	if (FPlatformMisc::HasActiveWiFiConnection())
	{
		NetworkInfo.NetworkType = EDeskillzNetworkType::WiFi;
	}
	else if (FPlatformMisc::HasCellularConnection())
	{
		// Default to 4G, more specific detection would require native code
		NetworkInfo.NetworkType = EDeskillzNetworkType::Cellular4G;
	}
	else
	{
		NetworkInfo.NetworkType = EDeskillzNetworkType::None;
	}
#else
	// Desktop platforms - simplified check
	NetworkInfo.bIsConnected = true;
	NetworkInfo.NetworkType = EDeskillzNetworkType::Ethernet;
#endif
}

bool UDeskillzPlatform::IsOnCellular() const
{
	return NetworkInfo.NetworkType == EDeskillzNetworkType::Cellular2G ||
		   NetworkInfo.NetworkType == EDeskillzNetworkType::Cellular3G ||
		   NetworkInfo.NetworkType == EDeskillzNetworkType::Cellular4G ||
		   NetworkInfo.NetworkType == EDeskillzNetworkType::Cellular5G;
}

// ============================================================================
// Battery
// ============================================================================

void UDeskillzPlatform::RefreshBatteryInfo()
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	BatteryInfo.bHasBattery = true;
	
	// Get battery level using platform API
	FPlatformMisc::GetBatteryLevel(BatteryInfo.Level);
	
	// Estimate charging based on level changes (simplified)
	static int32 LastBatteryLevel = -1;
	if (LastBatteryLevel >= 0)
	{
		BatteryInfo.bIsCharging = BatteryInfo.Level > LastBatteryLevel;
	}
	LastBatteryLevel = BatteryInfo.Level;
	
	BatteryInfo.bIsLowBattery = BatteryInfo.Level < LowBatteryThreshold;
#else
	// Desktop - may or may not have battery
	BatteryInfo.bHasBattery = false;
	BatteryInfo.Level = 100;
	BatteryInfo.bIsCharging = true;
	BatteryInfo.bIsLowBattery = false;
#endif
}

// ============================================================================
// Platform Features
// ============================================================================

void UDeskillzPlatform::OpenURL(const FString& URL)
{
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
	UE_LOG(LogDeskillz, Log, TEXT("Opening URL: %s"), *URL);
}

void UDeskillzPlatform::OpenAppStore()
{
	FString StoreURL;
	
#if PLATFORM_IOS
	// iOS App Store URL
	StoreURL = TEXT("https://apps.apple.com/app/deskillz/id0000000000");
#elif PLATFORM_ANDROID
	// Google Play Store URL
	StoreURL = TEXT("market://details?id=com.deskillz.app");
#else
	// Fallback to web
	StoreURL = TEXT("https://www.deskillz.games");
#endif
	
	OpenURL(StoreURL);
}

void UDeskillzPlatform::ShareText(const FString& Text, const FString& Title)
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	// Use native sharing dialog
	FPlatformMisc::ShareURL(Text, FText::FromString(Title), TEXT(""));
#else
	// Desktop - copy to clipboard
	CopyToClipboard(Text);
	UE_LOG(LogDeskillz, Log, TEXT("Share not supported on desktop, copied to clipboard"));
#endif
}

void UDeskillzPlatform::CopyToClipboard(const FString& Text)
{
	FPlatformApplicationMisc::ClipboardCopy(*Text);
	UE_LOG(LogDeskillz, Verbose, TEXT("Copied to clipboard: %s"), *Text);
}

FString UDeskillzPlatform::GetFromClipboard()
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
	return ClipboardContent;
}

void UDeskillzPlatform::TriggerHaptic(float Intensity, float Duration)
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	if (DeviceInfo.bSupportsHaptics)
	{
		// Use input subsystem for haptics
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().PlayHapticFeedback(
				static_cast<int32>(Intensity * 100),
				0, // Controller ID
				EHapticFeedbackTargetPlatform::Platform
			);
		}
	}
#endif
}

void UDeskillzPlatform::SetScreenBrightness(float Brightness)
{
	Brightness = FMath::Clamp(Brightness, 0.0f, 1.0f);
	
#if PLATFORM_IOS || PLATFORM_ANDROID
	// Platform-specific brightness control would require native code
	UE_LOG(LogDeskillz, Verbose, TEXT("Set screen brightness: %.2f"), Brightness);
#endif
}

void UDeskillzPlatform::SetKeepScreenOn(bool bKeepOn)
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	FPlatformMisc::ControlScreensaver(bKeepOn ? FPlatformMisc::EScreenSaverAction::Disable : 
		FPlatformMisc::EScreenSaverAction::Enable);
	UE_LOG(LogDeskillz, Log, TEXT("Keep screen on: %s"), bKeepOn ? TEXT("true") : TEXT("false"));
#endif
}

// ============================================================================
// Monitoring
// ============================================================================

void UDeskillzPlatform::StartMonitoring()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(
			MonitorTimerHandle,
			this,
			&UDeskillzPlatform::MonitorStatus,
			5.0f, // Check every 5 seconds
			true
		);
	}
}

void UDeskillzPlatform::StopMonitoring()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(MonitorTimerHandle);
	}
}

void UDeskillzPlatform::MonitorStatus()
{
	// Refresh network status
	RefreshNetworkInfo();
	
	// Check for network status change
	if (NetworkInfo.bIsConnected != bPreviousNetworkConnected)
	{
		OnNetworkStatusChanged.Broadcast(NetworkInfo.bIsConnected);
		bPreviousNetworkConnected = NetworkInfo.bIsConnected;
		
		UE_LOG(LogDeskillz, Log, TEXT("Network status changed: %s"), 
			NetworkInfo.bIsConnected ? TEXT("Connected") : TEXT("Disconnected"));
	}
	
	// Refresh battery status
	RefreshBatteryInfo();
	
	// Check for low battery
	if (BatteryInfo.bIsLowBattery && BatteryInfo.bHasBattery)
	{
		OnBatteryLow.Broadcast(BatteryInfo.Level);
	}
	
	// Check memory
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	int32 AvailableMB = static_cast<int32>(MemStats.AvailablePhysical / (1024 * 1024));
	DeviceInfo.AvailableRAM = AvailableMB;
	
	// Memory warning if less than 256MB available
	if (AvailableMB < 256)
	{
		OnMemoryWarning.Broadcast(AvailableMB);
		UE_LOG(LogDeskillz, Warning, TEXT("Low memory warning: %dMB available"), AvailableMB);
	}
}
