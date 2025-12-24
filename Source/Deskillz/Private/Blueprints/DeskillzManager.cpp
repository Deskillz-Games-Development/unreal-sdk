// Copyright Deskillz Games. All Rights Reserved.
// DeskillzManager_Updated.cpp - UPDATED for centralized lobby architecture
//
// CHANGES:
// - Added initialization of DeepLinkHandler
// - Added handling of deep link match launches
// - Added integration with DeskillzBridge for match lifecycle
// - New delegates for lobby events

#include "Blueprints/DeskillzManager.h"
#include "Core/DeskillzSDK.h"
#include "Deskillz.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

// NEW: Lobby includes
#include "Lobby/DeskillzDeepLinkHandler.h"
#include "Lobby/DeskillzBridge.h"
#include "Lobby/DeskillzLobbyClient.h"

// Singleton instance
ADeskillzManager* ADeskillzManager::Instance = nullptr;

ADeskillzManager::ADeskillzManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	// Sensible defaults for zero-config setup
	bAutoInitialize = true;
	bUseBuiltInUI = true;
	bPersistAcrossLevels = true;
	bShowDebugInfo = false;
}

ADeskillzManager* ADeskillzManager::Get(const UObject* WorldContextObject)
{
	// Return existing instance
	if (Instance && Instance->IsValidLowLevel() && !Instance->IsPendingKillPending())
	{
		return Instance;
	}
	
	// Try to find existing in world
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			TArray<AActor*> FoundManagers;
			UGameplayStatics::GetAllActorsOfClass(World, ADeskillzManager::StaticClass(), FoundManagers);
			
			if (FoundManagers.Num() > 0)
			{
				Instance = Cast<ADeskillzManager>(FoundManagers[0]);
				return Instance;
			}
			
			// Auto-spawn if none exists
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = TEXT("DeskillzManager_AutoSpawned");
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			Instance = World->SpawnActor<ADeskillzManager>(ADeskillzManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			
			if (Instance)
			{
				UE_LOG(LogDeskillz, Log, TEXT("Auto-spawned DeskillzManager"));
			}
			
			return Instance;
		}
	}
	
	return nullptr;
}

bool ADeskillzManager::IsManagerAvailable()
{
	return Instance && Instance->IsValidLowLevel() && !Instance->IsPendingKillPending();
}

void ADeskillzManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Set as singleton
	if (Instance && Instance != this)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Multiple DeskillzManagers detected! Destroying duplicate."));
		Destroy();
		return;
	}
	
	Instance = this;
	
	// Persist across level loads
	if (bPersistAcrossLevels)
	{
		SetLifeSpan(0); // Don't auto-destroy
	}
	
	// Get SDK from GameInstance
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		SDK = GameInstance->GetSubsystem<UDeskillzSDK>();
	}
	
	if (SDK)
	{
		BindSDKEvents();
		
		// Auto-initialize if enabled
		if (bAutoInitialize)
		{
			Initialize();
		}
	}
	else
	{
		UE_LOG(LogDeskillz, Error, TEXT("DeskillzManager: Could not get SDK subsystem!"));
	}
	
	// =========================================================================
	// NEW: Initialize Lobby Components
	// =========================================================================
	InitializeLobbyComponents();
	// =========================================================================
	
	UE_LOG(LogDeskillz, Log, TEXT("DeskillzManager BeginPlay - AutoInit: %s, BuiltInUI: %s"), 
		bAutoInitialize ? TEXT("Yes") : TEXT("No"),
		bUseBuiltInUI ? TEXT("Yes") : TEXT("No"));
}

void ADeskillzManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// =========================================================================
	// NEW: Cleanup Lobby Components
	// =========================================================================
	CleanupLobbyComponents();
	// =========================================================================
	
	if (Instance == this)
	{
		Instance = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void ADeskillzManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update match time every second
	if (SDK && SDK->IsInMatch())
	{
		MatchTimeUpdateTimer += DeltaTime;
		if (MatchTimeUpdateTimer >= 1.0f)
		{
			MatchTimeUpdateTimer = 0.0f;
			OnMatchTimeUpdate.Broadcast(SDK->GetRemainingTime(), SDK->GetElapsedTime());
		}
	}
	
	// Debug display
	if (bShowDebugInfo && GEngine)
	{
		FString DebugInfo = FString::Printf(TEXT("Deskillz SDK v%s (Centralized Lobby)\n"), *UDeskillzSDK::GetSDKVersion());
		
		if (SDK)
		{
			DebugInfo += FString::Printf(TEXT("State: %s\n"), 
				SDK->IsReady() ? TEXT("Ready") : TEXT("Not Ready"));
			DebugInfo += FString::Printf(TEXT("Authenticated: %s\n"), 
				SDK->IsAuthenticated() ? TEXT("Yes") : TEXT("No"));
			
			if (SDK->IsInMatch())
			{
				DebugInfo += FString::Printf(TEXT("In Match: Yes\n"));
				DebugInfo += FString::Printf(TEXT("Time: %.1fs / Score: %lld\n"), 
					SDK->GetElapsedTime(), SDK->GetCurrentScore());
			}
			else if (SDK->IsInPractice())
			{
				DebugInfo += TEXT("Mode: Practice\n");
			}
			else
			{
				// NEW: Show lobby status
				DebugInfo += TEXT("Status: Waiting for deep link\n");
				DebugInfo += TEXT("Join matches at deskillz.games\n");
			}
		}
		
		// NEW: Show deep link status
		UDeskillzDeepLinkHandler* DeepLinkHandler = UDeskillzDeepLinkHandler::Get();
		if (DeepLinkHandler && DeepLinkHandler->HasValidLaunchData())
		{
			DebugInfo += FString::Printf(TEXT("Match ID: %s\n"), *DeepLinkHandler->GetCurrentMatchId());
		}
		
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, DebugInfo);
	}
}

void ADeskillzManager::Initialize()
{
	if (bIsInitialized)
	{
		UE_LOG(LogDeskillz, Warning, TEXT("DeskillzManager already initialized"));
		return;
	}
	
	if (!SDK)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Cannot initialize - SDK not available"));
		return;
	}
	
	UE_LOG(LogDeskillz, Log, TEXT("DeskillzManager initializing SDK..."));
	
	// SDK initialization happens automatically through GameInstance subsystem
	// But we can trigger it manually if needed
	if (!SDK->IsReady())
	{
		SDK->InitializeSDK();
	}
	else
	{
		// Already initialized
		bIsInitialized = true;
		OnSDKInitialized.Broadcast(true, FDeskillzError::None());
		
		// NEW: Process any pending deep link
		ProcessPendingDeepLink();
	}
}

// ============================================================================
// NEW: Lobby Component Initialization
// ============================================================================

void ADeskillzManager::InitializeLobbyComponents()
{
	UE_LOG(LogDeskillz, Log, TEXT("Initializing Lobby Components..."));
	
	// Initialize Deep Link Handler
	UDeskillzDeepLinkHandler* DeepLinkHandler = UDeskillzDeepLinkHandler::Get();
	if (DeepLinkHandler)
	{
		DeepLinkHandler->Initialize();
		
		// Bind to deep link events
		DeepLinkHandler->OnMatchLaunchReceived.AddDynamic(this, &ADeskillzManager::HandleMatchLaunchReceived);
		DeepLinkHandler->OnMatchReady.AddDynamic(this, &ADeskillzManager::HandleMatchReady);
		DeepLinkHandler->OnValidationFailed.AddDynamic(this, &ADeskillzManager::HandleLaunchValidationFailed);
		
		UE_LOG(LogDeskillz, Log, TEXT("Deep Link Handler initialized and bound"));
		
		// Check for pending deep link (app launched from deep link)
		if (DeepLinkHandler->HasPendingLaunch())
		{
			UE_LOG(LogDeskillz, Log, TEXT("Found pending deep link - will process after SDK init"));
		}
	}
	else
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Could not get Deep Link Handler"));
	}
}

void ADeskillzManager::CleanupLobbyComponents()
{
	UE_LOG(LogDeskillz, Log, TEXT("Cleaning up Lobby Components..."));
	
	// Unbind from deep link handler
	UDeskillzDeepLinkHandler* DeepLinkHandler = UDeskillzDeepLinkHandler::Get();
	if (DeepLinkHandler)
	{
		DeepLinkHandler->OnMatchLaunchReceived.RemoveDynamic(this, &ADeskillzManager::HandleMatchLaunchReceived);
		DeepLinkHandler->OnMatchReady.RemoveDynamic(this, &ADeskillzManager::HandleMatchReady);
		DeepLinkHandler->OnValidationFailed.RemoveDynamic(this, &ADeskillzManager::HandleLaunchValidationFailed);
	}
}

void ADeskillzManager::ProcessPendingDeepLink()
{
	UDeskillzDeepLinkHandler* DeepLinkHandler = UDeskillzDeepLinkHandler::Get();
	if (DeepLinkHandler && DeepLinkHandler->HasPendingLaunch())
	{
		UE_LOG(LogDeskillz, Log, TEXT("Processing pending deep link..."));
		DeepLinkHandler->ProcessPendingLaunch();
	}
}

// ============================================================================
// NEW: Deep Link Event Handlers
// ============================================================================

void ADeskillzManager::HandleMatchLaunchReceived(const FDeskillzMatchLaunchData& LaunchData)
{
	UE_LOG(LogDeskillz, Log, TEXT("Match launch received - MatchId: %s"), *LaunchData.MatchId);
	
	// Store the launch data
	CurrentLaunchData = LaunchData;
	
	// Broadcast to Blueprint
	OnDeepLinkReceived.Broadcast(LaunchData);
}

void ADeskillzManager::HandleMatchReady(const FDeskillzMatchLaunchData& LaunchData)
{
	UE_LOG(LogDeskillz, Log, TEXT("Match ready to start - MatchId: %s, Duration: %ds"), 
		*LaunchData.MatchId, LaunchData.DurationSeconds);
	
	// Initialize the Bridge
	UDeskillzBridge* Bridge = UDeskillzBridge::Get();
	if (Bridge)
	{
		Bridge->Initialize(LaunchData);
		UE_LOG(LogDeskillz, Log, TEXT("Bridge initialized for match"));
	}
	
	// Initialize the Lobby Client
	UDeskillzLobbyClient* LobbyClient = UDeskillzLobbyClient::Get();
	if (LobbyClient)
	{
		LobbyClient->InitializeFromLaunchData(LaunchData);
		UE_LOG(LogDeskillz, Log, TEXT("Lobby Client initialized for match"));
	}
	
	// Convert launch data to FDeskillzMatchInfo for compatibility with existing code
	FDeskillzMatchInfo MatchInfo;
	MatchInfo.MatchId = LaunchData.MatchId;
	MatchInfo.TournamentId = LaunchData.TournamentId;
	MatchInfo.MatchType = LaunchData.MatchType;
	MatchInfo.DurationSeconds = LaunchData.DurationSeconds;
	MatchInfo.RandomSeed = LaunchData.RandomSeed;
	MatchInfo.Status = EDeskillzMatchStatus::Ready;
	MatchInfo.Opponent = LaunchData.Opponent;
	
	if (SDK)
	{
		// Set the current player info
		MatchInfo.LocalPlayer = SDK->GetCurrentPlayer();
	}
	
	// Broadcast match ready - game should prepare to start gameplay
	OnMatchReady.Broadcast(MatchInfo, FDeskillzError::None());
	
	// Broadcast lobby-specific event
	OnLobbyMatchReady.Broadcast(LaunchData);
}

void ADeskillzManager::HandleLaunchValidationFailed(const FString& Reason, const FDeskillzMatchLaunchData& LaunchData)
{
	UE_LOG(LogDeskillz, Error, TEXT("Launch validation failed: %s"), *Reason);
	
	FDeskillzError Error(EDeskillzErrorCode::Unknown, Reason);
	OnError.Broadcast(Error);
	
	// Broadcast lobby-specific event
	OnLobbyError.Broadcast(Reason);
}

// ============================================================================
// NEW: Lobby Helper Methods
// ============================================================================

void ADeskillzManager::StartMatchFromLobby()
{
	UDeskillzBridge* Bridge = UDeskillzBridge::Get();
	if (Bridge && Bridge->IsInitialized())
	{
		Bridge->ReportMatchStarted();
		
		// Also start match in SDK
		if (SDK)
		{
			SDK->StartMatch();
		}
		
		// Broadcast gameplay start
		FDeskillzMatchInfo MatchInfo = GetMatchInfo();
		OnGameplayStart.Broadcast(MatchInfo);
		
		UE_LOG(LogDeskillz, Log, TEXT("Match started from lobby"));
	}
	else
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot start match - Bridge not initialized"));
	}
}

void ADeskillzManager::CompleteMatchFromLobby(int64 FinalScore)
{
	UE_LOG(LogDeskillz, Log, TEXT("Completing match from lobby - Score: %lld"), FinalScore);
	
	// Submit score via SDK
	if (SDK)
	{
		SDK->SubmitScore(FinalScore);
	}
	
	// Also complete via Bridge
	UDeskillzBridge* Bridge = UDeskillzBridge::Get();
	if (Bridge && Bridge->IsInitialized())
	{
		FDeskillzMatchCompletionData CompletionData;
		CompletionData.MatchId = Bridge->GetMatchId();
		CompletionData.FinalScore = FinalScore;
		CompletionData.bCompletedNormally = true;
		
		Bridge->CompleteMatch(CompletionData);
	}
}

void ADeskillzManager::ReturnToLobby(bool bShowResults)
{
	UDeskillzBridge* Bridge = UDeskillzBridge::Get();
	if (Bridge && Bridge->CanReturnToMainApp())
	{
		EDeskillzReturnDestination Destination = bShowResults 
			? EDeskillzReturnDestination::Results 
			: EDeskillzReturnDestination::Lobby;
		
		Bridge->ReturnToMainApp(Destination);
		
		UE_LOG(LogDeskillz, Log, TEXT("Returning to main app - Destination: %s"), 
			bShowResults ? TEXT("Results") : TEXT("Lobby"));
	}
	else
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot return to main app - Bridge not available"));
	}
}

FDeskillzMatchLaunchData ADeskillzManager::GetCurrentLaunchData() const
{
	return CurrentLaunchData;
}

bool ADeskillzManager::HasLaunchData() const
{
	return CurrentLaunchData.bIsValid;
}

// ============================================================================
// Original Methods (Updated)
// ============================================================================

void ADeskillzManager::BindSDKEvents()
{
	if (!SDK)
	{
		return;
	}
	
	// Bind to SDK events and forward to our Blueprint-assignable events
	SDK->OnInitialized.AddDynamic(this, &ADeskillzManager::HandleSDKInitialized);
	SDK->OnMatchStarted.AddDynamic(this, &ADeskillzManager::HandleMatchStarted);
	SDK->OnMatchCompleted.AddDynamic(this, &ADeskillzManager::HandleMatchCompleted);
	SDK->OnError.AddDynamic(this, &ADeskillzManager::HandleError);
}

void ADeskillzManager::HandleSDKInitialized(bool bSuccess, const FDeskillzError& Error)
{
	bIsInitialized = bSuccess;
	
	UE_LOG(LogDeskillz, Log, TEXT("SDK Initialization %s"), bSuccess ? TEXT("succeeded") : TEXT("failed"));
	
	// Forward to Blueprint
	OnSDKInitialized.Broadcast(bSuccess, Error);
	
	if (!bSuccess)
	{
		OnError.Broadcast(Error);
	}
	else
	{
		// NEW: Process pending deep link after SDK is ready
		ProcessPendingDeepLink();
	}
}

void ADeskillzManager::HandleMatchStarted(const FDeskillzMatchInfo& MatchInfo, const FDeskillzError& Error)
{
	UE_LOG(LogDeskillz, Log, TEXT("Match started: %s vs %s"), 
		*MatchInfo.LocalPlayer.Username, *MatchInfo.Opponent.Username);
	
	// Forward to Blueprint
	OnMatchReady.Broadcast(MatchInfo, Error);
	
	// After a brief delay, signal gameplay start
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this, MatchInfo]()
	{
		OnGameplayStart.Broadcast(MatchInfo);
	});
	
	GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.5f, false);
}

void ADeskillzManager::HandleMatchCompleted(const FDeskillzMatchResult& Result, const FDeskillzError& Error)
{
	UE_LOG(LogDeskillz, Log, TEXT("Match completed: %s (Score: %lld)"), 
		Result.IsWin() ? TEXT("WIN") : (Result.IsLoss() ? TEXT("LOSS") : TEXT("DRAW")),
		Result.PlayerScore);
	
	// Forward to Blueprint
	OnMatchCompleted.Broadcast(Result, Error);
}

void ADeskillzManager::HandleError(const FDeskillzError& Error)
{
	UE_LOG(LogDeskillz, Error, TEXT("SDK Error: %s"), *Error.Message);
	
	// Forward to Blueprint
	OnError.Broadcast(Error);
}

FDeskillzMatchInfo ADeskillzManager::GetMatchInfo() const
{
	// NEW: Check Bridge first for lobby-launched matches
	UDeskillzBridge* Bridge = UDeskillzBridge::Get();
	if (Bridge && Bridge->IsInitialized())
	{
		FDeskillzMatchLaunchData LaunchData = Bridge->GetLaunchData();
		if (LaunchData.bIsValid)
		{
			FDeskillzMatchInfo MatchInfo;
			MatchInfo.MatchId = LaunchData.MatchId;
			MatchInfo.TournamentId = LaunchData.TournamentId;
			MatchInfo.MatchType = LaunchData.MatchType;
			MatchInfo.DurationSeconds = LaunchData.DurationSeconds;
			MatchInfo.RandomSeed = LaunchData.RandomSeed;
			MatchInfo.Opponent = LaunchData.Opponent;
			
			if (SDK)
			{
				MatchInfo.LocalPlayer = SDK->GetCurrentPlayer();
				MatchInfo.Status = SDK->IsInMatch() ? EDeskillzMatchStatus::InProgress : EDeskillzMatchStatus::Ready;
			}
			
			return MatchInfo;
		}
	}
	
	// Fall back to SDK
	if (SDK)
	{
		return SDK->GetCurrentMatch();
	}
	return FDeskillzMatchInfo();
}

bool ADeskillzManager::IsReady() const
{
	return SDK && SDK->IsReady();
}

bool ADeskillzManager::IsInMatch() const
{
	// Check both SDK and Bridge
	if (SDK && SDK->IsInMatch())
	{
		return true;
	}
	
	UDeskillzBridge* Bridge = UDeskillzBridge::Get();
	if (Bridge && Bridge->IsInitialized())
	{
		return true;
	}
	
	return false;
}