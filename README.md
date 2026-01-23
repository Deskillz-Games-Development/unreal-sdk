# Deskillz Unreal Engine SDK

<p align="center">
  <img src="https://deskillz.games/logo.png" alt="Deskillz.Games" width="200"/>
</p>

<p align="center">
  <strong>Integrate competitive tournaments into your Unreal Engine games</strong>
</p>

<p align="center">
  <a href="https://github.com/Deskillz-Games-Development/unreal-sdk/releases"><img src="https://img.shields.io/badge/version-2.8.0-blue.svg" alt="Version"></a>
  <a href="https://www.unrealengine.com"><img src="https://img.shields.io/badge/unreal-4.27%2B%20%7C%205.0%2B-black.svg" alt="Unreal"></a>
  <a href="https://github.com/Deskillz-Games-Development/unreal-sdk/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License"></a>
</p>

<p align="center">
  <a href="#getting-your-credentials">Get Credentials</a> |
  <a href="#installation">Installation</a> |
  <a href="#quick-start">Quick Start</a> |
  <a href="#self-sufficient-authentication">Self-Sufficient Auth</a> |
  <a href="#features">Features</a> |
  <a href="#auto-updater">Auto-Updater</a> |
  <a href="#private-rooms">Private Rooms</a> |
  <a href="#host-system">Host System</a> |
  <a href="#social-games">Social Games</a> |
  <a href="#spectator-mode">Spectator Mode</a> |
  <a href="#navigation-deep-links">Navigation Links</a> |
  <a href="#documentation">Documentation</a> |
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
- **C++ Standard:** C++17
- **Build Tools:** Xcode 14+ (iOS), Android NDK (Android)

---

## Getting Your Credentials

**IMPORTANT: Start here before installation!**

The SDK requires a Game ID and API Key to initialize. With our **Credentials-First Flow**, you can get these instantly.

### Step 1: Access Developer Portal

1. Go to [deskillz.games/developer](https://deskillz.games/developer)
2. Connect your wallet or create an account
3. Click **"Register New Game"**

### Step 2: Generate Credentials Instantly

1. Enter your **Game Name** (e.g., "Block Puzzle Master")
2. Select your **Target Platform** (Android / iOS / Both)
3. Click **"Generate Game ID & API Key"**

### Step 3: You Receive Immediately

| Credential | Example | Purpose |
|------------|---------|---------|
| **Game ID** | `a1b2c3d4-e5f6-7890-abcd-ef1234567890` | Unique identifier |
| **API Key** | `dsk_live_abc123def456ghi789...` | Public key for SDK |
| **API Secret** | `dss_xyz789abc456def123...` | Private key for HMAC signing |
| **Deep Link Scheme** | `deskillz-blockpuzzlemaster` | Custom URL scheme |

### CRITICAL: Save Your API Secret!

> **WARNING:** Your API Secret is displayed **only once**. Copy it immediately and store it securely:
> - Save it in a secure password manager
> - Never commit it to source control
> - You cannot retrieve it later - you would need to regenerate

### Step 4: Create DeskillzConfig Data Asset

1. In Unreal Editor: **Content Browser > Right Click > Miscellaneous > Data Asset**
2. Select `UDeskillzConfigAsset` as the class
3. Name it `DeskillzConfig` and place in `Content/Config/`
4. Enter your credentials:

```cpp
// Content/Config/DeskillzConfig.uasset should contain:
UCLASS()
class UDeskillzConfigAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Credentials")
    FString GameId = TEXT("YOUR_GAME_ID");
    
    UPROPERTY(EditAnywhere, Category = "Credentials")
    FString ApiKey = TEXT("YOUR_API_KEY");
    
    UPROPERTY(EditAnywhere, Category = "Security")
    FString ApiSecret = TEXT("YOUR_API_SECRET"); // For HMAC signing
    
    UPROPERTY(EditAnywhere, Category = "Settings")
    FString DeepLinkScheme = TEXT("deskillz-yourgame");
    
    UPROPERTY(EditAnywhere, Category = "Settings")
    bool bUseSandbox = true;
    
    // Self-Sufficient Architecture (NEW in v2.8)
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    bool bSelfSufficientMode = true;
    
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    FString AuthLevelName = TEXT("/Game/Maps/AuthLevel");
    
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    FString LobbyLevelName = TEXT("/Game/Maps/LobbyLevel");
    
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    FString GameLevelName = TEXT("/Game/Maps/GameLevel");
    
    UPROPERTY(EditAnywhere, Category = "API")
    FString ApiBaseUrl = TEXT("https://api.deskillz.games/api/v1");
};
```

**Add to .gitignore:**
```
# Deskillz credentials - do not commit!
Content/Config/DeskillzConfig.uasset
```

### Step 5: Complete Registration (When Ready)

After verifying your SDK integration works, return to Developer Portal to:
1. Complete the full game submission form
2. Upload screenshots, icon, and video
3. Upload your APK/IPA build
4. Submit for review

---

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
#include "DeskillzAuth.h"
#include "DeskillzAuthController.h"
#include "Lobby/DeepLinkHandler.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Load config
    UDeskillzConfigAsset* Config = LoadObject<UDeskillzConfigAsset>(
        nullptr, TEXT("/Game/Config/DeskillzConfig.DeskillzConfig"));
    
    if (!Config || Config->GameId.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Missing Deskillz credentials!"));
        return;
    }
    
    // Initialize SDK
    FDeskillzConfig SDKConfig;
    SDKConfig.GameId = Config->GameId;
    SDKConfig.ApiKey = Config->ApiKey;
    SDKConfig.Environment = Config->bUseSandbox 
        ? EDeskillzEnvironment::Sandbox 
        : EDeskillzEnvironment::Production;
    SDKConfig.bSelfSufficientMode = Config->bSelfSufficientMode;
    SDKConfig.ApiBaseUrl = Config->ApiBaseUrl;
    
    UDeskillzSDK::Get()->Initialize(SDKConfig);
    UDeskillzSDK::Get()->OnInitialized.AddDynamic(this, &AMyGameMode::OnSDKReady);
    UDeskillzSDK::Get()->OnError.AddDynamic(this, &AMyGameMode::OnSDKError);
}

void AMyGameMode::OnSDKReady()
{
    UE_LOG(LogTemp, Log, TEXT("Deskillz SDK Ready!"));
    
    // Initialize authentication (NEW in v2.8)
    UDeskillzAuth::Get()->Initialize();
    
    // Bind auth events
    UDeskillzAuth::Get()->OnLoginSuccess.AddDynamic(this, &AMyGameMode::OnLoginSuccess);
    UDeskillzAuth::Get()->OnSignUpSuccess.AddDynamic(this, &AMyGameMode::OnSignUpSuccess);
    UDeskillzAuth::Get()->OnLogout.AddDynamic(this, &AMyGameMode::OnLogout);
    UDeskillzAuth::Get()->OnAuthError.AddDynamic(this, &AMyGameMode::OnAuthError);
    
    // Initialize scene controller
    UDeskillzAuthController::Get()->Initialize();
    
    // Check for existing session
    if (UDeskillzAuth::Get()->IsAuthenticated())
    {
        FAuthUser User = UDeskillzAuth::Get()->GetCurrentUser();
        UE_LOG(LogTemp, Log, TEXT("Session restored for: %s"), *User.Username);
        UDeskillzAuthController::Get()->GoToLobby();
    }
    else
    {
        UDeskillzAuthController::Get()->GoToAuth();
    }
    
    // Initialize deep link handler
    UDeepLinkHandler* DeepLinkHandler = UDeepLinkHandler::Get();
    DeepLinkHandler->Initialize();
    DeepLinkHandler->OnMatchReady.AddDynamic(this, &AMyGameMode::OnMatchReady);
    
    // Check for updates
    UDeskillzUpdater::Get()->CheckForUpdates();
    
    // Process pending deep links (cold start)
    if (DeepLinkHandler->HasPendingDeepLink())
    {
        DeepLinkHandler->ProcessPendingDeepLinks();
    }
}

void AMyGameMode::OnLoginSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("Login success: %s"), *User.Username);
    UDeskillzAuthController::Get()->GoToLobby();
}

void AMyGameMode::OnSignUpSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("Sign up success: %s"), *User.Username);
    UDeskillzAuthController::Get()->GoToLobby();
}

void AMyGameMode::OnLogout()
{
    UE_LOG(LogTemp, Log, TEXT("User logged out"));
    UDeskillzAuthController::Get()->GoToAuth();
}

void AMyGameMode::OnAuthError(const FAuthError& Error)
{
    UE_LOG(LogTemp, Error, TEXT("Auth error: %s - %s"), *Error.Code, *Error.Message);
}

void AMyGameMode::OnSDKError(const FDeskillzError& Error)
{
    UE_LOG(LogTemp, Error, TEXT("SDK Error: %s - %s"), *Error.Code, *Error.Message);
}

void AMyGameMode::OnMatchReady(const FMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Log, TEXT("Match ready: %s"), *Data.MatchId);
    // Store match data and load game level
    UDeskillzAuthController::Get()->GoToGame();
}

void AMyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UDeepLinkHandler* DeepLinkHandler = UDeepLinkHandler::Get();
    DeepLinkHandler->OnMatchReady.RemoveDynamic(this, &AMyGameMode::OnMatchReady);
    
    Super::EndPlay(EndPlayReason);
}
```

### 2. Handle Navigation Deep Links

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
            if (Parameters.Contains(TEXT("id")))
            {
                UIManager->ShowGameDetails(Parameters[TEXT("id")]);
            }
            break;
            
        case EDeskillzNavigationAction::Settings:
            UIManager->ShowSettings();
            break;
    }
}
```

### 3. Submit Score

```cpp
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
                ReturnToLobby();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Submission failed: %s"), *Result.Error);
                ReturnToLobby();
            }
        })
    );
}

FString AGameController::GenerateScoreHash(const FString& MatchId, int32 Score, 
    int32 Duration, int64 Timestamp, const FString& Nonce)
{
    // Format: matchId:score:duration:timestamp:nonce
    FString Data = FString::Printf(TEXT("%s:%d:%d:%lld:%s"), 
        *MatchId, Score, Duration, Timestamp, *Nonce);
    
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

void AGameController::ReturnToLobby()
{
    UDeskillzAuthController::Get()->GoToLobby();
}
```

---

## Self-Sufficient Authentication (NEW in v2.8)

**Your game can now be completely standalone!** Players can login, browse tournaments, and play matches entirely within your app - no external Deskillz app required.

### Architecture Options

| Mode | Description | Best For |
|------|-------------|----------|
| **Self-Sufficient** | All features built into your game | New games, better UX |
| **Centralized Lobby** | Deskillz app handles lobby, your game handles gameplay | Existing integrations |

### Self-Sufficient Setup

```cpp
// MyGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DeskillzSDK.h"
#include "DeskillzAuth.h"
#include "DeskillzAuthController.h"
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
    void OnLoginSuccess(const FAuthUser& User);
    
    UFUNCTION()
    void OnSignUpSuccess(const FAuthUser& User);
    
    UFUNCTION()
    void OnLogout();
    
    UFUNCTION()
    void OnAuthError(const FAuthError& Error);
    
    UFUNCTION()
    void OnWalletLinked(const FString& Address);
    
    UFUNCTION()
    void OnWalletDisconnected();
};

// MyGameMode.cpp
#include "MyGameMode.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Load config
    UDeskillzConfigAsset* Config = LoadObject<UDeskillzConfigAsset>(
        nullptr, TEXT("/Game/Config/DeskillzConfig.DeskillzConfig"));
    
    // Configure SDK for self-sufficient mode
    FDeskillzConfig SDKConfig;
    SDKConfig.GameId = Config->GameId;
    SDKConfig.ApiKey = Config->ApiKey;
    SDKConfig.bSelfSufficientMode = true;  // Enable self-sufficient
    SDKConfig.Environment = Config->bUseSandbox 
        ? EDeskillzEnvironment::Sandbox 
        : EDeskillzEnvironment::Production;
    SDKConfig.ApiBaseUrl = Config->ApiBaseUrl;
    
    UDeskillzSDK::Get()->Initialize(SDKConfig);
    UDeskillzSDK::Get()->OnInitialized.AddDynamic(this, &AMyGameMode::OnSDKReady);
}

void AMyGameMode::OnSDKReady()
{
    UE_LOG(LogTemp, Log, TEXT("Deskillz SDK Ready!"));
    
    // Initialize authentication
    UDeskillzAuth::Get()->Initialize();
    
    // Bind auth events
    UDeskillzAuth::Get()->OnLoginSuccess.AddDynamic(this, &AMyGameMode::OnLoginSuccess);
    UDeskillzAuth::Get()->OnSignUpSuccess.AddDynamic(this, &AMyGameMode::OnSignUpSuccess);
    UDeskillzAuth::Get()->OnLogout.AddDynamic(this, &AMyGameMode::OnLogout);
    UDeskillzAuth::Get()->OnAuthError.AddDynamic(this, &AMyGameMode::OnAuthError);
    UDeskillzAuth::Get()->OnWalletLinked.AddDynamic(this, &AMyGameMode::OnWalletLinked);
    UDeskillzAuth::Get()->OnWalletDisconnected.AddDynamic(this, &AMyGameMode::OnWalletDisconnected);
    
    // Initialize scene controller
    UDeskillzAuthController::Get()->Initialize();
    
    // Navigate based on auth state
    if (UDeskillzAuth::Get()->IsAuthenticated())
    {
        FAuthUser User = UDeskillzAuth::Get()->GetCurrentUser();
        UE_LOG(LogTemp, Log, TEXT("Welcome back, %s!"), *User.Username);
        UDeskillzAuthController::Get()->GoToLobby();
    }
    else
    {
        UDeskillzAuthController::Get()->GoToAuth();
    }
}

void AMyGameMode::OnLoginSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("Login success: %s"), *User.Username);
    UDeskillzAuthController::Get()->GoToLobby();
}

void AMyGameMode::OnSignUpSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("Account created: %s"), *User.Username);
    UDeskillzAuthController::Get()->GoToLobby();
}

void AMyGameMode::OnLogout()
{
    UE_LOG(LogTemp, Log, TEXT("User logged out"));
    UDeskillzAuthController::Get()->GoToAuth();
}

void AMyGameMode::OnAuthError(const FAuthError& Error)
{
    UE_LOG(LogTemp, Error, TEXT("Auth error: %s - %s"), *Error.Code, *Error.Message);
}

void AMyGameMode::OnWalletLinked(const FString& Address)
{
    UE_LOG(LogTemp, Log, TEXT("Wallet linked: %s"), *Address);
}

void AMyGameMode::OnWalletDisconnected()
{
    UE_LOG(LogTemp, Log, TEXT("Wallet disconnected"));
}
```

### Login with Email/Password

```cpp
// LoginWidget.cpp
void ULoginWidget::OnLoginButtonClicked()
{
    FString Email = EmailInput->GetText().ToString().TrimStartAndEnd();
    FString Password = PasswordInput->GetText().ToString();
    bool bRememberMe = RememberMeCheckbox->IsChecked();
    
    if (Email.IsEmpty() || Password.IsEmpty())
    {
        ShowError(TEXT("Please enter email and password"));
        return;
    }
    
    LoginButton->SetIsEnabled(false);
    UDeskillzAuth::Get()->Login(Email, Password, bRememberMe);
}

void ULoginWidget::OnSignUpButtonClicked()
{
    UDeskillzAuthController::Get()->ShowSignUp();
}

void ULoginWidget::OnForgotPasswordClicked()
{
    UDeskillzAuthController::Get()->ShowForgotPassword();
}
```

### Sign Up

```cpp
// SignUpWidget.cpp
void USignUpWidget::OnSignUpButtonClicked()
{
    FString Username = UsernameInput->GetText().ToString().TrimStartAndEnd();
    FString Email = EmailInput->GetText().ToString().TrimStartAndEnd();
    FString Password = PasswordInput->GetText().ToString();
    FString ConfirmPassword = ConfirmPasswordInput->GetText().ToString();
    
    // Validation
    if (Username.Len() < 3)
    {
        ShowError(TEXT("Username must be at least 3 characters"));
        return;
    }
    
    if (!Email.Contains(TEXT("@")))
    {
        ShowError(TEXT("Please enter a valid email"));
        return;
    }
    
    if (Password.Len() < 8)
    {
        ShowError(TEXT("Password must be at least 8 characters"));
        return;
    }
    
    if (Password != ConfirmPassword)
    {
        ShowError(TEXT("Passwords do not match"));
        return;
    }
    
    SignUpButton->SetIsEnabled(false);
    UDeskillzAuth::Get()->SignUp(Email, Password, Username);
}
```

### Social Login

```cpp
// SocialLoginWidget.cpp
void USocialLoginWidget::OnGoogleLoginClicked()
{
    UDeskillzAuth::Get()->SocialLogin(ESocialProvider::Google);
}

void USocialLoginWidget::OnAppleLoginClicked()
{
    // Apple Sign-In (iOS only)
    #if PLATFORM_IOS
    UDeskillzAuth::Get()->SocialLogin(ESocialProvider::Apple);
    #endif
}

void USocialLoginWidget::OnFacebookLoginClicked()
{
    UDeskillzAuth::Get()->SocialLogin(ESocialProvider::Facebook);
}
```

### Optional Wallet Connection

Wallet is optional in self-sufficient mode - only needed for paid tournaments.

```cpp
// ProfileWidget.cpp
void UProfileWidget::OnConnectWalletClicked()
{
    // Opens wallet connection flow (MetaMask, WalletConnect, etc.)
    UDeskillzAuth::Get()->ConnectWallet();
}

void UProfileWidget::OnDisconnectWalletClicked()
{
    UDeskillzAuth::Get()->DisconnectWallet();
}

void UProfileWidget::UpdateWalletUI()
{
    FAuthUser User = UDeskillzAuth::Get()->GetCurrentUser();
    
    if (!User.WalletAddress.IsEmpty())
    {
        // Show truncated address
        FString TruncatedAddress = User.WalletAddress.Left(6) + TEXT("...") + User.WalletAddress.Right(4);
        WalletAddressText->SetText(FText::FromString(TruncatedAddress));
        ConnectWalletButton->SetVisibility(ESlateVisibility::Collapsed);
        DisconnectWalletButton->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        ConnectWalletButton->SetVisibility(ESlateVisibility::Visible);
        DisconnectWalletButton->SetVisibility(ESlateVisibility::Collapsed);
    }
}
```

### Level Configuration

Configure your levels in DeskillzConfig:

```cpp
// DeskillzConfigAsset.h
UCLASS()
class UDeskillzConfigAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    // ... existing fields ...
    
    // Self-Sufficient Architecture (NEW in v2.8)
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    bool bSelfSufficientMode = true;
    
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    FString AuthLevelName = TEXT("/Game/Maps/AuthLevel");
    
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    FString LobbyLevelName = TEXT("/Game/Maps/LobbyLevel");
    
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    FString GameLevelName = TEXT("/Game/Maps/GameLevel");
    
    UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
    FString LoadingLevelName = TEXT(""); // Optional
};
```

---

## Features

| Feature | Description |
|---------|-------------|
| [KEY] **Self-Sufficient Auth** | Email/password and social login in-game (NEW v2.8) |
| [TROPHY] **Tournaments** | Async and real-time competitive matches |
| [COIN] **Crypto Prizes** | BTC, ETH, SOL, XRP, BNB, USDT, USDC |
| [USERS] **Private Rooms** | Play with friends using room codes |
| [HOST] **Host System** | 6-tier host program with revenue sharing |
| [CARDS] **Social Games** | Rake-based games with buy-ins |
| [EYE] **Spectator Mode** | Watch live games in progress |
| [LIGHTNING] **Real-time Sync** | Sub-100ms latency multiplayer |
| [SHIELD] **Anti-Cheat** | Score encryption and validation |
| [DOWNLOAD] **Auto-Updater** | Forced and optional app updates |
| [ROBOT] **NPC Opponents** | AI players for off-peak hours |
| [CHART] **Analytics** | Built-in telemetry and insights |
| [OFFLINE] **Offline Support** | Automatic score caching and retry |
| [LOCK] **Score Encryption** | HMAC-SHA256 signed submission |

---

## Auto-Updater

Keep your game up-to-date with automatic version checking:

```cpp
#include "Core/DeskillzUpdater.h"

void AMyGameMode::BeginPlay()
{
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    Updater->SetCurrentVersion(GetGameVersion(), GetVersionCode());
    
    Updater->OnUpdateAvailable.AddDynamic(this, &AMyGameMode::HandleOptionalUpdate);
    Updater->OnForceUpdateRequired.AddDynamic(this, &AMyGameMode::HandleForcedUpdate);
    Updater->OnNoUpdateNeeded.AddDynamic(this, &AMyGameMode::HandleNoUpdate);
    
    Updater->CheckForUpdates();
}

void AMyGameMode::HandleOptionalUpdate(const FUpdateInfo& Info)
{
    UE_LOG(LogTemp, Log, TEXT("Update available: %s"), *Info.LatestVersion);
    // Show optional update dialog
}

void AMyGameMode::HandleForcedUpdate(const FUpdateInfo& Info)
{
    // Block app until user updates
    UpdaterWidget->ShowForcedUpdateDialog(Info);
}

void AMyGameMode::HandleNoUpdate()
{
    UE_LOG(LogTemp, Log, TEXT("App is up to date!"));
}
```

---

## Private Rooms

Create and join private rooms for playing with friends:

```cpp
#include "Rooms/DeskillzRooms.h"

// Create a room
void AMyGameMode::CreateRoom()
{
    FCreateRoomConfig Config;
    Config.Name = TEXT("My Private Room");
    Config.MaxPlayers = 4;
    Config.GameId = UDeskillzSDK::Get()->GetGameId();
    Config.EntryFee = 0.0f; // Free room
    Config.bIsPrivate = true;
    
    UDeskillzRooms::Get()->CreateRoom(Config,
        FOnRoomCreated::CreateLambda([](const FPrivateRoom& Room) {
            UE_LOG(LogTemp, Log, TEXT("Room created: %s"), *Room.RoomCode);
        }),
        FOnRoomError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("Failed: %s"), *Error);
        })
    );
}

// Join a room
void AMyGameMode::JoinRoom(const FString& RoomCode)
{
    UDeskillzRooms::Get()->JoinRoom(RoomCode,
        FOnRoomJoined::CreateLambda([](const FPrivateRoom& Room) {
            UE_LOG(LogTemp, Log, TEXT("Joined room: %s"), *Room.Name);
        }),
        FOnRoomError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("Failed: %s"), *Error);
        })
    );
}
```

---

## Host System (NEW in v2.6)

Become a verified host to create and manage rooms with revenue sharing:

### Host Tiers

| Tier | Monthly Rake Required | Host Share |
|------|----------------------|------------|
| Bronze | $0 | 50% |
| Silver | $500 | 55% |
| Gold | $2,000 | 60% |
| Platinum | $5,000 | 65% |
| Diamond | $10,000 | 70% |
| Elite | $25,000 | 75% |

```cpp
#include "Host/DeskillzHostManager.h"

void AMyGameMode::RegisterAsHost()
{
    UDeskillzHostManager::Get()->RegisterAsHost(
        FOnHostRegistered::CreateLambda([](const FHostProfile& Profile) {
            UE_LOG(LogTemp, Log, TEXT("Registered as host! Tier: %s"), 
                *UEnum::GetValueAsString(Profile.Tier));
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("Registration failed: %s"), *Error);
        })
    );
}

void AMyGameMode::CreateHostedRoom()
{
    FCreateHostRoomConfig Config;
    Config.Name = TEXT("Pro Tournament Room");
    Config.GameCategory = EGameCategory::Esports;
    Config.EntryFee = 10.0f;
    Config.EntryCurrency = TEXT("USDT");
    Config.MaxPlayers = 8;
    Config.Mode = ERoomMode::Sync;
    Config.Visibility = ERoomVisibility::PublicListed;
    
    UDeskillzHostManager::Get()->CreateHostRoom(Config,
        FOnRoomCreated::CreateLambda([](const FPrivateRoom& Room) {
            UE_LOG(LogTemp, Log, TEXT("Host room created: %s"), *Room.RoomCode);
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}
```

---

## Social Games (NEW in v2.6)

Create rake-based social games (Big 2, Mahjong, Poker):

```cpp
#include "Social/DeskillzSocialGameManager.h"
#include "Social/DeskillzRakeCalculator.h"
#include "Social/DeskillzBuyInManager.h"

void AMyGameMode::StartSocialGame()
{
    // Initialize managers
    UDeskillzSocialGameManager::Get()->Initialize(RoomId);
    UDeskillzBuyInManager::Get()->Initialize(RoomId);
    
    // Subscribe to events
    UDeskillzSocialGameManager::Get()->OnRoundEnded.AddDynamic(
        this, &AMyGameMode::HandleRoundEnd);
    UDeskillzBuyInManager::Get()->OnRebuyRequired.AddDynamic(
        this, &AMyGameMode::HandleRebuyRequired);
}

void AMyGameMode::EndRound(const FString& WinnerId, float PotAmount)
{
    UDeskillzSocialGameManager::Get()->EndRound(WinnerId, PotAmount);
    
    // Preview rake distribution
    float Rake = UDeskillzRakeCalculator::CalculateRakeWithCap(
        PotAmount, RakePercentage, RakeCap);
    
    FRakeDistribution Distribution = UDeskillzRakeCalculator::PreviewRakeDistribution(
        Rake, HostTier);
    UE_LOG(LogTemp, Log, TEXT("Host gets: $%.2f"), Distribution.HostShare);
}

void AMyGameMode::HandleRoundEnd(const FRoundResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("Round %d won by %s"), 
        Result.RoundNumber, *Result.WinnerId);
    UE_LOG(LogTemp, Log, TEXT("Pot: $%.2f, Rake: $%.2f"), 
        Result.PotAmount, Result.RakeAmount);
}

void AMyGameMode::HandleRebuyRequired(const FString& PlayerId, float Timeout)
{
    // Show rebuy widget
    RebuyWidget->Show(PlayerId, Timeout);
}
```

---

## Spectator Mode (NEW in v2.6)

Allow spectators to watch live games:

```cpp
#include "Spectator/DeskillzSpectatorManager.h"

void AMySpectatorMode::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzSpectatorManager::Get()->Initialize();
    
    // Subscribe to events
    UDeskillzSpectatorManager::Get()->OnGameStateUpdated.AddDynamic(
        this, &AMySpectatorMode::HandleStateUpdate);
    UDeskillzSpectatorManager::Get()->OnRoundEnded.AddDynamic(
        this, &AMySpectatorMode::HandleRoundEnd);
    UDeskillzSpectatorManager::Get()->OnPlayerAction.AddDynamic(
        this, &AMySpectatorMode::HandleAction);
}

void AMySpectatorMode::FetchRooms()
{
    FSpectatorRoomFilter Filter;
    Filter.GameId = CurrentGameId;
    Filter.GameCategory = EGameCategory::Social;
    Filter.MinPlayers = 2;
    
    UDeskillzSpectatorManager::Get()->FetchSpectatorRooms(Filter,
        FOnSpectatorRoomsResult::CreateLambda([this](const TArray<FSpectatorRoom>& Rooms) {
            DisplayRoomList(Rooms);
        }),
        FOnSpectatorError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AMySpectatorMode::JoinAsSpectator(const FString& RoomId)
{
    UDeskillzSpectatorManager::Get()->JoinAsSpectator(RoomId,
        FOnJoinedSpectator::CreateLambda([this](const FSpectatorState& State) {
            UE_LOG(LogTemp, Log, TEXT("Spectating: %s"), *State.RoomName);
            UpdateUI(State);
        }),
        FOnSpectatorError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AMySpectatorMode::HandleStateUpdate(const FSpectatorState& State)
{
    UpdateGameBoard(State);
    UpdateScorePanel(State.Scores);
}

void AMySpectatorMode::HandleRoundEnd(const FSpectatorRoundResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("Round %d winner: %s"), 
        Result.RoundNumber, *Result.WinnerUsername);
    ShowRoundSummary(Result);
}

void AMySpectatorMode::HandleAction(const FSpectatorAction& Action)
{
    UE_LOG(LogTemp, Log, TEXT("%s: %s"), *Action.PlayerUsername, *Action.ActionType);
    AnimateAction(Action);
}
```

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
DeskillzSDK/
+-- Source/
|   +-- DeskillzSDK/
|   |   +-- Public/
|   |   |   +-- Core/
|   |   |   |   +-- DeskillzSDK.h
|   |   |   |   +-- DeskillzConfig.h
|   |   |   |   +-- DeskillzEvents.h
|   |   |   |   +-- DeskillzNetwork.h
|   |   |   |   +-- DeskillzUpdater.h
|   |   |   +-- Auth/                    # NEW in v2.8
|   |   |   |   +-- DeskillzAuth.h
|   |   |   |   +-- DeskillzAuthController.h
|   |   |   |   +-- AuthTypes.h
|   |   |   +-- Match/
|   |   |   |   +-- MatchController.h
|   |   |   |   +-- MatchTypes.h
|   |   |   +-- Security/
|   |   |   |   +-- ScoreEncryption.h
|   |   |   |   +-- AntiCheat.h
|   |   |   +-- Lobby/
|   |   |   |   +-- DeepLinkHandler.h
|   |   |   |   +-- NavigationTypes.h
|   |   |   +-- Rooms/
|   |   |   |   +-- DeskillzRooms.h
|   |   |   |   +-- RoomTypes.h
|   |   |   +-- Host/                    # NEW in v2.6
|   |   |   |   +-- DeskillzHostManager.h
|   |   |   |   +-- HostTypes.h
|   |   |   +-- Social/                  # NEW in v2.6
|   |   |   |   +-- DeskillzSocialGameManager.h
|   |   |   |   +-- DeskillzRakeCalculator.h
|   |   |   |   +-- DeskillzBuyInManager.h
|   |   |   +-- Spectator/               # NEW in v2.6
|   |   |   |   +-- DeskillzSpectatorManager.h
|   |   |   |   +-- SpectatorTypes.h
|   |   |   +-- Widgets/
|   |   |       +-- Rooms/
|   |   |       +-- Host/
|   |   |       +-- Social/
|   |   |       +-- Spectator/
|   |   +-- Private/
|   |       +-- (Implementation files)
+-- Resources/
+-- DeskillzSDK.uplugin
```

---

## Platform Setup

### iOS Configuration

Add to `Info.plist`:
```xml
<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLSchemes</key>
        <array>
            <string>deskillz-yourgame</string>
        </array>
        <key>CFBundleURLName</key>
        <string>com.yourstudio.yourgame</string>
    </dict>
</array>
```

### Android Configuration

Add to `AndroidManifest.xml`:
```xml
<activity android:name="com.epicgames.unreal.GameActivity">
    <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:scheme="deskillz-yourgame" />
    </intent-filter>
</activity>
```

---

## Test Mode

Test your integration without real currency:

```cpp
// Test SDK initialization
UDeskillzSDK::Get()->EnableTestMode(true);

// Test navigation deep links
UDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://tournaments"));
UDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://wallet"));

// Test match launch
UDeepLinkHandler::Get()->SimulateDeepLink(TEXT("deskillz://launch?matchId=test&token=test"));

// Test room UI
UDeskillzPrivateRoomUI::Get()->ShowRoomList();
UDeskillzPrivateRoomUI::Get()->ShowCreateRoom();

// Test auto-updater
UDeskillzUpdater::Get()->CheckForUpdates();

// Test host system (NEW in v2.6)
UDeskillzHostManager::Get()->Initialize(TEXT("test-host-id"));

// Test social games (NEW in v2.6)
UDeskillzSocialGameManager::Get()->StartTestSession();

// Test spectator mode (NEW in v2.6)
UDeskillzSpectatorManager::Get()->FetchSpectatorRooms(FSpectatorRoomFilter());

// Test auth (NEW in v2.8)
UDeskillzAuth::Get()->Initialize();
UDeskillzAuthController::Get()->GoToAuth();
```

---

## Documentation

- [Quick Start Guide](https://docs.deskillz.games/unreal/quickstart)
- [API Reference](https://docs.deskillz.games/unreal/api)
- [Multiplayer Guide](https://docs.deskillz.games/unreal/multiplayer)
- [Deep Link Integration](https://docs.deskillz.games/unreal/deep-links)
- [Private Rooms Guide](https://docs.deskillz.games/unreal/private-rooms)
- [Host System Guide](https://docs.deskillz.games/unreal/host-system)
- [Social Games Guide](https://docs.deskillz.games/unreal/social-games)
- [Spectator Mode Guide](https://docs.deskillz.games/unreal/spectator)
- [Auto-Updater Guide](https://docs.deskillz.games/unreal/updater)
- [Custom UI Guide](https://docs.deskillz.games/unreal/custom-ui)
- [Troubleshooting](https://docs.deskillz.games/unreal/troubleshooting)

## Sample Project

Check out our sample game implementation:
[Deskillz Unreal Sample](https://github.com/Deskillz-Games-Development/unreal-sample)

---

## Changelog

See [CHANGELOG.md](./CHANGELOG.md) for version history.

### v2.8.0 (January 2026)
- **NEW:** Self-Sufficient Authentication (`UDeskillzAuth`)
- **NEW:** Scene Flow Controller (`UDeskillzAuthController`)
- **NEW:** Email/password login and registration
- **NEW:** Social login (Google, Apple, Facebook)
- **NEW:** Optional wallet connection (for paid tournaments)
- **NEW:** Auth events (OnLoginSuccess, OnSignUpSuccess, OnLogout, OnAuthError)
- **NEW:** Wallet events (OnWalletLinked, OnWalletDisconnected)
- **NEW:** Level configuration in DeskillzConfig
- **NEW:** Token persistence and auto-refresh
- Games can now be completely standalone - no external app required

### v2.7.0 (January 2026)
- **NEW:** Self-Sufficient Architecture foundation
- **NEW:** In-game lobby support
- **NEW:** Level navigation system
- Architecture options: Self-Sufficient vs Centralized Lobby

### v2.6.0 (January 2025)
- **NEW:** Host System with 6-tier progression
- **NEW:** UDeskillzHostManager for host registration and management
- **NEW:** Host Dashboard UI widgets (5 files)
- **NEW:** Social Game Manager for rake-based games
- **NEW:** UDeskillzRakeCalculator with tiered rake structure
- **NEW:** UDeskillzBuyInManager for buy-in/rebuy/cashout flows
- **NEW:** Social Game UI widgets (6 files)
- **NEW:** UDeskillzSpectatorManager for live game viewing
- **NEW:** Spectator UI widgets (3 files)
- **NEW:** 44 total new files for Private Room Enhancement
- Revenue sharing system (50%-75% based on tier)
- Real-time WebSocket updates for spectators
- Pause/resume functionality for social games

### v2.5.1 (January 2025)
- Fixed duplicate class definitions
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
- APK hosting integration with Cloudflare R2

### v2.2.0 (December 2024)
- **NEW:** Private Rooms API (`UDeskillzRooms`)
- **NEW:** Pre-built Room Widgets (6 components)
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

---

## Troubleshooting

### Deep links not working
1. Verify URL schemes are configured correctly in platform settings
2. Check app is properly signed
3. Test with: `adb shell am start -a android.intent.action.VIEW -d "deskillz://tournaments"`
4. Enable logging to see incoming deep links

### Navigation events not firing
1. Ensure `UDeepLinkHandler::Get()->Initialize()` is called first
2. Verify event bindings before processing
3. Check `HasPendingDeepLink()` and call `ProcessPendingDeepLinks()`
4. Test with `SimulateDeepLink()` first

### Room UI not showing
1. Ensure `UDeskillzRooms::Get()->Initialize()` is called
2. Check that widgets are properly created
3. Verify WebSocket connection is established
4. Test with `UDeskillzPrivateRoomUI::Get()->ShowRoomList()`

### Auto-updater not checking
1. Verify `SetCurrentVersion()` is called with correct values
2. Check network connectivity
3. Ensure Game ID is configured in DeskillzConfig
4. Enable logging to see API responses

### Self-Sufficient Auth not working
1. Verify `bSelfSufficientMode = true` in DeskillzConfig
2. Check API base URL is correct
3. Ensure `UDeskillzAuth::Get()->Initialize()` is called
4. Bind to `OnAuthError` to see error details
5. Verify email/password meet requirements (8+ chars)

### Host system not initializing
1. Ensure user is authenticated first
2. Call `UDeskillzHostManager::Get()->Initialize(UserId)`
3. Check for registration errors in callbacks
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
if (!UDeskillzSDK::Get()->IsInitialized())
{
    // Verify credentials in DeskillzConfig
    // Check network connectivity
    // Enable logging for details
    UE_LOG(LogTemp, Error, TEXT("SDK failed to initialize!"));
}
```

### iOS build errors
- Ensure Xcode 14+ is installed
- Check iOS deployment target is 12.0+
- Verify signing certificates

### Android build errors
- Check Min SDK is 21+
- Verify NDK is properly configured
- Check for duplicate AndroidManifest entries

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 2.8.0 | Jan 2026 | Self-Sufficient Authentication (UDeskillzAuth, UDeskillzAuthController), email/password login, social login, optional wallet |
| 2.7.0 | Jan 2026 | Self-Sufficient Architecture introduction |
| 2.6.0 | Jan 2026 | Host System, Social Games, Spectator Mode |
| 2.5.0 | Jan 2025 | Auto-Updater (UDeskillzUpdater) |
| 2.2.0 | Dec 2024 | Private Rooms (UDeskillzRooms, Widgets) |
| 2.1.0 | Dec 2024 | Navigation deep links |
| 2.0.0 | Nov 2024 | Centralized lobby architecture |
| 1.x | Legacy | SDK-based matchmaking (deprecated) |

---

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