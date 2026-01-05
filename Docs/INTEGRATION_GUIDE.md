# Deskillz SDK - Integration Guide

Complete guide for integrating Deskillz tournaments into your Unreal Engine game.

## Table of Contents

1. [Game Type Selection](#game-type-selection)
2. [Asynchronous Integration](#asynchronous-integration)
3. [Synchronous Integration](#synchronous-integration)
4. [Deep Link Navigation](#deep-link-navigation) ← NEW
5. [Wallet Integration](#wallet-integration)
6. [Analytics](#analytics)
7. [UI Customization](#ui-customization)
8. [Platform Setup](#platform-setup)
9. [Security Best Practices](#security-best-practices)
10. [Testing](#testing)
11. [Auto-Updater Integration](#auto-updater-integration) 

---

## Game Type Selection

### Asynchronous (Score Attack)
Players compete independently. Best for: puzzle games, endless runners, score-attack games.

### Synchronous (Real-time)
Players compete simultaneously. Best for: fighting games, racing, real-time competitive.

---

## Asynchronous Integration

```cpp
// 1. Initialize SDK
FDeskillzConfig Config;
Config.GameId = TEXT("your_game_id");
Config.ApiKey = TEXT("your_api_key");
UDeskillzSDK::Get()->Initialize(Config);

// 2. Enter Tournament
UDeskillzMatchmaking::Get()->OnMatchFound.AddDynamic(this, &OnMatchFound);
UDeskillzMatchmaking::Get()->StartMatchmaking(TournamentId);

// 3. Handle Match Found
void OnMatchFound(const FDeskillzMatch& Match)
{
    UDeskillzMatchManager::Get()->StartMatch(Match.MatchId);
    StartYourGameplay();
}

// 4. Submit Score
void OnGameplayComplete(int64 Score, float Duration)
{
    UDeskillzSecureSubmitter::Get()->SubmitScore(Score, Duration);
}
```

---

## Synchronous Integration

```cpp
// 1. After match found, connect WebSocket
UDeskillzWebSocket* WS = UDeskillzWebSocket::Get();
WS->OnConnected.AddDynamic(this, &OnConnected);
WS->Connect(WebSocketUrl);

// 2. Join Room
void OnConnected() { WS->JoinRoom(MatchId); }

// 3. Send State (in Tick)
TSharedPtr<FJsonObject> State = MakeShareable(new FJsonObject());
State->SetNumberField(TEXT("score"), CurrentScore);
State->SetNumberField(TEXT("x"), Position.X);
WS->SendGameState(State);

// 4. Receive Opponent State
WS->OnMessageReceived.AddLambda([](const FString& Event, TSharedPtr<FJsonObject> Data) {
    if (Event == TEXT("game_state")) {
        // Update opponent visualization
    }
});
```

---

## Deep Link Navigation (NEW in v2.0)

The Deskillz platform uses deep links to navigate users from the main app to your game. Your game must handle these navigation events to provide a seamless user experience.

### Why Deep Links?

In the centralized lobby architecture:
1. Users browse tournaments in the main Deskillz app
2. When ready to play, the platform sends a deep link to your game
3. Your game opens directly to the relevant screen (match, tournament, etc.)

### Setup Deep Link Handler

```cpp
#include "Lobby/DeskillzDeepLinkHandler.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the deep link handler
    UDeskillzDeepLinkHandler* Handler = UDeskillzDeepLinkHandler::Get();
    
    // Subscribe to navigation events
    Handler->OnNavigationReceived.AddDynamic(this, &AMyGameMode::HandleNavigation);
    
    // Subscribe to match launch events
    Handler->OnMatchLaunchReceived.AddDynamic(this, &AMyGameMode::HandleMatchLaunch);
    
    // Check for pending deep links (app was launched via deep link)
    Handler->ProcessPendingDeepLinks();
}
```

### Handle Navigation Events

```cpp
void AMyGameMode::HandleNavigation(EDeskillzNavigationAction Action, const TMap<FString, FString>& Parameters)
{
    switch (Action)
    {
        case EDeskillzNavigationAction::Tournaments:
            // User wants to see tournament list
            UDeskillzUIManager::Get()->ShowTournamentList();
            break;
            
        case EDeskillzNavigationAction::Wallet:
            // User wants to see their wallet
            UDeskillzUIManager::Get()->ShowWallet();
            break;
            
        case EDeskillzNavigationAction::Profile:
            // User wants to see their profile
            UDeskillzUIManager::Get()->ShowProfile();
            break;
            
        case EDeskillzNavigationAction::Game:
            {
                // User wants to see a specific game's details
                FString GameId = Parameters.FindRef(TEXT("id"));
                if (!GameId.IsEmpty())
                {
                    ShowGameDetails(GameId);
                }
            }
            break;
            
        case EDeskillzNavigationAction::Settings:
            // User wants to see settings
            ShowSettingsScreen();
            break;
            
        default:
            // Unknown action - show default screen
            ShowMainMenu();
            break;
    }
}
```

### Handle Match Launch

```cpp
void AMyGameMode::HandleMatchLaunch(const FString& MatchId, const FString& AuthToken)
{
    // Store the auth token for API calls
    UDeskillzApiService::Get()->SetAuthToken(AuthToken);
    
    // Log the match launch
    UE_LOG(LogDeskillz, Log, TEXT("Launching match: %s"), *MatchId);
    
    // Fetch match details
    UDeskillzApiService::Get()->GetMatchDetails(MatchId, 
        FOnMatchDetailsResult::CreateLambda([this](bool bSuccess, const FDeskillzMatch& Match)
        {
            if (bSuccess)
            {
                // Store current match
                CurrentMatch = Match;
                
                // Transition to match level
                LoadMatchLevel(Match);
            }
            else
            {
                // Show error
                UDeskillzUIManager::Get()->ShowPopup(
                    TEXT("Error"),
                    TEXT("Failed to load match details. Please try again.")
                );
            }
        })
    );
}
```

### Deep Link URL Formats

| URL Pattern | Action | Parameters |
|-------------|--------|------------|
| `deskillz://tournaments` | Show Tournaments | None |
| `deskillz://wallet` | Show Wallet | None |
| `deskillz://profile` | Show Profile | None |
| `deskillz://game?id={gameId}` | Show Game | `id`: Game ID |
| `deskillz://settings` | Show Settings | None |
| `deskillz://launch?matchId={id}&token={jwt}` | Launch Match | `matchId`, `token` |

### Blueprint Integration

For Blueprint-only projects:

1. Add **Deskillz Deep Link Handler** component to your GameMode
2. In the Details panel, find the Events section
3. Click **+** next to `OnNavigationReceived` and `OnMatchLaunchReceived`
4. Implement your navigation logic in the event graph

### Testing Deep Links

```cpp
// Simulate navigation deep link
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://tournaments"));

// Simulate match launch deep link
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(
    TEXT("deskillz://launch?matchId=test-123&token=test-token")
);
```

### Platform-Specific Setup

See [Platform Setup](#platform-setup) section for iOS Info.plist and Android AndroidManifest.xml configuration.

---

## Wallet Integration

```cpp
// Get Balance
UDeskillzApiService::Get()->GetWalletBalance(
    FOnWalletResult::CreateLambda([](bool bSuccess, const TArray<FDeskillzWalletBalance>& Balances) {
        for (const auto& B : Balances)
            UE_LOG(LogTemp, Log, TEXT("%s: %.4f"), *B.Currency, B.Available);
    })
);

// Show Wallet UI
UDeskillzUIManager::Get()->ShowWallet();
```

---

## Analytics

```cpp
// Track Events
TMap<FString, FString> Params;
Params.Add(TEXT("level"), TEXT("5"));
Params.Add(TEXT("score"), TEXT("15000"));
UDeskillzAnalytics::Get()->TrackEvent(TEXT("level_complete"), EDeskillzEventCategory::Gameplay, Params);

// Track Timed Events
UDeskillzEventTracker::Get()->StartTimedEvent(TEXT("boss_fight"));
// ... gameplay ...
UDeskillzEventTracker::Get()->EndTimedEvent(TEXT("boss_fight"));

// Track Revenue
UDeskillzEventTracker::Get()->TrackEntryFee(TournamentId, TEXT("USDT"), 10.0);
```

---

## UI Customization

```cpp
FDeskillzUITheme Theme;
Theme.PrimaryColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);
Theme.AccentColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
Theme.CornerRadius = 8.0f;
UDeskillzUIManager::Get()->SetTheme(Theme);
```

---

## Platform Setup

### iOS - Info.plist

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

### Android - AndroidManifest.xml

```xml
<activity android:name=".MainActivity"
          android:launchMode="singleTask">
    <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:scheme="deskillz" />
    </intent-filter>
    <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:scheme="yourgame" />
    </intent-filter>
</activity>
```

**Important:** Use `android:launchMode="singleTask"` to ensure deep links are handled by the existing app instance.

---

## Security Best Practices

1. Never hardcode API keys
2. Enable anti-cheat in production
3. Use HTTPS only
4. Validate server responses
5. Log security events
6. Validate deep link tokens before trusting them

```cpp
#if UE_BUILD_SHIPPING
    Config.Environment = EDeskillzEnvironment::Production;
    Config.bEnableLogging = false;
    Config.bEnableAntiCheat = true;
#endif
```

---

## Testing

```cpp
// Enable verbose logging
Config.bEnableLogging = true;
Config.LogLevel = EDeskillzLogLevel::Verbose;

// Use mock server for testing
FDeskillzMockServer::Get()->Start();
FDeskillzTestScenarios::SetupHappyPath();

// Test deep links
UDeskillzDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://tournaments"));
```
---

## Auto-Updater Integration ← NEW in v2.3

Ensure players always have the latest version of your game.

### Basic Setup (Unreal)
```cpp
#include "Core/DeskillzUpdater.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    Updater->SetCurrentVersion(TEXT("1.0.0"), 10000);
    
    // Subscribe to update events
    Updater->OnForceUpdateRequired.AddDynamic(this, &AMyGameMode::HandleForcedUpdate);
    Updater->OnUpdateAvailable.AddDynamic(this, &AMyGameMode::HandleOptionalUpdate);
    Updater->OnNoUpdateNeeded.AddDynamic(this, &AMyGameMode::HandleNoUpdate);
    Updater->OnUpdateCheckFailed.AddDynamic(this, &AMyGameMode::HandleCheckFailed);
    
    // Check for updates
    Updater->CheckForUpdates();
}

void AMyGameMode::HandleForcedUpdate(const FUpdateInfo& Info)
{
    // REQUIRED: Show blocking dialog - user MUST update
    ShowForcedUpdateUI(Info.LatestVersion, Info.ReleaseNotes, Info.DownloadUrl);
}

void AMyGameMode::HandleOptionalUpdate(const FUpdateInfo& Info)
{
    // Optional: Show update prompt with Update/Skip buttons
    ShowUpdatePrompt(Info.LatestVersion, Info.FileSizeFormatted);
}

void AMyGameMode::HandleNoUpdate()
{
    // App is up to date - continue normal flow
    StartGame();
}

void AMyGameMode::HandleCheckFailed(const FString& Error)
{
    // Network error - allow game to continue
    UE_LOG(LogTemp, Warning, TEXT("Update check failed: %s"), *Error);
    StartGame();
}
```

### Basic Setup (Unity)
```csharp
using Deskillz;

void Start()
{
    DeskillzUpdater.Instance.CurrentVersion = Application.version;
    DeskillzUpdater.Instance.CurrentVersionCode = GetVersionCode();
    
    DeskillzUpdater.OnForcedUpdateRequired += HandleForcedUpdate;
    DeskillzUpdater.OnUpdateAvailable += HandleOptionalUpdate;
    DeskillzUpdater.OnNoUpdateNeeded += () => StartGame();
    DeskillzUpdater.OnUpdateCheckFailed += (error) => StartGame(); // Continue on error
    
    DeskillzUpdater.Instance.CheckForUpdates();
}

void HandleForcedUpdate(UpdateInfo info)
{
    // Show blocking UI - must update to continue
    DeskillzUpdaterUI.Instance.ShowUpdateDialog(info, blocking: true);
}

void HandleOptionalUpdate(UpdateInfo info)
{
    // Show optional dialog with Update/Skip buttons
    DeskillzUpdaterUI.Instance.ShowUpdateDialog(info, blocking: false);
}
```

### Pre-built Update UI (Unity Only)

For quick integration, use the built-in UI:
```csharp
// Enable automatic UI handling
DeskillzUpdaterUI.Instance.ShowOnUpdateAvailable = true;
DeskillzUpdaterUI.Instance.BlockOnForcedUpdate = true;

// Customize appearance (optional)
DeskillzUpdaterUI.Instance.UpdateTitle = "New Version Available!";
DeskillzUpdaterUI.Instance.UpdateButtonText = "Update Now";
DeskillzUpdaterUI.Instance.SkipButtonText = "Later";

// Start update check - UI handles everything
DeskillzUpdater.Instance.CheckForUpdates();
```

### Custom UI Integration

Handle events and show your own UI:

| Event | Action |
|-------|--------|
| `OnUpdateAvailable` | Show optional dialog with Update/Skip buttons |
| `OnForcedUpdateRequired` | Show blocking dialog - Update button only |
| `OnUpdateSkipped` | User chose to skip - continue to game |
| `OnUpdateAccepted` | Open download page automatically |

### Handling User Actions
```csharp
// When user clicks "Update" button
public void OnUpdateButtonClicked()
{
    DeskillzUpdater.Instance.StartUpdate(); // Opens download URL
}

// When user clicks "Skip" button (optional updates only)
public void OnSkipButtonClicked()
{
    DeskillzUpdater.Instance.SkipUpdate(); // Remembers skipped version
    StartGame();
}
```

### Testing Updates
```csharp
// Unity - Enable test mode
DeskillzUpdater.Instance.TestMode = true;
DeskillzUpdater.Instance.TestUpdateAvailable = true;
DeskillzUpdater.Instance.TestForceUpdate = false; // or true for forced
DeskillzUpdater.Instance.CheckForUpdates();
```
```cpp
// Unreal - Enable test mode
UDeskillzUpdater::Get()->SetTestMode(true);
UDeskillzUpdater::Get()->SetTestUpdateAvailable(true);
UDeskillzUpdater::Get()->SetTestForceUpdate(false);
UDeskillzUpdater::Get()->CheckForUpdates();
```

### Best Practices

1. **Always check on startup** - Call `CheckForUpdates()` when game launches
2. **Handle forced updates properly** - Block all gameplay until updated
3. **Graceful failures** - If check fails, allow game to continue
4. **Show release notes** - Help users understand what's new
5. **Test both paths** - Test optional and forced update flows

---
See [API Reference](API_REFERENCE.md) for complete method documentation.