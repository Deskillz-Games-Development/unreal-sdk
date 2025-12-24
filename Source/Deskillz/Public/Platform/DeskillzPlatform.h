// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzPlatform.generated.h"

/**
 * Platform type
 */
UENUM(BlueprintType)
enum class EDeskillzPlatformType : uint8
{
	Unknown,
	Windows,
	Mac,
	Linux,
	iOS,
	Android,
	Xbox,
	PlayStation,
	Switch
};

/**
 * Device tier for performance scaling
 */
UENUM(BlueprintType)
enum class EDeskillzDeviceTier : uint8
{
	/** Low-end device */
	Low,
	
	/** Mid-range device */
	Medium,
	
	/** High-end device */
	High,
	
	/** Unknown tier */
	Unknown
};

/**
 * Network type
 */
UENUM(BlueprintType)
enum class EDeskillzNetworkType : uint8
{
	None,
	WiFi,
	Cellular2G,
	Cellular3G,
	Cellular4G,
	Cellular5G,
	Ethernet,
	Unknown
};

/**
 * Platform device info
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzDeviceInfo
{
	GENERATED_BODY()
	
	/** Platform type */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	EDeskillzPlatformType Platform = EDeskillzPlatformType::Unknown;
	
	/** Device tier */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	EDeskillzDeviceTier DeviceTier = EDeskillzDeviceTier::Unknown;
	
	/** Device model (e.g., "iPhone 14 Pro") */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString DeviceModel;
	
	/** Device manufacturer */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString Manufacturer;
	
	/** OS version */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString OSVersion;
	
	/** Unique device ID */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString DeviceId;
	
	/** Total RAM (MB) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	int32 TotalRAM = 0;
	
	/** Available RAM (MB) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	int32 AvailableRAM = 0;
	
	/** Screen width (pixels) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	int32 ScreenWidth = 0;
	
	/** Screen height (pixels) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	int32 ScreenHeight = 0;
	
	/** Screen DPI */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	float ScreenDPI = 0.0f;
	
	/** CPU cores */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	int32 CPUCores = 0;
	
	/** CPU brand */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString CPUBrand;
	
	/** GPU brand */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString GPUBrand;
	
	/** App version */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString AppVersion;
	
	/** App build number */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString BuildNumber;
	
	/** Bundle/package ID */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString BundleId;
	
	/** Current language code */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString LanguageCode;
	
	/** Current country code */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString CountryCode;
	
	/** Timezone identifier */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString TimeZone;
	
	/** Is tablet device */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bIsTablet = false;
	
	/** Has notch/cutout */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bHasNotch = false;
	
	/** Supports haptics */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bSupportsHaptics = false;
};

/**
 * Network info
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzNetworkInfo
{
	GENERATED_BODY()
	
	/** Network type */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	EDeskillzNetworkType NetworkType = EDeskillzNetworkType::Unknown;
	
	/** Is connected */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bIsConnected = false;
	
	/** Carrier name (mobile) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	FString CarrierName;
	
	/** Signal strength (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	int32 SignalStrength = 0;
	
	/** Is roaming */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bIsRoaming = false;
	
	/** Is metered connection */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bIsMetered = false;
};

/**
 * Battery info
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzBatteryInfo
{
	GENERATED_BODY()
	
	/** Battery level (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	int32 Level = 100;
	
	/** Is charging */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bIsCharging = false;
	
	/** Is low battery */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bIsLowBattery = false;
	
	/** Has battery (desktop may not) */
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	bool bHasBattery = false;
};

/** Platform delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkStatusChanged, bool, bIsConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBatteryLow, int32, BatteryLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryWarning, int32, AvailableRAM);

/**
 * Deskillz Platform Manager
 * 
 * Cross-platform abstraction layer:
 * - Device detection and info
 * - Network status monitoring
 * - Battery monitoring
 * - Platform-specific features
 * - Device tier classification
 * 
 * Usage:
 *   UDeskillzPlatform* Platform = UDeskillzPlatform::Get();
 *   FDeskillzDeviceInfo Info = Platform->GetDeviceInfo();
 *   
 *   if (Platform->IsMobile()) {
 *       // Mobile-specific code
 *   }
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzPlatform : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzPlatform();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Platform instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform", meta = (DisplayName = "Get Deskillz Platform"))
	static UDeskillzPlatform* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize platform services
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void Initialize();
	
	/**
	 * Shutdown platform services
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void Shutdown();
	
	// ========================================================================
	// Platform Detection
	// ========================================================================
	
	/**
	 * Get current platform type
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	EDeskillzPlatformType GetPlatformType() const { return DeviceInfo.Platform; }
	
	/**
	 * Get platform name string
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	FString GetPlatformName() const;
	
	/**
	 * Is mobile platform (iOS or Android)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsMobile() const;
	
	/**
	 * Is desktop platform
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsDesktop() const;
	
	/**
	 * Is console platform
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsConsole() const;
	
	/**
	 * Is iOS
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsIOS() const { return DeviceInfo.Platform == EDeskillzPlatformType::iOS; }
	
	/**
	 * Is Android
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsAndroid() const { return DeviceInfo.Platform == EDeskillzPlatformType::Android; }
	
	// ========================================================================
	// Device Info
	// ========================================================================
	
	/**
	 * Get device info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	FDeskillzDeviceInfo GetDeviceInfo() const { return DeviceInfo; }
	
	/**
	 * Get device tier
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	EDeskillzDeviceTier GetDeviceTier() const { return DeviceInfo.DeviceTier; }
	
	/**
	 * Get unique device ID
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	FString GetDeviceId() const { return DeviceInfo.DeviceId; }
	
	/**
	 * Is tablet device
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsTablet() const { return DeviceInfo.bIsTablet; }
	
	/**
	 * Refresh device info
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void RefreshDeviceInfo();
	
	// ========================================================================
	// Network
	// ========================================================================
	
	/**
	 * Get network info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	FDeskillzNetworkInfo GetNetworkInfo() const { return NetworkInfo; }
	
	/**
	 * Is network connected
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsNetworkConnected() const { return NetworkInfo.bIsConnected; }
	
	/**
	 * Is on WiFi
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsOnWiFi() const { return NetworkInfo.NetworkType == EDeskillzNetworkType::WiFi; }
	
	/**
	 * Is on cellular
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsOnCellular() const;
	
	/**
	 * Refresh network info
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void RefreshNetworkInfo();
	
	// ========================================================================
	// Battery
	// ========================================================================
	
	/**
	 * Get battery info
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	FDeskillzBatteryInfo GetBatteryInfo() const { return BatteryInfo; }
	
	/**
	 * Get battery level (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	int32 GetBatteryLevel() const { return BatteryInfo.Level; }
	
	/**
	 * Is battery charging
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Platform")
	bool IsBatteryCharging() const { return BatteryInfo.bIsCharging; }
	
	/**
	 * Refresh battery info
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void RefreshBatteryInfo();
	
	// ========================================================================
	// Platform Features
	// ========================================================================
	
	/**
	 * Open URL in external browser
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void OpenURL(const FString& URL);
	
	/**
	 * Open app store page
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void OpenAppStore();
	
	/**
	 * Share text
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void ShareText(const FString& Text, const FString& Title = TEXT(""));
	
	/**
	 * Copy to clipboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void CopyToClipboard(const FString& Text);
	
	/**
	 * Get from clipboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	FString GetFromClipboard();
	
	/**
	 * Trigger haptic feedback (mobile)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void TriggerHaptic(float Intensity = 0.5f, float Duration = 0.1f);
	
	/**
	 * Set screen brightness (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void SetScreenBrightness(float Brightness);
	
	/**
	 * Keep screen on
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Platform")
	void SetKeepScreenOn(bool bKeepOn);
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when network status changes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Platform")
	FOnNetworkStatusChanged OnNetworkStatusChanged;
	
	/** Called when battery is low */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Platform")
	FOnBatteryLow OnBatteryLow;
	
	/** Called on memory warning */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Platform")
	FOnMemoryWarning OnMemoryWarning;
	
protected:
	/** Device info */
	UPROPERTY()
	FDeskillzDeviceInfo DeviceInfo;
	
	/** Network info */
	UPROPERTY()
	FDeskillzNetworkInfo NetworkInfo;
	
	/** Battery info */
	UPROPERTY()
	FDeskillzBatteryInfo BatteryInfo;
	
	/** Is initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** Previous network status */
	bool bPreviousNetworkConnected = true;
	
	/** Low battery threshold */
	int32 LowBatteryThreshold = 20;
	
	/** Monitor timer */
	FTimerHandle MonitorTimerHandle;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Detect platform type */
	EDeskillzPlatformType DetectPlatform() const;
	
	/** Detect device tier */
	EDeskillzDeviceTier DetectDeviceTier() const;
	
	/** Collect device info */
	void CollectDeviceInfo();
	
	/** Monitor status updates */
	void MonitorStatus();
	
	/** Start monitoring */
	void StartMonitoring();
	
	/** Stop monitoring */
	void StopMonitoring();
};
