// Copyright Deskillz Games. All Rights Reserved.

#include "Analytics/DeskillzTelemetry.h"
#include "Analytics/DeskillzAnalytics.h"
#include "Network/DeskillzHttpClient.h"
#include "Deskillz.h"
#include "HAL/PlatformMemory.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Static singleton
static UDeskillzTelemetry* GTelemetry = nullptr;

UDeskillzTelemetry::UDeskillzTelemetry()
{
	FrameTimeHistory.Reserve(120); // 2 seconds at 60 FPS
	LatencyHistory.Reserve(60);
}

UDeskillzTelemetry::~UDeskillzTelemetry()
{
	StopMonitoring();
}

UDeskillzTelemetry* UDeskillzTelemetry::Get()
{
	if (!GTelemetry)
	{
		GTelemetry = NewObject<UDeskillzTelemetry>();
		GTelemetry->AddToRoot();
	}
	return GTelemetry;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzTelemetry::Initialize(const FDeskillzTelemetryConfig& Config)
{
	TelemetryConfig = Config;
	
	UE_LOG(LogDeskillz, Log, TEXT("Telemetry initialized - Sample interval: %.1fs"), Config.SampleInterval);
}

void UDeskillzTelemetry::InitializeDefault()
{
	FDeskillzTelemetryConfig DefaultConfig;
	Initialize(DefaultConfig);
}

// ============================================================================
// Monitoring Control
// ============================================================================

void UDeskillzTelemetry::StartMonitoring()
{
	if (!TelemetryConfig.bEnabled)
	{
		return;
	}
	
	bIsMonitoring = true;
	TimeSinceLastSample = 0.0f;
	TimeSinceLastReport = 0.0f;
	
	UE_LOG(LogDeskillz, Log, TEXT("Telemetry monitoring started"));
}

void UDeskillzTelemetry::StopMonitoring()
{
	if (!bIsMonitoring)
	{
		return;
	}
	
	// Generate final report
	GenerateReport();
	
	bIsMonitoring = false;
	
	UE_LOG(LogDeskillz, Log, TEXT("Telemetry monitoring stopped"));
}

void UDeskillzTelemetry::SetMatchContext(const FString& MatchId)
{
	CurrentMatchId = MatchId;
	
	// Clear samples for fresh match metrics
	ClearSamples();
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Telemetry match context: %s"), *MatchId);
}

void UDeskillzTelemetry::ClearMatchContext()
{
	// Generate report for the match before clearing
	if (!CurrentMatchId.IsEmpty())
	{
		GenerateReport();
	}
	
	CurrentMatchId.Empty();
}

// ============================================================================
// FTickableGameObject
// ============================================================================

void UDeskillzTelemetry::Tick(float DeltaTime)
{
	if (!bIsMonitoring)
	{
		return;
	}
	
	// Sample FPS every frame
	SampleFPS(DeltaTime);
	
	// Periodic sampling
	TimeSinceLastSample += DeltaTime;
	if (TimeSinceLastSample >= TelemetryConfig.SampleInterval)
	{
		SampleMetrics();
		TimeSinceLastSample = 0.0f;
	}
	
	// Periodic reporting
	TimeSinceLastReport += DeltaTime;
	if (TimeSinceLastReport >= TelemetryConfig.ReportInterval)
	{
		GenerateReport();
		TimeSinceLastReport = 0.0f;
	}
	
	// Check for warnings
	CheckWarnings();
}

// ============================================================================
// Metric Recording
// ============================================================================

void UDeskillzTelemetry::RecordMetric(const FString& Name, float Value)
{
	RecordMetric(EDeskillzMetricType::Custom, Name, Value);
}

void UDeskillzTelemetry::RecordMetric(EDeskillzMetricType Type, const FString& Name, float Value)
{
	FDeskillzPerformanceSample Sample;
	Sample.Type = Type;
	Sample.Name = Name;
	Sample.Value = Value;
	Sample.Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	Sample.Context = CurrentMatchId;
	
	AddSample(Name, Sample);
}

void UDeskillzTelemetry::RecordLatency(float LatencyMs)
{
	NetworkMetrics.Latency = LatencyMs;
	
	// Add to history for jitter calculation
	LatencyHistory.Add(LatencyMs);
	if (LatencyHistory.Num() > 60)
	{
		LatencyHistory.RemoveAt(0);
	}
	
	// Calculate jitter (variation in latency)
	if (LatencyHistory.Num() > 1)
	{
		float SumDiff = 0.0f;
		for (int32 i = 1; i < LatencyHistory.Num(); i++)
		{
			SumDiff += FMath::Abs(LatencyHistory[i] - LatencyHistory[i - 1]);
		}
		NetworkMetrics.Jitter = SumDiff / (LatencyHistory.Num() - 1);
	}
	
	RecordMetric(EDeskillzMetricType::Latency, TEXT("latency"), LatencyMs);
	
	UpdateNetworkQuality();
}

void UDeskillzTelemetry::UpdateNetworkMetrics(const FDeskillzNetworkMetrics& Metrics)
{
	NetworkMetrics = Metrics;
	UpdateNetworkQuality();
}

// ============================================================================
// Statistics
// ============================================================================

FDeskillzPerformanceStats UDeskillzTelemetry::GetStats(const FString& MetricName) const
{
	if (const TArray<FDeskillzPerformanceSample>* SampleArray = Samples.Find(MetricName))
	{
		return CalculateStats(*SampleArray);
	}
	
	return FDeskillzPerformanceStats();
}

TMap<FString, FDeskillzPerformanceStats> UDeskillzTelemetry::GetAllStats() const
{
	TMap<FString, FDeskillzPerformanceStats> AllStats;
	
	for (const auto& Pair : Samples)
	{
		AllStats.Add(Pair.Key, CalculateStats(Pair.Value));
	}
	
	return AllStats;
}

float UDeskillzTelemetry::GetMemoryUsageMB() const
{
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

// ============================================================================
// Reporting
// ============================================================================

void UDeskillzTelemetry::GenerateReport()
{
	TMap<FString, FDeskillzPerformanceStats> AllStats = GetAllStats();
	
	if (AllStats.Num() == 0)
	{
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("Telemetry Report - %d metrics"), AllStats.Num());
	
	for (const auto& Pair : AllStats)
	{
		UE_LOG(LogDeskillz, Verbose, TEXT("  %s: avg=%.2f min=%.2f max=%.2f p95=%.2f"),
			*Pair.Key, Pair.Value.Average, Pair.Value.Min, Pair.Value.Max, Pair.Value.P95);
	}
	
	// Send to server
	SendReport(AllStats);
	
	// Track in analytics
	UDeskillzAnalytics* Analytics = UDeskillzAnalytics::Get();
	if (Analytics->IsEnabled())
	{
		FDeskillzAnalyticsEvent Event(TEXT("performance_report"), EDeskillzEventCategory::System);
		
		if (const FDeskillzPerformanceStats* FpsStats = AllStats.Find(TEXT("fps")))
		{
			Event.AddValue(TEXT("fps_avg"), FpsStats->Average);
			Event.AddValue(TEXT("fps_min"), FpsStats->Min);
			Event.AddValue(TEXT("fps_p95"), FpsStats->P95);
		}
		
		if (const FDeskillzPerformanceStats* LatencyStats = AllStats.Find(TEXT("latency")))
		{
			Event.AddValue(TEXT("latency_avg"), LatencyStats->Average);
			Event.AddValue(TEXT("latency_max"), LatencyStats->Max);
		}
		
		Event.AddValue(TEXT("memory_mb"), GetMemoryUsageMB());
		Event.AddValue(TEXT("network_quality"), static_cast<double>(NetworkMetrics.Quality));
		
		if (!CurrentMatchId.IsEmpty())
		{
			Event.AddParam(TEXT("match_id"), CurrentMatchId);
		}
		
		Analytics->TrackEvent(Event);
	}
}

void UDeskillzTelemetry::ClearSamples()
{
	Samples.Empty();
	FrameTimeHistory.Empty();
	LatencyHistory.Empty();
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzTelemetry::SampleMetrics()
{
	// Sample FPS
	if (TelemetryConfig.bTrackFPS)
	{
		RecordMetric(EDeskillzMetricType::FPS, TEXT("fps"), CurrentFPS);
		RecordMetric(EDeskillzMetricType::FrameTime, TEXT("frame_time"), CurrentFrameTime);
	}
	
	// Sample memory
	if (TelemetryConfig.bTrackMemory)
	{
		SampleMemory();
	}
}

void UDeskillzTelemetry::SampleFPS(float DeltaTime)
{
	// Track frame times
	FrameTimeHistory.Add(DeltaTime);
	if (FrameTimeHistory.Num() > 120)
	{
		FrameTimeHistory.RemoveAt(0);
	}
	
	// Calculate average FPS from recent frames
	if (FrameTimeHistory.Num() > 0)
	{
		float TotalTime = 0.0f;
		for (float FrameTime : FrameTimeHistory)
		{
			TotalTime += FrameTime;
		}
		
		float AverageFrameTime = TotalTime / FrameTimeHistory.Num();
		CurrentFrameTime = AverageFrameTime * 1000.0f; // Convert to ms
		CurrentFPS = AverageFrameTime > 0.0f ? 1.0f / AverageFrameTime : 0.0f;
	}
}

void UDeskillzTelemetry::SampleMemory()
{
	float MemoryMB = GetMemoryUsageMB();
	RecordMetric(EDeskillzMetricType::Memory, TEXT("memory_mb"), MemoryMB);
}

void UDeskillzTelemetry::CheckWarnings()
{
	// Check FPS
	if (TelemetryConfig.bTrackFPS && CurrentFPS > 0.0f && CurrentFPS < TelemetryConfig.LowFPSThreshold)
	{
		FString Warning = FString::Printf(TEXT("Low FPS: %.1f (threshold: %.1f)"), 
			CurrentFPS, TelemetryConfig.LowFPSThreshold);
		OnPerformanceWarning.Broadcast(Warning);
	}
	
	// Check latency
	if (TelemetryConfig.bTrackNetwork && NetworkMetrics.Latency > TelemetryConfig.HighLatencyThreshold)
	{
		FString Warning = FString::Printf(TEXT("High latency: %.1fms (threshold: %.1fms)"), 
			NetworkMetrics.Latency, TelemetryConfig.HighLatencyThreshold);
		OnPerformanceWarning.Broadcast(Warning);
	}
}

FDeskillzPerformanceStats UDeskillzTelemetry::CalculateStats(const TArray<FDeskillzPerformanceSample>& SampleArray) const
{
	FDeskillzPerformanceStats Stats;
	
	if (SampleArray.Num() == 0)
	{
		return Stats;
	}
	
	Stats.Name = SampleArray[0].Name;
	Stats.SampleCount = SampleArray.Num();
	
	// Extract values
	TArray<float> Values;
	Values.Reserve(SampleArray.Num());
	
	float Sum = 0.0f;
	Stats.Min = SampleArray[0].Value;
	Stats.Max = SampleArray[0].Value;
	
	for (const FDeskillzPerformanceSample& Sample : SampleArray)
	{
		Values.Add(Sample.Value);
		Sum += Sample.Value;
		Stats.Min = FMath::Min(Stats.Min, Sample.Value);
		Stats.Max = FMath::Max(Stats.Max, Sample.Value);
	}
	
	// Average
	Stats.Average = Sum / Values.Num();
	
	// Sort for percentiles
	Values.Sort();
	
	// Median
	int32 MidIndex = Values.Num() / 2;
	if (Values.Num() % 2 == 0)
	{
		Stats.Median = (Values[MidIndex - 1] + Values[MidIndex]) / 2.0f;
	}
	else
	{
		Stats.Median = Values[MidIndex];
	}
	
	// Percentiles
	Stats.P95 = CalculatePercentile(Values, 0.95f);
	Stats.P99 = CalculatePercentile(Values, 0.99f);
	
	// Standard deviation
	float SumSquaredDiff = 0.0f;
	for (float Value : Values)
	{
		float Diff = Value - Stats.Average;
		SumSquaredDiff += Diff * Diff;
	}
	Stats.StdDev = FMath::Sqrt(SumSquaredDiff / Values.Num());
	
	return Stats;
}

float UDeskillzTelemetry::CalculatePercentile(TArray<float>& Values, float Percentile) const
{
	if (Values.Num() == 0)
	{
		return 0.0f;
	}
	
	// Values should already be sorted
	float Index = Percentile * (Values.Num() - 1);
	int32 LowerIndex = FMath::FloorToInt(Index);
	int32 UpperIndex = FMath::CeilToInt(Index);
	
	if (LowerIndex == UpperIndex)
	{
		return Values[LowerIndex];
	}
	
	float Fraction = Index - LowerIndex;
	return Values[LowerIndex] * (1.0f - Fraction) + Values[UpperIndex] * Fraction;
}

void UDeskillzTelemetry::UpdateNetworkQuality()
{
	// Calculate quality score (0-100) based on latency, jitter, and packet loss
	int32 Quality = 100;
	
	// Latency penalty (each 50ms above 50ms reduces quality by 10)
	if (NetworkMetrics.Latency > 50.0f)
	{
		int32 LatencyPenalty = static_cast<int32>((NetworkMetrics.Latency - 50.0f) / 5.0f);
		Quality -= FMath::Min(LatencyPenalty, 40);
	}
	
	// Jitter penalty
	if (NetworkMetrics.Jitter > 10.0f)
	{
		int32 JitterPenalty = static_cast<int32>((NetworkMetrics.Jitter - 10.0f) / 2.0f);
		Quality -= FMath::Min(JitterPenalty, 20);
	}
	
	// Packet loss penalty
	Quality -= static_cast<int32>(NetworkMetrics.PacketLoss * 4.0f);
	
	Quality = FMath::Clamp(Quality, 0, 100);
	NetworkMetrics.Quality = Quality;
	
	// Broadcast if quality changed significantly (>10 points)
	if (FMath::Abs(Quality - PreviousNetworkQuality) > 10)
	{
		OnNetworkQualityChanged.Broadcast(Quality);
		PreviousNetworkQuality = Quality;
	}
}

void UDeskillzTelemetry::AddSample(const FString& Name, const FDeskillzPerformanceSample& Sample)
{
	TArray<FDeskillzPerformanceSample>& SampleArray = Samples.FindOrAdd(Name);
	
	// Enforce max samples
	if (SampleArray.Num() >= TelemetryConfig.MaxSamples)
	{
		SampleArray.RemoveAt(0);
	}
	
	SampleArray.Add(Sample);
}

void UDeskillzTelemetry::SendReport(const TMap<FString, FDeskillzPerformanceStats>& Stats)
{
	// Build JSON payload
	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	
	Payload->SetNumberField(TEXT("timestamp"), FDateTime::UtcNow().ToUnixTimestamp() * 1000);
	
	if (!CurrentMatchId.IsEmpty())
	{
		Payload->SetStringField(TEXT("match_id"), CurrentMatchId);
	}
	
	// Add metrics
	TSharedPtr<FJsonObject> MetricsObj = MakeShareable(new FJsonObject());
	for (const auto& Pair : Stats)
	{
		TSharedPtr<FJsonObject> StatObj = MakeShareable(new FJsonObject());
		StatObj->SetNumberField(TEXT("min"), Pair.Value.Min);
		StatObj->SetNumberField(TEXT("max"), Pair.Value.Max);
		StatObj->SetNumberField(TEXT("avg"), Pair.Value.Average);
		StatObj->SetNumberField(TEXT("median"), Pair.Value.Median);
		StatObj->SetNumberField(TEXT("p95"), Pair.Value.P95);
		StatObj->SetNumberField(TEXT("p99"), Pair.Value.P99);
		StatObj->SetNumberField(TEXT("stddev"), Pair.Value.StdDev);
		StatObj->SetNumberField(TEXT("samples"), Pair.Value.SampleCount);
		
		MetricsObj->SetObjectField(Pair.Key, StatObj);
	}
	Payload->SetObjectField(TEXT("metrics"), MetricsObj);
	
	// Add network info
	TSharedPtr<FJsonObject> NetworkObj = MakeShareable(new FJsonObject());
	NetworkObj->SetNumberField(TEXT("latency"), NetworkMetrics.Latency);
	NetworkObj->SetNumberField(TEXT("jitter"), NetworkMetrics.Jitter);
	NetworkObj->SetNumberField(TEXT("packet_loss"), NetworkMetrics.PacketLoss);
	NetworkObj->SetNumberField(TEXT("quality"), NetworkMetrics.Quality);
	NetworkObj->SetNumberField(TEXT("bytes_sent"), static_cast<double>(NetworkMetrics.BytesSent));
	NetworkObj->SetNumberField(TEXT("bytes_received"), static_cast<double>(NetworkMetrics.BytesReceived));
	Payload->SetObjectField(TEXT("network"), NetworkObj);
	
	// Send to server
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->PostJson(TEXT("/api/v1/telemetry/report"), Payload,
		FOnDeskillzHttpResponse::CreateLambda([](const FDeskillzHttpResponse& Response)
		{
			if (!Response.IsOk())
			{
				UE_LOG(LogDeskillz, Warning, TEXT("Failed to send telemetry report"));
			}
		})
	);
}
