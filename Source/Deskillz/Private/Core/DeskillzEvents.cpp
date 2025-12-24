// Copyright Deskillz Games. All Rights Reserved.

#include "Core/DeskillzEvents.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

// Static singleton instance
UDeskillzEvents* UDeskillzEvents::Instance = nullptr;

UDeskillzEvents::UDeskillzEvents()
{
	// Store as singleton
	Instance = this;
}

UDeskillzEvents* UDeskillzEvents::Get(const UObject* WorldContextObject)
{
	// Return cached instance if valid
	if (Instance && Instance->IsValidLowLevel())
	{
		return Instance;
	}
	
	// Create new instance if needed
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				// Create as a subobject of the game instance so it persists
				Instance = NewObject<UDeskillzEvents>(GameInstance, TEXT("DeskillzEvents"));
				Instance->AddToRoot(); // Prevent GC
				return Instance;
			}
		}
	}
	
	return nullptr;
}

// ============================================================================
// Event Broadcasting
// ============================================================================

void UDeskillzEvents::BroadcastSDKInitialized(bool bSuccess, const FDeskillzError& Error)
{
	OnSDKInitialized.Broadcast(bSuccess, Error);
	
	if (!bSuccess)
	{
		BroadcastError(Error);
	}
}

void UDeskillzEvents::BroadcastError(const FDeskillzError& Error)
{
	OnError.Broadcast(Error);
}

void UDeskillzEvents::BroadcastAuthStateChanged(bool bIsAuthenticated, const FDeskillzPlayer& Player)
{
	OnAuthStateChanged.Broadcast(bIsAuthenticated, Player);
}

void UDeskillzEvents::BroadcastMatchStarted(const FDeskillzMatchInfo& MatchInfo, const FDeskillzError& Error)
{
	OnMatchStarted.Broadcast(MatchInfo, Error);
	
	if (!Error.IsError())
	{
		OnOpponentFound.Broadcast(MatchInfo.Opponent);
	}
}

void UDeskillzEvents::BroadcastMatchCompleted(const FDeskillzMatchResult& Result, const FDeskillzError& Error)
{
	OnMatchCompleted.Broadcast(Result, Error);
}

void UDeskillzEvents::BroadcastScoreSubmitted(bool bSuccess, const FDeskillzError& Error)
{
	OnScoreSubmitted.Broadcast(bSuccess, Error);
	
	if (!bSuccess)
	{
		BroadcastError(Error);
	}
}

// ============================================================================
// Blueprint Function Library Implementation
// ============================================================================

bool UDeskillzEventLibrary::IsSDKReady(const UObject* WorldContextObject)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(WorldContextObject);
	return SDK && SDK->IsReady();
}

float UDeskillzEventLibrary::GetMatchRemainingTime(const UObject* WorldContextObject)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(WorldContextObject);
	return SDK ? SDK->GetRemainingTime() : 0.0f;
}

int64 UDeskillzEventLibrary::GetCurrentScore(const UObject* WorldContextObject)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(WorldContextObject);
	return SDK ? SDK->GetCurrentScore() : 0;
}

bool UDeskillzEventLibrary::IsInMatch(const UObject* WorldContextObject)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(WorldContextObject);
	return SDK && SDK->IsInMatch();
}

bool UDeskillzEventLibrary::IsInPractice(const UObject* WorldContextObject)
{
	UDeskillzSDK* SDK = UDeskillzSDK::Get(WorldContextObject);
	return SDK && SDK->IsInPractice();
}
