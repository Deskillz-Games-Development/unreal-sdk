# Deskillz SDK - Quick Start Guide

**SDK Version: 2.6.0** | Get your game integrated with Deskillz in minutes.

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Installation](#2-installation)
3. [Basic Setup](#3-basic-setup)
4. [Handle Match Launch](#4-handle-match-launch)
5. [Submit Score](#5-submit-score)
6. [Enable Auto-Updates](#6-enable-auto-updates)
7. [Host Registration Quick Start](#7-host-registration-quick-start) - NEW
8. [Social Game Quick Start](#8-social-game-quick-start) - NEW
9. [Host Spectator Mode Quick Start](#9-host-spectator-mode-quick-start) - NEW
10. [Test Your Integration](#10-test-your-integration)

---

## 1. Prerequisites

Before you begin, ensure you have:

- [x] A Deskillz Developer Account ([Sign up here](https://developer.deskillz.games))
- [x] Your Game ID and API Key from the Developer Portal
- [x] Unity 2020.3+ or Unreal Engine 4.27+
- [x] Basic knowledge of C# (Unity) or C++ (Unreal)

---

## 2. Installation

### Unity

**Option A: Package Import**
1. Download `DeskillzSDK-v2.6.0.unitypackage` from the Developer Portal
2. In Unity: Assets > Import Package > Custom Package
3. Select the downloaded package and import all files

**Option B: Package Manager**
Add to your `manifest.json`:
```json
{
  "dependencies": {
    "com.deskillz.sdk": "https://github.com/Deskillz-Games-Development/unity-sdk.git#v2.6.0"
  }
}
```

### Unreal

1. Download the Deskillz SDK plugin from the Developer Portal
2. Extract to `YourProject/Plugins/DeskillzSDK/`
3. Regenerate project files (right-click .uproject > Generate Visual Studio Files)
4. Open project and enable plugin: Edit > Plugins > Search "Deskillz"

---

## 3. Basic Setup

### Unity Setup

Create a `DeskillzManager.cs` script and attach it to a GameObject in your initial scene:

```csharp
using UnityEngine;
using Deskillz;
using Deskillz.Lobby;

public class DeskillzManager : MonoBehaviour
{
    // Your credentials from Developer Portal
    [SerializeField] private string gameId = "your-game-id";
    [SerializeField] private string apiKey = "your-api-key";
    
    void Awake()
    {
        // Ensure only one instance
        DontDestroyOnLoad(gameObject);
        
        // Configure SDK
        var config = new DeskillzConfig
        {
            GameId = gameId,
            ApiKey = apiKey,
            Environment = DeskillzEnvironment.Sandbox, // Use Production when ready
            EnableLogging = true
        };
        
        // Initialize
        DeskillzSDK.Instance.Initialize(config);
        DeskillzSDK.Instance.OnInitialized += OnSDKReady;
    }
    
    void OnSDKReady()
    {
        Debug.Log("Deskillz SDK Ready!");
        
        // Initialize deep link handler
        DeepLinkHandler.Instance.Initialize();
        DeepLinkHandler.Instance.OnMatchReady += OnMatchReady;
        
        // Check for pending launch
        if (DeepLinkHandler.Instance.HasPendingLaunch())
        {
            DeepLinkHandler.Instance.ProcessPendingLaunch();
        }
    }
    
    void OnMatchReady(MatchLaunchData data)
    {
        Debug.Log($"Match ready: {data.MatchId}");
        // Store match data and load game scene
        GameSession.CurrentMatch = data;
        UnityEngine.SceneManagement.SceneManager.LoadScene("GameScene");
    }
}
```

### Unreal Setup

In your GameMode or GameInstance:

```cpp
// MyGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DeskillzSDK.h"
#include "Lobby/DeepLinkHandler.h"
#include "MyGameMode.generated.h"

UCLASS()
class AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()
    
public:
    virtual void BeginPlay() override;
    
private:
    UFUNCTION()
    void OnSDKReady();
    
    UFUNCTION()
    void OnMatchReady(const FMatchLaunchData& Data);
};

// MyGameMode.cpp
#include "MyGameMode.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    FDeskillzConfig Config;
    Config.GameId = TEXT("your-game-id");
    Config.ApiKey = TEXT("your-api-key");
    Config.Environment = EDeskillzEnvironment::Sandbox;
    Config.bEnableLogging = true;
    
    UDeskillzSDK::Get()->Initialize(Config);
    UDeskillzSDK::Get()->OnInitialized.AddDynamic(this, &AMyGameMode::OnSDKReady);
}

void AMyGameMode::OnSDKReady()
{
    UE_LOG(LogTemp, Log, TEXT("Deskillz SDK Ready!"));
    
    UDeepLinkHandler::Get()->Initialize();
    UDeepLinkHandler::Get()->OnMatchReady.AddDynamic(this, &AMyGameMode::OnMatchReady);
    
    if (UDeepLinkHandler::Get()->HasPendingLaunch())
    {
        UDeepLinkHandler::Get()->ProcessPendingLaunch();
    }
}

void AMyGameMode::OnMatchReady(const FMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Log, TEXT("Match ready: %s"), *Data.MatchId);
    // Store and load game level
}
```

---

## 4. Handle Match Launch

When your game is launched from the Deskillz app, you receive match data via deep link.

### Unity

```csharp
using Deskillz;
using Deskillz.Lobby;

public class GameController : MonoBehaviour
{
    private MatchLaunchData matchData;
    private float startTime;
    
    void Start()
    {
        // Get stored match data
        matchData = GameSession.CurrentMatch;
        
        if (matchData != null)
        {
            StartMatch();
        }
    }
    
    void StartMatch()
    {
        startTime = Time.time;
        
        // Use the random seed for fair play
        UnityEngine.Random.InitState(matchData.RandomSeed);
        
        // Show opponent info
        Debug.Log($"Playing against: {matchData.Opponents[0].Username}");
        Debug.Log($"Entry fee: {matchData.EntryFee} {matchData.Currency}");
        
        // Start your game logic
        BeginGameplay();
    }
}
```

### Unreal

```cpp
void AGameController::BeginPlay()
{
    Super::BeginPlay();
    
    // Get match data from game instance
    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (GI && GI->CurrentMatch.IsValid())
    {
        StartMatch(GI->CurrentMatch);
    }
}

void AGameController::StartMatch(const FMatchLaunchData& Data)
{
    MatchData = Data;
    StartTime = GetWorld()->GetTimeSeconds();
    
    // Use random seed
    FMath::RandInit(Data.RandomSeed);
    
    // Start gameplay
    BeginGameplay();
}
```

---

## 5. Submit Score

When the match ends, submit the score securely.

### Unity

```csharp
using Deskillz;
using Deskillz.Lobby;

public class GameController : MonoBehaviour
{
    public void OnGameOver(int finalScore)
    {
        float duration = Time.time - startTime;
        
        // Submit score and return to main app
        DeskillzBridge.Instance.SubmitScore(finalScore, result =>
        {
            if (result.Success)
            {
                Debug.Log("Score submitted!");
                DeskillzBridge.Instance.ReturnToMainApp("results");
            }
            else
            {
                Debug.LogError($"Failed: {result.Error}");
            }
        });
    }
}
```

### Unreal

```cpp
void AGameController::OnGameOver(int32 FinalScore)
{
    float Duration = GetWorld()->GetTimeSeconds() - StartTime;
    
    UDeskillzBridge::Get()->SubmitScore(FinalScore,
        FOnScoreSubmitted::CreateLambda([](const FScoreSubmitResult& Result) {
            if (Result.bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("Score submitted!"));
                UDeskillzBridge::Get()->ReturnToMainApp(TEXT("results"));
            }
        })
    );
}
```

---

## 6. Enable Auto-Updates

Keep players on the latest version with automatic update checks.

### Unity

```csharp
using Deskillz;

public class UpdateChecker : MonoBehaviour
{
    void Start()
    {
        DeskillzUpdater.Instance.OnForcedUpdateRequired += HandleForcedUpdate;
        DeskillzUpdater.Instance.OnUpdateAvailable += HandleOptionalUpdate;
        
        // Check on startup
        DeskillzUpdater.Instance.CheckForUpdates();
    }
    
    void HandleForcedUpdate(UpdateInfo info)
    {
        // Must update - block gameplay
        ShowBlockingUpdateDialog(info);
    }
    
    void HandleOptionalUpdate(UpdateInfo info)
    {
        // Optional - show dismissable dialog
        ShowOptionalUpdateDialog(info);
    }
    
    public void OpenUpdateLink()
    {
        DeskillzUpdater.Instance.OpenUpdateUrl();
    }
}
```

### Unreal

```cpp
void AUpdateChecker::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzUpdater::Get()->OnForcedUpdateRequired.AddDynamic(this, &AUpdateChecker::HandleForcedUpdate);
    UDeskillzUpdater::Get()->OnUpdateAvailable.AddDynamic(this, &AUpdateChecker::HandleOptionalUpdate);
    
    UDeskillzUpdater::Get()->CheckForUpdates();
}

void AUpdateChecker::HandleForcedUpdate(const FUpdateInfo& Info)
{
    ShowBlockingUpdateDialog(Info);
}

void AUpdateChecker::HandleOptionalUpdate(const FUpdateInfo& Info)
{
    ShowOptionalUpdateDialog(Info);
}
```

---

## 7. Host Registration Quick Start

Allow players to become hosts and earn revenue from rooms they create.

### Unity

```csharp
using Deskillz.Host;

public class HostQuickStart : MonoBehaviour
{
    void Start()
    {
        // Initialize host manager
        HostManager.Instance.Initialize(currentUserId);
        
        // Subscribe to key events
        HostManager.Instance.OnTierChanged += (old, newTier) => 
            Debug.Log($"Tier upgraded to {newTier}!");
        HostManager.Instance.OnEarningsUpdated += (total, pending) =>
            Debug.Log($"Earnings: ${total} (${pending} pending)");
    }
    
    public void BecomeHost()
    {
        // Register as host
        HostManager.Instance.RegisterAsHost(
            profile => {
                Debug.Log($"Now a {profile.TierName} host!");
                Debug.Log($"Revenue share: {GetRevenueShare(profile.Tier)}%");
            },
            error => Debug.LogError(error)
        );
    }
    
    public void CreateHostRoom()
    {
        var config = new CreateHostRoomConfig
        {
            Name = "My First Room",
            GameCategory = GameCategory.Social,
            PointValue = 0.01m,        // $0.01 per point
            MinBuyIn = 10.0m,
            MaxBuyIn = 100.0m,
            DefaultBuyIn = 20.0m,
            RakePercentage = 5.0m,     // 5% rake
            RakeCap = 2.0m,            // Max $2 per pot
            EntryCurrency = "USDT",
            MaxPlayers = 6,
            Mode = RoomMode.Sync,
            AllowSpectators = true
        };
        
        HostManager.Instance.CreateHostRoom(config,
            room => Debug.Log($"Room created! Code: {room.RoomCode}"),
            error => Debug.LogError(error)
        );
    }
    
    public void CheckEarnings()
    {
        float total = HostManager.Instance.GetTotalEarnings();
        float pending = HostManager.Instance.GetPendingEarnings();
        Debug.Log($"Total: ${total}, Available to withdraw: ${pending}");
    }
    
    public void Withdraw()
    {
        float pending = HostManager.Instance.GetPendingEarnings();
        HostManager.Instance.WithdrawEarnings(pending,
            () => Debug.Log("Withdrawal initiated!"),
            error => Debug.LogError(error)
        );
    }
    
    string GetRevenueShare(HostTier tier)
    {
        return tier switch
        {
            HostTier.Starter => "50",
            HostTier.Bronze => "55",
            HostTier.Silver => "60",
            HostTier.Gold => "65",
            HostTier.Platinum => "70",
            HostTier.Diamond => "75",
            _ => "50"
        };
    }
}
```

### Unreal

```cpp
#include "Host/DeskillzHostManager.h"

void AHostQuickStart::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzHostManager::Get()->Initialize(CurrentUserId);
    
    UDeskillzHostManager::Get()->OnTierChanged.AddDynamic(this, &AHostQuickStart::HandleTierChange);
    UDeskillzHostManager::Get()->OnEarningsUpdated.AddDynamic(this, &AHostQuickStart::HandleEarnings);
}

void AHostQuickStart::BecomeHost()
{
    UDeskillzHostManager::Get()->RegisterAsHost(
        FOnHostProfileResult::CreateLambda([](const FHostProfile& Profile) {
            UE_LOG(LogTemp, Log, TEXT("Now a %s host!"), *Profile.TierName);
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AHostQuickStart::CreateHostRoom()
{
    FCreateHostRoomConfig Config;
    Config.Name = TEXT("My First Room");
    Config.GameCategory = EGameCategory::Social;
    Config.PointValue = 0.01f;
    Config.MinBuyIn = 10.0f;
    Config.MaxBuyIn = 100.0f;
    Config.DefaultBuyIn = 20.0f;
    Config.RakePercentage = 5.0f;
    Config.RakeCap = 2.0f;
    Config.EntryCurrency = TEXT("USDT");
    Config.MaxPlayers = 6;
    Config.Mode = ERoomMode::Sync;
    Config.bAllowSpectators = true;
    
    UDeskillzHostManager::Get()->CreateHostRoom(Config,
        FOnRoomCreated::CreateLambda([](const FPrivateRoom& Room) {
            UE_LOG(LogTemp, Log, TEXT("Room created! Code: %s"), *Room.RoomCode);
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}
```

---

## 8. Social Game Quick Start

Implement rake-based social games with buy-in, rebuy, and cash-out flows.

### Unity

```csharp
using Deskillz.Social;

public class SocialGameQuickStart : MonoBehaviour
{
    void Start()
    {
        // Initialize managers for this room
        SocialGameManager.Instance.Initialize(roomId);
        BuyInManager.Instance.Initialize(roomId);
        
        // Subscribe to events
        SocialGameManager.Instance.OnRoundEnded += HandleRoundEnd;
        BuyInManager.Instance.OnRebuyRequired += HandleRebuyRequired;
        BuyInManager.Instance.OnLowBalanceWarning += HandleLowBalance;
    }
    
    public void StartSession()
    {
        SocialGameManager.Instance.StartSession(
            session => Debug.Log($"Session started: {session.Id}"),
            error => Debug.LogError(error)
        );
    }
    
    public void PlayerBuyIn(string playerId, float amount)
    {
        BuyInManager.Instance.ProcessBuyIn(playerId, amount,
            result => Debug.Log($"{playerId} bought in for ${amount}"),
            error => Debug.LogError(error)
        );
    }
    
    public void EndRound(string winnerId, float potAmount)
    {
        // End round - rake is automatically calculated
        SocialGameManager.Instance.EndRound(winnerId, potAmount);
    }
    
    void HandleRoundEnd(RoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber} complete");
        Debug.Log($"Winner: {result.WinnerId}");
        Debug.Log($"Pot: ${result.PotAmount}");
        Debug.Log($"Rake: ${result.RakeAmount}");
        
        // Start next round
        SocialGameManager.Instance.StartRound();
    }
    
    void HandleRebuyRequired(string playerId, float timeout)
    {
        Debug.Log($"{playerId} is out! {timeout}s to rebuy");
        // Show rebuy UI
    }
    
    void HandleLowBalance(string playerId, float balance, float threshold)
    {
        Debug.Log($"Warning: {playerId} balance (${balance}) below ${threshold}");
    }
    
    public void PlayerCashOut(string playerId)
    {
        BuyInManager.Instance.ProcessCashOut(playerId,
            result => Debug.Log($"{playerId} cashed out ${result.Amount}"),
            error => Debug.LogError(error)
        );
    }
    
    public void PreviewRake(float potAmount)
    {
        // Show players what rake will be taken
        float rake = RakeCalculator.CalculateRakeWithCap(potAmount, 5.0f, 3.0f);
        Debug.Log($"Pot: ${potAmount} -> Rake: ${rake} -> Winner gets: ${potAmount - rake}");
    }
}
```

### Unreal

```cpp
#include "Social/DeskillzSocialGameManager.h"
#include "Social/DeskillzBuyInManager.h"
#include "Social/DeskillzRakeCalculator.h"

void ASocialGameQuickStart::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzSocialGameManager::Get()->Initialize(RoomId);
    UDeskillzBuyInManager::Get()->Initialize(RoomId);
    
    UDeskillzSocialGameManager::Get()->OnRoundEnded.AddDynamic(this, &ASocialGameQuickStart::HandleRoundEnd);
    UDeskillzBuyInManager::Get()->OnRebuyRequired.AddDynamic(this, &ASocialGameQuickStart::HandleRebuyRequired);
}

void ASocialGameQuickStart::StartSession()
{
    UDeskillzSocialGameManager::Get()->StartSession(
        FOnSessionStarted::CreateLambda([](const FSocialGameSession& Session) {
            UE_LOG(LogTemp, Log, TEXT("Session started: %s"), *Session.Id);
        }),
        FOnSocialError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void ASocialGameQuickStart::PlayerBuyIn(const FString& PlayerId, float Amount)
{
    UDeskillzBuyInManager::Get()->ProcessBuyIn(PlayerId, Amount,
        FOnBuyInResult::CreateLambda([PlayerId, Amount](const FBuyInResult& Result) {
            UE_LOG(LogTemp, Log, TEXT("%s bought in for $%.2f"), *PlayerId, Amount);
        }),
        FOnBuyInError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void ASocialGameQuickStart::EndRound(const FString& WinnerId, float PotAmount)
{
    UDeskillzSocialGameManager::Get()->EndRound(WinnerId, PotAmount);
}

void ASocialGameQuickStart::HandleRoundEnd(const FRoundResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("Round %d - Winner: %s, Pot: $%.2f, Rake: $%.2f"),
        Result.RoundNumber, *Result.WinnerId, Result.PotAmount, Result.RakeAmount);
    
    // Start next round
    UDeskillzSocialGameManager::Get()->StartRound();
}
```

---

## 9. Host Spectator Mode Quick Start

Let hosts monitor their private social rooms (host-only feature).

> **Note:** Only hosts can spectate their own rooms. General public spectating is not available.
> Hosts can see board/scores but NOT player hands (anti-cheat).

### Unity

```csharp
using Deskillz.Host;

public class HostSpectatorQuickStart : MonoBehaviour
{
    void Start()
    {
        // Host must be authenticated first
        HostSpectatorManager.Instance.Initialize();
        
        // Subscribe to events for YOUR rooms
        HostSpectatorManager.Instance.OnGameStateUpdated += HandleStateUpdate;
        HostSpectatorManager.Instance.OnRoundEnded += HandleRoundEnd;
    }
    
    public void FetchMyRooms()
    {
        var filter = new HostRoomFilter
        {
            GameCategory = GameCategory.Social, // Social rooms only
            Status = RoomStatus.Active,
            IsActive = true
        };
        
        HostSpectatorManager.Instance.FetchHostRooms(filter,
            rooms => {
                foreach (var room in rooms)
                {
                    Debug.Log($"{room.RoomName}: {room.PlayerCount} players");
                }
            },
            error => Debug.LogError(error)
        );
    }
    
    public void SpectateMyRoom(string roomId)
    {
        // Join YOUR room as spectator (see board, NOT hands)
        HostSpectatorManager.Instance.SpectateRoom(roomId,
            state => {
                Debug.Log($"Watching your room: {state.RoomName}");
                Debug.Log($"Round: {state.CurrentRound}");
                // Note: Player hands NOT visible (anti-cheat)
            },
            error => Debug.LogError(error)
        );
    }
    
    void HandleStateUpdate(HostSpectatorState state)
    {
        // Update board (NO hands visible)
        Debug.Log($"Scores updated");
    }
    
    void HandleRoundEnd(HostRoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber} winner: {result.WinnerUsername}");
    }
    
    public void SwitchRoom(string otherRoomId)
    {
        // Switch between YOUR rooms (multi-room hosting)
        HostSpectatorManager.Instance.SwitchRoom(otherRoomId);
    }
    
    public void StopSpectating()
    {
        HostSpectatorManager.Instance.StopSpectating();
    }
}
```

### Unreal

```cpp
#include "Host/DeskillzHostSpectatorManager.h"

void AHostSpectatorQuickStart::BeginPlay()
{
    Super::BeginPlay();
    
    // Host must be authenticated first
    UDeskillzHostSpectatorManager::Get()->Initialize();
    
    UDeskillzHostSpectatorManager::Get()->OnGameStateUpdated.AddDynamic(
        this, &AHostSpectatorQuickStart::HandleStateUpdate);
    UDeskillzHostSpectatorManager::Get()->OnRoundEnded.AddDynamic(
        this, &AHostSpectatorQuickStart::HandleRoundEnd);
}

void AHostSpectatorQuickStart::FetchMyRooms()
{
    FHostRoomFilter Filter;
    Filter.GameCategory = EGameCategory::Social; // Social only
    Filter.bIsActive = true;
    
    UDeskillzHostSpectatorManager::Get()->FetchHostRooms(Filter,
        FOnHostRoomsResult::CreateLambda([](const TArray<FHostRoom>& Rooms) {
            for (const auto& Room : Rooms)
            {
                UE_LOG(LogTemp, Log, TEXT("%s: %d players"),
                    *Room.RoomName, Room.PlayerCount);
            }
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AHostSpectatorQuickStart::SpectateMyRoom(const FString& RoomId)
{
    // Join YOUR room (see board, NOT hands)
    UDeskillzHostSpectatorManager::Get()->SpectateRoom(RoomId,
        FOnJoinedSpectator::CreateLambda([](const FHostSpectatorState& State) {
            UE_LOG(LogTemp, Log, TEXT("Watching your room: %s"), *State.RoomName);
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AHostSpectatorQuickStart::HandleStateUpdate(const FHostSpectatorState& State)
{
    // Board update (NO hands visible)
    UE_LOG(LogTemp, Log, TEXT("Scores updated"));
}

void AHostSpectatorQuickStart::HandleRoundEnd(const FHostRoundResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("Round %d winner: %s"), Result.RoundNumber, *Result.WinnerUsername);
}
```

---

## 10. Test Your Integration

### Test Mode

Test without real currency in the Unity Editor or with test flags:

```csharp
// Unity - Test deep link launch
DeepLinkHandler.SimulateDeepLink("deskillz://launch?matchId=test123&token=testtoken");

// Test room UI
PrivateRoomUI.Instance.ShowRoomList();

// Test host system
HostManager.Instance.Initialize("test-user");
HostDashboardUI.Instance.Show();

// Test social games
SocialGameManager.Instance.StartTestSession();
BuyInModal.Instance.Show(10f, 200f);

// Test host spectator mode (host-only)
HostSpectatorManager.Instance.FetchHostRooms(new HostRoomFilter());
HostSpectatorView.Instance.Show();

// Test auto-updater
DeskillzUpdater.Instance.CheckForUpdates();
```

### Sandbox vs Production

| Environment | Use For | API Base |
|-------------|---------|----------|
| Sandbox | Development & Testing | sandbox.api.deskillz.games |
| Production | Live Release | api.deskillz.games |

```csharp
// Development
config.Environment = DeskillzEnvironment.Sandbox;

// Release
config.Environment = DeskillzEnvironment.Production;
```

### Pre-Launch Checklist

- [ ] SDK initializes without errors
- [ ] Deep link match launch works
- [ ] Score submission succeeds
- [ ] Auto-updater checks work
- [ ] Private room creation/joining works
- [ ] Host registration works (if applicable)
- [ ] Social game buy-in/cashout works (if applicable)
- [ ] Spectator mode works (if applicable)
- [ ] Test in both Sandbox and Production
- [ ] No test/debug code in release build

---

## What's Next?

- [API Reference](API_REFERENCE.md) - Complete method documentation
- [Integration Guide](INTEGRATION_GUIDE.md) - Detailed integration guide
- [Unity SDK Repository](https://github.com/Deskillz-Games-Development/unity-sdk)
- [Unreal SDK Repository](https://github.com/Deskillz-Games-Development/unreal-sdk)
- [Developer Portal](https://developer.deskillz.games) - Manage your games
- [Documentation](https://docs.deskillz.games) - Full documentation
- [Support](https://support.deskillz.games) - Get help

---

## Version History

| Version | Changes |
|---------|---------|
| 2.6.0 | Added Host, Social Game, and Spectator quick starts |
| 2.5.0 | Added Auto-Updater section |
| 2.2.0 | Added Private Rooms |
| 2.0.0 | Deep Link architecture |