# Deskillz Unreal Engine SDK

<p align="center">
  <img src="https://deskillz.games/logo.png" alt="Deskillz.Games" width="200"/>
</p>

<p align="center">
  <strong>Integrate competitive tournaments into your Unreal Engine games</strong>
</p>

<p align="center">
  <a href="https://github.com/deskillz-games/unreal-sdk/releases"><img src="https://img.shields.io/badge/version-2.3.0-blue.svg" alt="Version"></a>
  <a href="https://unrealengine.com"><img src="https://img.shields.io/badge/unreal-4.27%2B%20%7C%205.x-black.svg" alt="Unreal"></a>
  <a href="https://github.com/deskillz-games/unreal-sdk/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License"></a>
</p>

<p align="center">
  <a href="#installation">Installation</a> •
  <a href="#quick-start">Quick Start</a> •
  <a href="#features">Features</a> •
  <a href="#auto-updater">Auto-Updater</a> •
  <a href="#private-rooms">Private Rooms</a> •
  <a href="#navigation-deep-links">Navigation Links</a> •
  <a href="#documentation">Documentation</a> •
  <a href="#support">Support</a>
</p>

---

## Overview

The Deskillz Unreal SDK enables game developers to integrate their Unreal Engine games with the Deskillz.Games competitive gaming platform. Players can compete in skill-based tournaments, create private rooms to play with friends, and win cryptocurrency prizes (BTC, ETH, SOL, XRP, BNB, USDT, USDC).

### How It Works (Global Lobby Architecture)

```
┌─────────────────────────────────────────────────────────────────┐
│                    PLAYER JOURNEY                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. Player opens Deskillz.Games website/app                     │
│              ↓                                                  │
│  2. Player browses Global Lobby                                 │
│     • Select game                                               │
│     • Choose tournament/match type                              │
│     • Join matchmaking queue OR private room                    │
│              ↓                                                  │
│  3. Match found → Deep link sent to your game                   │
│     deskillz://launch?matchId=abc123&token=xyz...               │
│              ↓                                                  │
│  4. Your game app opens via deep link                           │
│              ↓                                                  │
│  5. SDK receives match data → Start gameplay                    │
│              ↓                                                  │
│  6. Player plays → Score submitted → Results shown              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Key Point:** Matchmaking happens in the Deskillz platform, NOT in your game. Your SDK just needs to:
1. Handle deep links (navigation + match launch)
2. Start the match
3. Submit scores securely

## Requirements

- **Unreal Engine:** 4.27+ or 5.x
- **Platforms:** iOS 12+, Android 6.0+ (API 23)
- **Build Tools:** Xcode 14+ (iOS), Android NDK (Android)
- **C++ Standard:** C++17

## Installation

### Option 1: Clone to Plugins Folder (Recommended)

```bash
cd YourProject/Plugins
git clone https://github.com/deskillz/unreal-sdk.git DeskillzSDK
```

Then regenerate project files (right-click `.uproject` → "Generate Visual Studio project files")

### Option 2: Download ZIP

1. Download latest from [deskillz.games/developer](https://deskillz.games/developer)
2. Extract to `YourProject/Plugins/DeskillzSDK/`
3. Regenerate project files

### Enable the Plugin

1. Open your project in Unreal Editor
2. Go to **Edit → Plugins**
3. Search for "Deskillz"
4. Check **Enabled**
5. Restart the editor

Or add to your `.uproject` file:

```json
{
  "Plugins": [
    {
      "Name": "Deskillz",
      "Enabled": true
    }
  ]
}
```

### Add Module Dependency

In your game's `Build.cs` file:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Deskillz"
});
```

### Configuration

Go to **Project Settings → Plugins → Deskillz SDK**:

| Setting | Description |
|---------|-------------|
| API Key | Your API key from Developer Portal |
| Game ID | Your game's unique identifier |
| Environment | Sandbox (testing) or Production |
| Enable Logging | Show debug logs |

Or set in C++:

```cpp
UDeskillzConfig* Config = GetMutableDefault<UDeskillzConfig>();
Config->ApiKey = TEXT("your-api-key");
Config->GameId = TEXT("your-game-id");
Config->Environment = EDeskillzEnvironment::Sandbox;
```

## Quick Start

### 1. Initialize the SDK (C++)

```cpp
// YourGameMode.h
#pragma once
#include "Core/DeskillzSDK.h"
#include "Core/DeskillzUpdater.h"
#include "Lobby/DeskillzDeepLinkHandler.h"
#include "GameFramework/GameModeBase.h"
#include "YourGameMode.generated.h"

UCLASS()
class AYourGameMode : public AGameModeBase
{
    GENERATED_BODY()
    
public:
    virtual void BeginPlay() override;
    
private:
    UFUNCTION()
    void HandleNavigation(EDeskillzNavigationAction Action, const TMap<FString, FString>& Parameters);
    
    UFUNCTION()
    void HandleMatchLaunch(const FString& MatchId, const FString& AuthToken);
    
    UFUNCTION()
    void HandleMatchReady(const FDeskillzMatchLaunchData& LaunchData);
    
    UFUNCTION()
    void HandleValidationFailed(const FString& Reason, const FDeskillzMatchLaunchData& Data);
    
    UFUNCTION()
    void HandleUpdateAvailable(const FDeskillzUpdateInfo& UpdateInfo);
};
```

```cpp
// YourGameMode.cpp
#include "YourGameMode.h"

void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize SDK
    UDeskillzSDK* SDK = UDeskillzSDK::Get(this);
    SDK->Initialize();
    
    // Initialize Deep Link Handler
    UDeskillzDeepLinkHandler* Handler = UDeskillzDeepLinkHandler::Get();
    Handler->Initialize();
    
    // Navigation events (NEW in v2.0)
    Handler->OnNavigationReceived.AddDynamic(this, &AYourGameMode::HandleNavigation);
    
    // Match launch events
    Handler->OnMatchLaunchReceived.AddDynamic(this, &AYourGameMode::HandleMatchLaunch);
    Handler->OnMatchReady.AddDynamic(this, &AYourGameMode::HandleMatchReady);
    Handler->OnValidationFailed.AddDynamic(this, &AYourGameMode::HandleValidationFailed);
    
    // Check for updates (NEW in v2.3)
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    Updater->SetCurrentVersion(TEXT("1.0.0"), 1);
    Updater->OnUpdateAvailable.AddDynamic(this, &AYourGameMode::HandleUpdateAvailable);
    Updater->CheckForUpdates();
    
    // Process pending deep links (cold start)
    if (Handler->HasPendingDeepLink())
    {
        Handler->ProcessPendingDeepLinks();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Deskillz SDK 2.3.0 initialized!"));
}

void AYourGameMode::HandleNavigation(EDeskillzNavigationAction Action, const TMap<FString, FString>& Parameters)
{
    switch (Action)
    {
        case EDeskillzNavigationAction::Tournaments:
            ShowTournaments();
            break;
        case EDeskillzNavigationAction::Wallet:
            ShowWallet();
            break;
        case EDeskillzNavigationAction::Profile:
            ShowProfile();
            break;
        case EDeskillzNavigationAction::Game:
            if (const FString* GameId = Parameters.Find(TEXT("id")))
            {
                ShowGameDetails(*GameId);
            }
            break;
        case EDeskillzNavigationAction::Settings:
            ShowSettings();
            break;
    }
}

void AYourGameMode::HandleMatchLaunch(const FString& MatchId, const FString& AuthToken)
{
    // Store auth token for API calls
    UDeskillzApi::SetAuthToken(AuthToken);
    
    // Load match level
    UGameplayStatics::OpenLevel(this, TEXT("MatchLevel"));
    
    UE_LOG(LogTemp, Log, TEXT("Launching match: %s"), *MatchId);
}

void AYourGameMode::HandleMatchReady(const FDeskillzMatchLaunchData& LaunchData)
{
    UE_LOG(LogTemp, Log, TEXT("Match ready: %s"), *LaunchData.MatchId);
    UE_LOG(LogTemp, Log, TEXT("Opponent: %s (Rating: %d)"), *LaunchData.OpponentName, LaunchData.OpponentRating);
    UE_LOG(LogTemp, Log, TEXT("Entry Fee: %.2f %s"), LaunchData.EntryFee, *LaunchData.Currency);
    
    // Start your game
    StartGame(LaunchData);
}

void AYourGameMode::HandleValidationFailed(const FString& Reason, const FDeskillzMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Error, TEXT("Match validation failed: %s"), *Reason);
    ShowErrorDialog(Reason);
}

void AYourGameMode::HandleUpdateAvailable(const FDeskillzUpdateInfo& UpdateInfo)
{
    UE_LOG(LogTemp, Log, TEXT("Update available: %s"), *UpdateInfo.LatestVersion);
    ShowUpdateDialog(UpdateInfo);
}
```

### 2. Initialize the SDK (Blueprint)

1. Get **Deskillz SDK** node
2. Call **Initialize** with your API key
3. Bind to **OnMatchLaunch** and **OnNavigationReceived** events

### 3. Submit Score

```cpp
#include "Lobby/DeskillzBridge.h"

void AYourGameMode::OnGameOver(int64 FinalScore)
{
    UDeskillzBridge* Bridge = UDeskillzBridge::Get();
    
    // Submit score (auto-encrypted with HMAC-SHA256)
    Bridge->SubmitScore(FinalScore, FDeskillzCallback::CreateLambda([Bridge]()
    {
        UE_LOG(LogTemp, Log, TEXT("Score submitted!"));
        
        // Return to Deskillz app
        Bridge->ReturnToMainApp(EDeskillzReturnDestination::Results);
    }));
}
```

---

## Auto-Updater (NEW in v2.3.0)

The SDK includes automatic update checking to ensure players always have the latest version of your game. This integrates with the Deskillz APK Hosting system.

### Basic Usage (C++)

```cpp
#include "Core/DeskillzUpdater.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    
    // Set current app version (must match your APK settings)
    Updater->SetCurrentVersion(TEXT("1.0.0"), 1);
    
    // Bind events
    Updater->OnUpdateCheckStarted.AddDynamic(this, &AMyGameMode::OnCheckStarted);
    Updater->OnUpdateAvailable.AddDynamic(this, &AMyGameMode::OnUpdateAvailable);
    Updater->OnForceUpdateRequired.AddDynamic(this, &AMyGameMode::OnForcedUpdate);
    Updater->OnNoUpdateNeeded.AddDynamic(this, &AMyGameMode::OnNoUpdate);
    Updater->OnUpdateCheckFailed.AddDynamic(this, &AMyGameMode::OnCheckFailed);
    
    // Check for updates
    Updater->CheckForUpdates();
}

void AMyGameMode::OnCheckStarted()
{
    // Show loading indicator
    ShowLoadingSpinner();
}

void AMyGameMode::OnUpdateAvailable(const FDeskillzUpdateInfo& Info)
{
    UE_LOG(LogTemp, Log, TEXT("Optional update available: %s (%s)"), 
        *Info.LatestVersion, *UDeskillzUpdater::FormatFileSize(Info.FileSize));
    UE_LOG(LogTemp, Log, TEXT("Release notes: %s"), *Info.ReleaseNotes);
    
    // Show custom update dialog (can skip)
    ShowUpdateDialog(Info, true);
}

void AMyGameMode::OnForcedUpdate(const FDeskillzUpdateInfo& Info)
{
    UE_LOG(LogTemp, Warning, TEXT("FORCED update required: %s"), *Info.LatestVersion);
    
    // Show forced update dialog (no skip option)
    ShowUpdateDialog(Info, false);
    
    // Pause game - user must update
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void AMyGameMode::OnNoUpdate()
{
    UE_LOG(LogTemp, Log, TEXT("App is up to date!"));
    HideLoadingSpinner();
    ShowMainMenu();
}

void AMyGameMode::OnCheckFailed(const FString& Error)
{
    UE_LOG(LogTemp, Warning, TEXT("Update check failed: %s"), *Error);
    // Continue anyway - don't block users on network errors
    HideLoadingSpinner();
    ShowMainMenu();
}

// Called from Update button in your UI
void AMyGameMode::AcceptUpdate()
{
    UDeskillzUpdater::Get()->StartUpdate(); // Opens download URL in browser
}

// Called from Skip button (optional updates only)
void AMyGameMode::SkipUpdate()
{
    UDeskillzUpdater::Get()->SkipUpdate(); // Remembers skipped version
    ShowMainMenu();
}
```

### Blueprint Usage

1. **Get Deskillz Updater** node
2. Call **Set Current Version** with your version string and code
3. Bind to events: **On Update Available**, **On Force Update Required**, **On No Update Needed**, **On Update Check Failed**
4. Call **Check For Updates**
5. Use **Start Update** to open download URL
6. Use **Skip Update** to dismiss (optional updates only)

### Configuration Options

```cpp
UDeskillzUpdater* Updater = UDeskillzUpdater::Get();

// Enable/disable features
Updater->SetAutoCheckEnabled(true);           // Check automatically on startup
Updater->SetShowOptionalUpdates(true);        // Show dialog for optional updates
Updater->SetRememberSkippedVersion(true);     // Don't re-prompt for skipped versions

// Clear remembered skip (e.g., after certain time)
Updater->ClearSkippedVersion();
```

### FDeskillzUpdateInfo Properties

| Property | Type | Description |
|----------|------|-------------|
| `LatestVersion` | FString | Version string (e.g., "1.2.0") |
| `VersionCode` | int32 | Integer version code (e.g., 10200) |
| `bUpdateAvailable` | bool | Whether an update is available |
| `bIsForced` | bool | Whether update is required (can't skip) |
| `DownloadUrl` | FString | Direct APK download URL |
| `FileSize` | int64 | File size in bytes |
| `ReleaseNotes` | FString | Changelog/release notes text |
| `ErrorMessage` | FString | Error message if check failed |

### Update Events (Delegates)

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnUpdateCheckStarted` | None | Update check has begun |
| `OnUpdateCheckCompleted` | `EDeskillzUpdateResult`, `FDeskillzUpdateInfo` | Check completed |
| `OnUpdateAvailable` | `FDeskillzUpdateInfo` | Optional update available |
| `OnForceUpdateRequired` | `FDeskillzUpdateInfo` | Required update - must update |
| `OnNoUpdateNeeded` | None | App is already up to date |
| `OnUpdateCheckFailed` | `FString Error` | Network or parsing error |
| `OnUpdateAccepted` | `FDeskillzUpdateInfo` | User clicked "Update" |
| `OnUpdateSkipped` | `FDeskillzUpdateInfo` | User clicked "Skip" |

### Utility Functions

```cpp
// Compare two version strings
int32 Result = UDeskillzUpdater::CompareVersions(TEXT("1.2.0"), TEXT("1.1.5"));
// Result > 0: First is newer
// Result < 0: Second is newer
// Result == 0: Same version

// Format file size for display
FString Size = UDeskillzUpdater::FormatFileSize(54976512);
// Returns: "52.4 MB"
```

### Version Code Best Practices

```cpp
// Recommended version code format: MAJOR * 10000 + MINOR * 100 + PATCH
// Examples:
// 1.0.0  → 10000
// 1.2.0  → 10200
// 1.2.3  → 10203
// 2.0.0  → 20000

// In your Android build settings (DefaultEngine.ini):
// [/Script/AndroidRuntimeSettings.AndroidRuntimeSettings]
// VersionDisplayName=1.0.0
// StoreVersion=10000
```

---

## Private Rooms (NEW in v2.2.0)

Players can create private rooms to play with friends! The SDK includes a full Room API and pre-built UMG widgets.

### Room API (C++)

```cpp
#include "Rooms/DeskillzRooms.h"

// Get room subsystem
UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);

// Create a room
FCreateRoomConfig Config;
Config.Name = TEXT("My Room");
Config.EntryFee = 5.00f;
Config.EntryCurrency = TEXT("USDT");
Config.MinPlayers = 2;
Config.MaxPlayers = 4;
Config.Mode = ERoomMode::Sync;
Config.Visibility = ERoomVisibility::PublicListed;

FOnRoomSuccess OnSuccess;
OnSuccess.BindLambda([](const FPrivateRoom& Room) {
    UE_LOG(LogTemp, Log, TEXT("Room created: %s"), *Room.RoomCode);
});

FOnRoomError OnError;
OnError.BindLambda([](const FRoomError& Error) {
    UE_LOG(LogTemp, Error, TEXT("Error: %s"), *Error.Message);
});

Rooms->CreateRoom(Config, OnSuccess, OnError);

// Quick create with defaults
Rooms->QuickCreateRoom(TEXT("Quick Match"), 1.00f, OnSuccess, OnError);

// Join by code
Rooms->JoinRoom(TEXT("DSKZ-AB3C"), OnSuccess, OnError);

// Browse public rooms
FOnRoomListSuccess OnListSuccess;
OnListSuccess.BindLambda([](const TArray<FPrivateRoom>& RoomList) {
    for (const FPrivateRoom& Room : RoomList)
    {
        UE_LOG(LogTemp, Log, TEXT("Room: %s"), *Room.Name);
    }
});
Rooms->GetPublicRooms(OnListSuccess, OnError);

// Get room by code (preview before joining)
Rooms->GetRoomByCode(TEXT("DSKZ-AB3C"), OnSuccess, OnError);

// Ready up
Rooms->SetReady(true);

// Send chat message
Rooms->SendChat(TEXT("Hello everyone!"));

// Leave room
FOnRoomActionSuccess OnActionSuccess;
OnActionSuccess.BindLambda([]() {
    UE_LOG(LogTemp, Log, TEXT("Left room"));
});
Rooms->LeaveRoom(OnActionSuccess, OnError);

// Host: Cancel room
Rooms->CancelRoom(OnActionSuccess, OnError);

// Host: Kick player
Rooms->KickPlayer(PlayerId, OnActionSuccess, OnError);

// Host: Start match
Rooms->StartMatch(OnActionSuccess, OnError);
```

### Room Events

```cpp
// Get room subsystem
UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);

// Subscribe to room events
Rooms->OnRoomJoined.AddDynamic(this, &AMyClass::HandleRoomJoined);
Rooms->OnRoomUpdated.AddDynamic(this, &AMyClass::HandleRoomUpdated);
Rooms->OnPlayerJoined.AddDynamic(this, &AMyClass::HandlePlayerJoined);
Rooms->OnPlayerLeft.AddDynamic(this, &AMyClass::HandlePlayerLeft);
Rooms->OnPlayerReadyChanged.AddDynamic(this, &AMyClass::HandleReadyChanged);
Rooms->OnCountdownStarted.AddDynamic(this, &AMyClass::HandleCountdownStarted);
Rooms->OnCountdownTick.AddDynamic(this, &AMyClass::HandleCountdownTick);
Rooms->OnMatchLaunching.AddDynamic(this, &AMyClass::HandleMatchLaunching);
Rooms->OnChatReceived.AddDynamic(this, &AMyClass::HandleChatReceived);
Rooms->OnKickedFromRoom.AddDynamic(this, &AMyClass::HandleKicked);
Rooms->OnRoomCancelled.AddDynamic(this, &AMyClass::HandleRoomCancelled);
Rooms->OnRoomLeft.AddDynamic(this, &AMyClass::HandleRoomLeft);
```

### Pre-Built Room UI (UMG Widgets)

The SDK includes ready-to-use UMG widgets for private rooms:

```cpp
#include "Widgets/Rooms/DeskillzPrivateRoomUI.h"

// Get or create the UI
UDeskillzPrivateRoomUI* RoomUI = UDeskillzPrivateRoomUI::GetOrCreate(this);

// Show room browser (list of public rooms)
RoomUI->ShowRoomList();

// Show create room form
RoomUI->ShowCreateRoom();

// Show join by code dialog
RoomUI->ShowJoinRoom();

// Show with pre-filled code (e.g., from deep link or share)
RoomUI->ShowJoinRoomWithCode(TEXT("DSKZ-AB3C"));

// Show room lobby (waiting room)
RoomUI->ShowRoomLobby();

// Show room lobby with specific room data
RoomUI->ShowRoomLobbyWithRoom(Room);

// Quick actions (create/join and auto-show lobby)
RoomUI->QuickCreateRoom(TEXT("My Room"), 5.00f);
RoomUI->QuickJoinRoom(TEXT("DSKZ-AB3C"));

// Hide all room UI
RoomUI->HideAll();

// Close and remove from viewport
RoomUI->Close();
```

### Room UI Widgets

| Widget | Files | Description |
|--------|-------|-------------|
| **DeskillzPrivateRoomUI** | `.h/.cpp` | Main UI manager |
| **DeskillzRoomListWidget** | `.h/.cpp` | Browse public rooms with search/filter/sort |
| **DeskillzCreateRoomWidget** | `.h/.cpp` | Room creation form with validation |
| **DeskillzJoinRoomWidget** | `.h/.cpp` | Enter room code dialog with preview |
| **DeskillzRoomLobbyWidget** | `.h/.cpp` | Waiting room with player list, chat, countdown |
| **DeskillzRoomPlayerCard** | `.h/.cpp` | Individual player card with ready status |

Widget locations:
- Headers: `Source/Deskillz/Public/Widgets/Rooms/`
- Implementation: `Source/Deskillz/Private/Widgets/Rooms/`

### Room UI Events

```cpp
// Subscribe to UI navigation events
RoomUI->OnPanelShown.AddDynamic(this, &AMyClass::HandlePanelShown);
RoomUI->OnRoomCreatedFromUI.AddDynamic(this, &AMyClass::HandleRoomCreated);
RoomUI->OnRoomJoinedFromUI.AddDynamic(this, &AMyClass::HandleRoomJoined);
RoomUI->OnAllHidden.AddDynamic(this, &AMyClass::HandleUIHidden);
```

### Room API (Blueprint)

All room functionality is available in Blueprints:

1. Get **Deskillz Rooms** subsystem
2. Call **Create Room**, **Join Room**, or **Get Public Rooms**
3. Bind to success/error events

Available Blueprint nodes:
- **Create Room** - Create a new private room
- **Join Room** - Join by room code
- **Get Public Rooms** - Browse available rooms
- **Set Ready** - Toggle ready status
- **Leave Room** - Exit current room
- **Send Chat** - Send chat message
- **Start Match** (Host) - Begin the game
- **Cancel Room** (Host) - Cancel the room
- **Kick Player** (Host) - Remove a player

---

## Navigation Deep Links (NEW in v2.0)

The Deskillz platform can send navigation deep links to your game for seamless user experience.

### Supported Navigation Actions

| URL | Action | Description |
|-----|--------|-------------|
| `deskillz://tournaments` | Tournaments | Show tournament list |
| `deskillz://wallet` | Wallet | Show wallet screen |
| `deskillz://profile` | Profile | Show user profile |
| `deskillz://game?id=xxx` | Game | Show specific game details |
| `deskillz://settings` | Settings | Show settings screen |
| `deskillz://launch?matchId=xxx` | Match | Start match |

### EDeskillzNavigationAction Enum

```cpp
UENUM(BlueprintType)
enum class EDeskillzNavigationAction : uint8
{
    None,
    Tournaments,
    Wallet,
    Profile,
    Game,
    Settings
};
```

### Testing Navigation Links

```cpp
// Simulate navigation deep links for testing
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://tournaments"));
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://wallet"));
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://game?id=battle-blocks"));

// Test match launch
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(
    TEXT("deskillz://launch?matchId=test-123&token=test-token"));
```

## Events

Subscribe to SDK events for full control:

```cpp
// SDK ready
UDeskillzSDK::Get()->OnReady.AddDynamic(this, &AMyClass::OnSDKReady);

// Match lifecycle
UDeskillzSDK::Get()->OnMatchReady.AddDynamic(this, &AMyClass::OnMatchReady);
UDeskillzSDK::Get()->OnMatchStart.AddDynamic(this, &AMyClass::OnMatchStart);
UDeskillzSDK::Get()->OnMatchComplete.AddDynamic(this, &AMyClass::OnMatchComplete);

// Real-time multiplayer
UDeskillzSDK::Get()->OnPlayerJoined.AddDynamic(this, &AMyClass::OnPlayerJoined);
UDeskillzSDK::Get()->OnMessageReceived.AddDynamic(this, &AMyClass::OnMessageReceived);

// Deep Link Navigation (NEW in v2.0)
UDeskillzDeepLinkHandler::Get()->OnNavigationReceived.AddDynamic(this, &AMyClass::HandleNavigation);
UDeskillzDeepLinkHandler::Get()->OnMatchLaunchReceived.AddDynamic(this, &AMyClass::HandleMatchLaunch);

// Private Room Events (NEW in v2.2)
UDeskillzRooms::Get(this)->OnRoomJoined.AddDynamic(this, &AMyClass::HandleRoomJoined);
UDeskillzRooms::Get(this)->OnMatchLaunching.AddDynamic(this, &AMyClass::HandleRoomMatchStart);

// Update Events (NEW in v2.3)
UDeskillzUpdater::Get()->OnUpdateAvailable.AddDynamic(this, &AMyClass::HandleUpdateAvailable);
UDeskillzUpdater::Get()->OnForceUpdateRequired.AddDynamic(this, &AMyClass::HandleForcedUpdate);
```

## Match Launch Deep Link Format

Your game will receive match launch deep links in this format:

```
deskillz://launch?
  matchId=<match_id>&
  token=<auth_token>&
  gameId=<game_id>&
  mode=<SYNC|ASYNC>&
  opponentId=<opponent_id>&
  entryFee=<amount>&
  currency=<BTC|ETH|USDT|etc>&
  duration=<seconds>&
  seed=<random_seed>
```

The SDK parses this automatically - you just handle the `OnMatchReady` or `OnMatchLaunchReceived` events.

## FMatchLaunchData Fields

| Field | Type | Description |
|-------|------|-------------|
| `MatchId` | FString | Unique match identifier |
| `TournamentId` | FString | Tournament this match belongs to |
| `Token` | FString | Authentication token for API calls |
| `Duration` | int32 | Match duration in seconds |
| `RandomSeed` | int32 | Seed for deterministic gameplay |
| `EntryFee` | float | Entry fee amount |
| `Currency` | FString | Currency (BTC, ETH, USDT, etc.) |
| `MatchType` | EMatchType | Synchronous or Asynchronous |
| `OpponentId` | FString | Opponent player ID |
| `OpponentName` | FString | Opponent display name |
| `OpponentRating` | int32 | Opponent skill rating |

## Architecture: Global Lobby vs Old SDK-Based

| Aspect | Old (SDK-Based) | New (Global Lobby) ✅ |
|--------|-----------------|----------------------|
| Matchmaking Location | Inside your game | Deskillz website/app |
| Player Pool | Fragmented per-game | Unified across all games |
| User Experience | Inconsistent | Consistent platform UI |
| SDK Complexity | High (matchmaking logic) | Low (deep links only) |
| Developer Burden | Heavy | Minimal |
| Private Rooms | Complex to implement | Built into platform + SDK |
| NPC Opponents | SDK handles | Platform handles |
| Navigation | N/A | Full deep link support |

## Features

| Feature | Description |
|---------|-------------|
| 🔄 **Auto-Updater** | Automatic game updates with forced/optional prompts (NEW in v2.3) |
| 🔗 **Navigation Deep Links** | Navigate to any screen from Deskillz app (NEW in v2.0) |
| 🎮 **Match Launch Deep Links** | Receive match data from Global Lobby |
| 🚪 **Private Rooms** | Create/join rooms with friends (NEW in v2.2) |
| 🎨 **Pre-built Room UI** | Ready-to-use UMG widgets (NEW in v2.2) |
| 🏆 **Asynchronous Tournaments** | Players compete separately, scores compared |
| ⚡ **Real-time Multiplayer** | 2-10 players competing simultaneously |
| 🎯 **Custom Stages** | Player-created private rooms |
| 💰 **Cryptocurrency Prizes** | BTC, ETH, SOL, XRP, BNB, USDT, USDC |
| 🎨 **Built-in UI** | Pre-made UMG widget components with themes |
| 🛡️ **Anti-Cheat** | Server-side validation and protection |
| 🔐 **Score Encryption** | HMAC-SHA256 signed submission |
| 📘 **Blueprint Support** | Full Blueprint integration |

## SDK Structure

```
deskillz-unreal-sdk/
├── Source/DeskillzSDK/
│   ├── Public/
│   │   ├── Core/
│   │   │   ├── DeskillzSDK.h
│   │   │   ├── DeskillzConfig.h
│   │   │   ├── DeskillzTypes.h
│   │   │   ├── DeskillzEvents.h
│   │   │   └── DeskillzUpdater.h           # NEW in v2.3
│   │   ├── Match/
│   │   │   ├── DeskillzMatchmaking.h
│   │   │   ├── DeskillzMatchManager.h
│   │   │   └── DeskillzTournamentManager.h
│   │   ├── Security/
│   │   │   ├── DeskillzScoreEncryption.h
│   │   │   ├── DeskillzSecureSubmitter.h
│   │   │   ├── DeskillzAntiCheat.h
│   │   │   └── DeskillzSecurityModule.h
│   │   ├── Rooms/                          # NEW in v2.2
│   │   │   ├── DeskillzRooms.h
│   │   │   ├── DeskillzRoomTypes.h
│   │   │   └── DeskillzRoomClient.h
│   │   ├── Widgets/
│   │   │   ├── DeskillzUIManager.h
│   │   │   ├── DeskillzBaseWidget.h
│   │   │   ├── DeskillzHUDWidget.h
│   │   │   ├── DeskillzPopupWidget.h
│   │   │   ├── DeskillzResultsWidget.h
│   │   │   └── Rooms/                      # NEW in v2.2
│   │   │       ├── DeskillzPrivateRoomUI.h
│   │   │       ├── DeskillzRoomListWidget.h
│   │   │       ├── DeskillzCreateRoomWidget.h
│   │   │       ├── DeskillzJoinRoomWidget.h
│   │   │       ├── DeskillzRoomLobbyWidget.h
│   │   │       └── DeskillzRoomPlayerCard.h
│   │   ├── Lobby/
│   │   │   ├── DeskillzDeepLinkHandler.h
│   │   │   ├── DeskillzBridge.h
│   │   │   ├── DeskillzLobbyClient.h
│   │   │   └── DeskillzLobbyTypes.h
│   │   ├── Network/
│   │   │   ├── DeskillzHttpClient.h
│   │   │   ├── DeskillzApiService.h
│   │   │   ├── DeskillzNetworkManager.h
│   │   │   └── DeskillzWebSocket.h
│   │   ├── Platform/
│   │   │   ├── DeskillzPlatform.h
│   │   │   ├── DeskillzDeepLink.h
│   │   │   ├── DeskillzAppLifecycle.h
│   │   │   └── DeskillzPushNotifications.h
│   │   └── Analytics/
│   │       ├── DeskillzAnalytics.h
│   │       ├── DeskillzEventTracker.h
│   │       └── DeskillzTelemetry.h
│   └── Private/
│       └── [Implementation files - mirrors Public structure]
├── Content/
│   └── UI/
│       └── [UMG Widget Blueprints]
├── Resources/
│   └── [Icons, Images]
├── Deskillz.uplugin
└── README.md
```

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

## Android Setup

Add to `AndroidManifest.xml`:

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

## Test Mode

Test your integration without real currency:

```cpp
// Enable sandbox mode in config
Config.Environment = EDeskillzEnvironment::Sandbox;

// Start a test match
UDeskillzSDK::Get()->StartTestMatch(EMatchMode::Asynchronous);

// Simulate opponent score
UDeskillzSDK::Get()->SimulateOpponentScore(1000);

// Test navigation deep links (NEW)
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://tournaments"));
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://wallet"));

// Test match launch
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(
    TEXT("deskillz://launch?matchId=test&token=test"));

// Test room UI (NEW in v2.2)
UDeskillzPrivateRoomUI* RoomUI = UDeskillzPrivateRoomUI::GetOrCreate(this);
RoomUI->ShowRoomList();
RoomUI->ShowCreateRoom();

// Test auto-updater (NEW in v2.3)
UDeskillzUpdater::Get()->CheckForUpdates();
```

## Documentation

- [Quick Start Guide](https://docs.deskillz.games/unreal/quickstart)
- [API Reference](https://docs.deskillz.games/unreal/api)
- [Multiplayer Guide](https://docs.deskillz.games/unreal/multiplayer)
- [Deep Link Integration](https://docs.deskillz.games/unreal/deep-links)
- [Private Rooms Guide](https://docs.deskillz.games/unreal/private-rooms)
- [Auto-Updater Guide](https://docs.deskillz.games/unreal/updater)
- [Blueprint Integration](https://docs.deskillz.games/unreal/blueprints)
- [Troubleshooting](https://docs.deskillz.games/unreal/troubleshooting)

## Sample Project

Check out our sample game implementation:
[Deskillz Unreal Sample](https://github.com/deskillz/unreal-sample)

## Changelog

See [CHANGELOG.md](./CHANGELOG.md) for version history.

### v2.3.0 (January 2025)
- **NEW:** Auto-Updater (`UDeskillzUpdater`)
- **NEW:** Forced vs optional update support
- **NEW:** Remember skipped versions
- **NEW:** Version comparison utilities
- **NEW:** Blueprint-compatible update events
- **NEW:** File size formatting helper
- APK hosting integration with Cloudflare R2

### v2.2.0 (December 2024)
- **NEW:** Private Rooms API (`UDeskillzRooms`)
- **NEW:** Pre-built Room UI (12 widget files - 6 headers, 6 implementations)
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
1. Ensure `UDeskillzDeepLinkHandler::Get()->Initialize()` is called
2. Verify event subscriptions before processing
3. Check for pending deep links with `HasPendingDeepLink()`
4. Test with `SimulateDeepLink()` first

### Room UI not showing
1. Ensure `UDeskillzRooms::Get(this)` returns valid subsystem
2. Check that widget classes are properly loaded
3. Verify WebSocket connection is established
4. Test with `RoomUI->ShowRoomList()`

### Auto-updater not checking
1. Verify `SetCurrentVersion()` was called with correct values
2. Check network connectivity
3. Ensure Game ID is configured in Project Settings
4. Enable logging to see API responses
5. Test manually: `UDeskillzUpdater::Get()->CheckForUpdates()`

### Module Not Found
Ensure `"Deskillz"` is in your `Build.cs` PublicDependencyModuleNames.

### Blueprint Nodes Missing
Restart the editor after enabling the plugin.

### SDK Not Initializing
```cpp
// Check initialization status
if (!UDeskillzSDK::Get()->IsInitialized())
{
    // Verify credentials in config
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
- Verify Gradle version compatibility
- Check for duplicate AndroidManifest entries

## Support

- **Email:** sdk@deskillz.games
- **Discord:** [discord.gg/deskillz](https://discord.gg/deskillz)
- **Documentation:** [docs.deskillz.games](https://docs.deskillz.games)
- **Developer Portal:** [deskillz.games/developer](https://deskillz.games/developer)

## License

Copyright © 2025 Deskillz.Games. All rights reserved.

---

<p align="center">
  Made with ❤️ by <a href="https://deskillz.games">Deskillz.Games</a>
</p>