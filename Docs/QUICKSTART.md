# Deskillz SDK - Quick Start Guide

**SDK Version: 2.6.0** | Get your game integrated with Deskillz in minutes.

## Table of Contents

0. [Get Your Game Credentials](#0-get-your-game-credentials) - NEW! Start Here
1. [Prerequisites](#1-prerequisites)
2. [Installation](#2-installation)
3. [Basic Setup](#3-basic-setup)
4. [Handle Match Launch](#4-handle-match-launch)
5. [Submit Score](#5-submit-score)
6. [Return to Main App](#6-return-to-main-app)
7. [Enable Auto-Updates](#7-enable-auto-updates)
8. [Host Registration Quick Start](#8-host-registration-quick-start)
9. [Social Game Quick Start](#9-social-game-quick-start)
10. [Host Spectator Mode Quick Start](#10-host-spectator-mode-quick-start)
11. [Test Your Integration](#11-test-your-integration)

---

## 0. Get Your Game Credentials

**IMPORTANT: Start here!** Before writing any code, you need your Game ID and API credentials.

### Why Credentials First?

The Deskillz SDK requires a Game ID to initialize. With our **Credentials-First Flow**, you can get your credentials immediately and start building - no need to complete the full registration form first.

### Step 1: Access Developer Portal

1. Go to [deskillz.games/developer](https://deskillz.games/developer)
2. Connect your wallet or create an account
3. Click **"Register New Game"**

### Step 2: Generate Credentials Instantly

1. Enter your **Game Name** (e.g., "Block Puzzle Master")
2. Select your **Target Platform** (Android / iOS / Both)
3. Click **"Generate Game ID & API Key"**

You will immediately receive:

| Credential | Example | Purpose |
|------------|---------|---------|
| **Game ID** | `a1b2c3d4-e5f6-7890-abcd-ef1234567890` | Unique identifier for your game |
| **API Key** | `dsk_live_abc123def456ghi789...` | Public key for SDK authentication |
| **API Secret** | `dss_xyz789abc456def123...` | Private key for HMAC signing |
| **Deep Link Scheme** | `deskillz-blockpuzzlemaster` | Custom URL scheme for app launching |

### Step 3: Save Your API Secret Immediately!

**WARNING:** Your API Secret is displayed **only once**! 

- Copy it immediately and store it securely
- Never commit it to source control
- You cannot retrieve it later - you would need to regenerate (invalidating the old one)

### Step 4: Configure Your Project

**Unity - Store in a ScriptableObject or config file:**

```csharp
// Create: Assets/Resources/DeskillzConfig.asset
// Add to .gitignore to keep credentials out of source control

[CreateAssetMenu(fileName = "DeskillzConfig", menuName = "Deskillz/Config")]
public class DeskillzConfig : ScriptableObject
{
    public string GameId = "YOUR_GAME_ID";
    public string ApiKey = "YOUR_API_KEY";
    public string ApiSecret = "YOUR_API_SECRET"; // Keep secure!
    public string DeepLinkScheme = "deskillz-yourgame";
    public bool UseSandbox = true;
}
```

**Unreal - Store in a Data Asset:**

```cpp
// Create: Content/Config/DeskillzConfig.uasset
// Add Config/ folder to .gitignore

UCLASS()
class UDeskillzConfigAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    FString GameId = TEXT("YOUR_GAME_ID");
    
    UPROPERTY(EditAnywhere)
    FString ApiKey = TEXT("YOUR_API_KEY");
    
    UPROPERTY(EditAnywhere)
    FString ApiSecret = TEXT("YOUR_API_SECRET"); // Keep secure!
    
    UPROPERTY(EditAnywhere)
    FString DeepLinkScheme = TEXT("deskillz-yourgame");
    
    UPROPERTY(EditAnywhere)
    bool bUseSandbox = true;
};
```

### Step 5: Complete Registration (When Ready)

After verifying your SDK integration works:

1. Return to Developer Portal > My Games > [Your Draft Game]
2. Complete the remaining form sections:
   - Game description and category
   - Tournament configuration
   - Monetization settings
   - Screenshots, icon, and video
   - Upload your APK/IPA build
3. Submit for review

**Your game stays in DRAFT status until you submit the full form.**

---

## 1. Prerequisites

Before you begin, ensure you have:

- [x] **Game Credentials** from Step 0 above (Game ID, API Key, API Secret)
- [x] Unity 2020.3+ or Unreal Engine 4.27+
- [x] Basic knowledge of C# (Unity) or C++ (Unreal)
- [x] Android SDK / Xcode for mobile builds

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
    // Load credentials from config asset (see Step 0)
    [SerializeField] private DeskillzConfig config;
    
    void Awake()
    {
        // Ensure only one instance
        DontDestroyOnLoad(gameObject);
        
        // Validate credentials
        if (string.IsNullOrEmpty(config.GameId) || string.IsNullOrEmpty(config.ApiKey))
        {
            Debug.LogError("Missing Deskillz credentials! See QUICKSTART Step 0.");
            return;
        }
        
        // Configure SDK
        var sdkConfig = new DeskillzSDKConfig
        {
            GameId = config.GameId,
            ApiKey = config.ApiKey,
            Environment = config.UseSandbox 
                ? DeskillzEnvironment.Sandbox 
                : DeskillzEnvironment.Production,
            EnableLogging = true
        };
        
        // Initialize
        DeskillzSDK.Instance.Initialize(sdkConfig);
        DeskillzSDK.Instance.OnInitialized += OnSDKReady;
        DeskillzSDK.Instance.OnError += OnSDKError;
    }
    
    void OnSDKReady()
    {
        Debug.Log("Deskillz SDK Ready!");
        
        // Initialize deep link handler
        DeepLinkHandler.Instance.Initialize();
        DeepLinkHandler.Instance.OnMatchReady += OnMatchReady;
        
        // Check for pending launch (cold start from deep link)
        if (DeepLinkHandler.Instance.HasPendingLaunch())
        {
            DeepLinkHandler.Instance.ProcessPendingLaunch();
        }
        
        // Check for updates
        DeskillzUpdater.Instance.CheckForUpdates();
    }
    
    void OnSDKError(DeskillzError error)
    {
        Debug.LogError($"SDK Error: {error.Code} - {error.Message}");
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
    void OnSDKError(const FDeskillzError& Error);
    
    UFUNCTION()
    void OnMatchReady(const FMatchLaunchData& Data);
};

// MyGameMode.cpp
#include "MyGameMode.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Load config from Data Asset (see Step 0)
    UDeskillzConfigAsset* Config = LoadObject<UDeskillzConfigAsset>(
        nullptr, TEXT("/Game/Config/DeskillzConfig.DeskillzConfig"));
    
    if (!Config || Config->GameId.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Missing Deskillz credentials! See QUICKSTART Step 0."));
        return;
    }
    
    FDeskillzConfig SDKConfig;
    SDKConfig.GameId = Config->GameId;
    SDKConfig.ApiKey = Config->ApiKey;
    SDKConfig.Environment = Config->bUseSandbox 
        ? EDeskillzEnvironment::Sandbox 
        : EDeskillzEnvironment::Production;
    SDKConfig.bEnableLogging = true;
    
    UDeskillzSDK::Get()->Initialize(SDKConfig);
    UDeskillzSDK::Get()->OnInitialized.AddDynamic(this, &AMyGameMode::OnSDKReady);
    UDeskillzSDK::Get()->OnError.AddDynamic(this, &AMyGameMode::OnSDKError);
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
    
    // Check for updates
    UDeskillzUpdater::Get()->CheckForUpdates();
}

void AMyGameMode::OnSDKError(const FDeskillzError& Error)
{
    UE_LOG(LogTemp, Error, TEXT("SDK Error: %s - %s"), *Error.Code, *Error.Message);
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
        else
        {
            Debug.LogError("No match data! Game should be launched from Deskillz app.");
        }
    }
    
    void StartMatch()
    {
        startTime = Time.time;
        
        // Use the random seed for fair play (both players get same seed)
        UnityEngine.Random.InitState(matchData.RandomSeed);
        
        // Show opponent info (optional)
        if (matchData.Opponents != null && matchData.Opponents.Length > 0)
        {
            Debug.Log($"Playing against: {matchData.Opponents[0].Username}");
        }
        Debug.Log($"Entry fee: {matchData.EntryFee} {matchData.Currency}");
        
        // Start your game logic
        BeginGameplay();
    }
    
    void BeginGameplay()
    {
        // Your game logic here
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
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No match data! Game should be launched from Deskillz app."));
    }
}

void AGameController::StartMatch(const FMatchLaunchData& Data)
{
    MatchData = Data;
    StartTime = GetWorld()->GetTimeSeconds();
    
    // Use random seed for fair play
    FMath::RandInit(Data.RandomSeed);
    
    // Log opponent info
    if (Data.Opponents.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Playing against: %s"), *Data.Opponents[0].Username);
    }
    
    // Start gameplay
    BeginGameplay();
}
```

---

## 5. Submit Score

When the match ends, submit the score securely using HMAC signing.

### Unity

```csharp
using Deskillz;
using Deskillz.Security;
using System;
using System.Security.Cryptography;
using System.Text;

public class GameController : MonoBehaviour
{
    [SerializeField] private DeskillzConfig config; // Contains ApiSecret
    
    public void OnGameOver(int finalScore)
    {
        float duration = Time.time - startTime;
        long timestamp = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        string nonce = Guid.NewGuid().ToString("N");
        
        // Generate HMAC hash for security
        string hash = GenerateScoreHash(
            matchData.MatchId, 
            finalScore, 
            (int)duration, 
            timestamp, 
            nonce
        );
        
        var scoreData = new ScoreSubmissionData
        {
            MatchId = matchData.MatchId,
            Score = finalScore,
            Duration = (int)duration,
            Timestamp = timestamp,
            Nonce = nonce,
            Hash = hash
        };
        
        // Submit score
        DeskillzAPI.SubmitScore(scoreData, result =>
        {
            if (result.Success)
            {
                Debug.Log("Score submitted successfully!");
                ReturnToMainApp();
            }
            else
            {
                Debug.LogError($"Score submission failed: {result.Error}");
                // Still return to main app with error
                ReturnToMainApp(result.Error);
            }
        });
    }
    
    private string GenerateScoreHash(string matchId, int score, int duration, long timestamp, string nonce)
    {
        // Format: matchId:score:duration:timestamp:nonce
        string data = $"{matchId}:{score}:{duration}:{timestamp}:{nonce}";
        
        using (var hmac = new HMACSHA256(Encoding.UTF8.GetBytes(config.ApiSecret)))
        {
            byte[] hashBytes = hmac.ComputeHash(Encoding.UTF8.GetBytes(data));
            return Convert.ToBase64String(hashBytes);
        }
    }
    
    private void ReturnToMainApp(string error = null)
    {
        string destination = string.IsNullOrEmpty(error) ? "results" : "error";
        DeskillzSDK.Instance.ReturnToMainApp(matchData.MatchId, destination);
    }
}
```

### Unreal

```cpp
#include "DeskillzAPI.h"
#include "Misc/SecureHash.h"

void AGameController::OnGameOver(int32 FinalScore)
{
    float Duration = GetWorld()->GetTimeSeconds() - StartTime;
    int64 Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
    FString Nonce = FGuid::NewGuid().ToString(EGuidFormats::DigitsLower);
    
    // Generate HMAC hash
    FString Hash = GenerateScoreHash(MatchData.MatchId, FinalScore, (int32)Duration, Timestamp, Nonce);
    
    FScoreSubmissionData ScoreData;
    ScoreData.MatchId = MatchData.MatchId;
    ScoreData.Score = FinalScore;
    ScoreData.Duration = (int32)Duration;
    ScoreData.Timestamp = Timestamp;
    ScoreData.Nonce = Nonce;
    ScoreData.Hash = Hash;
    
    UDeskillzAPI::Get()->SubmitScore(ScoreData,
        FOnScoreSubmitted::CreateLambda([this](const FScoreResult& Result) {
            if (Result.bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("Score submitted!"));
                ReturnToMainApp();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Submission failed: %s"), *Result.Error);
                ReturnToMainApp(Result.Error);
            }
        })
    );
}

FString AGameController::GenerateScoreHash(const FString& MatchId, int32 Score, int32 Duration, int64 Timestamp, const FString& Nonce)
{
    // Format: matchId:score:duration:timestamp:nonce
    FString Data = FString::Printf(TEXT("%s:%d:%d:%lld:%s"), *MatchId, Score, Duration, Timestamp, *Nonce);
    
    // Get API Secret from config
    UDeskillzConfigAsset* Config = LoadObject<UDeskillzConfigAsset>(
        nullptr, TEXT("/Game/Config/DeskillzConfig.DeskillzConfig"));
    
    // Generate HMAC-SHA256
    TArray<uint8> DataBytes;
    FTCHARToUTF8 DataConverter(*Data);
    DataBytes.Append((uint8*)DataConverter.Get(), DataConverter.Length());
    
    TArray<uint8> KeyBytes;
    FTCHARToUTF8 KeyConverter(*Config->ApiSecret);
    KeyBytes.Append((uint8*)KeyConverter.Get(), KeyConverter.Length());
    
    TArray<uint8> HashBytes;
    FSHA256Signature::HMACSHA256(DataBytes, KeyBytes, HashBytes);
    
    return FBase64::Encode(HashBytes);
}
```

---

## 6. Return to Main App

Always return the player to the Deskillz main app after the match ends.

### Unity

```csharp
using Deskillz;

public class AppNavigator : MonoBehaviour
{
    public static void ReturnToMainApp(string matchId, string destination = "results")
    {
        // Build deep link URL
        string url = $"deskillz://{destination}?matchId={matchId}";
        
        #if UNITY_ANDROID
        using (var intent = new AndroidJavaObject("android.content.Intent", 
            "android.intent.action.VIEW", 
            new AndroidJavaClass("android.net.Uri").CallStatic<AndroidJavaObject>("parse", url)))
        {
            using (var activity = new AndroidJavaClass("com.unity3d.player.UnityPlayer")
                .GetStatic<AndroidJavaObject>("currentActivity"))
            {
                activity.Call("startActivity", intent);
            }
        }
        #elif UNITY_IOS
        Application.OpenURL(url);
        #endif
    }
}
```

### Unreal

```cpp
void UDeskillzAppNavigator::ReturnToMainApp(const FString& MatchId, const FString& Destination)
{
    FString URL = FString::Printf(TEXT("deskillz://%s?matchId=%s"), *Destination, *MatchId);
    
    #if PLATFORM_ANDROID
    FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
    #elif PLATFORM_IOS
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:URL.GetNSString()]
                                       options:@{} 
                             completionHandler:nil];
    #endif
}
```

---

## 7. Enable Auto-Updates

Ensure players always have the latest version of your game.

### Unity

```csharp
using Deskillz.Updater;

public class AppInitializer : MonoBehaviour
{
    void Start()
    {
        // Set current version
        DeskillzUpdater.Instance.CurrentVersion = Application.version;
        DeskillzUpdater.Instance.CurrentVersionCode = GetVersionCode();
        
        // Subscribe to update events
        DeskillzUpdater.Instance.OnUpdateAvailable += HandleUpdateAvailable;
        DeskillzUpdater.Instance.OnForceUpdateRequired += HandleForceUpdate;
        
        // Check for updates
        DeskillzUpdater.Instance.CheckForUpdates();
    }
    
    void HandleUpdateAvailable(UpdateInfo info)
    {
        // Optional update - show dialog
        DeskillzUpdaterUI.Instance.ShowUpdateDialog(info);
    }
    
    void HandleForceUpdate(UpdateInfo info)
    {
        // Required update - must update to continue
        DeskillzUpdaterUI.Instance.ShowForceUpdateDialog(info);
    }
    
    private int GetVersionCode()
    {
        #if UNITY_ANDROID
        using (var version = new AndroidJavaClass("com.unity3d.player.UnityPlayer")
            .GetStatic<AndroidJavaObject>("currentActivity")
            .Call<AndroidJavaObject>("getPackageManager")
            .Call<AndroidJavaObject>("getPackageInfo", Application.identifier, 0))
        {
            return version.Get<int>("versionCode");
        }
        #else
        return int.Parse(Application.version.Replace(".", ""));
        #endif
    }
}
```

### Unreal

```cpp
#include "Updater/DeskillzUpdater.h"

void AMyGameMode::InitializeUpdater()
{
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    
    Updater->SetCurrentVersion(GetGameVersion(), GetVersionCode());
    
    Updater->OnUpdateAvailable.AddDynamic(this, &AMyGameMode::HandleUpdateAvailable);
    Updater->OnForceUpdateRequired.AddDynamic(this, &AMyGameMode::HandleForceUpdate);
    
    Updater->CheckForUpdates();
}

void AMyGameMode::HandleUpdateAvailable(const FUpdateInfo& Info)
{
    // Show optional update dialog
    UDeskillzUpdaterUI::Get()->ShowUpdateDialog(Info);
}

void AMyGameMode::HandleForceUpdate(const FUpdateInfo& Info)
{
    // Show mandatory update dialog (blocks gameplay)
    UDeskillzUpdaterUI::Get()->ShowForceUpdateDialog(Info);
}
```

---

## 8. Host Registration Quick Start

Enable players to become hosts and create private rooms.

### Unity

```csharp
using Deskillz.Host;

public class HostQuickStart : MonoBehaviour
{
    void Start()
    {
        // Initialize host system
        HostManager.Instance.Initialize();
        
        // Check if user is already a host
        if (HostManager.Instance.IsHost)
        {
            Debug.Log($"Host Level: {HostManager.Instance.HostLevel}");
        }
    }
    
    public void RegisterAsHost()
    {
        HostManager.Instance.RegisterAsHost(
            success => {
                if (success)
                {
                    Debug.Log("Now a host!");
                    HostDashboardUI.Instance.Show();
                }
            },
            error => Debug.LogError(error)
        );
    }
}
```

### Unreal

```cpp
#include "Host/DeskillzHostManager.h"

void AHostQuickStart::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzHostManager::Get()->Initialize();
    
    if (UDeskillzHostManager::Get()->IsHost())
    {
        UE_LOG(LogTemp, Log, TEXT("Host Level: %d"), 
            UDeskillzHostManager::Get()->GetHostLevel());
    }
}

void AHostQuickStart::RegisterAsHost()
{
    UDeskillzHostManager::Get()->RegisterAsHost(
        FOnHostRegistered::CreateLambda([](bool bSuccess) {
            if (bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("Now a host!"));
                UDeskillzHostDashboardUI::Get()->Show();
            }
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}
```

---

## 9. Social Game Quick Start

Implement social games with buy-ins, rake, and multi-round sessions.

### Unity

```csharp
using Deskillz.Social;

public class SocialGameQuickStart : MonoBehaviour
{
    private string roomId;
    
    void Start()
    {
        SocialGameManager.Instance.Initialize(roomId);
        BuyInManager.Instance.Initialize(roomId);
        
        SocialGameManager.Instance.OnRoundEnded += HandleRoundEnd;
        BuyInManager.Instance.OnRebuyRequired += HandleRebuyRequired;
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
        SocialGameManager.Instance.EndRound(winnerId, potAmount);
    }
    
    void HandleRoundEnd(RoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber} - Winner: {result.WinnerId}");
        Debug.Log($"Pot: ${result.PotAmount} | Rake: ${result.RakeAmount}");
        
        // Start next round
        SocialGameManager.Instance.StartRound();
    }
    
    void HandleRebuyRequired(string playerId)
    {
        // Show rebuy modal
        RebuyModal.Instance.Show(playerId);
    }
}
```

### Unreal

```cpp
#include "Social/DeskillzSocialGameManager.h"
#include "Social/DeskillzBuyInManager.h"

void ASocialGameQuickStart::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzSocialGameManager::Get()->Initialize(RoomId);
    UDeskillzBuyInManager::Get()->Initialize(RoomId);
    
    UDeskillzSocialGameManager::Get()->OnRoundEnded.AddDynamic(
        this, &ASocialGameQuickStart::HandleRoundEnd);
    UDeskillzBuyInManager::Get()->OnRebuyRequired.AddDynamic(
        this, &ASocialGameQuickStart::HandleRebuyRequired);
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
    
    UDeskillzSocialGameManager::Get()->StartRound();
}
```

---

## 10. Host Spectator Mode Quick Start

Let hosts monitor their private social rooms (host-only feature).

> **Note:** Only hosts can spectate their own rooms. Hosts can see board/scores but NOT player hands (anti-cheat).

### Unity

```csharp
using Deskillz.Host;

public class HostSpectatorQuickStart : MonoBehaviour
{
    void Start()
    {
        HostSpectatorManager.Instance.Initialize();
        HostSpectatorManager.Instance.OnGameStateUpdated += HandleStateUpdate;
        HostSpectatorManager.Instance.OnRoundEnded += HandleRoundEnd;
    }
    
    public void FetchMyRooms()
    {
        var filter = new HostRoomFilter
        {
            GameCategory = GameCategory.Social,
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
        HostSpectatorManager.Instance.SpectateRoom(roomId,
            state => {
                Debug.Log($"Watching: {state.RoomName}");
                // Note: Player hands NOT visible (anti-cheat)
            },
            error => Debug.LogError(error)
        );
    }
    
    void HandleStateUpdate(HostSpectatorState state)
    {
        Debug.Log($"Scores updated - Round: {state.CurrentRound}");
    }
    
    void HandleRoundEnd(HostRoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber} winner: {result.WinnerUsername}");
    }
    
    public void SwitchRoom(string otherRoomId)
    {
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
    
    UDeskillzHostSpectatorManager::Get()->Initialize();
    
    UDeskillzHostSpectatorManager::Get()->OnGameStateUpdated.AddDynamic(
        this, &AHostSpectatorQuickStart::HandleStateUpdate);
    UDeskillzHostSpectatorManager::Get()->OnRoundEnded.AddDynamic(
        this, &AHostSpectatorQuickStart::HandleRoundEnd);
}

void AHostSpectatorQuickStart::FetchMyRooms()
{
    FHostRoomFilter Filter;
    Filter.GameCategory = EGameCategory::Social;
    Filter.bIsActive = true;
    
    UDeskillzHostSpectatorManager::Get()->FetchHostRooms(Filter,
        FOnHostRoomsResult::CreateLambda([](const TArray<FHostRoom>& Rooms) {
            for (const auto& Room : Rooms)
            {
                UE_LOG(LogTemp, Log, TEXT("%s: %d players"), *Room.RoomName, Room.PlayerCount);
            }
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AHostSpectatorQuickStart::SpectateMyRoom(const FString& RoomId)
{
    UDeskillzHostSpectatorManager::Get()->SpectateRoom(RoomId,
        FOnJoinedSpectator::CreateLambda([](const FHostSpectatorState& State) {
            UE_LOG(LogTemp, Log, TEXT("Watching: %s"), *State.RoomName);
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AHostSpectatorQuickStart::HandleStateUpdate(const FHostSpectatorState& State)
{
    UE_LOG(LogTemp, Log, TEXT("Scores updated - Round: %d"), State.CurrentRound);
}

void AHostSpectatorQuickStart::HandleRoundEnd(const FHostRoundResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("Round %d winner: %s"), Result.RoundNumber, *Result.WinnerUsername);
}
```

---

## 11. Test Your Integration

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

// Test host spectator mode
HostSpectatorManager.Instance.FetchHostRooms(new HostRoomFilter());
HostSpectatorView.Instance.Show();

// Test auto-updater
DeskillzUpdater.Instance.TestUpdateAvailable = true;
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

- [ ] Game credentials obtained (Game ID, API Key, API Secret)
- [ ] SDK initializes without errors
- [ ] Deep link match launch works
- [ ] Score submission succeeds with HMAC hash
- [ ] Return to main app works correctly
- [ ] Auto-updater checks work
- [ ] Private room creation/joining works
- [ ] Host registration works (if applicable)
- [ ] Social game buy-in/cashout works (if applicable)
- [ ] Spectator mode works (if applicable)
- [ ] Test in both Sandbox and Production
- [ ] No test/debug code in release build
- [ ] API Secret not committed to source control

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
| 2.6.0 | Added Credentials-First Flow (Step 0), Host, Social Game, and Spectator quick starts |
| 2.5.0 | Added Auto-Updater section |
| 2.2.0 | Added Private Rooms |
| 2.0.0 | Deep Link architecture, Centralized Lobby |

---

**SDK Version:** 2.6.0  
**Guide Version:** 2.0  
**Last Updated:** January 2026