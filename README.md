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
  <a href="#navigation-deep-links">Navigation Links</a> •
  <a href="#documentation">Documentation</a> •
  <a href="#support">Support</a>
</p>

---

## Overview

The Deskillz Unreal SDK enables game developers to integrate their Unreal Engine games with the Deskillz.Games competitive gaming platform. Players can compete in skill-based tournaments and win cryptocurrency prizes.

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
// Called when player navigates from Deskillz app to your game
void AMyGameMode::HandleNavigation(
    EDeskillzNavigationAction Action, 
    const TMap<FString, FString>& Parameters)
{
    switch (Action)
    {
        case EDeskillzNavigationAction::Tournaments:
            UDeskillzUIManager::Get()->ShowTournamentList();
            break;
        case EDeskillzNavigationAction::Wallet:
            UDeskillzUIManager::Get()->ShowWallet();
            break;
        case EDeskillzNavigationAction::Profile:
            UDeskillzUIManager::Get()->ShowProfile();
            break;
        case EDeskillzNavigationAction::Game:
            {
                FString GameId = Parameters.FindRef(TEXT("id"));
                ShowGameDetails(GameId);
            }
            break;
        case EDeskillzNavigationAction::Settings:
            ShowSettingsScreen();
            break;
    }
}
```

### 3. Handle Match Launch Deep Links

```cpp
// Simple handler - just matchId and token
void AMyGameMode::HandleMatchLaunch(const FString& MatchId, const FString& AuthToken)
{
    // Store auth token for API calls
    UDeskillzApiService::Get()->SetAuthToken(AuthToken);
    
    // Load match level
    UGameplayStatics::OpenLevel(this, TEXT("MatchLevel"));
}

// Full match data handler
void AMyGameMode::OnMatchReady(const FDeskillzMatch& Match)
{
    UE_LOG(LogDeskillz, Log, TEXT("Match ready: %s"), *Match.MatchId);
    UE_LOG(LogDeskillz, Log, TEXT("Opponent: %s"), *Match.Opponent.Username);
    UE_LOG(LogDeskillz, Log, TEXT("Entry Fee: %f %s"), Match.EntryFee, *Match.Currency);
    
    CurrentMatch = Match;
    StartGameplay();
}
```

### 4. Handle Deep Links (Blueprint)

1. Add **Deskillz Initialize** node to your Game Mode's BeginPlay
2. Bind to **On Navigation Received** event (NEW)
3. Bind to **On Match Launch Received** event (NEW)
4. Bind to **On Match Ready** event
5. When match ready → Start your gameplay

### 5. Submit Score (C++)

```cpp
// When game ends - score is automatically signed with HMAC-SHA256
void AMyGameMode::OnGameComplete(int32 FinalScore)
{
    float Duration = GetWorld()->GetTimeSeconds() - MatchStartTime;
    
    UDeskillzSecureSubmitter* Submitter = UDeskillzSecureSubmitter::Get();
    Submitter->OnScoreSubmitted.AddDynamic(this, &AMyGameMode::OnScoreSubmitted);
    Submitter->SubmitScore(FinalScore, Duration);
}
```

### 6. Handle Results

```cpp
void AMyGameMode::OnMatchComplete(const FDeskillzMatchResult& Result)
{
    if (Result.bIsWinner)
    {
        ShowVictoryScreen(Result.PrizeAmount, Result.CryptoType);
    }
    else
    {
        ShowResultsScreen(Result.FinalRanking, Result.WinnerScore);
    }
    
    // Return player to Deskillz app
    UDeskillzSDK::Get()->ReturnToLobby();
}
```

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
| `deskillz://launch?matchId=xxx&token=yyy` | Match Launch | Launch into a match |

### Navigation Action Enum

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
UDeskillzDeepLinkHandler* Handler = UDeskillzDeepLinkHandler::Get();
Handler->SimulateDeepLink(TEXT("deskillz://tournaments"));
Handler->SimulateDeepLink(TEXT("deskillz://wallet"));
Handler->SimulateDeepLink(TEXT("deskillz://game?id=battle-blocks"));
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
  currency=<BTC|ETH|USDT|etc>
```

The SDK parses this automatically - you just handle the `OnMatchReady` or `OnMatchLaunchReceived` events.

## Architecture: Global Lobby vs Old SDK-Based

| Aspect | Old (SDK-Based) | New (Global Lobby) ✅ |
|--------|-----------------|----------------------|
| Matchmaking Location | Inside your game | Deskillz website/app |
| Player Pool | Fragmented per-game | Unified across all games |
| User Experience | Inconsistent | Consistent platform UI |
| SDK Complexity | High (matchmaking logic) | Low (deep links only) |
| Developer Burden | Heavy | Minimal |
| Private Rooms | Complex to implement | Built into platform |
| NPC Opponents | SDK handles | Platform handles |
| Navigation | N/A | Full deep link support |

## Features

| Feature | Description |
|---------|-------------|
| **Navigation Deep Links** | Navigate to any screen from Deskillz app (NEW) |
| **Match Launch Deep Links** | Receive match data from Global Lobby |
| **Blueprint Support** | Full Blueprint integration |
| **C++ API** | Native C++ for advanced integration |
| **Score Encryption** | HMAC-SHA256 signed submission |
| **Anti-Cheat** | Built-in validation and monitoring |
| **UMG Widgets** | Optional UI components |
| **Analytics** | Track player engagement metrics |
| **Sync/Async Support** | Both game modes supported |

## Plugin Structure

```
Source/DeskillzSDK/
├── Public/
│   ├── Core/           # Initialization, config
│   ├── Lobby/          # Deep link handling (PRIMARY)
│   │   └── DeskillzDeepLinkHandler.h  # Navigation + Match launch
│   ├── Match/          # Match lifecycle
│   ├── Security/       # Score encryption
│   ├── Network/        # API service
│   └── UI/             # Optional UMG widgets
└── Private/
    └── [Implementation files]
```

## Blueprint Nodes

### Core Nodes
- `Deskillz Initialize` - Initialize the SDK
- `Deskillz Submit Score` - Submit game score
- `Deskillz Return to Lobby` - Return to Deskillz app

### Event Nodes (Primary Integration)
- `On Navigation Received` - Navigation deep link (NEW)
- `On Match Launch Received` - Match launch with ID and token (NEW)
- `On Match Ready` - Full parsed match data, ready to play
- `On Score Submitted` - Score submission confirmed
- `On Match Complete` - Final results received
- `On Error` - Handle errors

### UI Nodes (Optional)
- `Show Match HUD` - Display in-game HUD
- `Show Results Screen` - Display match results
- `Show Tournament List` - Display tournaments
- `Show Wallet` - Display wallet

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
<activity android:name=".MainActivity"
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

Or add to `DefaultEngine.ini`:

```ini
[/Script/AndroidRuntimeSettings.AndroidRuntimeSettings]
bEnableGooglePlaySupport=False
MinSDKVersion=21
```

The SDK automatically configures deep link intent filters.

## Test Mode

Test your integration without real currency:

```cpp
// Enable sandbox environment
Config.Environment = EDeskillzEnvironment::Sandbox;

// Enable verbose logging
Config.bEnableLogging = true;
Config.LogLevel = EDeskillzLogLevel::Verbose;

// Test navigation deep links
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://tournaments"));
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://wallet"));

// Test match launch
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(
    TEXT("deskillz://launch?matchId=test-123&token=test-token"));
```

## Documentation

- [Quick Start Guide](QUICKSTART.md)
- [Integration Guide](INTEGRATION_GUIDE.md)
- [API Reference](API_REFERENCE.md)
- [Blueprint Guide](./Docs/BLUEPRINT_GUIDE.md)
- [Deep Link Guide](./Docs/DEEP_LINK_GUIDE.md)
- [Online Docs](https://docs.deskillz.games/unreal)

## Sample Project

Check out our sample game implementation:
[Deskillz Unreal Sample](https://github.com/Deskillz-Games-Development/unreal-sample)

## Changelog

See [CHANGELOG.md](./CHANGELOG.md) for version history.

### v2.0.0 (Latest)
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
1. Ensure `ProcessPendingDeepLinks()` is called after initialization
2. Verify event bindings are set up before processing
3. Test with `SimulateDeepLink()` first

### Plugin not showing in Editor
1. Ensure plugin is in `Plugins/DeskillzSDK/` folder
2. Regenerate project files
3. Rebuild the project

### iOS build errors
- Ensure Xcode 14+ is installed
- Check iOS deployment target is 12.0+
- Verify signing certificates

## Support

- **Documentation:** [docs.deskillz.games](https://docs.deskillz.games)
- **Developer Portal:** [deskillz.games/developer](https://deskillz.games/developer)
- **Email:** developers@deskillz.games
- **Discord:** [Join our community](https://discord.gg/deskillz)

## License

This SDK is licensed under the MIT License. See [LICENSE](./LICENSE) for details.

---

<p align="center">
  Made with ❤️ by <a href="https://deskillz.games">Deskillz.Games</a>
</p>