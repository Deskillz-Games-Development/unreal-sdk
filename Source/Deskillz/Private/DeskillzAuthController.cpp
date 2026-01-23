// Copyright Deskillz Games. All Rights Reserved.

#include "DeskillzAuthController.h"
#include "DeskillzAuth.h"
#include "DeskillzSDK.h"
#include "DeskillzConfig.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// Static instance
UDeskillzAuthController* UDeskillzAuthController::Instance = nullptr;

UDeskillzAuthController::UDeskillzAuthController()
{
    // Default constructor
}

UDeskillzAuthController* UDeskillzAuthController::Get(const UObject* WorldContextObject)
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzAuthController>();
        Instance->AddToRoot(); // Prevent garbage collection
    }

    if (WorldContextObject)
    {
        Instance->WorldContext = WorldContextObject->GetWorld();
    }

    return Instance;
}

void UDeskillzAuthController::Initialize()
{
    if (bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Deskillz] AuthController initializing..."));

    // Load level names from config
    UDeskillzConfig* Config = UDeskillzConfig::Get();
    if (Config)
    {
        if (!Config->AuthLevelName.IsEmpty())
            AuthLevelName = Config->AuthLevelName;
        if (!Config->LobbyLevelName.IsEmpty())
            LobbyLevelName = Config->LobbyLevelName;
        if (!Config->GameLevelName.IsEmpty())
            GameLevelName = Config->GameLevelName;
        if (!Config->LoadingLevelName.IsEmpty())
            LoadingLevelName = Config->LoadingLevelName;
    }

    // Initialize auth system
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->Initialize();

        // Bind to auth events
        Auth->OnLoginSuccess.AddDynamic(this, &UDeskillzAuthController::HandleLoginSuccess);
        Auth->OnLogout.AddDynamic(this, &UDeskillzAuthController::HandleLogout);
        Auth->OnAuthError.AddDynamic(this, &UDeskillzAuthController::HandleAuthError);
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("[Deskillz] AuthController ready"));
}

void UDeskillzAuthController::Shutdown()
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Deskillz] AuthController shutting down..."));

    // Unbind from auth events
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->OnLoginSuccess.RemoveDynamic(this, &UDeskillzAuthController::HandleLoginSuccess);
        Auth->OnLogout.RemoveDynamic(this, &UDeskillzAuthController::HandleLogout);
        Auth->OnAuthError.RemoveDynamic(this, &UDeskillzAuthController::HandleAuthError);
    }

    bIsInitialized = false;
    bHasPendingMatch = false;
    PendingMatchData = FDeskillzMatchLaunchData();

    UE_LOG(LogTemp, Log, TEXT("[Deskillz] AuthController shutdown complete"));
}

// ============================================================================
// Navigation
// ============================================================================

void UDeskillzAuthController::GoToAuth()
{
    NavigateToLevel(AuthLevelName);
}

void UDeskillzAuthController::GoToLobby()
{
    if (!IsAuthenticated())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Deskillz] Cannot go to lobby - not authenticated"));
        GoToAuth();
        return;
    }

    NavigateToLevel(LobbyLevelName);
}

void UDeskillzAuthController::GoToGame()
{
    NavigateToLevel(GameLevelName);
}

void UDeskillzAuthController::LaunchMatch(const FDeskillzMatchLaunchData& MatchData)
{
    if (!IsAuthenticated())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Deskillz] Cannot launch match - not authenticated"));
        bHasPendingMatch = true;
        PendingMatchData = MatchData;
        GoToAuth();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Deskillz] Launching match: %s"), *MatchData.MatchId);

    // Store match data for game level
    // SDK will pick this up when game level loads

    NavigateToLevel(GameLevelName);
}

void UDeskillzAuthController::ReturnToLobby()
{
    GoToLobby();
}

void UDeskillzAuthController::LogoutAndGoToAuth()
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->Logout();
    }
    GoToAuth();
}

void UDeskillzAuthController::DetermineInitialNavigation()
{
    UE_LOG(LogTemp, Log, TEXT("[Deskillz] Determining initial navigation..."));

    // Check for pending match from deep link
    if (bHasPendingMatch && PendingMatchData.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("[Deskillz] Pending match found: %s"), *PendingMatchData.MatchId);

        if (IsAuthenticated())
        {
            LaunchMatch(PendingMatchData);
            bHasPendingMatch = false;
        }
        else
        {
            GoToAuth();
        }
        return;
    }

    // No pending match - check auth state
    if (IsAuthenticated())
    {
        UE_LOG(LogTemp, Log, TEXT("[Deskillz] User authenticated - going to lobby"));
        GoToLobby();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[Deskillz] User not authenticated - going to auth"));
        GoToAuth();
    }
}

void UDeskillzAuthController::NavigateToLevel(const FString& LevelName)
{
    if (bIsTransitioning)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Deskillz] Already transitioning, queuing: %s"), *LevelName);
        PendingDestination = LevelName;
        return;
    }

    if (LevelName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[Deskillz] Cannot navigate to empty level name"));
        return;
    }

    FString CurrentLevel = GetCurrentLevelName();
    if (CurrentLevel == LevelName)
    {
        UE_LOG(LogTemp, Log, TEXT("[Deskillz] Already in level: %s"), *LevelName);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Deskillz] Navigating to: %s"), *LevelName);

    bIsTransitioning = true;
    OnLevelChanging.Broadcast(LevelName);

    // Open the level
    UWorld* World = WorldContext.Get();
    if (World)
    {
        UGameplayStatics::OpenLevel(World, FName(*LevelName));
    }
    else if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* FirstWorld = GEngine->GetWorldContexts()[0].World();
        if (FirstWorld)
        {
            UGameplayStatics::OpenLevel(FirstWorld, FName(*LevelName));
        }
    }

    // Note: OnLevelLoaded will be called when level finishes loading
    // For now, we'll simulate completion after a brief delay
    bIsTransitioning = false;
    OnLevelChanged.Broadcast(LevelName);

    ProcessPendingNavigation();
}

void UDeskillzAuthController::OnLevelLoaded()
{
    bIsTransitioning = false;

    FString CurrentLevel = GetCurrentLevelName();
    OnLevelChanged.Broadcast(CurrentLevel);

    ProcessPendingNavigation();
}

void UDeskillzAuthController::ProcessPendingNavigation()
{
    if (!PendingDestination.IsEmpty())
    {
        FString Dest = PendingDestination;
        PendingDestination.Empty();
        NavigateToLevel(Dest);
    }
}

// ============================================================================
// State
// ============================================================================

bool UDeskillzAuthController::IsAuthenticated() const
{
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    return Auth ? Auth->IsAuthenticated() : false;
}

FString UDeskillzAuthController::GetCurrentLevelName() const
{
    UWorld* World = WorldContext.Get();
    if (World)
    {
        return World->GetMapName();
    }

    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* FirstWorld = GEngine->GetWorldContexts()[0].World();
        if (FirstWorld)
        {
            return FirstWorld->GetMapName();
        }
    }

    return TEXT("");
}

// ============================================================================
// Configuration
// ============================================================================

void UDeskillzAuthController::SetLevelNames(const FString& AuthLevel, const FString& LobbyLevel,
                                            const FString& GameLevel, const FString& LoadingLevel)
{
    if (!AuthLevel.IsEmpty())
        AuthLevelName = AuthLevel;
    if (!LobbyLevel.IsEmpty())
        LobbyLevelName = LobbyLevel;
    if (!GameLevel.IsEmpty())
        GameLevelName = GameLevel;
    if (!LoadingLevel.IsEmpty())
        LoadingLevelName = LoadingLevel;
}

void UDeskillzAuthController::GetLevelNames(FString& OutAuthLevel, FString& OutLobbyLevel,
                                            FString& OutGameLevel, FString& OutLoadingLevel) const
{
    OutAuthLevel = AuthLevelName;
    OutLobbyLevel = LobbyLevelName;
    OutGameLevel = GameLevelName;
    OutLoadingLevel = LoadingLevelName;
}

// ============================================================================
// Event Handlers
// ============================================================================

void UDeskillzAuthController::HandleLoginSuccess(const FDeskillzAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("[Deskillz] Login success: %s"), *User.Username);

    OnAuthFlowComplete.Broadcast(User);

    // Check for pending match
    if (bHasPendingMatch && PendingMatchData.IsValid())
    {
        FDeskillzMatchLaunchData MatchData = PendingMatchData;
        bHasPendingMatch = false;
        PendingMatchData = FDeskillzMatchLaunchData();
        LaunchMatch(MatchData);
    }
    else
    {
        GoToLobby();
    }
}

void UDeskillzAuthController::HandleLogout()
{
    UE_LOG(LogTemp, Log, TEXT("[Deskillz] Logout - returning to auth"));

    bHasPendingMatch = false;
    PendingMatchData = FDeskillzMatchLaunchData();

    OnLogoutComplete.Broadcast();

    GoToAuth();
}

void UDeskillzAuthController::HandleAuthError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("[Deskillz] Auth error: %s"), *Error);
    // Stay on auth level, UI will show error
}

void UDeskillzAuthController::HandleMatchLaunchReceived(const FDeskillzMatchLaunchData& MatchData)
{
    UE_LOG(LogTemp, Log, TEXT("[Deskillz] Deep link match received: %s"), *MatchData.MatchId);

    if (IsAuthenticated())
    {
        LaunchMatch(MatchData);
    }
    else
    {
        bHasPendingMatch = true;
        PendingMatchData = MatchData;
        GoToAuth();
    }
}

void UDeskillzAuthController::HandleNormalLaunch()
{
    UE_LOG(LogTemp, Log, TEXT("[Deskillz] Normal launch (no deep link)"));
    // Navigation already handled by DetermineInitialNavigation
}