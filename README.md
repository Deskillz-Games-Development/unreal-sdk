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
│     deskillz://match?id=abc123&token=xyz...                     │
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
1. Handle deep links
2. Start the match
3. Submit scores securely

## Requirements

- **Unreal Engine:** 5.1 or newer
- **Platforms:** iOS 12+, Android API 21+
- **Build Tools:** Visual Studio 2022 (Windows) or Xcode 14+ (macOS)

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

## Quick Start

### 1. Initialize the SDK (C++)

```cpp
#include "DeskillzSDK.h"
#include "DeskillzDeepLinkHandler.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize SDK
    FDeskillzConfig Config;
    Config.GameId = TEXT("your-game-id");
    Config.ApiKey = TEXT("your-api-key");
    Config.Environment = EDeskillzEnvironment::Sandbox;
    
    UDeskillzSDK::Initialize(Config);
    
    // Register for deep link events (THIS IS THE KEY INTEGRATION)
    UDeskillzDeepLinkHandler::OnDeepLinkReceived.AddDynamic(
        this, &AMyGameMode::HandleDeepLink);
    UDeskillzDeepLinkHandler::OnMatchReady.AddDynamic(
        this, &AMyGameMode::OnMatchReady);
}
```

### 2. Handle Deep Links (Primary Integration Point)

```cpp
// Called when player launches your game from the Deskillz Global Lobby
void AMyGameMode::HandleDeepLink(const FDeskillzDeepLinkData& Data)
{
    UE_LOG(LogDeskillz, Log, TEXT("Deep link received: %s"), *Data.MatchId);
    
    // SDK automatically parses the deep link and prepares match data
    // The OnMatchReady event will fire when everything is ready
}

void AMyGameMode::OnMatchReady(const FDeskillzMatch& Match)
{
    // Match data received from the Deskillz platform
    UE_LOG(LogDeskillz, Log, TEXT("Match ready: %s"), *Match.MatchId);
    UE_LOG(LogDeskillz, Log, TEXT("Opponent: %s"), *Match.Opponent.Username);
    UE_LOG(LogDeskillz, Log, TEXT("Entry Fee: %f %s"), Match.EntryFee, *Match.Currency);
    UE_LOG(LogDeskillz, Log, TEXT("Game Mode: %s"), 
        Match.GameMode == EDeskillzGameMode::Sync ? TEXT("SYNC") : TEXT("ASYNC"));
    
    // Store match reference
    CurrentMatch = Match;
    
    // Start your gameplay
    StartGameplay();
}
```

### 3. Handle Deep Links (Blueprint)

1. Add **Deskillz Initialize** node to your Game Mode's BeginPlay
2. Bind to **On Deep Link Received** event
3. Bind to **On Match Ready** event
4. When match ready → Start your gameplay

### 4. Submit Score (C++)

```cpp
// When game ends - score is automatically signed with HMAC-SHA256
void AMyGameMode::OnGameComplete(int32 FinalScore)
{
    FScoreSubmission Submission;
    Submission.Score = FinalScore;
    Submission.MatchId = CurrentMatch.MatchId;
    Submission.Metadata.Add(TEXT("level"), FString::FromInt(CurrentLevel));
    Submission.Metadata.Add(TEXT("accuracy"), FString::SanitizeFloat(HitAccuracy));
    
    UDeskillzSDK::SubmitScore(Submission,
        FOnScoreSubmitted::CreateUObject(this, &AMyGameMode::OnScoreSubmitted));
}
```

### 5. Handle Results

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
    UDeskillzSDK::ReturnToLobby();
}
```

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

## Features

| Feature | Description |
|---------|-------------|
| **Deep Link Handling** | Receive match data from Global Lobby |
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
│   ├── DeepLink/       # Deep link handling (PRIMARY)
│   ├── Match/          # Match lifecycle
│   ├── Security/       # Score encryption
│   └── UI/             # Optional UMG widgets
└── Private/
    └── [Implementation files]
```

## Deep Link Format

Your game will receive deep links in this format:

```
deskillz://match?
  id=<match_id>&
  token=<auth_token>&
  game=<game_id>&
  mode=<SYNC|ASYNC>&
  opponent=<opponent_id>&
  entry_fee=<amount>&
  currency=<BTC|ETH|USDT|etc>
```

The SDK parses this automatically - you just handle the `OnMatchReady` event.

## Blueprint Nodes

### Core Nodes
- `Deskillz Initialize` - Initialize the SDK
- `Deskillz Submit Score` - Submit game score
- `Deskillz Return to Lobby` - Return to Deskillz app

### Event Nodes (Primary Integration)
- `On Deep Link Received` - Raw deep link data
- `On Match Ready` - Parsed match data, ready to play
- `On Score Submitted` - Score submission confirmed
- `On Match Complete` - Final results received
- `On Error` - Handle errors

### UI Nodes (Optional)
- `Show Match HUD` - Display in-game HUD
- `Show Results Screen` - Display match results

## iOS Setup

Add URL scheme to `Info.plist`:

```xml
<key>CFBundleURLTypes</key>
<array>
  <dict>
    <key>CFBundleURLSchemes</key>
    <array>
      <string>deskillz</string>
      <string>deskillz-yourgameid</string>
    </array>
  </dict>
</array>
```

## Android Setup

Add to `DefaultEngine.ini`:

```ini
[/Script/AndroidRuntimeSettings.AndroidRuntimeSettings]
bEnableGooglePlaySupport=False
MinSDKVersion=21
```

The SDK automatically configures deep link intent filters.

## Documentation

- [API Reference](./Docs/API_REFERENCE.md)
- [Blueprint Guide](./Docs/BLUEPRINT_GUIDE.md)
- [Deep Link Guide](./Docs/DEEP_LINK_GUIDE.md)
- [Online Docs](https://docs.deskillz.games/unreal)

## Sample Project

Check out our sample game implementation:
[Deskillz Unreal Sample](https://github.com/Deskillz-Games-Development/unreal-sample)

## Changelog

See [CHANGELOG.md](./CHANGELOG.md) for version history.

## Troubleshooting

### Deep links not working
1. Verify URL schemes are configured correctly
2. Check app is properly signed
3. Test with: `adb shell am start -a android.intent.action.VIEW -d "deskillz://match?id=test"`

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
