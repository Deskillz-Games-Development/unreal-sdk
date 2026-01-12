# Deskillz Unreal Engine SDK

<p align="center">
  <img src="https://deskillz.games/logo.png" alt="Deskillz.Games" width="200"/>
</p>

<p align="center">
  <strong>Integrate competitive tournaments into your Unreal Engine games</strong>
</p>

<p align="center">
  <a href="https://github.com/Deskillz-Games-Development/unreal-sdk/releases"><img src="https://img.shields.io/badge/version-2.6.0-blue.svg" alt="Version"></a>
  <a href="https://www.unrealengine.com"><img src="https://img.shields.io/badge/unreal-4.27%2B%20%7C%205.0%2B-black.svg" alt="Unreal"></a>
  <a href="https://github.com/Deskillz-Games-Development/unreal-sdk/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License"></a>
</p>

<p align="center">
  <a href="#installation">Installation</a> â€¢
  <a href="#quick-start">Quick Start</a> â€¢
  <a href="#features">Features</a> â€¢
  <a href="#auto-updater">Auto-Updater</a> â€¢
  <a href="#private-rooms">Private Rooms</a> â€¢
  <a href="#host-system">Host System</a> â€¢
  <a href="#social-games">Social Games</a> â€¢
  <a href="#host-spectator-mode">Host Spectator Mode</a> [B]
  <a href="#navigation-deep-links">Navigation Links</a> â€¢
  <a href="#documentation">Documentation</a> â€¢
  <a href="#support">Support</a>
</p>

---

## Overview

The Deskillz Unreal Engine SDK enables game developers to integrate their UE games with the Deskillz.Games competitive gaming platform. Players can compete in skill-based tournaments, create private rooms to play with friends, host social games with rake systems, and win cryptocurrency prizes (BTC, ETH, SOL, XRP, BNB, USDT, USDC).

### How It Works (Global Lobby Architecture)

```
+------------------------------------------------------------------+
|                    PLAYER JOURNEY                                |
+------------------------------------------------------------------+
|                                                                  |
|  1. Player opens Deskillz.Games website/app                      |
|              |                                                   |
|  2. Player browses Global Lobby                                  |
|     - Select game                                                |
|     - Choose tournament/match type                               |
|     - Join matchmaking queue OR private room                     |
|              |                                                   |
|  3. Match found -> Deep link sent to your game                   |
|     deskillz://launch?matchId=abc123&token=xyz...                |
|              |                                                   |
|  4. Your game app opens via deep link                            |
|              |                                                   |
|  5. SDK receives match data -> Start gameplay                    |
|              |                                                   |
|  6. Player plays -> Score submitted -> Results shown             |
|                                                                  |
+------------------------------------------------------------------+
```

**Key Point:** Matchmaking happens in the Deskillz platform, NOT in your game. Your SDK just needs to:
1. Handle deep links (navigation + match launch)
2. Start the match
3. Submit scores securely

## Requirements

- **Unreal Engine:** 4.27+ or 5.0+
- **Platforms:** iOS 12+, Android 5.0+ (API 21)
- **Build Tools:** Xcode 14+ (iOS), Android NDK (Android)
- **C++ Standard:** C++17 or later

## Installation

### Option 1: Git Clone (Recommended)

```bash
# Navigate to your project's Plugins folder
cd YourProject/Plugins

# Clone the SDK
git clone https://github.com/Deskillz-Games-Development/unreal-sdk.git DeskillzSDK

# Regenerate project files
```

### Option 2: Download ZIP

1. Download latest release from [GitHub Releases](https://github.com/Deskillz-Games-Development/unreal-sdk/releases)
2. Extract to `YourProject/Plugins/DeskillzSDK/`
3. Regenerate project files

### Option 3: Git Submodule

```bash
cd YourProject
git submodule add https://github.com/Deskillz-Games-Development/unreal-sdk.git Plugins/DeskillzSDK
```

### Setup

1. Enable the plugin in **Edit -> Plugins -> Deskillz SDK**
2. Add to your `.Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "DeskillzSDK" });
```

3. Create a `DeskillzConfig` data asset in your Content folder
4. Enter your API Key and Game ID (get from [deskillz.games/developer](https://deskillz.games/developer))

## Quick Start

### 1. Initialize the SDK

```cpp
#include "DeskillzSDK.h"
#include "Lobby/DeepLinkHandler.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize SDK
    UDeskillzSDK::Initialize();
    
    // Initialize Deep Link Handler
    UDeepLinkHandler* DeepLinkHandler = UDeepLinkHandler::Get();
    DeepLinkHandler->Initialize();
    
    // Bind navigation events (NEW in v2.0)
    DeepLinkHandler->OnNavigationReceived.AddDynamic(
        this, &AMyGameMode::HandleNavigation);
    
    // Bind match launch events
    DeepLinkHandler->OnMatchLaunchReceived.AddDynamic(
        this, &AMyGameMode::HandleMatchLaunch);
    DeepLinkHandler->OnMatchReady.AddDynamic(
        this, &AMyGameMode::OnMatchReady);
    DeepLinkHandler->OnValidationFailed.AddDynamic(
        this, &AMyGameMode::OnValidationFailed);
    
    // Check for updates (NEW in v2.3)
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    Updater->SetCurrentVersion(GetGameVersion(), GetVersionCode());
    Updater->CheckForUpdates();
    
    // Process pending deep links (cold start)
    if (DeepLinkHandler->HasPendingDeepLink())
    {
        DeepLinkHandler->ProcessPendingDeepLinks();
    }
}

void AMyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UDeepLinkHandler* DeepLinkHandler = UDeepLinkHandler::Get();
    DeepLinkHandler->OnNavigationReceived.RemoveDynamic(
        this, &AMyGameMode::HandleNavigation);
    DeepLinkHandler->OnMatchLaunchReceived.RemoveDynamic(
        this, &AMyGameMode::HandleMatchLaunch);
    
    Super::EndPlay(EndPlayReason);
}
```

### 2. Handle Navigation Deep Links (NEW in v2.0)

```cpp
void AMyGameMode::HandleNavigation(
    EDeskillzNavigationAction Action,
    const TMap<FString, FString>& Parameters)
{
    switch (Action)
    {
        case EDeskillzNavigationAction::Tournaments:
            UIManager->ShowTournaments();
            break;
            
        case EDeskillzNavigationAction::Wallet:
            UIManager->ShowWallet();
            break;
            
        case EDeskillzNavigationAction::Profile:
            UIManager->ShowProfile();
            break;
            
        case EDeskillzNavigationAction::Game:
            if (const FString* GameId = Parameters.Find(TEXT("id")))
            {
                UIManager->ShowGameDetails(*GameId);
            }
            break;
            
        case EDeskillzNavigationAction::Settings:
            UIManager->ShowSettings();
            break;
    }
}
```

### 3. Handle Match Launch Deep Links

```cpp
// Simple handler - just matchId and token
void AMyGameMode::HandleMatchLaunch(
    const FString& MatchId,
    const FString& AuthToken)
{
    // Store auth token for API calls
    UDeskillzApi::SetAuthToken(AuthToken);
    
    // Load match level
    UGameplayStatics::OpenLevel(this, TEXT("MatchLevel"));
    
    UE_LOG(LogDeskillz, Log, TEXT("Launching match: %s"), *MatchId);
}

// Full match data handler
void AMyGameMode::OnMatchReady(const FMatchLaunchData& Data)
{
    UE_LOG(LogDeskillz, Log, TEXT("Match ready: %s"), *Data.MatchId);
    UE_LOG(LogDeskillz, Log, TEXT("Opponent: %s (Rating: %d)"),
        *Data.OpponentName, Data.OpponentRating);
    UE_LOG(LogDeskillz, Log, TEXT("Entry Fee: %.2f %s"),
        Data.EntryFee, *Data.Currency);
    UE_LOG(LogDeskillz, Log, TEXT("Duration: %ds"), Data.Duration);
    
    // Store match data
    CurrentMatchData = Data;
    
    // Load game level
    UGameplayStatics::OpenLevel(this, TEXT("GameLevel"));
}

void AMyGameMode::OnValidationFailed(
    const FString& Reason,
    const FMatchLaunchData& Data)
{
    UE_LOG(LogDeskillz, Error, TEXT("Match validation failed: %s"), *Reason);
    // Show error and return to main menu
    ShowErrorDialog(Reason);
}
```

### 4. Submit Score

```cpp
// When player finishes the game
void AMyGameMode::OnGameComplete(int32 FinalScore, float PlayDuration)
{
    // Score is automatically encrypted with HMAC-SHA256
    UDeskillzSDK::SubmitScore(
        FinalScore,
        PlayDuration,
        FOnScoreSubmitted::CreateUObject(this, &AMyGameMode::OnScoreSubmitted)
    );
}

void AMyGameMode::OnScoreSubmitted(bool bSuccess, const FString& Message)
{
    if (bSuccess)
    {
        UE_LOG(LogDeskillz, Log, TEXT("Score submitted successfully!"));
        UDeskillzSDK::EndMatch();
    }
    else
    {
        UE_LOG(LogDeskillz, Error, TEXT("Score submission failed: %s"), *Message);
    }
}
```

## Features

| Feature | Description |
|---------|-------------|
| [TROPHY] **Tournaments** | Async and real-time competitive matches |
| [COIN] **Crypto Prizes** | BTC, ETH, SOL, XRP, BNB, USDT, USDC |
| [USERS] **Private Rooms** | Play with friends using room codes |
| [HOST] **Host System** | 6-tier host program with revenue sharing (NEW v2.6) |
| [CARDS] **Social Games** | Rake-based games with buy-ins (NEW v2.6) |
| [EYE] **Host Spectator Mode** | Hosts monitor their social rooms (NEW v2.6) |
| [LIGHTNING] **Real-time Sync** | Sub-100ms latency multiplayer |
| [SHIELD] **Anti-Cheat** | Score encryption and validation |
| [DOWNLOAD] **Auto-Updater** | Forced and optional app updates |
| [ROBOT] **NPC Opponents** | AI players for off-peak hours |
| [CHART] **Analytics** | Built-in telemetry and insights |
| [OFFLINE] **Offline Support** | Automatic score caching and retry |
| [LOCK] **Score Encryption** | HMAC-SHA256 signed submission |
| [BLUEPRINT] **Blueprint Support** | Full Blueprint integration |

---

## Host System (NEW in v2.6)

The Host System enables users to become verified hosts who can create and manage private rooms, earning revenue through the rake system.

### Host Tiers

| Tier | Name | Revenue Share | Requirements |
|------|------|---------------|--------------|
| 0 | Starter | 50% | None |
| 1 | Bronze | 55% | 10 rooms, $100 earned, 3.5 rating |
| 2 | Silver | 60% | 50 rooms, $500 earned, 4.0 rating |
| 3 | Gold | 65% | 150 rooms, $2,000 earned, 4.3 rating |
| 4 | Platinum | 70% | 500 rooms, $10,000 earned, 4.5 rating |
| 5 | Diamond | 75% | 1,000 rooms, $50,000 earned, 4.7 rating |

### Host Manager Usage

```cpp
#include "Host/DeskillzHostManager.h"

void AMyGameMode::InitializeHostSystem()
{
    UDeskillzHostManager* HostManager = UDeskillzHostManager::Get();
    HostManager->Initialize(UserId);
    
    // Bind events
    HostManager->OnHostRegistrationComplete.AddDynamic(
        this, &AMyGameMode::OnHostRegistered);
    HostManager->OnTierUpgraded.AddDynamic(
        this, &AMyGameMode::OnTierUpgraded);
    HostManager->OnEarningsUpdated.AddDynamic(
        this, &AMyGameMode::OnEarningsUpdated);
}

void AMyGameMode::RegisterAsHost()
{
    FDeskillzHostRegistrationRequest Request;
    Request.DisplayName = TEXT("ProHost");
    Request.Email = TEXT("host@example.com");
    Request.bAcceptedTerms = true;
    
    UDeskillzHostManager::Get()->RegisterAsHost(Request);
}

void AMyGameMode::OnHostRegistered(bool bSuccess, const FString& Error)
{
    if (bSuccess)
    {
        UE_LOG(LogDeskillz, Log, TEXT("Registered as host!"));
    }
}

void AMyGameMode::OnTierUpgraded(int32 OldTier, int32 NewTier)
{
    UE_LOG(LogDeskillz, Log, TEXT("Upgraded from tier %d to %d!"), OldTier, NewTier);
}

void AMyGameMode::CreateHostRoom()
{
    FDeskillzCreateRoomRequest Request;
    Request.RoomName = TEXT("Pro Poker Night");
    Request.GameId = TEXT("poker-texas-holdem");
    Request.MaxPlayers = 8;
    Request.MinBuyIn = 10.0f;
    Request.MaxBuyIn = 200.0f;
    Request.PointValue = 0.01f;
    Request.RakePercent = 5.0f;
    Request.RakeCap = 3.0f;
    
    UDeskillzHostManager::Get()->CreateRoom(Request);
}

void AMyGameMode::CheckEarnings()
{
    UDeskillzHostManager::Get()->FetchEarnings(EDeskillzEarningsPeriod::Month);
}

void AMyGameMode::OnEarningsUpdated(const FDeskillzHostEarnings& Earnings)
{
    UE_LOG(LogDeskillz, Log, TEXT("Total: $%.2f"), Earnings.TotalEarnings);
    UE_LOG(LogDeskillz, Log, TEXT("Rake: $%.2f"), Earnings.RakeEarnings);
    UE_LOG(LogDeskillz, Log, TEXT("Available: $%.2f"), Earnings.AvailableBalance);
}
```

---

## Social Games (NEW in v2.6)

Social games enable real-money gameplay with rake collection, buy-ins, rebuys, and cash-outs.

### Social Game Manager Usage

```cpp
#include "Social/DeskillzSocialGameManager.h"
#include "Social/DeskillzRakeCalculator.h"
#include "Social/DeskillzBuyInManager.h"

void AMyGameMode::StartSocialSession()
{
    FDeskillzSocialSessionConfig Config;
    Config.RoomId = TEXT("room-123");
    Config.GameId = TEXT("poker-texas-holdem");
    Config.HostId = TEXT("host-456");
    Config.PointValue = 0.01f;
    Config.RakePercent = 5.0f;
    Config.RakeCap = 3.0f;
    Config.MinBuyIn = 10.0f;
    Config.MaxBuyIn = 200.0f;
    Config.bAllowRebuy = true;
    Config.RebuyPeriodRounds = 5;
    
    UDeskillzSocialGameManager* Manager = UDeskillzSocialGameManager::Get();
    Manager->OnSessionStarted.AddDynamic(this, &AMyGameMode::OnSessionStarted);
    Manager->OnRoundEnded.AddDynamic(this, &AMyGameMode::OnRoundEnded);
    Manager->StartSession(Config);
}

void AMyGameMode::AddPlayerToSession(const FString& PlayerId, float BuyInAmount)
{
    UDeskillzSocialGameManager::Get()->AddPlayer(PlayerId, BuyInAmount);
}

void AMyGameMode::ProcessRound(const FString& WinnerId, float PotAmount)
{
    UDeskillzSocialGameManager* Manager = UDeskillzSocialGameManager::Get();
    Manager->StartRound();
    // ... gameplay ...
    Manager->EndRound(WinnerId, PotAmount);
}

void AMyGameMode::OnRoundEnded(
    int32 RoundNumber,
    const FString& WinnerId,
    float WinnerPayout,
    float RakeAmount)
{
    UE_LOG(LogDeskillz, Log, TEXT("Round %d - Winner: %s, Payout: $%.2f, Rake: $%.2f"),
        RoundNumber, *WinnerId, WinnerPayout, RakeAmount);
}
```

### Rake Calculator

```cpp
#include "Social/DeskillzRakeCalculator.h"

void AMyGameMode::CalculateRake(float PotAmount)
{
    UDeskillzRakeCalculator* Calculator = UDeskillzRakeCalculator::Get();
    
    FDeskillzRakeBreakdown Breakdown = Calculator->GetRakeBreakdown(PotAmount);
    
    UE_LOG(LogDeskillz, Log, TEXT("Pot: $%.2f"), Breakdown.PotAmount);
    UE_LOG(LogDeskillz, Log, TEXT("Rake: $%.2f"), Breakdown.TotalRake);
    UE_LOG(LogDeskillz, Log, TEXT("Winner Payout: $%.2f"), Breakdown.WinnerPayout);
    UE_LOG(LogDeskillz, Log, TEXT("Host Share: $%.2f"), Breakdown.HostShare);
    UE_LOG(LogDeskillz, Log, TEXT("Platform Share: $%.2f"), Breakdown.PlatformShare);
}

float AMyGameMode::EstimateHostEarnings()
{
    return UDeskillzRakeCalculator::Get()->EstimateHostEarnings(
        100.0f,  // Expected pot size
        50,      // Expected hands
        3        // Host tier
    );
}
```

### Buy-In Manager

```cpp
#include "Social/DeskillzBuyInManager.h"

void AMyGameMode::ValidateBuyIn(const FString& PlayerId, float Amount)
{
    UDeskillzBuyInManager* Manager = UDeskillzBuyInManager::Get();
    
    FDeskillzBuyInValidationResult Result = Manager->GetValidationResult(
        PlayerId,
        Amount,
        false  // bIsRebuy
    );
    
    if (Result.bIsValid)
    {
        UE_LOG(LogDeskillz, Log, TEXT("Chips: %d"), Result.ChipsReceived);
        Manager->ProcessBuyIn(PlayerId, Amount);
    }
    else
    {
        UE_LOG(LogDeskillz, Error, TEXT("Invalid: %s"), *Result.ErrorMessage);
    }
}

TArray<float> AMyGameMode::GetPresetAmounts()
{
    return UDeskillzBuyInManager::Get()->GetPresetBuyInAmounts();
    // Returns: [MinBuyIn, 100x, 200x, MaxBuyIn]
}
```

---

## Host Spectator Mode (NEW in v2.6)

Allow hosts to monitor their private social rooms without participating.

> **Important:** This is a **host-only** feature. Only the creator of a private social room can spectate it. General public spectating is not available. Hosts can see the game board and scores but **NOT player hands** (anti-cheat protection).

### Host Spectator Limitations

| Can See | Cannot See |
|---------|------------|
| Game board/table state | Player hands/tiles |
| Current scores/points | Hidden cards |
| Player turn indicator | Private player info |
| Chat messages | - |
| Round results | - |

### Host Spectator Manager Usage

```cpp
#include "Host/DeskillzHostSpectatorManager.h"

void AMyGameMode::InitializeHostSpectatorMode()
{
    // Host must be authenticated first
    UDeskillzHostSpectatorManager* Manager = UDeskillzHostSpectatorManager::Get();
    Manager->Initialize();
    
    // Bind events for YOUR rooms
    Manager->OnRoomsFetched.AddDynamic(this, &AMyGameMode::OnHostRoomsFetched);
    Manager->OnGameStateUpdated.AddDynamic(this, &AMyGameMode::OnGameStateUpdated);
    Manager->OnRoundEnded.AddDynamic(this, &AMyGameMode::OnRoundEnded);
    Manager->OnRoomSwitched.AddDynamic(this, &AMyGameMode::OnRoomSwitched);
}

void AMyGameMode::FetchMyHostRooms()
{
    FDeskillzHostRoomFilter Filter;
    Filter.GameCategory = EDeskillzGameCategory::Social; // Social rooms only
    Filter.bIsActive = true;
    
    UDeskillzHostSpectatorManager::Get()->FetchHostRooms(Filter);
}

void AMyGameMode::OnHostRoomsFetched(const TArray<FDeskillzHostRoomInfo>& Rooms)
{
    for (const FDeskillzHostRoomInfo& Room : Rooms)
    {
        UE_LOG(LogDeskillz, Log, TEXT("%s: %d/%d players"),
            *Room.RoomName, Room.CurrentPlayers, Room.MaxPlayers);
    }
}

void AMyGameMode::SpectateMyRoom(const FString& RoomId)
{
    // Join YOUR room (see board, NOT hands)
    UDeskillzHostSpectatorManager::Get()->SpectateRoom(RoomId);
}

void AMyGameMode::SwitchToAnotherRoom(const FString& OtherRoomId)
{
    // Switch between YOUR rooms (multi-room hosting)
    UDeskillzHostSpectatorManager::Get()->SwitchRoom(OtherRoomId);
}

void AMyGameMode::OnGameStateUpdated(const FDeskillzHostSpectatorState& State)
{
    // Update board (NO hands visible - anti-cheat)
    UE_LOG(LogDeskillz, Log, TEXT("Round %d - Scores updated"),
        State.CurrentRound);
}

void AMyGameMode::OnRoundEnded(int32 RoundNumber, const FString& WinnerId)
{
    UE_LOG(LogDeskillz, Log, TEXT("Round %d winner: %s"),
        RoundNumber, *WinnerId);
}
```

---

## Private Rooms

Create custom rooms for friends or public tournaments.

### Room Creation

```cpp
#include "Rooms/DeskillzRooms.h"

void AMyGameMode::CreatePrivateRoom()
{
    FDeskillzRoomConfig Config;
    Config.Name = TEXT("Friday Night Tournament");
    Config.GameId = TEXT("your-game-id");
    Config.MaxPlayers = 8;
    Config.EntryFee = 5.0f;
    Config.Currency = TEXT("USDT");
    Config.bIsPrivate = true;
    
    UDeskillzRooms::Get()->CreateRoom(
        Config,
        FOnRoomCreated::CreateUObject(this, &AMyGameMode::OnRoomCreated)
    );
}

void AMyGameMode::OnRoomCreated(const FDeskillzRoom& Room)
{
    UE_LOG(LogDeskillz, Log, TEXT("Room created! Code: %s"), *Room.RoomCode);
    // Share Room.RoomCode with friends
}

// Join via code
void AMyGameMode::JoinRoomByCode(const FString& Code)
{
    UDeskillzRooms::Get()->JoinRoomByCode(
        Code,
        FOnRoomJoined::CreateUObject(this, &AMyGameMode::OnRoomJoined)
    );
}

// Browse public rooms
void AMyGameMode::BrowsePublicRooms()
{
    UDeskillzRooms::Get()->GetPublicRooms(
        GameId,
        FOnRoomsLoaded::CreateUObject(this, &AMyGameMode::OnRoomsLoaded)
    );
}
```

### Room Events

```cpp
void AMyGameMode::BindRoomEvents()
{
    UDeskillzRooms* Rooms = UDeskillzRooms::Get();
    
    Rooms->OnPlayerJoined.AddDynamic(this, &AMyGameMode::OnPlayerJoined);
    Rooms->OnPlayerLeft.AddDynamic(this, &AMyGameMode::OnPlayerLeft);
    Rooms->OnPlayerReady.AddDynamic(this, &AMyGameMode::OnPlayerReady);
    Rooms->OnCountdownStarted.AddDynamic(this, &AMyGameMode::OnCountdownStarted);
    Rooms->OnMatchStarting.AddDynamic(this, &AMyGameMode::OnMatchStarting);
    Rooms->OnChatMessage.AddDynamic(this, &AMyGameMode::OnChatMessage);
}

void AMyGameMode::OnPlayerJoined(const FDeskillzPlayer& Player)
{
    UE_LOG(LogDeskillz, Log, TEXT("%s joined the room"), *Player.Name);
}

// Host controls
void AMyGameMode::HostStartMatch()
{
    UDeskillzRooms::Get()->StartMatch();  // Host only
}

void AMyGameMode::HostCancelRoom()
{
    UDeskillzRooms::Get()->CancelRoom();  // Host only
}

void AMyGameMode::HostKickPlayer(const FString& PlayerId)
{
    UDeskillzRooms::Get()->KickPlayer(PlayerId);  // Host only
}
```

---

## Auto-Updater

Automatically check for and prompt app updates.

### Basic Setup

```cpp
#include "Core/DeskillzUpdater.h"

void AMyGameMode::InitializeUpdater()
{
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    Updater->SetCurrentVersion(TEXT("1.0.0"), 10000);
    
    // Bind events
    Updater->OnUpdateAvailable.AddDynamic(
        this, &AMyGameMode::HandleOptionalUpdate);
    Updater->OnForceUpdateRequired.AddDynamic(
        this, &AMyGameMode::HandleForcedUpdate);
    Updater->OnNoUpdateNeeded.AddDynamic(
        this, &AMyGameMode::HandleNoUpdate);
    Updater->OnUpdateCheckFailed.AddDynamic(
        this, &AMyGameMode::HandleCheckFailed);
    
    // Check for updates
    Updater->CheckForUpdates();
}

void AMyGameMode::HandleOptionalUpdate(const FUpdateInfo& Info)
{
    UE_LOG(LogDeskillz, Log, TEXT("Update available: %s"), *Info.LatestVersion);
    UE_LOG(LogDeskillz, Log, TEXT("Size: %s"), *Info.FileSizeFormatted);
    // Show optional update dialog
}

void AMyGameMode::HandleForcedUpdate(const FUpdateInfo& Info)
{
    // Block app until user updates
    ShowForcedUpdateDialog(Info);
}

void AMyGameMode::HandleNoUpdate()
{
    UE_LOG(LogDeskillz, Log, TEXT("App is up to date"));
}

void AMyGameMode::HandleCheckFailed(const FString& Error)
{
    UE_LOG(LogDeskillz, Warning, TEXT("Update check failed: %s"), *Error);
}
```

### UpdateInfo Properties

| Property | Type | Description |
|----------|------|-------------|
| LatestVersion | FString | Version string (e.g., "1.2.0") |
| VersionCode | int32 | Integer version (e.g., 10200) |
| bUpdateAvailable | bool | Whether update exists |
| bIsForced | bool | Whether update is required |
| DownloadUrl | FString | APK download URL |
| FileSize | int64 | File size in bytes |
| FileSizeFormatted | FString | Human-readable (e.g., "52.4 MB") |
| ReleaseNotes | FString | Changelog text |

---

## Navigation Deep Links

Handle platform navigation requests:

| URL | Action | Description |
|-----|--------|-------------|
| `deskillz://tournaments` | Tournaments | Show tournament list |
| `deskillz://wallet` | Wallet | Show wallet/balance |
| `deskillz://profile` | Profile | Show user profile |
| `deskillz://settings` | Settings | Show settings |
| `deskillz://game?id=X` | Game | Show specific game |
| `deskillz://leaderboard?id=X` | Leaderboard | Show leaderboard |

---

## SDK Structure

```
deskillz-unreal-sdk/
+-- Source/
|   +-- DeskillzSDK/
|   |   +-- Public/
|   |   |   +-- Core/
|   |   |   |   +-- DeskillzSDK.h
|   |   |   |   +-- DeskillzConfig.h
|   |   |   |   +-- DeskillzTypes.h
|   |   |   |   +-- DeskillzEvents.h
|   |   |   |   +-- DeskillzUpdater.h
|   |   |   +-- Match/
|   |   |   |   +-- MatchController.h
|   |   |   |   +-- MatchTimer.h
|   |   |   +-- Security/
|   |   |   |   +-- ScoreManager.h
|   |   |   |   +-- ScoreEncryption.h
|   |   |   +-- Rooms/
|   |   |   |   +-- DeskillzRooms.h
|   |   |   |   +-- DeskillzRoomTypes.h
|   |   |   |   +-- DeskillzRoomClient.h
|   |   |   +-- Host/                              # NEW in v2.6
|   |   |   |   +-- DeskillzHostManager.h          # Host registration, tiers
|   |   |   |   +-- DeskillzHostTypes.h            # Host data structures
|   |   |   +-- Social/                            # NEW in v2.6
|   |   |   |   +-- DeskillzSocialGameManager.h    # Session management
|   |   |   |   +-- DeskillzRakeCalculator.h       # Rake calculation
|   |   |   |   +-- DeskillzBuyInManager.h         # Buy-in/rebuy/cashout
|   |   |   |   +-- DeskillzSocialTypes.h          # Social data structures
|   |   |   +-- Spectator/                         # NEW in v2.6 (Host-only)
|   |   |   |   +-- DeskillzHostSpectatorManager.h # Host spectator mode
|   |   |   |   +-- DeskillzHostSpectatorTypes.h   # Host spectator data
|   |   |   +-- Widgets/
|   |   |   |   +-- Rooms/
|   |   |   |   |   +-- DeskillzRoomListWidget.h
|   |   |   |   |   +-- DeskillzCreateRoomWidget.h
|   |   |   |   |   +-- DeskillzJoinRoomWidget.h
|   |   |   |   |   +-- DeskillzRoomLobbyWidget.h
|   |   |   |   +-- Host/                          # NEW in v2.6
|   |   |   |   |   +-- DeskillzHostDashboardWidget.h
|   |   |   |   |   +-- DeskillzHostProfileCardWidget.h
|   |   |   |   |   +-- DeskillzHostTierProgressWidget.h
|   |   |   |   |   +-- DeskillzHostBadgeGridWidget.h
|   |   |   |   |   +-- DeskillzHostEarningsWidget.h
|   |   |   |   +-- Social/                        # NEW in v2.6
|   |   |   |   |   +-- DeskillzBuyInModalWidget.h
|   |   |   |   |   +-- DeskillzRebuyModalWidget.h
|   |   |   |   |   +-- DeskillzCashOutModalWidget.h
|   |   |   |   |   +-- DeskillzSocialGameSettingsWidget.h
|   |   |   |   |   +-- DeskillzTurnTimerWidget.h
|   |   |   |   |   +-- DeskillzPauseRequestWidget.h
|   |   |   |   +-- Spectator/                     # NEW in v2.6 (Host-only)
|   |   |   |   |   +-- DeskillzHostSpectatorViewWidget.h
|   |   |   |   |   +-- DeskillzHostScorePanelWidget.h
|   |   |   |   |   +-- DeskillzHostRoomSwitcherWidget.h
|   |   |   +-- Lobby/
|   |   |   |   +-- DeepLinkHandler.h
|   |   |   |   +-- DeskillzBridge.h
|   |   |   +-- Multiplayer/
|   |   |   |   +-- SyncManager.h
|   |   |   +-- API/
|   |   |       +-- DeskillzApiService.h
|   |   |       +-- DeskillzWebSocket.h
|   |   +-- Private/
|   |   |   +-- Core/
|   |   |   +-- Match/
|   |   |   +-- Security/
|   |   |   +-- Rooms/
|   |   |   +-- Host/                              # NEW in v2.6
|   |   |   |   +-- DeskillzHostManager.cpp
|   |   |   +-- Social/                            # NEW in v2.6
|   |   |   |   +-- DeskillzSocialGameManager.cpp
|   |   |   |   +-- DeskillzRakeCalculator.cpp
|   |   |   |   +-- DeskillzBuyInManager.cpp
|   |   |   +-- Spectator/                         # NEW in v2.6 (Host-only)
|   |   |   |   +-- DeskillzHostSpectatorManager.cpp
|   |   |   +-- Widgets/
|   |   |   |   +-- Host/                          # NEW in v2.6
|   |   |   |   |   +-- DeskillzHostDashboardWidget.cpp
|   |   |   |   |   +-- DeskillzHostProfileCardWidget.cpp
|   |   |   |   |   +-- DeskillzHostTierProgressWidget.cpp
|   |   |   |   |   +-- DeskillzHostBadgeGridWidget.cpp
|   |   |   |   |   +-- DeskillzHostEarningsWidget.cpp
|   |   |   |   +-- Social/                        # NEW in v2.6
|   |   |   |   |   +-- DeskillzBuyInModalWidget.cpp
|   |   |   |   |   +-- DeskillzRebuyModalWidget.cpp
|   |   |   |   |   +-- DeskillzCashOutModalWidget.cpp
|   |   |   |   |   +-- DeskillzSocialGameSettingsWidget.cpp
|   |   |   |   |   +-- DeskillzTurnTimerWidget.cpp
|   |   |   |   |   +-- DeskillzPauseRequestWidget.cpp
|   |   |   |   +-- Spectator/                     # NEW in v2.6 (Host-only)
|   |   |   |   |   +-- DeskillzHostSpectatorViewWidget.cpp
|   |   |   |   |   +-- DeskillzHostScorePanelWidget.cpp
|   |   |   |   |   +-- DeskillzHostRoomSwitcherWidget.cpp
+-- Content/
|   +-- Widgets/
|   +-- Materials/
+-- Resources/
+-- Docs/
+-- Deskillz.uplugin
+-- README.md
```

---

## iOS Setup

Add URL scheme to `Info.plist`:

```xml
<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLName</key>
        <string>com.yourstudio.yourgame</string>
        <key>CFBundleURLSchemes</key>
        <array>
            <string>deskillz</string>
            <string>yourgame</string>
        </array>
    </dict>
</array>
```

In Unreal, add to **Project Settings -> iOS -> Additional Plist Data**.

## Android Setup

Add to your project's `AndroidManifest.xml` template:

```xml
<activity android:name="com.epicgames.ue4.GameActivity"
          android:launchMode="singleTask">
    <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:scheme="deskillz" />
        <data android:scheme="yourgame" />
    </intent-filter>
</activity>
```

**Important:** Use `android:launchMode="singleTask"` to ensure deep links are handled by the existing app instance.

## Blueprint Integration

All SDK features are exposed to Blueprints:

```
// Blueprint nodes available:
- Initialize Deskillz
- Submit Score
- End Match
- Create Room
- Join Room By Code
- Get Public Rooms
- Initialize Host Manager
- Register As Host
- Create Host Room
- Start Social Session
- Add Player
- Process Buy In
- Calculate Rake
- Spectate Host Room
- Switch Host Room
- Check For Updates
- Simulate Deep Link
```

## Test Mode

Test your integration without real currency:

```cpp
// Enable test mode in DeskillzConfig
// Or programmatically:
UDeskillzSDK::SetTestMode(true);

// Start a test match
UDeskillzSDK::StartTestMatch(EDeskillzMatchMode::Asynchronous);

// Simulate opponent score
UDeskillzSDK::SimulateOpponentScore(1000);

// Test navigation deep links
UDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://tournaments"));
UDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://wallet"));

// Test match launch
UDeepLinkHandler::Get()->SimulateDeepLink(
    TEXT("deskillz://launch?matchId=test&token=test"));

// Test host system (NEW in v2.6)
UDeskillzHostManager::Get()->Initialize(TEXT("test-host-id"));

// Test social games (NEW in v2.6)
UDeskillzSocialGameManager::Get()->StartTestSession();

// Test host spectator mode (NEW in v2.6) - Host-only feature
UDeskillzHostSpectatorManager::Get()->FetchHostRooms(FDeskillzHostRoomFilter());

// Test auto-updater
UDeskillzUpdater::Get()->CheckForUpdates();
```

## Documentation

- [Quick Start Guide](https://docs.deskillz.games/unreal/quickstart)
- [API Reference](https://docs.deskillz.games/unreal/api)
- [Blueprint Guide](https://docs.deskillz.games/unreal/blueprints)
- [Multiplayer Guide](https://docs.deskillz.games/unreal/multiplayer)
- [Deep Link Integration](https://docs.deskillz.games/unreal/deep-links)
- [Private Rooms Guide](https://docs.deskillz.games/unreal/private-rooms)
- [Host System Guide](https://docs.deskillz.games/unreal/host-system)
- [Social Games Guide](https://docs.deskillz.games/unreal/social-games)
- [Host Spectator Mode Guide](https://docs.deskillz.games/unreal/host-spectator)
- [Auto-Updater Guide](https://docs.deskillz.games/unreal/updater)
- [Widget Customization](https://docs.deskillz.games/unreal/widgets)
- [Troubleshooting](https://docs.deskillz.games/unreal/troubleshooting)

## Sample Project

Check out our sample game implementation:
[Deskillz Unreal Sample](https://github.com/Deskillz-Games-Development/unreal-sample)

## Changelog

See [CHANGELOG.md](./CHANGELOG.md) for version history.

### v2.6.0 (January 2025)
- **NEW:** Host System with 6-tier progression
- **NEW:** DeskillzHostManager for host registration and management
- **NEW:** Host Dashboard UI widgets (5 headers + 5 implementations)
- **NEW:** DeskillzSocialGameManager for rake-based games
- **NEW:** DeskillzRakeCalculator with tiered rake structure
- **NEW:** DeskillzBuyInManager for buy-in/rebuy/cashout flows
- **NEW:** Social Game UI widgets (6 headers + 6 implementations)
- **NEW:** DeskillzHostSpectatorManager for host room monitoring (host-only)
- **NEW:** Host Spectator UI widgets (3 headers + 3 implementations)
- **NEW:** 44 total new files for Private Room Enhancement
- Revenue sharing system (50%-75% based on tier)
- Real-time WebSocket updates for host spectator mode
- Pause/resume functionality for social games
- Full Blueprint support for all new features

### v2.5.1 (January 2025)
- Fixed duplicate struct definitions
- README URL corrections

### v2.5.0 (January 2025)
- Enhanced README documentation
- SDK testing procedures
- APK hosting improvements

### v2.3.0 (January 2025)
- **NEW:** Auto-Updater (`UDeskillzUpdater`)
- **NEW:** Forced vs optional update support
- **NEW:** Remember skipped versions
- **NEW:** Version comparison utilities
- **NEW:** Update delegates and callbacks
- APK hosting integration with Cloudflare R2

### v2.2.0 (December 2024)
- **NEW:** Private Rooms API (`UDeskillzRooms`)
- **NEW:** Pre-built Room Widgets (4 components)
- **NEW:** Real-time WebSocket for rooms
- **NEW:** Room events (join, leave, ready, chat, countdown)
- Room list with search, filter, and sort
- Room lobby with player cards and ready status
- Host controls (start, cancel, kick)

### v2.1.0 (December 2024)
- Deep link improvements
- Bug fixes and stability

### v2.0.0 (December 2024)
- **NEW:** Navigation deep links (`OnNavigationReceived`)
- **NEW:** Simplified match launch (`OnMatchLaunchReceived`)
- **NEW:** `SimulateDeepLink()` for testing
- **NEW:** `EDeskillzNavigationAction` enum
- Improved deep link parsing
- Better error handling

## Troubleshooting

### Deep links not working
1. Verify URL schemes are configured correctly
2. Check app is properly signed
3. Test with: `adb shell am start -a android.intent.action.VIEW -d "deskillz://tournaments"`
4. Enable logging to see incoming deep links

### Navigation events not firing
1. Ensure `UDeepLinkHandler::Get()->Initialize()` is called first
2. Verify delegate bindings before processing
3. Check `HasPendingDeepLink()` and call `ProcessPendingDeepLinks()`
4. Test with `SimulateDeepLink()` first

### Room widgets not displaying
1. Ensure `UDeskillzRooms::Get()->Initialize()` is called
2. Check that widget classes are properly loaded
3. Verify WebSocket connection is established
4. Check UMG widget hierarchy

### Auto-updater not checking
1. Verify version and version code are set correctly
2. Check network connectivity
3. Ensure Game ID is configured in DeskillzConfig
4. Enable logging to see API responses
5. Test manually: `UDeskillzUpdater::Get()->CheckForUpdates()`

### Host system not initializing
1. Ensure user is authenticated first
2. Call `UDeskillzHostManager::Get()->Initialize(UserId)`
3. Check for registration errors in delegates
4. Verify API connectivity

### Social game session errors
1. Validate session config before starting
2. Check buy-in amounts are within range
3. Ensure all players have sufficient balance
4. Monitor WebSocket connection status

### Spectator mode not connecting
1. Verify room exists and is active
2. Check spectating is enabled for the room
3. Ensure WebSocket connection is established
4. Monitor for connection timeout errors

### SDK Not Initializing
```cpp
// Check initialization status
if (!UDeskillzSDK::IsInitialized())
{
    // Verify credentials in DeskillzConfig
    // Check network connectivity
    // Enable logging for details
}
```

### iOS build errors
- Ensure Xcode 14+ is installed
- Check iOS deployment target is 12.0+
- Verify signing certificates

### Android build errors
- Check Min SDK is 21+
- Verify NDK version compatibility
- Check for duplicate AndroidManifest entries

## Support

- **Email:** sdk@deskillz.games
- **Discord:** [discord.gg/deskillz](https://discord.gg/deskillz)
- **Documentation:** [docs.deskillz.games](https://docs.deskillz.games)
- **Developer Portal:** [deskillz.games/developer](https://deskillz.games/developer)
- **GitHub Issues:** [github.com/Deskillz-Games-Development/unreal-sdk/issues](https://github.com/Deskillz-Games-Development/unreal-sdk/issues)

## License

Copyright (c) 2025 Deskillz.Games. All rights reserved.

MIT License - see [LICENSE](https://github.com/Deskillz-Games-Development/unreal-sdk/blob/main/LICENSE) for details.

---

<p align="center">
  Made with love by <a href="https://deskillz.games">Deskillz.Games</a>
</p>