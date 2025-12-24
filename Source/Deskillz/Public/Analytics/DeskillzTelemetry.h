// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzTelemetry.generated.h"

/**
 * Performance metric type
 */
UENUM(BlueprintType)
enum class EDeskillzMetricType : uint8
{
	/** Frame rate */
	FPS,
	
	/** Frame time */
	FrameTime,
	
	/** Memory usage */
	Memory,
	
	/** Network latency */
	Latency,
	
	/** CPU usage */
	CPU,
	
	/** GPU usage */
	GPU,
	
	/** Custom metric */
	Custom
};

/**
 * Performance sample
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzPerformanceSample
{
	GENERATED_BODY()
	
	/** Metric type */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	EDeskillzMetricType Type = EDeskillzMetricType::Custom;
	
	/** Metric name */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	FString Name;
	
	/** Sample value */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Value = 0.0f;
	
	/** Timestamp */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	int64 Timestamp = 0;
	
	/** Context (match ID, etc) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	FString Context;
};

/**
 * Aggregated performance stats
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzPerformanceStats
{
	GENERATED_BODY()
	
	/** Metric name */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	FString Name;
	
	/** Minimum value */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Min = 0.0f;
	
	/** Maximum value */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Max = 0.0f;
	
	/** Average value */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Average = 0.0f;
	
	/** Median value */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Median = 0.0f;
	
	/** 95th percentile */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float P95 = 0.0f;
	
	/** 99th percentile */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float P99 = 0.0f;
	
	/** Sample count */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	int32 SampleCount = 0;
	
	/** Standard deviation */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float StdDev = 0.0f;
};

/**
 * Network quality metrics
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzNetworkMetrics
{
	GENERATED_BODY()
	
	/** Round-trip latency (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Latency = 0.0f;
	
	/** Packet loss percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float PacketLoss = 0.0f;
	
	/** Jitter (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Jitter = 0.0f;
	
	/** Bandwidth (KB/s) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float Bandwidth = 0.0f;
	
	/** Connection quality (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	int32 Quality = 100;
	
	/** Bytes sent */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	int64 BytesSent = 0;
	
	/** Bytes received */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	int64 BytesReceived = 0;
};

/**
 * Telemetry configuration
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzTelemetryConfig
{
	GENERATED_BODY()
	
	/** Enable telemetry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	bool bEnabled = true;
	
	/** Sample interval (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float SampleInterval = 1.0f;
	
	/** Max samples to keep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	int32 MaxSamples = 1000;
	
	/** Report interval (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float ReportInterval = 60.0f;
	
	/** Track FPS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	bool bTrackFPS = true;
	
	/** Track memory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	bool bTrackMemory = true;
	
	/** Track network */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	bool bTrackNetwork = true;
	
	/** Low FPS threshold for warnings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float LowFPSThreshold = 30.0f;
	
	/** High latency threshold for warnings (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float HighLatencyThreshold = 150.0f;
};

/** Telemetry delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, const FString&, Warning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkQualityChanged, int32, Quality);

/**
 * Deskillz Telemetry System
 * 
 * Real-time performance and network monitoring:
 * - FPS and frame time tracking
 * - Memory usage monitoring
 * - Network latency and quality
 * - Custom metric support
 * - Statistical aggregation
 * 
 * Features:
 * - Automatic sampling
 * - Performance warnings
 * - Match-specific metrics
 * - Report generation
 * 
 * Usage:
 *   UDeskillzTelemetry* Telemetry = UDeskillzTelemetry::Get();
 *   Telemetry->StartMonitoring();
 *   
 *   // During gameplay
 *   Telemetry->RecordMetric("input_lag", 16.5f);
 *   
 *   // Get stats
 *   FDeskillzPerformanceStats FpsStats = Telemetry->GetStats("fps");
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzTelemetry : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UDeskillzTelemetry();
	~UDeskillzTelemetry();
	
	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bIsMonitoring; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UDeskillzTelemetry, STATGROUP_Tickables); }
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Telemetry instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Telemetry", meta = (DisplayName = "Get Deskillz Telemetry"))
	static UDeskillzTelemetry* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize telemetry
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void Initialize(const FDeskillzTelemetryConfig& Config);
	
	/**
	 * Initialize with defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void InitializeDefault();
	
	// ========================================================================
	// Monitoring Control
	// ========================================================================
	
	/**
	 * Start monitoring
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void StartMonitoring();
	
	/**
	 * Stop monitoring
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void StopMonitoring();
	
	/**
	 * Is currently monitoring
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Telemetry")
	bool IsMonitoring() const { return bIsMonitoring; }
	
	/**
	 * Set match context (for match-specific metrics)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void SetMatchContext(const FString& MatchId);
	
	/**
	 * Clear match context
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void ClearMatchContext();
	
	// ========================================================================
	// Metric Recording
	// ========================================================================
	
	/**
	 * Record custom metric
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void RecordMetric(const FString& Name, float Value);
	
	/**
	 * Record metric with type
	 */
	void RecordMetric(EDeskillzMetricType Type, const FString& Name, float Value);
	
	/**
	 * Record network latency
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void RecordLatency(float LatencyMs);
	
	/**
	 * Update network metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void UpdateNetworkMetrics(const FDeskillzNetworkMetrics& Metrics);
	
	// ========================================================================
	// Statistics
	// ========================================================================
	
	/**
	 * Get current FPS
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Telemetry")
	float GetCurrentFPS() const { return CurrentFPS; }
	
	/**
	 * Get current latency
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Telemetry")
	float GetCurrentLatency() const { return NetworkMetrics.Latency; }
	
	/**
	 * Get network metrics
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Telemetry")
	FDeskillzNetworkMetrics GetNetworkMetrics() const { return NetworkMetrics; }
	
	/**
	 * Get stats for metric
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	FDeskillzPerformanceStats GetStats(const FString& MetricName) const;
	
	/**
	 * Get all stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	TMap<FString, FDeskillzPerformanceStats> GetAllStats() const;
	
	/**
	 * Get memory usage (MB)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Telemetry")
	float GetMemoryUsageMB() const;
	
	/**
	 * Get network quality (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Telemetry")
	int32 GetNetworkQuality() const { return NetworkMetrics.Quality; }
	
	// ========================================================================
	// Reporting
	// ========================================================================
	
	/**
	 * Generate performance report
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void GenerateReport();
	
	/**
	 * Clear all samples
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Telemetry")
	void ClearSamples();
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called on performance warning */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Telemetry")
	FOnPerformanceWarning OnPerformanceWarning;
	
	/** Called when network quality changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Telemetry")
	FOnNetworkQualityChanged OnNetworkQualityChanged;
	
protected:
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Telemetry configuration */
	UPROPERTY()
	FDeskillzTelemetryConfig TelemetryConfig;
	
	/** Is monitoring active */
	UPROPERTY()
	bool bIsMonitoring = false;
	
	/** Current match context */
	UPROPERTY()
	FString CurrentMatchId;
	
	// ========================================================================
	// Metrics
	// ========================================================================
	
	/** Current FPS */
	UPROPERTY()
	float CurrentFPS = 0.0f;
	
	/** Current frame time */
	UPROPERTY()
	float CurrentFrameTime = 0.0f;
	
	/** Network metrics */
	UPROPERTY()
	FDeskillzNetworkMetrics NetworkMetrics;
	
	/** Previous network quality (for change detection) */
	UPROPERTY()
	int32 PreviousNetworkQuality = 100;
	
	// ========================================================================
	// Samples
	// ========================================================================
	
	/** Performance samples by metric name */
	TMap<FString, TArray<FDeskillzPerformanceSample>> Samples;
	
	/** Frame time history for FPS calculation */
	TArray<float> FrameTimeHistory;
	
	/** Latency history */
	TArray<float> LatencyHistory;
	
	// ========================================================================
	// Timing
	// ========================================================================
	
	/** Time since last sample */
	float TimeSinceLastSample = 0.0f;
	
	/** Time since last report */
	float TimeSinceLastReport = 0.0f;
	
	/** Report timer handle */
	FTimerHandle ReportTimerHandle;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Sample current metrics */
	void SampleMetrics();
	
	/** Sample FPS */
	void SampleFPS(float DeltaTime);
	
	/** Sample memory */
	void SampleMemory();
	
	/** Check for warnings */
	void CheckWarnings();
	
	/** Calculate stats from samples */
	FDeskillzPerformanceStats CalculateStats(const TArray<FDeskillzPerformanceSample>& SampleArray) const;
	
	/** Calculate percentile */
	float CalculatePercentile(TArray<float>& Values, float Percentile) const;
	
	/** Update network quality */
	void UpdateNetworkQuality();
	
	/** Add sample */
	void AddSample(const FString& Name, const FDeskillzPerformanceSample& Sample);
	
	/** Send report to server */
	void SendReport(const TMap<FString, FDeskillzPerformanceStats>& Stats);
};
