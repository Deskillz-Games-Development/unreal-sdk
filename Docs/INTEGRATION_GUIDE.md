# Deskillz SDK - Integration Guide

**SDK Version: 2.6.0** | Complete guide for integrating the Deskillz SDK into your Unity or Unreal Engine game.

## Table of Contents

1. [Overview](#1-overview)
2. [Installation](#2-installation)
3. [SDK Initialization](#3-sdk-initialization)
4. [Deep Link Integration (SDK 2.0+)](#4-deep-link-integration-sdk-20)
5. [Match Flow](#5-match-flow)
6. [Score Submission](#6-score-submission)
7. [Multiplayer Synchronization](#7-multiplayer-synchronization)
8. [Private Rooms](#8-private-rooms)
9. [Auto-Updater](#9-auto-updater)
10. [Testing](#10-testing)
11. [Private Rooms Enhancement](#11-private-rooms-enhancement) - NEW
12. [Host System Integration](#12-host-system-integration) - NEW
13. [Social Games Integration](#13-social-games-integration) - NEW
14. [Spectator Mode Integration](#14-spectator-mode-integration) - NEW

---

## 1. Overview

The Deskillz SDK enables your game to participate in the Deskillz competitive gaming platform. Players browse and enter tournaments through the main Deskillz app, then your game is launched via deep link to play matches.

### Architecture (SDK 2.0+)

```
[Deskillz Main App] --> [Deep Link] --> [Your Game]
     |                                       |
     |<---- Score Submission + Return -------|
```

Key concepts:
- **Main App Handles**: Tournament browsing, matchmaking, wallet, leaderboards
- **Your Game Handles**: Gameplay, score calculation, match completion
- **Deep Links**: Launch your game with match data
- **Score Submission**: Secure, encrypted score reporting

### Supported Platforms

| Platform | Unity | Unreal |
|----------|-------|--------|
| Android | Yes | Yes |
| iOS | Yes | Yes |
| Windows | Yes | Yes |
| macOS | Yes | Yes |

---

## 2. Installation

### Unity Installation

1. Download the Deskillz Unity SDK package from the Developer Portal
2. Import the package: Assets > Import Package > Custom Package
3. Select `DeskillzSDK.unitypackage`
4. Import all files

**Package Manager (Alternative):**
```json
{
  "dependencies": {
    "com.deskillz.sdk": "https://github.com/Deskillz-Games-Development/unity-sdk.git#v2.6.0"
  }
}
```

### Unreal Installation

1. Download the Deskillz Unreal SDK from the Developer Portal
2. Copy the `DeskillzSDK` folder to your project's `Plugins` directory
3. Regenerate project files
4. Enable the plugin in Edit > Plugins > Deskillz SDK

**Project Structure:**
```
YourProject/
  Plugins/
    DeskillzSDK/
      Source/
      Resources/
      DeskillzSDK.uplugin
```

---

## 3. SDK Initialization

### Unity Initialization

```csharp
using Deskillz;

public class GameManager : MonoBehaviour
{
    void Awake()
    {
        var config = new DeskillzConfig
        {
            GameId = "your-game-id",
            ApiKey = "your-api-key",
            Environment = DeskillzEnvironment.Sandbox,
            EnableLogging = true,
            EnableAnalytics = true,
            EnableAntiCheat = true
        };
        
        DeskillzSDK.Instance.Initialize(config);
        
        // Subscribe to SDK events
        DeskillzSDK.Instance.OnInitialized += HandleInitialized;
        DeskillzSDK.Instance.OnError += HandleError;
    }
    
    void HandleInitialized()
    {
        Debug.Log("Deskillz SDK initialized");
        
        // Initialize deep link handler
        DeepLinkHandler.Instance.Initialize();
    }
    
    void HandleError(string error)
    {
        Debug.LogError($"SDK Error: {error}");
    }
}
```

### Unreal Initialization

```cpp
#include "DeskillzSDK.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    FDeskillzConfig Config;
    Config.GameId = TEXT("your-game-id");
    Config.ApiKey = TEXT("your-api-key");
    Config.Environment = EDeskillzEnvironment::Sandbox;
    Config.bEnableLogging = true;
    Config.bEnableAnalytics = true;
    Config.bEnableAntiCheat = true;
    
    UDeskillzSDK::Get()->Initialize(Config);
    
    // Bind to events
    UDeskillzSDK::Get()->OnInitialized.AddDynamic(this, &AMyGameMode::HandleInitialized);
    UDeskillzSDK::Get()->OnError.AddDynamic(this, &AMyGameMode::HandleError);
}

void AMyGameMode::HandleInitialized()
{
    UE_LOG(LogTemp, Log, TEXT("Deskillz SDK initialized"));
    
    // Initialize deep link handler
    UDeepLinkHandler::Get()->Initialize();
}

void AMyGameMode::HandleError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("SDK Error: %s"), *Error);
}
```

---

## 4. Deep Link Integration (SDK 2.0+)

In SDK 2.0+, your game is launched via deep link from the main Deskillz app with match data.

### URL Scheme Setup

**Unity (Android):** Add to `AndroidManifest.xml`:
```xml
<activity android:name="com.unity3d.player.UnityPlayerActivity">
    <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:scheme="deskillz-yourgameid" />
    </intent-filter>
</activity>
```

**Unity (iOS):** Add to `Info.plist`:
```xml
<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLSchemes</key>
        <array>
            <string>deskillz-yourgameid</string>
        </array>
    </dict>
</array>
```

### Handling Match Launch

**Unity:**
```csharp
using Deskillz;
using Deskillz.Lobby;

public class MatchLauncher : MonoBehaviour
{
    void Start()
    {
        DeepLinkHandler.Instance.Initialize();
        
        // Primary event for match launches
        DeepLinkHandler.Instance.OnMatchReady += HandleMatchReady;
        
        // Handle validation failures
        DeepLinkHandler.Instance.OnValidationFailed += HandleValidationFailed;
        
        // Check for pending launch (app was opened via deep link)
        if (DeepLinkHandler.Instance.HasPendingLaunch())
        {
            DeepLinkHandler.Instance.ProcessPendingLaunch();
        }
    }
    
    void HandleMatchReady(MatchLaunchData data)
    {
        Debug.Log($"Match ready: {data.MatchId}");
        Debug.Log($"Tournament: {data.TournamentId}");
        Debug.Log($"Entry fee: {data.EntryFee} {data.Currency}");
        Debug.Log($"Opponents: {data.Opponents.Count}");
        
        // Store match data for gameplay
        GameSession.CurrentMatch = data;
        
        // Load game scene
        SceneManager.LoadScene("GameScene");
    }
    
    void HandleValidationFailed(string reason, MatchLaunchData data)
    {
        Debug.LogError($"Launch validation failed: {reason}");
        // Show error to player and return to main app
        DeskillzBridge.Instance.ReturnToMainApp("error");
    }
}
```

**Unreal:**
```cpp
#include "Lobby/DeepLinkHandler.h"
#include "Lobby/DeskillzBridge.h"

void AMatchLauncher::BeginPlay()
{
    Super::BeginPlay();
    
    UDeepLinkHandler::Get()->Initialize();
    
    // Bind to events
    UDeepLinkHandler::Get()->OnMatchReady.AddDynamic(this, &AMatchLauncher::HandleMatchReady);
    UDeepLinkHandler::Get()->OnValidationFailed.AddDynamic(this, &AMatchLauncher::HandleValidationFailed);
    
    // Check for pending launch
    if (UDeepLinkHandler::Get()->HasPendingLaunch())
    {
        UDeepLinkHandler::Get()->ProcessPendingLaunch();
    }
}

void AMatchLauncher::HandleMatchReady(const FMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Log, TEXT("Match ready: %s"), *Data.MatchId);
    
    // Store match data
    GameInstance->CurrentMatch = Data;
    
    // Load game level
    UGameplayStatics::OpenLevel(this, TEXT("GameLevel"));
}

void AMatchLauncher::HandleValidationFailed(const FString& Reason, const FMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Error, TEXT("Launch validation failed: %s"), *Reason);
    UDeskillzBridge::Get()->ReturnToMainApp(TEXT("error"));
}
```

---

## 5. Match Flow

### Standard Match Lifecycle

```
1. Deep Link Received --> 2. Match Started --> 3. Gameplay --> 4. Submit Score --> 5. Return to App
```

**Unity:**
```csharp
public class MatchController : MonoBehaviour
{
    private MatchLaunchData matchData;
    private float matchStartTime;
    
    public void StartMatch(MatchLaunchData data)
    {
        matchData = data;
        matchStartTime = Time.time;
        
        // Apply random seed for fair play
        UnityEngine.Random.InitState(data.RandomSeed);
        
        // Start gameplay
        BeginGameplay();
    }
    
    public void EndMatch(int finalScore)
    {
        float duration = Time.time - matchStartTime;
        
        // Submit score securely
        DeskillzBridge.Instance.SubmitScore(finalScore, result =>
        {
            if (result.Success)
            {
                Debug.Log("Score submitted successfully");
                // Return to main app
                DeskillzBridge.Instance.ReturnToMainApp("results");
            }
            else
            {
                Debug.LogError($"Score submission failed: {result.Error}");
            }
        });
    }
}
```

**Unreal:**
```cpp
void AMatchController::StartMatch(const FMatchLaunchData& Data)
{
    MatchData = Data;
    MatchStartTime = GetWorld()->GetTimeSeconds();
    
    // Apply random seed
    FMath::RandInit(Data.RandomSeed);
    
    // Start gameplay
    BeginGameplay();
}

void AMatchController::EndMatch(int32 FinalScore)
{
    float Duration = GetWorld()->GetTimeSeconds() - MatchStartTime;
    
    UDeskillzBridge::Get()->SubmitScore(FinalScore,
        FOnScoreSubmitted::CreateLambda([this](const FScoreSubmitResult& Result) {
            if (Result.bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("Score submitted successfully"));
                UDeskillzBridge::Get()->ReturnToMainApp(TEXT("results"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Score submission failed: %s"), *Result.Error);
            }
        })
    );
}
```

---

## 6. Score Submission

Scores are encrypted and validated to prevent cheating.

### Secure Score Submission

**Unity:**
```csharp
using Deskillz.Security;

public class ScoreManager : MonoBehaviour
{
    public void SubmitFinalScore(int score, float matchDuration)
    {
        // Validate score locally first
        if (!DeskillzAntiCheat.Instance.ValidateScore(score, matchDuration))
        {
            Debug.LogWarning("Score validation failed locally");
            return;
        }
        
        // Create secure submission
        var submission = new ScoreSubmission
        {
            Score = score,
            Duration = matchDuration,
            MatchId = GameSession.CurrentMatch.MatchId,
            Checksum = DeskillzScoreEncryption.GenerateChecksum(score, matchDuration)
        };
        
        DeskillzSecureSubmitter.Instance.SubmitScore(submission,
            () => Debug.Log("Score accepted"),
            error => Debug.LogError($"Submission failed: {error}")
        );
    }
}
```

**Unreal:**
```cpp
#include "Security/DeskillzAntiCheat.h"
#include "Security/DeskillzSecureSubmitter.h"

void AScoreManager::SubmitFinalScore(int32 Score, float MatchDuration)
{
    // Validate locally
    if (!UDeskillzAntiCheat::Get()->ValidateScore(Score, MatchDuration))
    {
        UE_LOG(LogTemp, Warning, TEXT("Score validation failed locally"));
        return;
    }
    
    // Create secure submission
    FScoreSubmission Submission;
    Submission.Score = Score;
    Submission.Duration = MatchDuration;
    Submission.MatchId = GameInstance->CurrentMatch.MatchId;
    Submission.Checksum = UDeskillzScoreEncryption::GenerateChecksum(Score, MatchDuration);
    
    UDeskillzSecureSubmitter::Get()->SubmitScore(Submission,
        FOnScoreAccepted::CreateLambda([]() {
            UE_LOG(LogTemp, Log, TEXT("Score accepted"));
        }),
        FOnScoreError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("Submission failed: %s"), *Error);
        })
    );
}
```

---

## 7. Multiplayer Synchronization

For synchronous (real-time) matches, use the multiplayer manager.

### Real-Time Messaging

**Unity:**
```csharp
using Deskillz.Multiplayer;

public class MultiplayerGame : MonoBehaviour
{
    void Start()
    {
        // Subscribe to messages
        DeskillzSDK.Multiplayer.OnMessageReceived += HandleMessage;
        DeskillzSDK.Multiplayer.OnPlayerDisconnected += HandleDisconnect;
    }
    
    public void SendPosition(Vector3 position)
    {
        var data = new PositionMessage
        {
            X = position.x,
            Y = position.y,
            Z = position.z,
            Timestamp = Time.time
        };
        
        // Use unreliable for frequent updates
        DeskillzSDK.Multiplayer.SendMessage(
            MessageSerializer.Serialize(data),
            reliable: false
        );
    }
    
    public void SendAction(string actionType, int value)
    {
        var data = new ActionMessage
        {
            Type = actionType,
            Value = value
        };
        
        // Use reliable for important events
        DeskillzSDK.Multiplayer.SendMessage(
            MessageSerializer.Serialize(data),
            reliable: true
        );
    }
    
    void HandleMessage(string playerId, byte[] data)
    {
        // Determine message type and handle
        var message = MessageSerializer.Deserialize(data);
        
        if (message is PositionMessage pos)
        {
            UpdateOpponentPosition(playerId, pos);
        }
        else if (message is ActionMessage action)
        {
            ProcessOpponentAction(playerId, action);
        }
    }
    
    void HandleDisconnect(string playerId)
    {
        Debug.Log($"Player {playerId} disconnected");
        // Handle accordingly (pause, AI takeover, etc.)
    }
}
```

---

## 8. Private Rooms

Allow players to create custom rooms and invite friends.

### Creating a Room

**Unity:**
```csharp
using Deskillz.Rooms;

public class RoomManager : MonoBehaviour
{
    public void CreateRoom()
    {
        var config = new CreateRoomConfig
        {
            Name = "My Room",
            EntryFee = 5.0m,
            EntryCurrency = "USDT",
            MaxPlayers = 4,
            MinPlayers = 2,
            Mode = RoomMode.Sync,
            Visibility = RoomVisibility.PrivateCode
        };
        
        DeskillzRooms.Instance.CreateRoom(config,
            room => {
                Debug.Log($"Room created: {room.RoomCode}");
                // Share room code with friends
            },
            error => Debug.LogError($"Failed: {error}")
        );
    }
}
```

### Joining a Room

**Unity:**
```csharp
public void JoinRoom(string roomCode)
{
    DeskillzRooms.Instance.JoinRoom(roomCode,
        room => {
            Debug.Log($"Joined room: {room.Name}");
            // Subscribe to room events
            DeskillzRooms.Instance.OnPlayerJoined += HandlePlayerJoined;
            DeskillzRooms.Instance.OnMatchLaunching += HandleMatchLaunching;
        },
        error => Debug.LogError($"Join failed: {error}")
    );
}

void HandlePlayerJoined(RoomPlayer player)
{
    Debug.Log($"{player.Username} joined the room");
}

void HandleMatchLaunching(string matchId)
{
    Debug.Log($"Match starting: {matchId}");
    // Transition to game scene
}
```

---

## 9. Auto-Updater

Ensure players always have the latest version.

### Version Check

**Unity:**
```csharp
using Deskillz;

public class UpdateChecker : MonoBehaviour
{
    void Start()
    {
        DeskillzUpdater.Instance.OnUpdateAvailable += HandleUpdateAvailable;
        DeskillzUpdater.Instance.OnForcedUpdateRequired += HandleForcedUpdate;
        
        // Check on startup
        DeskillzUpdater.Instance.CheckForUpdates();
    }
    
    void HandleUpdateAvailable(UpdateInfo info)
    {
        Debug.Log($"Update available: {info.Version}");
        // Show optional update dialog
        ShowUpdateDialog(info, optional: true);
    }
    
    void HandleForcedUpdate(UpdateInfo info)
    {
        Debug.Log($"Required update: {info.Version}");
        // Block gameplay until updated
        ShowUpdateDialog(info, optional: false);
    }
    
    void ShowUpdateDialog(UpdateInfo info, bool optional)
    {
        // Display update UI
        updatePanel.SetActive(true);
        versionText.text = $"Version {info.Version} available";
        releaseNotesText.text = info.ReleaseNotes;
        
        if (!optional)
        {
            skipButton.gameObject.SetActive(false);
        }
    }
    
    public void OnUpdateClicked()
    {
        DeskillzUpdater.Instance.OpenUpdateUrl();
    }
}
```

---

## 10. Testing

### Test Mode

**Unity:**
```csharp
// Enable test mode (automatic in Unity Editor)
DeskillzConfig.TestMode = true;

// Simulate match launch
DeepLinkHandler.SimulateDeepLink("deskillz://launch?matchId=test123&token=testtoken");

// Simulate opponent score (async matches)
Deskillz.SimulateOpponentScore(1000);

// Test room UI
PrivateRoomUI.Instance.ShowRoomList();

// Test auto-updater
DeskillzUpdater.Instance.CheckForUpdates();
```

### Sandbox Environment

Always test in Sandbox before Production:
```csharp
var config = new DeskillzConfig
{
    Environment = DeskillzEnvironment.Sandbox, // Use Sandbox for testing
    // ...
};
```

---

## 11. Private Rooms Enhancement

SDK 2.6.0 introduces enhanced private rooms with two game categories: Esports and Social.

### Game Categories

| Category | Revenue Model | Best For |
|----------|--------------|----------|
| **Esports** | Entry fee (winner takes pot) | Tournament-style competition |
| **Social** | Rake from each pot | Casual cash games |

### Creating Enhanced Rooms

**Unity:**
```csharp
using Deskillz.Rooms;
using Deskillz.Host;

public class EnhancedRoomManager : MonoBehaviour
{
    // Create an Esports room (tournament-style)
    public void CreateEsportsRoom()
    {
        var config = new CreateRoomConfig
        {
            Name = "Pro Tournament Room",
            GameCategory = GameCategory.Esports,
            EntryFee = 10.0m,
            EntryCurrency = "USDT",
            MaxPlayers = 8,
            MinPlayers = 2,
            Mode = RoomMode.Sync,
            Visibility = RoomVisibility.PublicListed
        };
        
        DeskillzRooms.Instance.CreateRoom(config,
            room => Debug.Log($"Esports room: {room.RoomCode}"),
            error => Debug.LogError(error)
        );
    }
    
    // Create a Social room (cash game style)
    public void CreateSocialRoom()
    {
        var config = new CreateRoomConfig
        {
            Name = "Casual Cash Game",
            GameCategory = GameCategory.Social,
            
            // Social-specific settings
            PointValue = 0.01m,           // $0.01 per point
            MinBuyIn = 10.0m,             // Minimum buy-in
            MaxBuyIn = 200.0m,            // Maximum buy-in
            DefaultBuyIn = 50.0m,         // Default buy-in
            
            RakePercentage = 5.0m,        // 5% rake
            RakeCap = 3.0m,               // Max $3 rake per pot
            
            TurnTimeSeconds = 30,         // 30 second turns
            AllowSpectators = true,       // Enable spectating
            
            EntryCurrency = "USDT",
            MaxPlayers = 6,
            Mode = RoomMode.Sync,
            Visibility = RoomVisibility.PublicListed
        };
        
        DeskillzRooms.Instance.CreateRoom(config,
            room => Debug.Log($"Social room: {room.RoomCode}"),
            error => Debug.LogError(error)
        );
    }
}
```

**Unreal:**
```cpp
#include "Rooms/DeskillzRooms.h"

void AEnhancedRoomManager::CreateEsportsRoom()
{
    FCreateRoomConfig Config;
    Config.Name = TEXT("Pro Tournament Room");
    Config.GameCategory = EGameCategory::Esports;
    Config.EntryFee = 10.0f;
    Config.EntryCurrency = TEXT("USDT");
    Config.MaxPlayers = 8;
    Config.MinPlayers = 2;
    Config.Mode = ERoomMode::Sync;
    Config.Visibility = ERoomVisibility::PublicListed;
    
    UDeskillzRooms::Get()->CreateRoom(Config,
        FOnRoomCreated::CreateLambda([](const FPrivateRoom& Room) {
            UE_LOG(LogTemp, Log, TEXT("Esports room: %s"), *Room.RoomCode);
        }),
        FOnRoomError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AEnhancedRoomManager::CreateSocialRoom()
{
    FCreateRoomConfig Config;
    Config.Name = TEXT("Casual Cash Game");
    Config.GameCategory = EGameCategory::Social;
    Config.PointValue = 0.01f;
    Config.MinBuyIn = 10.0f;
    Config.MaxBuyIn = 200.0f;
    Config.DefaultBuyIn = 50.0f;
    Config.RakePercentage = 5.0f;
    Config.RakeCap = 3.0f;
    Config.TurnTimeSeconds = 30;
    Config.bAllowSpectators = true;
    Config.EntryCurrency = TEXT("USDT");
    Config.MaxPlayers = 6;
    Config.Mode = ERoomMode::Sync;
    Config.Visibility = ERoomVisibility::PublicListed;
    
    UDeskillzRooms::Get()->CreateRoom(Config,
        FOnRoomCreated::CreateLambda([](const FPrivateRoom& Room) {
            UE_LOG(LogTemp, Log, TEXT("Social room: %s"), *Room.RoomCode);
        }),
        FOnRoomError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}
```

---

## 12. Host System Integration

The Host System allows verified users to create and manage rooms, earning revenue share from gameplay.

### Host Tiers and Revenue Share

| Tier | Requirements | Revenue Share |
|------|--------------|---------------|
| Starter | New host | 50% |
| Bronze | 10+ rooms, $100+ | 55% |
| Silver | 25+ rooms, $500+ | 60% |
| Gold | 50+ rooms, $2,000+ | 65% |
| Platinum | 100+ rooms, $5,000+ | 70% |
| Diamond | 250+ rooms, $15,000+ | 75% |

### Initializing Host Manager

**Unity:**
```csharp
using Deskillz.Host;

public class HostController : MonoBehaviour
{
    void Start()
    {
        // Initialize host manager
        HostManager.Instance.Initialize(currentUserId);
        
        // Subscribe to events
        HostManager.Instance.OnHostProfileUpdated += HandleProfileUpdate;
        HostManager.Instance.OnTierChanged += HandleTierChange;
        HostManager.Instance.OnLevelUp += HandleLevelUp;
        HostManager.Instance.OnBadgeEarned += HandleBadgeEarned;
        HostManager.Instance.OnEarningsUpdated += HandleEarnings;
    }
    
    void HandleProfileUpdate(HostProfile profile)
    {
        Debug.Log($"Host: {profile.TierName} (Level {profile.Level})");
        Debug.Log($"Earnings: ${profile.TotalEarnings}");
    }
    
    void HandleTierChange(HostTier oldTier, HostTier newTier)
    {
        Debug.Log($"Tier upgraded: {oldTier} -> {newTier}!");
        ShowTierUpgradeAnimation(newTier);
    }
    
    void HandleLevelUp(int newLevel)
    {
        Debug.Log($"Level up! Now level {newLevel}");
        ShowLevelUpAnimation(newLevel);
    }
    
    void HandleBadgeEarned(HostBadge badge)
    {
        Debug.Log($"Badge earned: {badge.Name}");
        ShowBadgeNotification(badge);
    }
    
    void HandleEarnings(float total, float pending)
    {
        Debug.Log($"Total: ${total}, Pending: ${pending}");
        UpdateEarningsUI(total, pending);
    }
}
```

### Registering as a Host

**Unity:**
```csharp
public class HostRegistration : MonoBehaviour
{
    public void RegisterAsHost()
    {
        // Check if already a host
        if (HostManager.Instance.IsVerifiedHost())
        {
            Debug.Log("Already registered as host");
            return;
        }
        
        // Register
        HostManager.Instance.RegisterAsHost(
            profile => {
                Debug.Log($"Registered as {profile.TierName} host!");
                ShowHostDashboard();
            },
            error => {
                Debug.LogError($"Registration failed: {error}");
                ShowError(error);
            }
        );
    }
    
    public void VerifyAge(DateTime birthDate)
    {
        HostManager.Instance.VerifyAge(birthDate,
            () => Debug.Log("Age verified successfully"),
            error => Debug.LogError($"Verification failed: {error}")
        );
    }
}
```

**Unreal:**
```cpp
#include "Host/DeskillzHostManager.h"

void AHostRegistration::RegisterAsHost()
{
    if (UDeskillzHostManager::Get()->IsVerifiedHost())
    {
        UE_LOG(LogTemp, Log, TEXT("Already registered as host"));
        return;
    }
    
    UDeskillzHostManager::Get()->RegisterAsHost(
        FOnHostProfileResult::CreateLambda([this](const FHostProfile& Profile) {
            UE_LOG(LogTemp, Log, TEXT("Registered as %s host!"), *Profile.TierName);
            ShowHostDashboard();
        }),
        FOnHostError::CreateLambda([this](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("Registration failed: %s"), *Error);
            ShowError(Error);
        })
    );
}
```

### Creating Host Rooms

**Unity:**
```csharp
public class HostRoomCreation : MonoBehaviour
{
    public void CreateHostRoom()
    {
        // Must be verified host
        if (!HostManager.Instance.IsVerifiedHost())
        {
            Debug.LogError("Must be a verified host");
            return;
        }
        
        var config = new CreateHostRoomConfig
        {
            Name = "Pro Cash Game",
            GameCategory = GameCategory.Social,
            PointValue = 0.05m,
            MinBuyIn = 20.0m,
            MaxBuyIn = 500.0m,
            DefaultBuyIn = 100.0m,
            RakePercentage = 5.0m,
            RakeCap = 5.0m,
            TurnTimeSeconds = 30,
            AllowSpectators = true,
            EntryCurrency = "USDT",
            MaxPlayers = 8,
            Mode = RoomMode.Sync,
            Visibility = RoomVisibility.PublicListed
        };
        
        HostManager.Instance.CreateHostRoom(config,
            room => {
                Debug.Log($"Host room created: {room.RoomCode}");
                
                // Subscribe to host-specific events
                HostManager.Instance.OnPlayerJoinedHostedRoom += HandlePlayerJoined;
                HostManager.Instance.OnPlayerLeftHostedRoom += HandlePlayerLeft;
            },
            error => Debug.LogError(error)
        );
    }
    
    void HandlePlayerJoined(string roomId, RoomPlayer player)
    {
        Debug.Log($"{player.Username} joined your room");
    }
    
    void HandlePlayerLeft(string roomId, string playerId)
    {
        Debug.Log($"Player left your room");
    }
}
```

### Host Dashboard UI

**Unity:**
```csharp
using Deskillz.Host.UI;

public class HostDashboardController : MonoBehaviour
{
    [SerializeField] private HostDashboardUI dashboardUI;
    [SerializeField] private HostProfileCard profileCard;
    [SerializeField] private HostTierProgress tierProgress;
    [SerializeField] private HostEarningsChart earningsChart;
    [SerializeField] private HostBadgeGrid badgeGrid;
    
    public void ShowDashboard()
    {
        // Get host profile
        HostManager.Instance.GetHostProfile(
            profile => {
                // Update UI components
                profileCard.SetProfile(profile);
                tierProgress.SetProgress(profile.Tier, HostManager.Instance.GetTierProgress());
                badgeGrid.SetBadges(profile.Badges);
                
                // Load earnings history
                LoadEarningsChart();
                
                // Show dashboard
                dashboardUI.Show();
            },
            error => Debug.LogError(error)
        );
    }
    
    void LoadEarningsChart()
    {
        // Earnings chart shows historical data
        var earnings = HostManager.Instance.GetTotalEarnings();
        var pending = HostManager.Instance.GetPendingEarnings();
        earningsChart.SetData(earnings, pending);
    }
    
    public void WithdrawEarnings()
    {
        var pending = HostManager.Instance.GetPendingEarnings();
        
        if (pending < 10.0f) // Minimum withdrawal
        {
            Debug.Log("Minimum withdrawal is $10");
            return;
        }
        
        HostManager.Instance.WithdrawEarnings(pending,
            () => Debug.Log("Withdrawal initiated"),
            error => Debug.LogError(error)
        );
    }
}
```

---

## 13. Social Games Integration

Social games use a rake-based revenue model where a small percentage is taken from each pot.

### Social Game Session Flow

```
1. Players buy-in --> 2. Game rounds --> 3. Rake collected --> 4. Players cash out
```

### Initializing Social Game Manager

**Unity:**
```csharp
using Deskillz.Social;

public class SocialGameController : MonoBehaviour
{
    void Start()
    {
        // Initialize for current room
        SocialGameManager.Instance.Initialize(currentRoomId);
        BuyInManager.Instance.Initialize(currentRoomId);
        
        // Subscribe to events
        SocialGameManager.Instance.OnSessionStarted += HandleSessionStarted;
        SocialGameManager.Instance.OnRoundStarted += HandleRoundStarted;
        SocialGameManager.Instance.OnRoundEnded += HandleRoundEnded;
        SocialGameManager.Instance.OnRakeCollected += HandleRakeCollected;
        SocialGameManager.Instance.OnPauseRequested += HandlePauseRequested;
        
        BuyInManager.Instance.OnBuyInProcessed += HandleBuyIn;
        BuyInManager.Instance.OnRebuyRequired += HandleRebuyRequired;
        BuyInManager.Instance.OnCashOutProcessed += HandleCashOut;
        BuyInManager.Instance.OnLowBalanceWarning += HandleLowBalance;
    }
    
    void HandleSessionStarted(SocialGameSession session)
    {
        Debug.Log($"Session started: {session.Id}");
    }
    
    void HandleRoundStarted(int roundNumber)
    {
        Debug.Log($"Round {roundNumber} started");
    }
    
    void HandleRoundEnded(RoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber}: {result.WinnerId} won ${result.PotAmount}");
        Debug.Log($"Rake collected: ${result.RakeAmount}");
    }
    
    void HandleRakeCollected(float amount, float total)
    {
        Debug.Log($"Rake: ${amount} (Total: ${total})");
    }
}
```

### Buy-In Flow

**Unity:**
```csharp
public class BuyInController : MonoBehaviour
{
    [SerializeField] private BuyInModal buyInModal;
    [SerializeField] private RebuyModal rebuyModal;
    [SerializeField] private CashOutModal cashOutModal;
    
    public void ShowBuyIn()
    {
        float minBuyIn = BuyInManager.Instance.GetMinBuyIn();
        float maxBuyIn = BuyInManager.Instance.GetMaxBuyIn();
        float defaultBuyIn = BuyInManager.Instance.GetDefaultBuyIn();
        
        buyInModal.Show(minBuyIn, maxBuyIn, defaultBuyIn, OnBuyInConfirmed);
    }
    
    void OnBuyInConfirmed(float amount)
    {
        BuyInManager.Instance.ProcessBuyIn(currentPlayerId, amount,
            result => {
                Debug.Log($"Buy-in successful: ${amount}");
                Debug.Log($"Starting balance: ${result.NewBalance}");
                buyInModal.Hide();
            },
            error => {
                Debug.LogError($"Buy-in failed: {error}");
                buyInModal.ShowError(error);
            }
        );
    }
    
    void HandleRebuyRequired(string playerId, float timeout)
    {
        if (playerId == currentPlayerId)
        {
            // Show rebuy modal with countdown
            rebuyModal.Show(timeout, OnRebuyConfirmed, OnRebuyDeclined);
        }
    }
    
    void OnRebuyConfirmed(float amount)
    {
        BuyInManager.Instance.ProcessRebuy(currentPlayerId, amount,
            result => {
                Debug.Log($"Rebuy successful: ${amount}");
                rebuyModal.Hide();
            },
            error => rebuyModal.ShowError(error)
        );
    }
    
    void OnRebuyDeclined()
    {
        // Player chose not to rebuy, will be removed when timeout expires
        rebuyModal.Hide();
    }
    
    public void RequestCashOut()
    {
        if (!BuyInManager.Instance.CanCashOut(currentPlayerId))
        {
            Debug.Log("Cannot cash out during active hand");
            return;
        }
        
        float balance = BuyInManager.Instance.GetPlayerBalance(currentPlayerId);
        cashOutModal.Show(balance, OnCashOutConfirmed);
    }
    
    void OnCashOutConfirmed()
    {
        BuyInManager.Instance.ProcessCashOut(currentPlayerId,
            result => {
                Debug.Log($"Cashed out: ${result.Amount}");
                cashOutModal.Hide();
                // Player will leave room
            },
            error => cashOutModal.ShowError(error)
        );
    }
    
    void HandleLowBalance(string playerId, float balance, float threshold)
    {
        if (playerId == currentPlayerId)
        {
            ShowLowBalanceWarning(balance, threshold);
        }
    }
}
```

**Unreal:**
```cpp
#include "Social/DeskillzBuyInManager.h"

void ABuyInController::ShowBuyIn()
{
    float MinBuyIn = UDeskillzBuyInManager::Get()->GetMinBuyIn();
    float MaxBuyIn = UDeskillzBuyInManager::Get()->GetMaxBuyIn();
    float DefaultBuyIn = UDeskillzBuyInManager::Get()->GetDefaultBuyIn();
    
    BuyInWidget->Show(MinBuyIn, MaxBuyIn, DefaultBuyIn);
}

void ABuyInController::OnBuyInConfirmed(float Amount)
{
    UDeskillzBuyInManager::Get()->ProcessBuyIn(CurrentPlayerId, Amount,
        FOnBuyInResult::CreateLambda([this](const FBuyInResult& Result) {
            UE_LOG(LogTemp, Log, TEXT("Buy-in successful: $%.2f"), Result.Amount);
            BuyInWidget->Hide();
        }),
        FOnBuyInError::CreateLambda([this](const FString& Error) {
            BuyInWidget->ShowError(Error);
        })
    );
}
```

### Rake Calculation

**Unity:**
```csharp
using Deskillz.Social;

public class RakeDisplay : MonoBehaviour
{
    public void ShowRakePreview(float potAmount)
    {
        // Get room settings
        float rakePercentage = 5.0f;  // 5%
        float rakeCap = 3.0f;         // $3 max
        
        // Calculate rake
        float rake = RakeCalculator.CalculateRakeWithCap(
            potAmount,
            rakePercentage,
            rakeCap
        );
        
        Debug.Log($"Pot: ${potAmount}, Rake: ${rake}");
        
        // Preview distribution by host tier
        var distribution = RakeCalculator.PreviewRakeDistribution(
            rake,
            HostManager.Instance.GetHostTier()
        );
        
        Debug.Log($"Host share: ${distribution.HostShare}");
        Debug.Log($"Platform share: ${distribution.PlatformShare}");
        Debug.Log($"Developer share: ${distribution.DeveloperShare}");
    }
}
```

### Round Management

**Unity:**
```csharp
public class RoundController : MonoBehaviour
{
    public void StartNewRound()
    {
        SocialGameManager.Instance.StartRound();
    }
    
    public void EndRoundWithWinner(string winnerId, float potAmount)
    {
        SocialGameManager.Instance.EndRound(winnerId, potAmount);
    }
    
    public void RecordPlayerAction(string playerId, string action, float amount)
    {
        // Record actions for session history
        SocialGameManager.Instance.RecordAction(playerId, action, amount);
    }
}
```

### Pause System

**Unity:**
```csharp
public class PauseController : MonoBehaviour
{
    [SerializeField] private PauseRequestModal pauseModal;
    
    public void RequestPause(string reason)
    {
        SocialGameManager.Instance.RequestPause(reason);
    }
    
    void HandlePauseRequested(string requesterId, string reason)
    {
        // Show vote UI to all players
        pauseModal.ShowVoteRequest(requesterId, reason, OnVote);
    }
    
    void OnVote(bool approve)
    {
        SocialGameManager.Instance.VotePause(approve);
    }
    
    void HandleGamePaused(DateTime resumeTime)
    {
        Debug.Log($"Game paused until {resumeTime}");
        ShowPausedOverlay(resumeTime);
    }
    
    void HandleGameResumed()
    {
        Debug.Log("Game resumed");
        HidePausedOverlay();
    }
}
```

---

## 14. Spectator Mode Integration

Allow players to watch live games without participating.

### Initializing Spectator Manager

**Unity:**
```csharp
using Deskillz.Spectator;

public class SpectatorController : MonoBehaviour
{
    void Start()
    {
        SpectatorManager.Instance.Initialize();
        
        // Subscribe to events
        SpectatorManager.Instance.OnJoinedAsSpectator += HandleJoined;
        SpectatorManager.Instance.OnLeftSpectator += HandleLeft;
        SpectatorManager.Instance.OnGameStateUpdated += HandleStateUpdate;
        SpectatorManager.Instance.OnRoundStarted += HandleRoundStart;
        SpectatorManager.Instance.OnRoundEnded += HandleRoundEnd;
        SpectatorManager.Instance.OnPlayerAction += HandlePlayerAction;
        SpectatorManager.Instance.OnScoresUpdated += HandleScoresUpdate;
        SpectatorManager.Instance.OnSpectatorCountChanged += HandleSpectatorCount;
    }
}
```

### Browsing Spectatable Rooms

**Unity:**
```csharp
public class SpectatorBrowser : MonoBehaviour
{
    [SerializeField] private Transform roomListContainer;
    [SerializeField] private SpectatorRoomCard roomCardPrefab;
    
    public void FetchRooms()
    {
        var filter = new SpectatorRoomFilter
        {
            GameId = currentGameId,
            GameCategory = GameCategory.Social,
            MinPlayers = 2,
            IsActive = true
        };
        
        SpectatorManager.Instance.FetchSpectatorRooms(filter,
            rooms => DisplayRooms(rooms),
            error => Debug.LogError(error)
        );
    }
    
    void DisplayRooms(List<SpectatorRoom> rooms)
    {
        // Clear existing
        foreach (Transform child in roomListContainer)
        {
            Destroy(child.gameObject);
        }
        
        // Display rooms
        foreach (var room in rooms)
        {
            var card = Instantiate(roomCardPrefab, roomListContainer);
            card.SetRoom(room);
            card.OnJoinClicked += () => JoinAsSpectator(room.RoomId);
        }
    }
    
    void JoinAsSpectator(string roomId)
    {
        SpectatorManager.Instance.JoinAsSpectator(roomId,
            state => {
                Debug.Log($"Now spectating: {state.RoomName}");
                ShowSpectatorView(state);
            },
            error => Debug.LogError(error)
        );
    }
}
```

**Unreal:**
```cpp
#include "Spectator/DeskillzSpectatorManager.h"

void ASpectatorBrowser::FetchRooms()
{
    FSpectatorRoomFilter Filter;
    Filter.GameId = CurrentGameId;
    Filter.GameCategory = EGameCategory::Social;
    Filter.MinPlayers = 2;
    Filter.bIsActive = true;
    
    UDeskillzSpectatorManager::Get()->FetchSpectatorRooms(Filter,
        FOnSpectatorRoomsResult::CreateLambda([this](const TArray<FSpectatorRoom>& Rooms) {
            DisplayRooms(Rooms);
        }),
        FOnSpectatorError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}
```

### Spectator View

**Unity:**
```csharp
using Deskillz.Spectator.UI;

public class SpectatorViewController : MonoBehaviour
{
    [SerializeField] private SpectatorView spectatorView;
    [SerializeField] private SpectatorGameBoard gameBoard;
    [SerializeField] private SpectatorScorePanel scorePanel;
    [SerializeField] private RoomSwitcher roomSwitcher;
    
    public void ShowSpectatorView(SpectatorState state)
    {
        // Initialize UI
        spectatorView.Show();
        gameBoard.Initialize(state);
        scorePanel.SetScores(state.Players);
        
        // Subscribe to updates
        SpectatorManager.Instance.OnGameStateUpdated += UpdateGameBoard;
        SpectatorManager.Instance.OnScoresUpdated += UpdateScores;
        SpectatorManager.Instance.OnPlayerAction += AnimateAction;
    }
    
    void UpdateGameBoard(SpectatorState state)
    {
        gameBoard.UpdateState(state);
    }
    
    void UpdateScores(List<SpectatorScore> scores)
    {
        scorePanel.SetScores(scores);
    }
    
    void AnimateAction(SpectatorAction action)
    {
        // Show action animation
        gameBoard.AnimatePlayerAction(action.PlayerId, action.ActionType, action.Amount);
    }
    
    public void SetViewMode(SpectatorViewMode mode)
    {
        SpectatorManager.Instance.SetViewMode(mode);
        
        switch (mode)
        {
            case SpectatorViewMode.Overview:
                gameBoard.ShowAllPlayers();
                break;
            case SpectatorViewMode.FollowPlayer:
                // Will focus on followed player
                break;
            case SpectatorViewMode.Scoreboard:
                gameBoard.Hide();
                scorePanel.ShowExpanded();
                break;
        }
    }
    
    public void FollowPlayer(string playerId)
    {
        SpectatorManager.Instance.SetViewMode(SpectatorViewMode.FollowPlayer);
        SpectatorManager.Instance.FollowPlayer(playerId);
        gameBoard.FocusOnPlayer(playerId);
    }
    
    public void LeaveSpectator()
    {
        SpectatorManager.Instance.LeaveSpectator();
        spectatorView.Hide();
    }
}
```

### Multi-Room Spectating

**Unity:**
```csharp
public class MultiRoomSpectator : MonoBehaviour
{
    [SerializeField] private RoomSwitcher roomSwitcher;
    
    private List<string> watchedRoomIds = new List<string>();
    
    public void AddRoomToWatch(string roomId)
    {
        if (!watchedRoomIds.Contains(roomId))
        {
            watchedRoomIds.Add(roomId);
            roomSwitcher.AddRoom(roomId);
        }
    }
    
    public void SwitchToRoom(string roomId)
    {
        SpectatorManager.Instance.SwitchRoom(roomId);
    }
    
    void HandleRoomSwitched(SpectatorState newState)
    {
        Debug.Log($"Switched to room: {newState.RoomName}");
        UpdateUIForRoom(newState);
    }
}
```

### Spectator Events

**Unity:**
```csharp
public class SpectatorEventHandler : MonoBehaviour
{
    void Start()
    {
        SpectatorManager.Instance.OnRoundStarted += OnRoundStart;
        SpectatorManager.Instance.OnRoundEnded += OnRoundEnd;
        SpectatorManager.Instance.OnSpectatorCountChanged += OnCountChanged;
        SpectatorManager.Instance.OnKickedFromSpectator += OnKicked;
    }
    
    void OnRoundStart(int roundNumber)
    {
        ShowRoundStartAnimation(roundNumber);
    }
    
    void OnRoundEnd(SpectatorRoundResult result)
    {
        ShowRoundEndSummary(result);
        HighlightWinner(result.WinnerId);
    }
    
    void OnCountChanged(int count)
    {
        UpdateSpectatorCountUI(count);
    }
    
    void OnKicked(string reason)
    {
        Debug.Log($"Kicked from spectating: {reason}");
        ShowKickedMessage(reason);
        ReturnToRoomBrowser();
    }
}
```

---

## Next Steps

After completing SDK integration:

1. **Test in Sandbox** - Thoroughly test all features in the Sandbox environment
2. **Submit for Review** - Submit your game for Deskillz review
3. **Go Live** - Switch to Production environment after approval

For additional help:
- [API Reference](API_REFERENCE.md) - Complete method documentation
- [Quick Start Guide](QUICKSTART.md) - Fast setup guide
- [Unity SDK Repository](https://github.com/Deskillz-Games-Development/unity-sdk)
- [Unreal SDK Repository](https://github.com/Deskillz-Games-Development/unreal-sdk)
- [Developer Portal](https://developer.deskillz.games) - Manage your games
- [Documentation](https://docs.deskillz.games) - Full documentation
- [Support](https://support.deskillz.games) - Get help

---

## Version History

| Version | Changes |
|---------|---------|
| 2.6.0 | Added Host System, Social Games, Spectator Mode (Sections 11-14) |
| 2.5.0 | Added Auto-Updater integration (Section 9) |
| 2.2.0 | Added Private Rooms (Section 8) |
| 2.0.0 | Deep Link architecture, centralized lobby |