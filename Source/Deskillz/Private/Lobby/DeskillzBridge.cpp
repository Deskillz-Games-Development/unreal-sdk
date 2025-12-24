// Copyright Deskillz Games. All Rights Reserved.
// DeskillzBridge.cpp - Implementation of bridge for app-to-app communication

#include "Lobby/DeskillzBridge.h"
#include "Lobby/DeskillzDeepLinkHandler.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#if PLATFORM_IOS
#include "IOS/IOSAppDelegate.h"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

// Singleton instance
UDeskillzBridge* UDeskillzBridge::Instance = nullptr;

UDeskillzBridge::UDeskillzBridge()
{
	// Constructor
}

UDeskillzBridge* UDeskillzBridge::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UDeskillzBridge>();
		Instance->AddToRoot(); // Prevent garbage collection
	}
	return Instance;
}

void UDeskillzBridge::Initialize(const FDeskillzMatchLaunchData& LaunchData)
{
	if (!LaunchData.bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzBridge] Cannot initialize with invalid launch data"));
		return;
	}
	
	CurrentLaunchData = LaunchData;
	bIsInitialized = true;
	bMatchStarted = false;
	bMatchCompleted = false;
	LastReportedScore = 0;
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Initialized for match: %s"), *LaunchData.MatchId);
	
	if (LaunchData.HasOpponent())
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Opponent: %s (Rating: %d)"), 
			*LaunchData.Opponent.Username, LaunchData.Opponent.Rating);
	}
}

void UDeskillzBridge::ReportMatchStarted()
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzBridge] Cannot report match start - not initialized"));
		return;
	}
	
	if (bMatchStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzBridge] Match already started"));
		return;
	}
	
	MatchStartTime = FDateTime::UtcNow();
	bMatchStarted = true;
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Match started: %s"), *CurrentLaunchData.MatchId);
	
	// TODO: Send match_started event to backend via WebSocket or API
}

void UDeskillzBridge::ReportScoreUpdate(int64 CurrentScore)
{
	if (!bIsInitialized || !bMatchStarted)
	{
		return;
	}
	
	LastReportedScore = CurrentScore;
	
	// For synchronous matches, send score updates via WebSocket
	if (CurrentLaunchData.IsSynchronous())
	{
		// TODO: Send score update via WebSocket
		UE_LOG(LogTemp, Verbose, TEXT("[DeskillzBridge] Score update: %lld"), CurrentScore);
	}
}

bool UDeskillzBridge::CompleteMatch(const FDeskillzMatchCompletionData& CompletionData)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzBridge] Cannot complete match - not initialized"));
		return false;
	}
	
	if (bMatchCompleted)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzBridge] Match already completed"));
		return false;
	}
	
	bMatchCompleted = true;
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Match completed: %s, Score: %lld, Result: %d"),
		*CompletionData.MatchId,
		CompletionData.FinalScore,
		(int32)CompletionData.Result);
	
	// Submit score to backend
	SubmitScoreToBackend(CompletionData);
	
	return true;
}

void UDeskillzBridge::AbortMatch(const FString& Reason)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzBridge] Cannot abort match - not initialized"));
		return;
	}
	
	FDeskillzMatchCompletionData CompletionData;
	CompletionData.MatchId = CurrentLaunchData.MatchId;
	CompletionData.FinalScore = LastReportedScore;
	CompletionData.Result = EDeskillzMatchResult::Forfeit;
	CompletionData.bCompletedNormally = false;
	CompletionData.AbortReason = Reason;
	
	if (bMatchStarted)
	{
		FTimespan Duration = FDateTime::UtcNow() - MatchStartTime;
		CompletionData.MatchDuration = Duration.GetTotalSeconds();
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Match aborted: %s, Reason: %s"),
		*CurrentLaunchData.MatchId, *Reason);
	
	bMatchCompleted = true;
	
	// Submit abort to backend
	SubmitScoreToBackend(CompletionData);
}

void UDeskillzBridge::ReturnToMainApp(EDeskillzReturnDestination Destination, const FString& Data)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeskillzBridge] Cannot return to main app - not initialized"));
		return;
	}
	
	FString ReturnURL = GenerateReturnURL(Destination, Data);
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Returning to main app: %s"), *ReturnURL);
	
	// Broadcast event before navigating
	OnReturnToMainApp.Broadcast(Destination, Data);
	
	// Try to open main app
	if (IsMainAppInstalled())
	{
		OpenMainAppDirect(ReturnURL);
	}
	else
	{
		// Fall back to web
		OpenWebFallback(Destination, Data);
	}
}

FString UDeskillzBridge::GenerateReturnURL(EDeskillzReturnDestination Destination, const FString& Data) const
{
	FString Path = GetDestinationPath(Destination);
	
	// Build URL with parameters
	FString URL = FString::Printf(TEXT("%s://%s"), *MainAppScheme, *Path);
	
	// Add match ID
	URL += FString::Printf(TEXT("?matchId=%s"), *CurrentLaunchData.MatchId);
	
	// Add data if provided
	if (!Data.IsEmpty())
	{
		URL += FString::Printf(TEXT("&data=%s"), *Data);
	}
	
	// Add token for authentication
	if (!CurrentLaunchData.PlayerToken.IsEmpty())
	{
		URL += FString::Printf(TEXT("&token=%s"), *CurrentLaunchData.PlayerToken);
	}
	
	return URL;
}

void UDeskillzBridge::OpenURL(const FString& URL)
{
	if (URL.IsEmpty())
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Opening URL: %s"), *URL);
	
#if PLATFORM_IOS || PLATFORM_ANDROID
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
#else
	// Desktop fallback
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
#endif
}

bool UDeskillzBridge::CanReturnToMainApp() const
{
	return bIsInitialized && !CurrentLaunchData.MatchId.IsEmpty();
}

FString UDeskillzBridge::GetCustomParameter(const FString& Key, const FString& DefaultValue) const
{
	if (CurrentLaunchData.CustomParams.Contains(Key))
	{
		return CurrentLaunchData.CustomParams[Key];
	}
	return DefaultValue;
}

void UDeskillzBridge::SubmitScoreToBackend(const FDeskillzMatchCompletionData& CompletionData)
{
	// TODO: Implement actual score submission via API
	// This would use DeskillzApiService to:
	// 1. Sign the score with HMAC-SHA256
	// 2. Submit to /api/v1/matches/{matchId}/scores
	// 3. Handle response
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Submitting score to backend..."));
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Match: %s, Score: %lld, Duration: %.1f"),
		*CompletionData.MatchId,
		CompletionData.FinalScore,
		CompletionData.MatchDuration);
	
	// For now, just log the stats
	for (const auto& Stat : CompletionData.Stats)
	{
		UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Stat - %s: %s"), *Stat.Key, *Stat.Value);
	}
}

FString UDeskillzBridge::GetDestinationPath(EDeskillzReturnDestination Destination) const
{
	switch (Destination)
	{
		case EDeskillzReturnDestination::Lobby:
			return TEXT("lobby");
			
		case EDeskillzReturnDestination::Results:
			return TEXT("match/results");
			
		case EDeskillzReturnDestination::Tournament:
			if (!CurrentLaunchData.TournamentId.IsEmpty())
			{
				return FString::Printf(TEXT("tournament/%s"), *CurrentLaunchData.TournamentId);
			}
			return TEXT("tournaments");
			
		case EDeskillzReturnDestination::Leaderboard:
			return TEXT("leaderboard");
			
		case EDeskillzReturnDestination::Rematch:
			return TEXT("match/rematch");
			
		default:
			return TEXT("lobby");
	}
}

bool UDeskillzBridge::IsMainAppInstalled() const
{
#if PLATFORM_IOS
	// iOS: Check if URL scheme can be opened
	// Would use UIApplication.canOpenURL
	return true; // Assume installed for now
#elif PLATFORM_ANDROID
	// Android: Check if package is installed
	// Would use PackageManager
	return true; // Assume installed for now
#else
	return false; // Desktop doesn't have "main app"
#endif
}

void UDeskillzBridge::OpenMainAppDirect(const FString& URL)
{
	OpenURL(URL);
}

void UDeskillzBridge::OpenWebFallback(EDeskillzReturnDestination Destination, const FString& Data)
{
	// Generate web URL instead of deep link
	FString WebURL = TEXT("https://deskillz.games");
	
	switch (Destination)
	{
		case EDeskillzReturnDestination::Results:
			WebURL += FString::Printf(TEXT("/match/%s/results"), *CurrentLaunchData.MatchId);
			break;
			
		case EDeskillzReturnDestination::Tournament:
			if (!CurrentLaunchData.TournamentId.IsEmpty())
			{
				WebURL += FString::Printf(TEXT("/tournament/%s"), *CurrentLaunchData.TournamentId);
			}
			else
			{
				WebURL += TEXT("/tournaments");
			}
			break;
			
		case EDeskillzReturnDestination::Leaderboard:
			WebURL += TEXT("/leaderboard");
			break;
			
		default:
			WebURL += TEXT("/lobby");
			break;
	}
	
	// Add token for auto-login
	if (!CurrentLaunchData.PlayerToken.IsEmpty())
	{
		WebURL += FString::Printf(TEXT("?token=%s"), *CurrentLaunchData.PlayerToken);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[DeskillzBridge] Opening web fallback: %s"), *WebURL);
	OpenURL(WebURL);
}