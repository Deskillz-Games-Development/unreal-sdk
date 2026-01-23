# Deskillz SDK - Quick Start Guide

**SDK Version: 2.8.0** | Get your game integrated with Deskillz in minutes.

## Table of Contents

0. [Get Your Game Credentials](#0-get-your-game-credentials) - Start Here
1. [Prerequisites](#1-prerequisites)
2. [Installation](#2-installation)
3. [Basic Setup](#3-basic-setup)
4. [Self-Sufficient Authentication](#4-self-sufficient-authentication) - NEW in v2.7!
5. [Scene Flow Controller](#5-scene-flow-controller) - NEW in v2.7!
6. [Handle Match Launch](#6-handle-match-launch)
7. [Submit Score](#7-submit-score)
8. [Return to Lobby](#8-return-to-lobby)
9. [Enable Auto-Updates](#9-enable-auto-updates)
10. [Host Registration Quick Start](#10-host-registration-quick-start)
11. [Social Game Quick Start](#11-social-game-quick-start)
12. [Host Spectator Mode Quick Start](#12-host-spectator-mode-quick-start)
13. [Test Your Integration](#13-test-your-integration)

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
    [Header("Credentials")]
    public string GameId = "YOUR_GAME_ID";
    public string ApiKey = "YOUR_API_KEY";
    public string ApiSecret = "YOUR_API_SECRET"; // Keep secure!
    public string DeepLinkScheme = "deskillz-yourgame";
    public bool UseSandbox = true;
    
    [Header("Scene Configuration (Self-Sufficient Architecture)")]
    public string AuthSceneName = "DeskillzAuth";
    public string LobbySceneName = "DeskillzLobby";
    public string GameSceneName = "Game";
    public string LoadingSceneName = "Loading";
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
    UPROPERTY(EditAnywhere, Category = "Credentials")
    FString GameId = TEXT("YOUR_GAME_ID");
    
    UPROPERTY(EditAnywhere, Category = "Credentials")
    FString ApiKey = TEXT("YOUR_API_KEY");
    
    UPROPERTY(EditAnywhere, Category = "Credentials")
    FString ApiSecret = TEXT("YOUR_API_SECRET"); // Keep secure!
    
    UPROPERTY(EditAnywhere, Category = "Credentials")
    FString DeepLinkScheme = TEXT("deskillz-yourgame");
    
    UPROPERTY(EditAnywhere, Category = "Credentials")
    bool bUseSandbox = true;
    
    // Scene Configuration (Self-Sufficient Architecture)
    UPROPERTY(EditAnywhere, Category = "Levels")
    FString AuthLevelName = TEXT("/Game/Deskillz/Maps/DeskillzAuth");
    
    UPROPERTY(EditAnywhere, Category = "Levels")
    FString LobbyLevelName = TEXT("/Game/Deskillz/Maps/DeskillzLobby");
    
    UPROPERTY(EditAnywhere, Category = "Levels")
    FString GameLevelName = TEXT("/Game/Maps/Game");
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
1. Download `DeskillzSDK-v2.8.0.unitypackage` from the Developer Portal
2. In Unity: Assets > Import Package > Custom Package
3. Select the downloaded package and import all files

**Option B: Package Manager**
Add to your `manifest.json`:
```json
{
  "dependencies": {
    "com.deskillz.sdk": "https://github.com/Deskillz-Games-Development/unity-sdk.git#v2.8.0"
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
        
        // Initialize authentication (NEW in v2.7)
        InitializeAuth();
        
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
    
    void InitializeAuth()
    {
        // Initialize the self-sufficient auth system
        DeskillzAuth.Instance.Initialize();
        
        // Subscribe to auth events
        DeskillzEvents.OnAuthLoginSuccess += HandleLoginSuccess;
        DeskillzEvents.OnAuthLogout += HandleLogout;
        DeskillzEvents.OnAuthError += HandleAuthError;
    }
    
    void HandleLoginSuccess(AuthUser user)
    {
        Debug.Log($"Login success: {user.Username}");
    }
    
    void HandleLogout()
    {
        Debug.Log("User logged out");
    }
    
    void HandleAuthError(string error)
    {
        Debug.LogError($"Auth error: {error}");
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
#include "DeskillzAuth.h"
#include "DeskillzAuthController.h"
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
    
    UFUNCTION()
    void OnLoginSuccess(const FAuthUser& User);
    
    UFUNCTION()
    void OnLogout();
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
    
    // Initialize SDK
    UDeskillzSDK::Get()->Initialize(SDKConfig);
    UDeskillzSDK::Get()->OnInitialized.AddDynamic(this, &AMyGameMode::OnSDKReady);
    UDeskillzSDK::Get()->OnError.AddDynamic(this, &AMyGameMode::OnSDKError);
}

void AMyGameMode::OnSDKReady()
{
    UE_LOG(LogTemp, Log, TEXT("Deskillz SDK Ready!"));
    
    // Initialize authentication (NEW in v2.7)
    UDeskillzAuth::Get()->Initialize();
    UDeskillzAuth::Get()->OnLoginSuccess.AddDynamic(this, &AMyGameMode::OnLoginSuccess);
    UDeskillzAuth::Get()->OnLogout.AddDynamic(this, &AMyGameMode::OnLogout);
    
    // Initialize auth controller for level navigation
    UDeskillzAuthController::Get(this)->Initialize();
    
    // Initialize deep link handler
    UDeskillzDeepLinkHandler::Get()->Initialize();
    UDeskillzDeepLinkHandler::Get()->OnMatchReady.AddDynamic(this, &AMyGameMode::OnMatchReady);
    
    // Check for pending launch
    if (UDeskillzDeepLinkHandler::Get()->HasPendingLaunch())
    {
        UDeskillzDeepLinkHandler::Get()->ProcessPendingLaunch();
    }
    
    // Check for updates
    UDeskillzUpdater::Get()->CheckForUpdates();
}

void AMyGameMode::OnLoginSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("Login success: %s"), *User.Username);
}

void AMyGameMode::OnLogout()
{
    UE_LOG(LogTemp, Log, TEXT("User logged out"));
}

void AMyGameMode::OnSDKError(const FDeskillzError& Error)
{
    UE_LOG(LogTemp, Error, TEXT("SDK Error: %s - %s"), *Error.Code, *Error.Message);
}

void AMyGameMode::OnMatchReady(const FMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Log, TEXT("Match ready: %s"), *Data.MatchId);
    // Store match data and open game level
    UGameplayStatics::OpenLevel(this, TEXT("/Game/Maps/GameLevel"));
}
```

---

## 4. Self-Sufficient Authentication

**NEW in v2.7!** Your game can now handle authentication entirely within the app - no external dependencies required.

### Architecture Overview

```
+-------------------------------------------------------+
|              YOUR STANDALONE GAME APP                 |
|                                                       |
|  [x] Built-in Login/SignUp (Email + Social)          |
|  [x] Built-in Tournament Lobby                        |
|  [x] Built-in Private Rooms                           |
|  [x] Built-in Player Profile                          |
|  [x] Built-in Wallet (optional)                       |
|  [x] NO external app dependency                       |
|                                                       |
|  User never leaves your app!                          |
+-------------------------------------------------------+
```

### Key Benefits

| Feature | Before (v2.6) | After (v2.7) |
|---------|---------------|--------------|
| Authentication | Wallet-first (SIWE) | Email/Password primary |
| Login Location | External main app | In-game AuthScene |
| Tournament Lobby | External main app | In-game LobbyScene |
| Match Launch | Deep link from external | In-app scene transition |
| Wallet | Required to start | Optional (in Profile) |

### Unity: DeskillzAuth Quick Start

```csharp
using Deskillz;

public class AuthQuickStart : MonoBehaviour
{
    void Start()
    {
        // Initialize auth system
        DeskillzAuth.Instance.Initialize();
        
        // Subscribe to events
        DeskillzAuth.OnLoginSuccess += OnLoginSuccess;
        DeskillzAuth.OnSignUpSuccess += OnSignUpSuccess;
        DeskillzAuth.OnLogout += OnLogout;
        DeskillzAuth.OnAuthError += OnAuthError;
        DeskillzAuth.OnAuthStateChanged += OnAuthStateChanged;
        
        // Check if already authenticated (session restore)
        if (DeskillzAuth.Instance.IsAuthenticated)
        {
            Debug.Log($"Already logged in as: {DeskillzAuth.Instance.CurrentUser.Username}");
        }
    }
    
    // Email/Password Login
    public async void Login(string email, string password, bool rememberMe = true)
    {
        try
        {
            AuthUser user = await DeskillzAuth.Instance.Login(email, password, rememberMe);
            Debug.Log($"Login successful: {user.Username}");
        }
        catch (AuthException ex)
        {
            Debug.LogError($"Login failed: {ex.Message}");
        }
    }
    
    // Email/Password Sign Up
    public async void SignUp(string email, string password, string username)
    {
        try
        {
            AuthUser user = await DeskillzAuth.Instance.SignUp(email, password, username);
            Debug.Log($"SignUp successful: {user.Username}");
        }
        catch (AuthException ex)
        {
            Debug.LogError($"SignUp failed: {ex.Message}");
        }
    }
    
    // Social Login (Google, Apple, Facebook)
    public async void SocialLogin(SocialProvider provider, string idToken)
    {
        try
        {
            AuthUser user = await DeskillzAuth.Instance.SocialLogin(provider, idToken);
            Debug.Log($"Social login successful: {user.Username}");
        }
        catch (AuthException ex)
        {
            Debug.LogError($"Social login failed: {ex.Message}");
        }
    }
    
    // Logout
    public void Logout()
    {
        DeskillzAuth.Instance.Logout();
    }
    
    // Optional: Link Wallet (for paid tournaments)
    public async void LinkWallet(string address, string signature, string message, string nonce)
    {
        try
        {
            await DeskillzAuth.Instance.LinkWallet(address, signature, message, nonce);
            Debug.Log("Wallet linked successfully");
        }
        catch (AuthException ex)
        {
            Debug.LogError($"Wallet link failed: {ex.Message}");
        }
    }
    
    // Optional: Disconnect Wallet
    public async void DisconnectWallet()
    {
        try
        {
            await DeskillzAuth.Instance.DisconnectWallet();
            Debug.Log("Wallet disconnected");
        }
        catch (AuthException ex)
        {
            Debug.LogError($"Wallet disconnect failed: {ex.Message}");
        }
    }
    
    // Forgot Password
    public async void ForgotPassword(string email)
    {
        try
        {
            await DeskillzAuth.Instance.ForgotPassword(email);
            Debug.Log("Password reset email sent");
        }
        catch (AuthException ex)
        {
            Debug.LogError($"Forgot password failed: {ex.Message}");
        }
    }
    
    // Event Handlers
    void OnLoginSuccess(AuthUser user)
    {
        Debug.Log($"Logged in: {user.Username} ({user.Email})");
        // Navigate to lobby
        AuthSceneController.Instance.GoToLobby();
    }
    
    void OnSignUpSuccess(AuthUser user)
    {
        Debug.Log($"New user created: {user.Username}");
        // Navigate to lobby
        AuthSceneController.Instance.GoToLobby();
    }
    
    void OnLogout()
    {
        Debug.Log("User logged out");
        // Navigate to auth screen
        AuthSceneController.Instance.GoToAuth();
    }
    
    void OnAuthError(string error)
    {
        Debug.LogError($"Auth error: {error}");
    }
    
    void OnAuthStateChanged(AuthState state)
    {
        Debug.Log($"Auth state: {state}");
        // AuthState: NotAuthenticated, Authenticating, Authenticated, Error
    }
}
```

### Unreal: DeskillzAuth Quick Start

```cpp
// AuthQuickStart.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeskillzAuth.h"
#include "DeskillzAuthController.h"
#include "AuthQuickStart.generated.h"

UCLASS()
class AAuthQuickStart : public AActor
{
    GENERATED_BODY()
    
public:
    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable)
    void Login(const FString& Email, const FString& Password, bool bRememberMe = true);
    
    UFUNCTION(BlueprintCallable)
    void SignUp(const FString& Email, const FString& Password, const FString& Username);
    
    UFUNCTION(BlueprintCallable)
    void SocialLogin(ESocialProvider Provider, const FString& IdToken);
    
    UFUNCTION(BlueprintCallable)
    void Logout();
    
    UFUNCTION(BlueprintCallable)
    void LinkWallet(const FString& Address, const FString& Signature, 
                    const FString& Message, const FString& Nonce);
    
    UFUNCTION(BlueprintCallable)
    void ForgotPassword(const FString& Email);
    
private:
    UFUNCTION()
    void OnLoginSuccess(const FAuthUser& User);
    
    UFUNCTION()
    void OnSignUpSuccess(const FAuthUser& User);
    
    UFUNCTION()
    void OnLogout();
    
    UFUNCTION()
    void OnAuthError(const FString& Error);
};

// AuthQuickStart.cpp
#include "AuthQuickStart.h"

void AAuthQuickStart::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize auth system
    UDeskillzAuth::Get()->Initialize();
    
    // Bind events
    UDeskillzAuth::Get()->OnLoginSuccess.AddDynamic(this, &AAuthQuickStart::OnLoginSuccess);
    UDeskillzAuth::Get()->OnSignUpSuccess.AddDynamic(this, &AAuthQuickStart::OnSignUpSuccess);
    UDeskillzAuth::Get()->OnLogout.AddDynamic(this, &AAuthQuickStart::OnLogout);
    UDeskillzAuth::Get()->OnAuthError.AddDynamic(this, &AAuthQuickStart::OnAuthError);
    
    // Check if already authenticated
    if (UDeskillzAuth::Get()->IsAuthenticated())
    {
        FAuthUser User = UDeskillzAuth::Get()->GetCurrentUser();
        UE_LOG(LogTemp, Log, TEXT("Already logged in as: %s"), *User.Username);
    }
}

void AAuthQuickStart::Login(const FString& Email, const FString& Password, bool bRememberMe)
{
    UDeskillzAuth::Get()->Login(Email, Password, bRememberMe);
}

void AAuthQuickStart::SignUp(const FString& Email, const FString& Password, const FString& Username)
{
    UDeskillzAuth::Get()->SignUp(Email, Password, Username);
}

void AAuthQuickStart::SocialLogin(ESocialProvider Provider, const FString& IdToken)
{
    UDeskillzAuth::Get()->SocialLogin(Provider, IdToken);
}

void AAuthQuickStart::Logout()
{
    UDeskillzAuth::Get()->Logout();
}

void AAuthQuickStart::LinkWallet(const FString& Address, const FString& Signature,
                                  const FString& Message, const FString& Nonce)
{
    UDeskillzAuth::Get()->LinkWallet(Address, Signature, Message, Nonce);
}

void AAuthQuickStart::ForgotPassword(const FString& Email)
{
    UDeskillzAuth::Get()->ForgotPassword(Email);
}

void AAuthQuickStart::OnLoginSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("Logged in: %s"), *User.Username);
    // Navigate to lobby
    UDeskillzAuthController::Get(this)->GoToLobby();
}

void AAuthQuickStart::OnSignUpSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("New user: %s"), *User.Username);
    // Navigate to lobby
    UDeskillzAuthController::Get(this)->GoToLobby();
}

void AAuthQuickStart::OnLogout()
{
    UE_LOG(LogTemp, Log, TEXT("User logged out"));
    // Navigate to auth level
    UDeskillzAuthController::Get(this)->GoToAuth();
}

void AAuthQuickStart::OnAuthError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("Auth error: %s"), *Error);
}
```

### Auth State Enum

```csharp
// Unity
public enum AuthState
{
    NotAuthenticated,  // No user logged in
    Authenticating,    // Login/SignUp in progress
    Authenticated,     // User logged in
    Error              // Authentication error occurred
}

// Unreal
UENUM(BlueprintType)
enum class EAuthState : uint8
{
    NotAuthenticated,
    Authenticating,
    Authenticated,
    Error
};
```

### AuthUser Model

```csharp
// Unity
public class AuthUser
{
    public string Id;
    public string Username;
    public string Email;
    public string AvatarUrl;
    public bool EmailVerified;
    public string WalletAddress;  // null if not linked
    public bool HasWallet => !string.IsNullOrEmpty(WalletAddress);
    public DateTime CreatedAt;
}

// Unreal
USTRUCT(BlueprintType)
struct FAuthUser
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly)
    FString Id;
    
    UPROPERTY(BlueprintReadOnly)
    FString Username;
    
    UPROPERTY(BlueprintReadOnly)
    FString Email;
    
    UPROPERTY(BlueprintReadOnly)
    FString AvatarUrl;
    
    UPROPERTY(BlueprintReadOnly)
    bool bEmailVerified;
    
    UPROPERTY(BlueprintReadOnly)
    FString WalletAddress;
    
    bool HasWallet() const { return !WalletAddress.IsEmpty(); }
};
```

---

## 5. Scene Flow Controller

**NEW in v2.7!** Automatically manage navigation between Auth, Lobby, and Game scenes.

### Scene Flow Diagram

```
App Launch
    |
    v
+-------------------+
|   AuthScene       |  <-- Login/SignUp screens
|  (Login/SignUp)   |
+--------+----------+
         | Login Success
         v
+-------------------+
|   LobbyScene      |  <-- Tournaments, Private Rooms, Profile
|  (Main Lobby)     |
+--------+----------+
         | Join Tournament / Start Match
         v
+-------------------+
|   GameScene       |  <-- Your gameplay
|  (Gameplay)       |
+--------+----------+
         | Match Complete
         v
+-------------------+
|  Return to Lobby  |  <-- Show results, return to lobby
+-------------------+
```

### Unity: AuthSceneController

```csharp
using Deskillz;
using UnityEngine.SceneManagement;

public class SceneFlowExample : MonoBehaviour
{
    void Start()
    {
        // Initialize the scene controller
        AuthSceneController.Instance.Initialize();
        
        // Subscribe to events
        AuthSceneController.OnSceneChanging += OnSceneChanging;
        AuthSceneController.OnSceneChanged += OnSceneChanged;
        AuthSceneController.OnAuthFlowComplete += OnAuthFlowComplete;
        AuthSceneController.OnLogoutComplete += OnLogoutComplete;
        
        // Determine where to navigate based on auth state
        AuthSceneController.Instance.DetermineInitialNavigation();
    }
    
    // Navigation Methods
    public void GoToAuth()
    {
        AuthSceneController.Instance.GoToAuth();
    }
    
    public void GoToLobby()
    {
        // Only works if authenticated
        AuthSceneController.Instance.GoToLobby();
    }
    
    public void GoToGame()
    {
        AuthSceneController.Instance.GoToGame();
    }
    
    public void LaunchMatch(MatchLaunchData matchData)
    {
        // If not authenticated, stores as pending match and goes to auth first
        AuthSceneController.Instance.LaunchMatch(matchData);
    }
    
    public void ReturnToLobby()
    {
        // Call after match completion
        AuthSceneController.Instance.ReturnToLobby();
    }
    
    public void LogoutAndGoToAuth()
    {
        AuthSceneController.Instance.LogoutAndGoToAuth();
    }
    
    // State Properties
    void CheckState()
    {
        bool isTransitioning = AuthSceneController.Instance.IsTransitioning;
        bool hasPendingMatch = AuthSceneController.Instance.HasPendingMatch;
        string currentScene = AuthSceneController.Instance.CurrentScene;
        bool isAuthenticated = AuthSceneController.Instance.IsAuthenticated;
    }
    
    // Event Handlers
    void OnSceneChanging(string sceneName)
    {
        Debug.Log($"Navigating to: {sceneName}");
        // Show loading UI
    }
    
    void OnSceneChanged(string sceneName)
    {
        Debug.Log($"Arrived at: {sceneName}");
        // Hide loading UI
    }
    
    void OnAuthFlowComplete()
    {
        Debug.Log("Auth complete - user is now in lobby");
    }
    
    void OnLogoutComplete()
    {
        Debug.Log("Logout complete - user is now at auth screen");
    }
}
```

### Unreal: DeskillzAuthController

```cpp
// SceneFlowExample.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeskillzAuthController.h"
#include "SceneFlowExample.generated.h"

UCLASS()
class ASceneFlowExample : public AActor
{
    GENERATED_BODY()
    
public:
    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable)
    void GoToAuth();
    
    UFUNCTION(BlueprintCallable)
    void GoToLobby();
    
    UFUNCTION(BlueprintCallable)
    void GoToGame();
    
    UFUNCTION(BlueprintCallable)
    void LaunchMatch(const FDeskillzMatchLaunchData& MatchData);
    
    UFUNCTION(BlueprintCallable)
    void ReturnToLobby();
    
private:
    UFUNCTION()
    void OnLevelChanging(const FString& LevelName);
    
    UFUNCTION()
    void OnLevelChanged(const FString& LevelName);
    
    UFUNCTION()
    void OnAuthFlowComplete();
};

// SceneFlowExample.cpp
#include "SceneFlowExample.h"

void ASceneFlowExample::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the auth controller
    UDeskillzAuthController* Controller = UDeskillzAuthController::Get(this);
    Controller->Initialize();
    
    // Bind events
    Controller->OnLevelChanging.AddDynamic(this, &ASceneFlowExample::OnLevelChanging);
    Controller->OnLevelChanged.AddDynamic(this, &ASceneFlowExample::OnLevelChanged);
    Controller->OnAuthFlowComplete.AddDynamic(this, &ASceneFlowExample::OnAuthFlowComplete);
    
    // Determine initial navigation
    Controller->DetermineInitialNavigation();
}

void ASceneFlowExample::GoToAuth()
{
    UDeskillzAuthController::Get(this)->GoToAuth();
}

void ASceneFlowExample::GoToLobby()
{
    UDeskillzAuthController::Get(this)->GoToLobby();
}

void ASceneFlowExample::GoToGame()
{
    UDeskillzAuthController::Get(this)->GoToGame();
}

void ASceneFlowExample::LaunchMatch(const FDeskillzMatchLaunchData& MatchData)
{
    UDeskillzAuthController::Get(this)->LaunchMatch(MatchData);
}

void ASceneFlowExample::ReturnToLobby()
{
    UDeskillzAuthController::Get(this)->ReturnToLobby();
}

void ASceneFlowExample::OnLevelChanging(const FString& LevelName)
{
    UE_LOG(LogTemp, Log, TEXT("Navigating to: %s"), *LevelName);
}

void ASceneFlowExample::OnLevelChanged(const FString& LevelName)
{
    UE_LOG(LogTemp, Log, TEXT("Arrived at: %s"), *LevelName);
}

void ASceneFlowExample::OnAuthFlowComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Auth complete - user in lobby"));
}
```

### Scene Setup (Unity)

**Build Settings - Scene Order:**
```
0: Scenes/DeskillzAuth      <-- First scene (entry point)
1: Scenes/DeskillzLobby
2: Scenes/Game
3: Scenes/Loading (optional)
```

**DeskillzAuth Scene Hierarchy:**
```
DeskillzAuth
+-- Managers
|   +-- DeskillzManager (with DeskillzManager.cs)
|   +-- DeskillzAuth (with DeskillzAuth.cs)
|   +-- AuthSceneController (with AuthSceneController.cs)
+-- Canvas-Auth
|   +-- LoginPanel
|   |   +-- EmailInput
|   |   +-- PasswordInput
|   |   +-- LoginButton
|   |   +-- ForgotPasswordButton
|   |   +-- SwitchToSignUpButton
|   +-- SignUpPanel
|   |   +-- UsernameInput
|   |   +-- EmailInput
|   |   +-- PasswordInput
|   |   +-- ConfirmPasswordInput
|   |   +-- SignUpButton
|   |   +-- SwitchToLoginButton
|   +-- SocialAuthButtons
|   +-- LoadingOverlay
+-- Canvas-Modals
|   +-- ErrorModal
|   +-- ForgotPasswordModal
+-- EventSystem
```

---

## 6. Handle Match Launch

When a match is ready (either from deep link or in-app tournament join):

### Unity

```csharp
using Deskillz;
using Deskillz.Lobby;

public class MatchHandler : MonoBehaviour
{
    void Start()
    {
        DeepLinkHandler.Instance.Initialize();
        DeepLinkHandler.Instance.OnMatchReady += StartMatch;
        DeepLinkHandler.Instance.OnValidationFailed += HandleValidationFailed;
        
        // Check for pending launch (app started from deep link)
        if (DeepLinkHandler.Instance.HasPendingLaunch())
        {
            DeepLinkHandler.Instance.ProcessPendingLaunch();
        }
    }
    
    void StartMatch(MatchLaunchData data)
    {
        // Store match data
        GameSession.MatchId = data.MatchId;
        GameSession.EntryFee = data.EntryFee;
        GameSession.PrizePool = data.PrizePool;
        GameSession.Opponent = data.OpponentInfo;
        GameSession.TournamentType = data.TournamentType;
        GameSession.TimeLimit = data.TimeLimit;
        GameSession.RandomSeed = data.RandomSeed;
        
        // Load game scene
        AuthSceneController.Instance.GoToGame();
    }
    
    void HandleValidationFailed(string reason, MatchLaunchData data)
    {
        Debug.LogError($"Match validation failed: {reason}");
        // Show error UI and return to lobby
        AuthSceneController.Instance.ReturnToLobby();
    }
}
```

### Unreal

```cpp
void AMatchHandler::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzDeepLinkHandler::Get()->Initialize();
    UDeskillzDeepLinkHandler::Get()->OnMatchReady.AddDynamic(this, &AMatchHandler::StartMatch);
    UDeskillzDeepLinkHandler::Get()->OnValidationFailed.AddDynamic(this, &AMatchHandler::HandleFailed);
    
    if (UDeskillzDeepLinkHandler::Get()->HasPendingLaunch())
    {
        UDeskillzDeepLinkHandler::Get()->ProcessPendingLaunch();
    }
}

void AMatchHandler::StartMatch(const FMatchLaunchData& Data)
{
    // Store match data in game instance
    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    GI->CurrentMatch = Data;
    
    // Open game level
    UDeskillzAuthController::Get(this)->GoToGame();
}

void AMatchHandler::HandleFailed(const FString& Reason, const FMatchLaunchData& Data)
{
    UE_LOG(LogTemp, Error, TEXT("Match failed: %s"), *Reason);
    UDeskillzAuthController::Get(this)->ReturnToLobby();
}
```

---

## 7. Submit Score

When the match ends, submit the player's score:

### Unity

```csharp
using Deskillz;

public class ScoreSubmitter : MonoBehaviour
{
    public void SubmitFinalScore(int score, float duration)
    {
        // Create score data
        var scoreData = new ScoreSubmission
        {
            MatchId = GameSession.MatchId,
            Score = score,
            Duration = duration,
            Metadata = new Dictionary<string, object>
            {
                { "level_reached", currentLevel },
                { "combos", totalCombos }
            }
        };
        
        // Submit with HMAC signature (SDK handles this automatically)
        DeskillzAPI.SubmitScore(scoreData, 
            onSuccess: (result) => {
                Debug.Log($"Score submitted! Position: {result.Position}");
                ShowResults(result);
            },
            onError: (error) => {
                Debug.LogError($"Score submission failed: {error}");
                // Retry logic or show error
            }
        );
    }
    
    void ShowResults(ScoreResult result)
    {
        // Display results UI
        resultsUI.SetResult(result.IsWinner, result.Position, result.Prize);
        resultsUI.Show();
    }
}
```

### Unreal

```cpp
void AScoreSubmitter::SubmitFinalScore(int32 Score, float Duration)
{
    FScoreSubmission Data;
    Data.MatchId = GameSession->MatchId;
    Data.Score = Score;
    Data.Duration = Duration;
    
    UDeskillzApiService::Get()->SubmitScore(Data,
        FOnScoreSubmitted::CreateLambda([this](const FScoreResult& Result) {
            UE_LOG(LogTemp, Log, TEXT("Score submitted! Position: %d"), Result.Position);
            ShowResults(Result);
        }),
        FOnScoreError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("Score failed: %s"), *Error);
        })
    );
}
```

---

## 8. Return to Lobby

After showing results, return the player to the lobby:

### Unity

```csharp
public class ResultsScreen : MonoBehaviour
{
    public void OnPlayAgainClicked()
    {
        // Return to in-app lobby (self-sufficient architecture)
        AuthSceneController.Instance.ReturnToLobby();
    }
    
    public void OnExitClicked()
    {
        // Return to lobby
        AuthSceneController.Instance.ReturnToLobby();
    }
}
```

### Unreal

```cpp
void AResultsScreen::OnPlayAgainClicked()
{
    // Return to in-app lobby
    UDeskillzAuthController::Get(this)->ReturnToLobby();
}

void AResultsScreen::OnExitClicked()
{
    UDeskillzAuthController::Get(this)->ReturnToLobby();
}
```

---

## 9. Enable Auto-Updates

Keep your players on the latest version:

### Unity

```csharp
using Deskillz;

public class UpdateChecker : MonoBehaviour
{
    void Start()
    {
        DeskillzUpdater.Instance.OnUpdateAvailable += HandleUpdateAvailable;
        DeskillzUpdater.Instance.OnForcedUpdateRequired += HandleForcedUpdate;
        DeskillzUpdater.Instance.OnNoUpdateNeeded += HandleNoUpdate;
        
        // Check for updates on startup
        DeskillzUpdater.Instance.CheckForUpdates();
    }
    
    void HandleUpdateAvailable(UpdateInfo info)
    {
        // Optional update - show dialog
        ShowUpdateDialog(info.Version, info.ReleaseNotes, isForced: false);
    }
    
    void HandleForcedUpdate(UpdateInfo info)
    {
        // Required update - must update to continue
        ShowUpdateDialog(info.Version, info.ReleaseNotes, isForced: true);
    }
    
    void HandleNoUpdate(UpdateInfo info)
    {
        // Already on latest version
        Debug.Log("App is up to date");
    }
    
    public void OpenDownloadPage()
    {
        DeskillzUpdater.Instance.OpenDownloadPage();
    }
    
    public void SkipVersion(string version)
    {
        // Only for optional updates
        DeskillzUpdater.Instance.SkipVersion(version);
    }
}
```

### Unreal

```cpp
void AUpdateChecker::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzUpdater::Get()->OnUpdateAvailable.AddDynamic(this, &AUpdateChecker::HandleUpdate);
    UDeskillzUpdater::Get()->OnForcedUpdateRequired.AddDynamic(this, &AUpdateChecker::HandleForced);
    
    UDeskillzUpdater::Get()->CheckForUpdates();
}

void AUpdateChecker::HandleUpdate(const FUpdateInfo& Info)
{
    ShowUpdateDialog(Info.Version, Info.ReleaseNotes, false);
}

void AUpdateChecker::HandleForced(const FUpdateInfo& Info)
{
    ShowUpdateDialog(Info.Version, Info.ReleaseNotes, true);
}

void AUpdateChecker::OpenDownloadPage()
{
    UDeskillzUpdater::Get()->OpenDownloadPage();
}
```

---

## 10. Host Registration Quick Start

Become a host to earn from private rooms you create.

### Unity

```csharp
using Deskillz.Host;

public class HostQuickStart : MonoBehaviour
{
    void Start()
    {
        // Initialize host system (requires authenticated user)
        HostManager.Instance.Initialize(DeskillzAuth.Instance.CurrentUser.Id);
        HostManager.Instance.OnProfileUpdated += OnProfileUpdated;
        HostManager.Instance.OnEarningsUpdated += OnEarningsUpdated;
        HostManager.Instance.OnTierUpgrade += OnTierUpgrade;
    }
    
    public void RegisterAsHost()
    {
        var data = new HostRegistrationData
        {
            DisplayName = "ProHost123",
            Bio = "Competitive gaming enthusiast"
        };
        
        HostManager.Instance.RegisterAsHost(data,
            profile => Debug.Log($"Registered! Tier: {profile.Tier}"),
            error => Debug.LogError(error)
        );
    }
    
    void OnProfileUpdated(HostProfile profile)
    {
        Debug.Log($"Tier: {profile.Tier}, Total Earnings: ${profile.TotalEarnings}");
    }
    
    void OnEarningsUpdated(HostEarnings earnings)
    {
        Debug.Log($"Today: ${earnings.Today}, Week: ${earnings.ThisWeek}");
    }
    
    void OnTierUpgrade(HostTier oldTier, HostTier newTier)
    {
        Debug.Log($"Congratulations! Upgraded from {oldTier} to {newTier}!");
    }
}
```

### Unreal

```cpp
void AHostQuickStart::BeginPlay()
{
    Super::BeginPlay();
    
    FString UserId = UDeskillzAuth::Get()->GetCurrentUser().Id;
    UDeskillzHostManager::Get()->Initialize(UserId);
    
    UDeskillzHostManager::Get()->OnProfileUpdated.AddDynamic(
        this, &AHostQuickStart::OnProfileUpdated);
    UDeskillzHostManager::Get()->OnTierUpgrade.AddDynamic(
        this, &AHostQuickStart::OnTierUpgrade);
}

void AHostQuickStart::RegisterAsHost()
{
    FHostRegistrationData Data;
    Data.DisplayName = TEXT("ProHost123");
    Data.Bio = TEXT("Competitive gaming enthusiast");
    
    UDeskillzHostManager::Get()->RegisterAsHost(Data,
        FOnHostRegistered::CreateLambda([](const FHostProfile& Profile) {
            UE_LOG(LogTemp, Log, TEXT("Registered as host!"));
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AHostQuickStart::OnTierUpgrade(EHostTier OldTier, EHostTier NewTier)
{
    UE_LOG(LogTemp, Log, TEXT("Tier upgraded!"));
}
```

---

## 11. Social Game Quick Start

Implement rake-based social games (poker, etc.) with buy-in/cashout.

### Unity

```csharp
using Deskillz.Social;

public class SocialGameQuickStart : MonoBehaviour
{
    [SerializeField] private string roomId;
    
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
    
    public void PlayerBuyIn(string playerId, decimal amount)
    {
        BuyInManager.Instance.ProcessBuyIn(playerId, amount,
            result => Debug.Log($"{playerId} bought in for ${amount}"),
            error => Debug.LogError(error)
        );
    }
    
    public void EndRound(string winnerId, decimal potAmount)
    {
        SocialGameManager.Instance.EndRound(winnerId, potAmount);
    }
    
    void HandleRoundEnd(RoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber} - Winner: {result.WinnerId}");
        Debug.Log($"Pot: ${result.PotAmount}, Rake: ${result.RakeAmount}");
        
        // Start next round
        SocialGameManager.Instance.StartRound();
    }
    
    void HandleRebuyRequired(string playerId, decimal minAmount)
    {
        // Show rebuy modal
        BuyInModal.Instance.Show(playerId, minAmount);
    }
}
```

### Unreal

```cpp
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

## 12. Host Spectator Mode Quick Start

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

## 13. Test Your Integration

### Test Mode

Test without real currency in the Unity Editor or with test flags:

```csharp
// Unity - Test auth flow
DeskillzAuth.Instance.Login("test@example.com", "password123", true);

// Test deep link launch
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

// Test scene navigation
AuthSceneController.Instance.GoToAuth();
AuthSceneController.Instance.GoToLobby();
AuthSceneController.Instance.GoToGame();
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
- [ ] **Authentication works (email/password login & signup)**
- [ ] **Session restore works (user stays logged in)**
- [ ] **Scene navigation works (Auth -> Lobby -> Game -> Lobby)**
- [ ] Deep link match launch works
- [ ] Score submission succeeds with HMAC hash
- [ ] Return to lobby works correctly (in-app navigation)
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
| 2.8.0 | Added Self-Sufficient Authentication (DeskillzAuth, AuthSceneController), in-app auth flow, scene navigation |
| 2.6.0 | Added Credentials-First Flow (Step 0), Host, Social Game, and Spectator quick starts |
| 2.5.0 | Added Auto-Updater section |
| 2.2.0 | Added Private Rooms |
| 2.0.0 | Deep Link architecture, Centralized Lobby |

---

**SDK Version:** 2.8.0  
**Guide Version:** 3.0  
**Last Updated:** January 2026