// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DeskillzTypes.h"
#include "DeskillzConfig.generated.h"

/**
 * UI Theme configuration
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzUITheme
{
	GENERATED_BODY()
	
	/** Primary brand color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FLinearColor PrimaryColor = FLinearColor(0.0f, 0.8f, 0.4f, 1.0f); // Green
	
	/** Secondary accent color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FLinearColor SecondaryColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f); // Blue
	
	/** Background color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FLinearColor BackgroundColor = FLinearColor(0.05f, 0.05f, 0.1f, 1.0f); // Dark
	
	/** Text color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FLinearColor TextColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
	
	/** Error/danger color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FLinearColor ErrorColor = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f); // Red
	
	/** Success color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FLinearColor SuccessColor = FLinearColor(0.3f, 1.0f, 0.3f, 1.0f); // Green
	
	/** Warning color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FLinearColor WarningColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Yellow
	
	/** Corner radius for UI elements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme", meta = (ClampMin = "0", ClampMax = "32"))
	float CornerRadius = 8.0f;
	
	/** Animation duration multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float AnimationSpeed = 1.0f;
	
	FDeskillzUITheme() = default;
};

/**
 * API endpoint configuration
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzEndpoints
{
	GENERATED_BODY()
	
	/** Base API URL (auto-set based on environment) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endpoints")
	FString BaseUrl = TEXT("https://api.deskillz.games/v1");
	
	/** WebSocket URL for real-time features */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endpoints")
	FString WebSocketUrl = TEXT("wss://ws.deskillz.games");
	
	/** CDN URL for assets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endpoints")
	FString CdnUrl = TEXT("https://cdn.deskillz.games");
	
	FDeskillzEndpoints() = default;
	
	/** Get endpoints for specific environment */
	static FDeskillzEndpoints ForEnvironment(EDeskillzEnvironment Environment)
	{
		FDeskillzEndpoints Endpoints;
		
		switch (Environment)
		{
			case EDeskillzEnvironment::Production:
				Endpoints.BaseUrl = TEXT("https://api.deskillz.games/v1");
				Endpoints.WebSocketUrl = TEXT("wss://ws.deskillz.games");
				Endpoints.CdnUrl = TEXT("https://cdn.deskillz.games");
				break;
				
			case EDeskillzEnvironment::Sandbox:
				Endpoints.BaseUrl = TEXT("https://sandbox-api.deskillz.games/v1");
				Endpoints.WebSocketUrl = TEXT("wss://sandbox-ws.deskillz.games");
				Endpoints.CdnUrl = TEXT("https://sandbox-cdn.deskillz.games");
				break;
				
			case EDeskillzEnvironment::Development:
				Endpoints.BaseUrl = TEXT("http://localhost:3001/api/v1");
				Endpoints.WebSocketUrl = TEXT("ws://localhost:3001");
				Endpoints.CdnUrl = TEXT("http://localhost:3001/cdn");
				break;
		}
		
		return Endpoints;
	}
};

/**
 * Deskillz SDK Configuration
 * 
 * Project settings for the Deskillz competitive gaming SDK.
 * Access via Project Settings > Plugins > Deskillz SDK
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Deskillz SDK"))
class DESKILLZ_API UDeskillzConfig : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UDeskillzConfig();
	
	// ========================================================================
	// Core Settings
	// ========================================================================
	
	/**
	 * Your Deskillz API Key (from developer portal)
	 * Required for SDK initialization
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core",
		meta = (DisplayName = "API Key"))
	FString APIKey;
	
	/**
	 * Your Game ID (from developer portal)
	 * Unique identifier for your game on the platform
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core",
		meta = (DisplayName = "Game ID"))
	FString GameId;
	
	/**
	 * SDK Environment
	 * Use Sandbox for testing, Production for live
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core",
		meta = (DisplayName = "Environment"))
	EDeskillzEnvironment Environment = EDeskillzEnvironment::Sandbox;
	
	/**
	 * Enable SDK (can disable for builds without Deskillz)
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core",
		meta = (DisplayName = "Enable SDK"))
	bool bEnableSDK = true;
	
	// ========================================================================
	// Match Settings
	// ========================================================================
	
	/**
	 * Default match type for your game
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Match",
		meta = (DisplayName = "Default Match Type"))
	EDeskillzMatchType DefaultMatchType = EDeskillzMatchType::Asynchronous;
	
	/**
	 * Default match duration in seconds
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Match",
		meta = (DisplayName = "Default Match Duration", ClampMin = "30", ClampMax = "3600"))
	int32 DefaultMatchDuration = 180;
	
	/**
	 * Allow practice mode (free play without entry fee)
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Match",
		meta = (DisplayName = "Enable Practice Mode"))
	bool bEnablePracticeMode = true;
	
	/**
	 * Auto-submit scores when match ends
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Match",
		meta = (DisplayName = "Auto Submit Scores"))
	bool bAutoSubmitScores = true;
	
	// ========================================================================
	// Security Settings
	// ========================================================================
	
	/**
	 * Enable anti-cheat detection
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Security",
		meta = (DisplayName = "Enable Anti-Cheat"))
	bool bEnableAntiCheat = true;
	
	/**
	 * Enable score encryption
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Security",
		meta = (DisplayName = "Enable Score Encryption"))
	bool bEnableScoreEncryption = true;
	
	/**
	 * Enable memory protection (prevents memory editing)
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Security",
		meta = (DisplayName = "Enable Memory Protection"))
	bool bEnableMemoryProtection = true;
	
	/**
	 * Minimum score (validates score submissions)
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Security",
		meta = (DisplayName = "Minimum Score"))
	int64 MinScore = 0;
	
	/**
	 * Maximum score (validates score submissions)
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Security",
		meta = (DisplayName = "Maximum Score"))
	int64 MaxScore = 999999999;
	
	// ========================================================================
	// UI Settings
	// ========================================================================
	
	/**
	 * Use Deskillz built-in UI widgets
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI",
		meta = (DisplayName = "Use Built-in UI"))
	bool bUseBuiltInUI = true;
	
	/**
	 * Show player avatars in UI
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI",
		meta = (DisplayName = "Show Avatars"))
	bool bShowAvatars = true;
	
	/**
	 * Show player ratings in UI
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI",
		meta = (DisplayName = "Show Ratings"))
	bool bShowRatings = true;
	
	/**
	 * UI theme customization
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI",
		meta = (DisplayName = "UI Theme"))
	FDeskillzUITheme UITheme;
	
	// ========================================================================
	// Network Settings
	// ========================================================================
	
	/**
	 * API request timeout in seconds
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network",
		meta = (DisplayName = "Request Timeout", ClampMin = "5", ClampMax = "60"))
	float RequestTimeout = 30.0f;
	
	/**
	 * Number of retry attempts for failed requests
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network",
		meta = (DisplayName = "Retry Attempts", ClampMin = "0", ClampMax = "5"))
	int32 RetryAttempts = 3;
	
	/**
	 * Enable WebSocket for real-time features
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network",
		meta = (DisplayName = "Enable WebSocket"))
	bool bEnableWebSocket = true;
	
	/**
	 * WebSocket heartbeat interval in seconds
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network",
		meta = (DisplayName = "WebSocket Heartbeat", ClampMin = "5", ClampMax = "60"))
	float WebSocketHeartbeat = 15.0f;
	
	/**
	 * Custom API endpoints (overrides environment defaults)
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network|Advanced",
		meta = (DisplayName = "Custom Endpoints"))
	FDeskillzEndpoints CustomEndpoints;
	
	/**
	 * Use custom endpoints instead of environment defaults
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Network|Advanced",
		meta = (DisplayName = "Use Custom Endpoints"))
	bool bUseCustomEndpoints = false;
	
	// ========================================================================
	// Analytics Settings
	// ========================================================================
	
	/**
	 * Enable analytics collection
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Analytics",
		meta = (DisplayName = "Enable Analytics"))
	bool bEnableAnalytics = true;
	
	/**
	 * Enable crash reporting
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Analytics",
		meta = (DisplayName = "Enable Crash Reporting"))
	bool bEnableCrashReporting = true;
	
	/**
	 * Enable performance telemetry
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Analytics",
		meta = (DisplayName = "Enable Telemetry"))
	bool bEnableTelemetry = true;
	
	/**
	 * Analytics batch size before sending
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Analytics",
		meta = (DisplayName = "Analytics Batch Size", ClampMin = "1", ClampMax = "100"))
	int32 AnalyticsBatchSize = 20;
	
	// ========================================================================
	// Debug Settings
	// ========================================================================
	
	/**
	 * Enable verbose logging
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Debug",
		meta = (DisplayName = "Verbose Logging"))
	bool bVerboseLogging = false;
	
	/**
	 * Log API requests/responses
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Debug",
		meta = (DisplayName = "Log API Calls"))
	bool bLogAPICalls = false;
	
	/**
	 * Simulate network latency (ms) - debug only
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Debug",
		meta = (DisplayName = "Simulated Latency", ClampMin = "0", ClampMax = "5000"))
	int32 SimulatedLatencyMs = 0;
	
	// ========================================================================
	// Accessors
	// ========================================================================
	
	/** Get the singleton config instance */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Config", meta = (DisplayName = "Get Deskillz Config"))
	static UDeskillzConfig* Get();
	
	/** Get the active API endpoints based on environment */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Config")
	FDeskillzEndpoints GetActiveEndpoints() const;
	
	/** Check if configuration is valid for initialization */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Config")
	bool IsValid() const;
	
	/** Get validation error message if config is invalid */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Config")
	FString GetValidationError() const;
	
	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
