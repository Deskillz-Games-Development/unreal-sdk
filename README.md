# Deskillz Unreal SDK

<p align="center">
  <img src="https://deskillz.games/logo.png" alt="Deskillz.Games" width="200"/>
</p>

<p align="center">
  <strong>Integrate competitive tournaments into your Unreal Engine games</strong>
</p>

<p align="center">
  <a href="#installation">Installation</a> •
  <a href="#quick-start">Quick Start</a> •
  <a href="#features">Features</a> •
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
- **Platforms:** iOS 12+, Android API 21+
- **Build Tools:** Visual Studio 2019+ (Windows) or Xcode 14+ (macOS)

## Installation

### Option 1: Download Release (Recommended)

1. Download the [latest release](https://github.com/Deskillz-Games-Development/unreal-sdk/releases/latest)
2. Extract to your project's `Plugins/` folder
3. Regenerate project files
4. Enable the plugin in **Edit → Plugins → Deskillz SDK**

### Option 2: Clone Repository

```bash
cd YourProject/Plugins
git clone https://github.com/Deskillz-Games-Development/unreal-sdk.git DeskillzSDK
```

### Option 3: Copy to Plugins Folder

1. Download the Deskillz SDK
2. Extract to your project's `Plugins` folder:

```
YourProject/
├── Content/
├── Source/
└── Plugins/
    └── Deskillz/           ← Extract here
        ├── Deskillz.uplugin
        └── Source/
```

3. Regenerate project files (right-click `.uproject` → "Generate Visual Studio project files")

### Add to Build.cs

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Deskillz" });
```

## Quick Start

### 1. Initialize the SDK (C++)

```cpp
#include "Core/DeskillzSDK.h"
#include "Lobby/DeskillzDeepLinkHandler.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize SDK
    FDeskillzConfig Config;
    Config.GameId = TEXT("your-game-id");
    Config.ApiKey = TEXT("your-api-key");
    Config.Environment = EDeskillzEnvironment::Sandbox;
    Config.bEnableLogging = true;
    Config.bEnableAnalytics = true;
    Config.bEnableAntiCheat = true;
    
    UDeskillzSDK::Get()->Initialize(Config);
    
    // Register for deep link events
    UDeskillzDeepLinkHandler* Handler = UDeskillzDeepLinkHandler::Get();
    
    // Navigation events (NEW in v2.0)
    Handler->OnNavigationReceived.AddDynamic(this, &AMyGameMode::HandleNavigation);
    
    // Match launch events
    Handler->OnMatchLaunchReceived.AddDynamic(this, &AMyGameMode::HandleMatchLaunch);
    Handler->OnMatchReady.AddDynamic(this, &AMyGameMode::OnMatchReady);
    
    // Process any pending deep links (cold start)
    Handler->ProcessPendingDeepLinks();
}
```

### 2. Handle Navigation Deep Links (NEW in v2.0)

```cpp
void AMyGameMode::HandleNavigation(EDeskillzNavigationAction Action, 
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
        {
            const FString* GameId = Parameters.Find(TEXT("id"));
            if (GameId)
            {
                UIManager->ShowGameDetails(*GameId);
            }
            break;
        }
            
        case EDeskillzNavigationAction::Settings:
            UIManager->ShowSettings();
            break;
    }
}
```

### 3. Handle Match Launch Deep Links

```cpp
// Simple handler
void AMyGameMode::HandleMatchLaunch(const FString& MatchId, const FString& AuthToken)
{
    // Store auth for API calls
    UDeskillzSDK::Get()->SetAuthToken(AuthToken);
    
    // Load match level
    UGameplayStatics::OpenLevel(this, TEXT("MatchLevel"));
    
    UE_LOG(LogTemp, Log, TEXT("Launching match: %s"), *MatchId);
}

// Full match data handler
void AMyGameMode::OnMatchReady(const FMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Log, TEXT("Match ready: %s"), *Data.MatchId);
    UE_LOG(LogTemp, Log, TEXT("Opponent: %s (Rating: %d)"), 
           *Data.OpponentName, Data.OpponentRating);
    UE_LOG(LogTemp, Log, TEXT("Entry Fee: %.2f %s"), 
           Data.EntryFee, *Data.Currency);
    
    // Store match data
    CurrentMatchData = Data;
    
    // Start game
    StartMatch(Data);
}
```

### 4. Submit Score

```cpp
void AMyGameMode::OnGameComplete(int32 FinalScore)
{
    // Score is automatically signed with HMAC-SHA256
    FOnScoreSubmitSuccess OnSuccess;
    OnSuccess.BindLambda([]()
    {
        UE_LOG(LogTemp, Log, TEXT("Score submitted successfully!"));
    });
    
    FOnScoreSubmitError OnError;
    OnError.BindLambda([](const FString& Error)
    {
        UE_LOG(LogTemp, Error, TEXT("Score submission failed: %s"), *Error);
    });
    
    UDeskillzSDK::Get()->SubmitScore(FinalScore, OnSuccess, OnError);
}
```

### 5. Initialize the SDK (Blueprint)

1. Get **Deskillz SDK** node
2. Call **Initialize** with your API key
3. Bind to **OnMatchLaunch** and **OnNavigationReceived** events

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
| **Navigation Deep Links** | Navigate to any screen from Deskillz app (NEW) |
| **Match Launch Deep Links** | Receive match data from Global Lobby |
| **Private Rooms** | Create/join rooms with friends (NEW in v2.2) |
| **Pre-built Room UI** | Ready-to-use UMG widgets (NEW in v2.2) |
| **Asynchronous Tournaments** | Players compete separately, scores compared |
| **Real-time Multiplayer** | 2-10 players competing simultaneously |
| **Custom Stages** | Player-created private rooms |
| **Cryptocurrency Prizes** | BTC, ETH, SOL, XRP, BNB, USDT, USDC |
| **Built-in UI** | Pre-made UMG widget components with themes |
| **Anti-Cheat** | Server-side validation and protection |
| **Score Encryption** | HMAC-SHA256 signed submission |
| **Blueprint Support** | Full Blueprint integration |

## SDK Structure

```
deskillz-unreal-sdk/
├── Source/Deskillz/
│   ├── Public/
│   │   ├── Core/
│   │   │   ├── DeskillzSDK.h
│   │   │   ├── DeskillzManager.h
│   │   │   ├── DeskillzConfig.h
│   │   │   └── DeskillzTypes.h
│   │   ├── Match/
│   │   │   ├── DeskillzMatchManager.h
│   │   │   └── DeskillzMatchmaking.h
│   │   ├── Security/
│   │   │   ├── DeskillzScoreEncryption.h
│   │   │   └── DeskillzAntiCheat.h
│   │   ├── Rooms/                          # NEW in v2.2
│   │   │   ├── DeskillzRooms.h             # Main room API
│   │   │   ├── DeskillzRoomTypes.h         # Room data structures
│   │   │   └── DeskillzRoomClient.h        # HTTP/WebSocket client
│   │   ├── Widgets/
│   │   │   ├── DeskillzBaseWidget.h
│   │   │   ├── DeskillzUIManager.h
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
│   │   │   └── DeskillzLobbyClient.h
│   │   └── Network/
│   │       ├── DeskillzHttpClient.h
│   │       └── DeskillzWebSocket.h
│   ├── Private/
│   │   ├── Core/
│   │   ├── Match/
│   │   ├── Security/
│   │   ├── Rooms/                          # NEW in v2.2
│   │   │   ├── DeskillzRooms.cpp
│   │   │   └── DeskillzRoomClient.cpp
│   │   ├── Widgets/
│   │   │   └── Rooms/                      # NEW in v2.2
│   │   │       ├── DeskillzPrivateRoomUI.cpp
│   │   │       ├── DeskillzRoomListWidget.cpp
│   │   │       ├── DeskillzCreateRoomWidget.cpp
│   │   │       ├── DeskillzJoinRoomWidget.cpp
│   │   │       ├── DeskillzRoomLobbyWidget.cpp
│   │   │       └── DeskillzRoomPlayerCard.cpp
│   │   ├── Lobby/
│   │   └── Network/
│   └── Deskillz.Build.cs
├── Resources/
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
```

## Documentation

- [Quick Start Guide](https://docs.deskillz.games/unreal/quickstart)
- [API Reference](https://docs.deskillz.games/unreal/api)
- [Multiplayer Guide](https://docs.deskillz.games/unreal/multiplayer)
- [Deep Link Integration](https://docs.deskillz.games/unreal/deep-links)
- [Private Rooms Guide](https://docs.deskillz.games/unreal/private-rooms)
- [Blueprint Integration](https://docs.deskillz.games/unreal/blueprints)
- [Troubleshooting](https://docs.deskillz.games/unreal/troubleshooting)

## Sample Project

Check out our sample game implementation:
[Deskillz Unreal Sample](https://github.com/deskillz/unreal-sample)

## Changelog

See [CHANGELOG.md](./CHANGELOG.md) for version history.

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

Copyright © 2024 Deskillz.Games. All rights reserved.

---

<p align="center">
  Made with ❤️ by <a href="https://deskillz.games">Deskillz.Games</a>
</p>