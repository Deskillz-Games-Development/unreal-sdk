// Copyright Deskillz Games. All Rights Reserved.

#include "Analytics/DeskillzAnalytics.h"
#include "Network/DeskillzHttpClient.h"
#include "Deskillz.h"
#include "Misc/Guid.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Static singleton
static UDeskillzAnalytics* GAnalytics = nullptr;

UDeskillzAnalytics::UDeskillzAnalytics()
{
}

UDeskillzAnalytics::~UDeskillzAnalytics()
{
	Shutdown();
}

UDeskillzAnalytics* UDeskillzAnalytics::Get()
{
	if (!GAnalytics)
	{
		GAnalytics = NewObject<UDeskillzAnalytics>();
		GAnalytics->AddToRoot();
	}
	return GAnalytics;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzAnalytics::Initialize(const FDeskillzAnalyticsConfig& Config)
{
	if (bIsInitialized)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Analytics already initialized"));
		return;
	}
	
	AnalyticsConfig = Config;
	
	// Generate session ID
	SessionId = GenerateSessionId();
	SessionStartTime = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	
	// Load persisted events
	if (AnalyticsConfig.bPersistOffline)
	{
		LoadPersistedQueue();
	}
	
	// Start flush timer
	StartFlushTimer();
	
	bIsInitialized = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("Analytics initialized - Session: %s"), *SessionId);
	
	// Track session start if auto events enabled
	if (AnalyticsConfig.bTrackAutoEvents)
	{
		TrackSessionStart();
	}
}

void UDeskillzAnalytics::InitializeDefault()
{
	FDeskillzAnalyticsConfig DefaultConfig;
	Initialize(DefaultConfig);
}

void UDeskillzAnalytics::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	// Track session end
	if (AnalyticsConfig.bTrackAutoEvents)
	{
		TrackSessionEnd();
	}
	
	// Flush remaining events
	Flush();
	
	// Persist any remaining events
	if (AnalyticsConfig.bPersistOffline && EventQueue.Num() > 0)
	{
		PersistQueue();
	}
	
	StopFlushTimer();
	
	bIsInitialized = false;
	UE_LOG(LogDeskillz, Log, TEXT("Analytics shutdown"));
}

// ============================================================================
// Event Tracking
// ============================================================================

void UDeskillzAnalytics::TrackEvent(const FDeskillzAnalyticsEvent& Event)
{
	if (!IsEnabled())
	{
		return;
	}
	
	// Check sampling
	if (!ShouldSampleEvent())
	{
		return;
	}
	
	// Prepare event
	FDeskillzAnalyticsEvent PreparedEvent = Event;
	PreparedEvent.SessionId = SessionId;
	PreparedEvent.UserId = UserId;
	PreparedEvent.SequenceNumber = ++EventSequence;
	
	if (PreparedEvent.Timestamp == 0)
	{
		PreparedEvent.Timestamp = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	}
	
	// Enqueue
	EnqueueEvent(PreparedEvent);
	
	// Broadcast
	OnEventTracked.Broadcast(PreparedEvent);
	
	if (AnalyticsConfig.bDebugMode)
	{
		UE_LOG(LogDeskillz, Log, TEXT("Analytics Event: %s [%d params]"), 
			*PreparedEvent.EventName, PreparedEvent.Parameters.Num());
	}
}

FDeskillzAnalyticsEvent& UDeskillzAnalytics::Track(const FString& EventName, EDeskillzEventCategory Category)
{
	// Create new pending event
	static FDeskillzAnalyticsEvent StaticEvent;
	StaticEvent = FDeskillzAnalyticsEvent(EventName, Category);
	PendingEvent = &StaticEvent;
	
	// Auto-track when event goes out of scope or next Track() is called
	// For now, we'll track immediately when parameters are added
	// The returned reference allows chaining
	
	return StaticEvent;
}

void UDeskillzAnalytics::K2_TrackEvent(const FString& EventName, EDeskillzEventCategory Category,
	const TMap<FString, FString>& Parameters)
{
	FDeskillzAnalyticsEvent Event(EventName, Category);
	Event.Parameters = Parameters;
	TrackEvent(Event);
}

// ============================================================================
// Pre-defined Events
// ============================================================================

void UDeskillzAnalytics::TrackSessionStart()
{
	FDeskillzAnalyticsEvent Event(TEXT("session_start"), EDeskillzEventCategory::System);
	
	// Add device info
	TMap<FString, FString> DeviceInfo = GetDeviceInfo();
	for (const auto& Pair : DeviceInfo)
	{
		Event.AddParam(Pair.Key, Pair.Value);
	}
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackSessionEnd()
{
	FDeskillzAnalyticsEvent Event(TEXT("session_end"), EDeskillzEventCategory::System);
	Event.AddValue(TEXT("duration_seconds"), GetSessionDuration());
	Event.AddValue(TEXT("events_count"), static_cast<double>(EventSequence));
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackLogin(const FString& Method)
{
	FDeskillzAnalyticsEvent Event(TEXT("login"), EDeskillzEventCategory::User);
	Event.AddParam(TEXT("method"), Method);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackRegistration(const FString& Method)
{
	FDeskillzAnalyticsEvent Event(TEXT("registration"), EDeskillzEventCategory::User);
	Event.AddParam(TEXT("method"), Method);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackMatchStart(const FString& MatchId, const FString& TournamentId, double EntryFee)
{
	FDeskillzAnalyticsEvent Event(TEXT("match_start"), EDeskillzEventCategory::Match);
	Event.AddParam(TEXT("match_id"), MatchId);
	Event.AddParam(TEXT("tournament_id"), TournamentId);
	Event.AddValue(TEXT("entry_fee"), EntryFee);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackMatchComplete(const FString& MatchId, int64 Score, bool bWon, double PrizeWon)
{
	FDeskillzAnalyticsEvent Event(TEXT("match_complete"), EDeskillzEventCategory::Match);
	Event.AddParam(TEXT("match_id"), MatchId);
	Event.AddParam(TEXT("outcome"), bWon ? TEXT("win") : TEXT("loss"));
	Event.AddValue(TEXT("score"), static_cast<double>(Score));
	Event.AddValue(TEXT("prize_won"), PrizeWon);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackTournamentEntry(const FString& TournamentId, double EntryFee, const FString& Currency)
{
	FDeskillzAnalyticsEvent Event(TEXT("tournament_entry"), EDeskillzEventCategory::Tournament);
	Event.AddParam(TEXT("tournament_id"), TournamentId);
	Event.AddParam(TEXT("currency"), Currency);
	Event.AddValue(TEXT("entry_fee"), EntryFee);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackDeposit(double Amount, const FString& Currency)
{
	FDeskillzAnalyticsEvent Event(TEXT("deposit"), EDeskillzEventCategory::Wallet);
	Event.AddParam(TEXT("currency"), Currency);
	Event.AddValue(TEXT("amount"), Amount);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackWithdrawal(double Amount, const FString& Currency)
{
	FDeskillzAnalyticsEvent Event(TEXT("withdrawal"), EDeskillzEventCategory::Wallet);
	Event.AddParam(TEXT("currency"), Currency);
	Event.AddValue(TEXT("amount"), Amount);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackScreenView(const FString& ScreenName)
{
	FDeskillzAnalyticsEvent Event(TEXT("screen_view"), EDeskillzEventCategory::UI);
	Event.AddParam(TEXT("screen_name"), ScreenName);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackButtonClick(const FString& ButtonName, const FString& ScreenName)
{
	FDeskillzAnalyticsEvent Event(TEXT("button_click"), EDeskillzEventCategory::UI);
	Event.AddParam(TEXT("button_name"), ButtonName);
	Event.AddParam(TEXT("screen_name"), ScreenName);
	
	TrackEvent(Event);
}

void UDeskillzAnalytics::TrackError(const FString& ErrorCode, const FString& ErrorMessage, const FString& Context)
{
	FDeskillzAnalyticsEvent Event(TEXT("error"), EDeskillzEventCategory::Error);
	Event.AddParam(TEXT("error_code"), ErrorCode);
	Event.AddParam(TEXT("error_message"), ErrorMessage);
	Event.AddParam(TEXT("context"), Context);
	
	TrackEvent(Event);
}

// ============================================================================
// User Properties
// ============================================================================

void UDeskillzAnalytics::SetUserProperties(const FDeskillzUserProperties& Properties)
{
	UserProperties = Properties;
	UserId = Properties.UserId;
	
	UE_LOG(LogDeskillz, Verbose, TEXT("Analytics user properties set: %s"), *UserId);
}

void UDeskillzAnalytics::SetUserId(const FString& InUserId)
{
	UserId = InUserId;
	UserProperties.UserId = InUserId;
}

void UDeskillzAnalytics::SetUserProperty(const FString& Key, const FString& Value)
{
	UserProperties.CustomProperties.Add(Key, Value);
}

void UDeskillzAnalytics::ClearUserData()
{
	UserId.Empty();
	UserProperties = FDeskillzUserProperties();
	
	UE_LOG(LogDeskillz, Log, TEXT("Analytics user data cleared"));
}

// ============================================================================
// Queue Management
// ============================================================================

void UDeskillzAnalytics::Flush()
{
	if (EventQueue.Num() == 0 || bIsFlushing)
	{
		return;
	}
	
	DoFlush();
}

void UDeskillzAnalytics::ClearQueue()
{
	FScopeLock Lock(&QueueLock);
	EventQueue.Empty();
	
	UE_LOG(LogDeskillz, Log, TEXT("Analytics queue cleared"));
}

// ============================================================================
// Session
// ============================================================================

float UDeskillzAnalytics::GetSessionDuration() const
{
	int64 Now = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	return static_cast<float>(Now - SessionStartTime) / 1000.0f;
}

void UDeskillzAnalytics::StartNewSession()
{
	// End current session if exists
	if (!SessionId.IsEmpty() && AnalyticsConfig.bTrackAutoEvents)
	{
		TrackSessionEnd();
	}
	
	// Generate new session
	SessionId = GenerateSessionId();
	SessionStartTime = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	EventSequence = 0;
	
	UE_LOG(LogDeskillz, Log, TEXT("New analytics session: %s"), *SessionId);
	
	if (AnalyticsConfig.bTrackAutoEvents)
	{
		TrackSessionStart();
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

FString UDeskillzAnalytics::GenerateSessionId() const
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
}

bool UDeskillzAnalytics::ShouldSampleEvent() const
{
	if (AnalyticsConfig.SampleRate >= 1.0f)
	{
		return true;
	}
	
	return FMath::FRand() < AnalyticsConfig.SampleRate;
}

void UDeskillzAnalytics::EnqueueEvent(const FDeskillzAnalyticsEvent& Event)
{
	FScopeLock Lock(&QueueLock);
	
	// Check max queue size
	if (EventQueue.Num() >= AnalyticsConfig.MaxQueueSize)
	{
		// Remove oldest events
		int32 ToRemove = EventQueue.Num() - AnalyticsConfig.MaxQueueSize + 1;
		EventQueue.RemoveAt(0, ToRemove);
		
		UE_LOG(LogDeskillz, Warning, TEXT("Analytics queue overflow - removed %d events"), ToRemove);
	}
	
	EventQueue.Add(Event);
	
	// Check if we should flush
	CheckFlush();
}

void UDeskillzAnalytics::CheckFlush()
{
	if (EventQueue.Num() >= AnalyticsConfig.BatchSize)
	{
		DoFlush();
	}
}

void UDeskillzAnalytics::DoFlush()
{
	if (bIsFlushing || EventQueue.Num() == 0)
	{
		return;
	}
	
	bIsFlushing = true;
	
	// Get events to send
	TArray<FDeskillzAnalyticsEvent> EventsToSend;
	{
		FScopeLock Lock(&QueueLock);
		int32 Count = FMath::Min(EventQueue.Num(), AnalyticsConfig.BatchSize);
		
		for (int32 i = 0; i < Count; i++)
		{
			EventsToSend.Add(EventQueue[i]);
		}
	}
	
	// Build JSON payload
	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	
	TArray<TSharedPtr<FJsonValue>> EventsArray;
	for (const FDeskillzAnalyticsEvent& Event : EventsToSend)
	{
		TSharedPtr<FJsonObject> EventJson = EventToJson(Event);
		EventsArray.Add(MakeShareable(new FJsonValueObject(EventJson)));
	}
	
	Payload->SetArrayField(TEXT("events"), EventsArray);
	Payload->SetStringField(TEXT("session_id"), SessionId);
	Payload->SetStringField(TEXT("user_id"), UserId);
	
	// Send to server
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->PostJson(AnalyticsConfig.AnalyticsEndpoint, Payload,
		FOnDeskillzHttpResponse::CreateLambda([this, Count = EventsToSend.Num()](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				// Remove sent events from queue
				FScopeLock Lock(&QueueLock);
				EventQueue.RemoveAt(0, Count);
				
				UE_LOG(LogDeskillz, Verbose, TEXT("Analytics flushed %d events"), Count);
			}
			else
			{
				UE_LOG(LogDeskillz, Warning, TEXT("Analytics flush failed: %s"), *Response.ErrorMessage);
				
				// Persist for retry
				if (AnalyticsConfig.bPersistOffline)
				{
					PersistQueue();
				}
			}
			
			bIsFlushing = false;
		})
	);
}

void UDeskillzAnalytics::OnFlushComplete(bool bSuccess)
{
	bIsFlushing = false;
}

void UDeskillzAnalytics::StartFlushTimer()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().SetTimer(
			FlushTimerHandle,
			this,
			&UDeskillzAnalytics::Flush,
			AnalyticsConfig.FlushInterval,
			true
		);
	}
}

void UDeskillzAnalytics::StopFlushTimer()
{
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(FlushTimerHandle);
	}
}

void UDeskillzAnalytics::PersistQueue()
{
	if (EventQueue.Num() == 0)
	{
		return;
	}
	
	// Serialize queue to JSON
	TArray<TSharedPtr<FJsonValue>> EventsArray;
	for (const FDeskillzAnalyticsEvent& Event : EventQueue)
	{
		EventsArray.Add(MakeShareable(new FJsonValueObject(EventToJson(Event))));
	}
	
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(EventsArray, Writer);
	
	// Save to file
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Analytics") / TEXT("pending_events.json");
	FFileHelper::SaveStringToFile(JsonString, *FilePath);
	
	UE_LOG(LogDeskillz, Log, TEXT("Persisted %d analytics events"), EventQueue.Num());
}

void UDeskillzAnalytics::LoadPersistedQueue()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Analytics") / TEXT("pending_events.json");
	
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return;
	}
	
	TArray<TSharedPtr<FJsonValue>> EventsArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(Reader, EventsArray))
	{
		return;
	}
	
	int32 LoadedCount = 0;
	for (const TSharedPtr<FJsonValue>& Value : EventsArray)
	{
		if (TSharedPtr<FJsonObject> EventJson = Value->AsObject())
		{
			FDeskillzAnalyticsEvent Event;
			Event.EventName = EventJson->GetStringField(TEXT("event_name"));
			Event.Timestamp = static_cast<int64>(EventJson->GetNumberField(TEXT("timestamp")));
			Event.SessionId = EventJson->GetStringField(TEXT("session_id"));
			Event.UserId = EventJson->GetStringField(TEXT("user_id"));
			
			// Parse parameters
			const TSharedPtr<FJsonObject>* ParamsObj;
			if (EventJson->TryGetObjectField(TEXT("parameters"), ParamsObj))
			{
				for (const auto& Pair : (*ParamsObj)->Values)
				{
					Event.Parameters.Add(Pair.Key, Pair.Value->AsString());
				}
			}
			
			EventQueue.Add(Event);
			LoadedCount++;
		}
	}
	
	// Delete the file after loading
	IFileManager::Get().Delete(*FilePath);
	
	UE_LOG(LogDeskillz, Log, TEXT("Loaded %d persisted analytics events"), LoadedCount);
}

TMap<FString, FString> UDeskillzAnalytics::GetDeviceInfo() const
{
	TMap<FString, FString> Info;
	
	Info.Add(TEXT("platform"), FPlatformMisc::GetUBTPlatform());
	Info.Add(TEXT("os_version"), FPlatformMisc::GetOSVersion());
	Info.Add(TEXT("device_model"), FPlatformMisc::GetDefaultDeviceProfileName());
	Info.Add(TEXT("cpu_brand"), FPlatformMisc::GetCPUBrand());
	Info.Add(TEXT("gpu_brand"), FPlatformMisc::GetPrimaryGPUBrand());
	Info.Add(TEXT("device_id"), FPlatformMisc::GetDeviceId());
	
	return Info;
}

TSharedPtr<FJsonObject> UDeskillzAnalytics::EventToJson(const FDeskillzAnalyticsEvent& Event) const
{
	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	
	Json->SetStringField(TEXT("event_name"), Event.EventName);
	Json->SetNumberField(TEXT("category"), static_cast<int32>(Event.Category));
	Json->SetNumberField(TEXT("timestamp"), static_cast<double>(Event.Timestamp));
	Json->SetStringField(TEXT("session_id"), Event.SessionId);
	Json->SetStringField(TEXT("user_id"), Event.UserId);
	Json->SetNumberField(TEXT("sequence"), Event.SequenceNumber);
	
	// Add string parameters
	TSharedPtr<FJsonObject> ParamsObj = MakeShareable(new FJsonObject());
	for (const auto& Pair : Event.Parameters)
	{
		ParamsObj->SetStringField(Pair.Key, Pair.Value);
	}
	Json->SetObjectField(TEXT("parameters"), ParamsObj);
	
	// Add numeric values
	TSharedPtr<FJsonObject> ValuesObj = MakeShareable(new FJsonObject());
	for (const auto& Pair : Event.NumericValues)
	{
		ValuesObj->SetNumberField(Pair.Key, Pair.Value);
	}
	Json->SetObjectField(TEXT("values"), ValuesObj);
	
	return Json;
}
