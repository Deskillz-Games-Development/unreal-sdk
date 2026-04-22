# Deskillz SDK - API Reference

**SDK Version: 2.8.0** | Complete API documentation for the Deskillz Unity and Unreal Engine SDKs.

## Table of Contents

- [Getting Started](#getting-started)
- [Developer Portal REST API](#developer-portal-rest-api)
- [Authentication Classes](#authentication-classes) - NEW in v2.8!
- [Scene Controller Classes](#scene-controller-classes) - NEW in v2.8!
- [Core Classes](#core-classes)
- [Match Classes](#match-classes)
- [Security Classes](#security-classes)
- [Network Classes](#network-classes)
- [Deep Link Classes](#deep-link-classes)
- [Private Rooms Classes](#private-rooms-classes)
- [Host System Classes](#host-system-classes)
- [Social Game Classes](#social-game-classes)
- [Spectator Classes](#spectator-classes)
- [Auto-Updater Classes](#auto-updater-classes)
- [Analytics Classes](#analytics-classes)
- [Platform Classes](#platform-classes)
- [UI Classes](#ui-classes)
- [Enums](#key-enums)
- [Structs](#key-structs)

---

## Getting Started

### Credentials-First Flow (NEW in v2.6)

Before writing any code, you need your Game ID and API credentials. With our **Credentials-First Flow**, you can get these instantly without completing the full registration form.

#### Step 1: Generate Credentials

1. Go to [Developer Portal](https://deskillz.games/developer)
2. Click **"Register New Game"**
3. Enter your **Game Name** and select **Platform**
4. Click **"Generate Game ID & API Key"**

#### Step 2: You Receive Immediately

| Credential | Example | Purpose |
|------------|---------|---------|
| **Game ID** | `a1b2c3d4-e5f6-7890-abcd-ef1234567890` | Unique identifier for SDK |
| **API Key** | `dsk_live_abc123def456ghi789jkl012mno345pqr678` | Public key for authentication |
| **API Secret** | `dss_xyz789abc456def123ghi012jkl345mno678pqr901stu234` | Private key for HMAC signing |
| **Deep Link Scheme** | `deskillz-yourgamename` | Custom URL scheme for app launching |

> **CRITICAL: Save Your API Secret!**
> Your API Secret is displayed **only once**. Copy it immediately and store it securely.
> Never commit it to source control. If lost, you must regenerate (invalidating the old key).

#### Step 3: Configure Your SDK

**Unity (DeskillzConfig.cs):**
```csharp
[CreateAssetMenu(fileName = "DeskillzConfig", menuName = "Deskillz/Config")]
public class DeskillzConfig : ScriptableObject
{
    [Header("Credentials (from Developer Portal)")]
    public string GameId = "YOUR_GAME_ID";
    public string ApiKey = "YOUR_API_KEY";
    
    [Header("Security (keep secure!)")]
    public string ApiSecret = "YOUR_API_SECRET"; // For HMAC signing
    
    [Header("Settings")]
    public string DeepLinkScheme = "deskillz-yourgame";
    public DeskillzEnvironment Environment = DeskillzEnvironment.Sandbox;
}
```

**Unreal (DeskillzConfigAsset.h):**
```cpp
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
};
```

**Add to .gitignore:**
```
# Deskillz credentials - do not commit!
Assets/Resources/DeskillzConfig.asset
Content/Config/DeskillzConfig.uasset
```

---

## Developer Portal REST API

These endpoints allow programmatic access to the Developer Portal for game registration and credential management.

**Base URL:** `https://api.deskillz.games/api/v1`

### POST /developer/games/draft

Create a draft game and receive credentials immediately.

**Request:**
```json
{
  "name": "Block Puzzle Master",
  "platform": "BOTH"
}
```

**Response (201 Created):**
```json
{
  "gameId": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
  "name": "Block Puzzle Master",
  "slug": "block-puzzle-master",
  "status": "DRAFT",
  "apiKey": "dsk_live_abc123def456ghi789jkl012mno345pqr678",
  "apiSecret": "dss_xyz789abc456def123ghi012jkl345mno678pqr901stu234",
  "environment": "sandbox",
  "deepLinkScheme": "deskillz-blockpuzzlemaster",
  "createdAt": "2026-01-20T12:00:00.000Z",
  "message": "Credentials generated! Use the Game ID and API Key in your Unity/Unreal SDK."
}
```

> **Warning:** `apiSecret` is shown only once! Save it immediately.

**Unity Example:**
```csharp
using UnityEngine.Networking;
using System.Text;

public async Task<DraftGameResponse> CreateDraftGame(string gameName, string platform = "BOTH")
{
    var request = new { name = gameName, platform = platform };
    string json = JsonUtility.ToJson(request);
    
    using (var www = new UnityWebRequest(
        "https://api.deskillz.games/api/v1/developer/games/draft", "POST"))
    {
        www.uploadHandler = new UploadHandlerRaw(Encoding.UTF8.GetBytes(json));
        www.downloadHandler = new DownloadHandlerBuffer();
        www.SetRequestHeader("Content-Type", "application/json");
        www.SetRequestHeader("Authorization", "Bearer " + authToken);
        
        await www.SendWebRequest();
        
        var response = JsonUtility.FromJson<DraftGameResponse>(www.downloadHandler.text);
        
        Debug.Log("Game ID: " + response.gameId);
        Debug.Log("API Secret: " + response.apiSecret); // SAVE THIS NOW!
        
        return response;
    }
}
```

**Unreal Example:**
```cpp
void UDeveloperAPI::CreateDraftGame(const FString& GameName, const FString& Platform)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://api.deskillz.games/api/v1/developer/games/draft"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("name"), GameName);
    JsonObject->SetStringField(TEXT("platform"), Platform);
    
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UDeveloperAPI::OnDraftGameCreated);
    Request->ProcessRequest();
}
```

### GET /developer/games/drafts

List all draft games for the authenticated developer.

**Response (200 OK):**
```json
[
  {
    "id": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
    "name": "Block Puzzle Master",
    "slug": "block-puzzle-master",
    "status": "DRAFT",
    "deepLinkScheme": "deskillz-blockpuzzlemaster",
    "platform": "BOTH",
    "hasApiKey": true,
    "apiKeyHint": "...r678",
    "createdAt": "2026-01-20T12:00:00.000Z"
  }
]
```

### GET /developer/games/:gameId/credentials

Retrieve credentials for an existing game. **Note:** API Secret is NOT included.

**Response (200 OK):**
```json
{
  "gameId": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
  "name": "Block Puzzle Master",
  "status": "DRAFT",
  "deepLinkScheme": "deskillz-blockpuzzlemaster",
  "apiKeys": [
    {
      "apiKey": "dsk_live_abc123def456ghi789jkl012mno345pqr678",
      "name": "Default Key",
      "environment": "sandbox",
      "createdAt": "2026-01-20T12:00:00.000Z"
    }
  ],
  "createdAt": "2026-01-20T12:00:00.000Z"
}
```

### POST /matches/:matchId/score

Submit a score with HMAC security.

**Request:**
```json
{
  "score": 15000,
  "duration": 180,
  "timestamp": 1705756800,
  "nonce": "abc123def456",
  "hash": "base64-encoded-hmac-sha256-hash"
}
```

**HMAC Hash Generation:**

Format: `matchId:score:duration:timestamp:nonce`

**Unity:**
```csharp
using System.Security.Cryptography;
using System.Text;

public string GenerateScoreHash(string matchId, int score, int duration, 
                                 long timestamp, string nonce, string apiSecret)
{
    string data = $"{matchId}:{score}:{duration}:{timestamp}:{nonce}";
    
    using (var hmac = new HMACSHA256(Encoding.UTF8.GetBytes(apiSecret)))
    {
        byte[] hash = hmac.ComputeHash(Encoding.UTF8.GetBytes(data));
        return Convert.ToBase64String(hash);
    }
}
```

**Unreal:**
```cpp
FString UScoreSubmitter::GenerateScoreHash(const FString& MatchId, int32 Score, 
    int32 Duration, int64 Timestamp, const FString& Nonce, const FString& ApiSecret)
{
    FString Data = FString::Printf(TEXT("%s:%d:%d:%lld:%s"), 
        *MatchId, Score, Duration, Timestamp, *Nonce);
    
    TArray<uint8> DataBytes;
    FTCHARToUTF8 DataConverter(*Data);
    DataBytes.Append((uint8*)DataConverter.Get(), DataConverter.Length());
    
    TArray<uint8> KeyBytes;
    FTCHARToUTF8 KeyConverter(*ApiSecret);
    KeyBytes.Append((uint8*)KeyConverter.Get(), KeyConverter.Length());
    
    TArray<uint8> HashBytes;
    FSHA256Signature::HMACSHA256(DataBytes, KeyBytes, HashBytes);
    
    return FBase64::Encode(HashBytes);
}
```

---

## Authentication Classes

**NEW in v2.8!** Self-Sufficient Authentication for standalone game apps.

### UDeskillzAuth / DeskillzAuth

Authentication manager for email/password and social login.

**Unity (DeskillzAuth.cs):**

| Method | Description |
|--------|-------------|
| `Instance` | Get singleton instance |
| `Initialize()` | Initialize auth system |
| `LoginWithEmail(email, password, rememberMe)` | Login with email/password |
| `SignUpWithEmail(email, password, username)` | Register new account |
| `SocialLogin(provider)` | Login with social provider (Google, Apple, Facebook) |
| `ForgotPassword(email)` | Request password reset |
| `Logout()` | Logout current user |
| `RefreshToken()` | Refresh access token |
| `ConnectWallet()` | Connect wallet to account |
| `DisconnectWallet()` | Disconnect wallet |
| `LinkEmail(email, password)` | Link email to wallet account |

| Property | Type | Description |
|----------|------|-------------|
| `IsAuthenticated` | bool | User is logged in |
| `CurrentUser` | AuthUser | Current user data |
| `AccessToken` | string | Current access token |
| `AuthState` | EAuthState | Current auth state |

**Unreal (DeskillzAuth.h):**

```cpp
UCLASS()
class DESKILLZSDK_API UDeskillzAuth : public UObject
{
    GENERATED_BODY()
    
public:
    static UDeskillzAuth* Get();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Initialize();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void LoginWithEmail(const FString& Email, const FString& Password, bool bRememberMe = true);
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void SignUpWithEmail(const FString& Email, const FString& Password, const FString& Username);
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void SocialLogin(ESocialProvider Provider);
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void ForgotPassword(const FString& Email);
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Logout();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void ConnectWallet();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void DisconnectWallet();
    
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    bool IsAuthenticated() const;
    
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    FAuthUser GetCurrentUser() const;
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth")
    FOnLoginSuccess OnLoginSuccess;
    
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth")
    FOnSignUpSuccess OnSignUpSuccess;
    
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth")
    FOnLogout OnLogout;
    
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth")
    FOnAuthError OnAuthError;
    
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth")
    FOnWalletLinked OnWalletLinked;
    
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth")
    FOnWalletDisconnected OnWalletDisconnected;
};
```

### Auth Events (NEW in v2.8)

| Event | Parameters | Description |
|-------|------------|-------------|
| `OnAuthLoginSuccess` | AuthUser user | Login succeeded |
| `OnAuthSignUpSuccess` | AuthUser user | Registration succeeded |
| `OnAuthLogout` | None | User logged out |
| `OnAuthError` | AuthError error | Auth error occurred |
| `OnAuthStateChanged` | EAuthState state | Auth state changed |
| `OnWalletLinked` | string address | Wallet linked to account |
| `OnWalletDisconnected` | None | Wallet disconnected |
| `OnPasswordResetSent` | None | Reset email sent |

**Unity Example:**
```csharp
using Deskillz;

public class AuthManager : MonoBehaviour
{
    void Start()
    {
        DeskillzAuth.Instance.Initialize();
        
        DeskillzEvents.OnAuthLoginSuccess += OnLoginSuccess;
        DeskillzEvents.OnAuthSignUpSuccess += OnSignUpSuccess;
        DeskillzEvents.OnAuthLogout += OnLogout;
        DeskillzEvents.OnAuthError += OnAuthError;
        DeskillzEvents.OnWalletLinked += OnWalletLinked;
        
        // Check for existing session
        if (DeskillzAuth.Instance.IsAuthenticated)
        {
            Debug.Log($"Welcome back, {DeskillzAuth.Instance.CurrentUser.Username}!");
        }
    }
    
    public void Login(string email, string password)
    {
        DeskillzAuth.Instance.LoginWithEmail(email, password, true);
    }
    
    public void SignUp(string email, string password, string username)
    {
        DeskillzAuth.Instance.SignUpWithEmail(email, password, username);
    }
    
    public void SocialLogin(SocialProvider provider)
    {
        DeskillzAuth.Instance.SocialLogin(provider);
    }
    
    void OnLoginSuccess(AuthUser user)
    {
        Debug.Log($"Login success: {user.Username}");
        AuthSceneController.Instance.GoToLobby();
    }
    
    void OnSignUpSuccess(AuthUser user)
    {
        Debug.Log($"Sign up success: {user.Username}");
        AuthSceneController.Instance.GoToLobby();
    }
    
    void OnLogout()
    {
        Debug.Log("User logged out");
        AuthSceneController.Instance.GoToAuth();
    }
    
    void OnAuthError(AuthError error)
    {
        Debug.LogError($"Auth error: {error.Code} - {error.Message}");
    }
    
    void OnWalletLinked(string address)
    {
        Debug.Log($"Wallet linked: {address}");
    }
}
```

**Unreal Example:**
```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzAuth::Get()->Initialize();
    
    UDeskillzAuth::Get()->OnLoginSuccess.AddDynamic(this, &AMyGameMode::OnLoginSuccess);
    UDeskillzAuth::Get()->OnSignUpSuccess.AddDynamic(this, &AMyGameMode::OnSignUpSuccess);
    UDeskillzAuth::Get()->OnLogout.AddDynamic(this, &AMyGameMode::OnLogout);
    UDeskillzAuth::Get()->OnAuthError.AddDynamic(this, &AMyGameMode::OnAuthError);
    
    if (UDeskillzAuth::Get()->IsAuthenticated())
    {
        FAuthUser User = UDeskillzAuth::Get()->GetCurrentUser();
        UE_LOG(LogTemp, Log, TEXT("Welcome back, %s!"), *User.Username);
    }
}

void AMyGameMode::OnLoginSuccess(const FAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("Login success: %s"), *User.Username);
    UDeskillzAuthController::Get()->GoToLobby();
}

void AMyGameMode::OnLogout()
{
    UE_LOG(LogTemp, Log, TEXT("User logged out"));
    UDeskillzAuthController::Get()->GoToAuth();
}
```

### Auth REST API Endpoints (NEW in v2.8)

**Base URL:** `https://api.deskillz.games/api/v1`

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/auth/register` | Register with email/password |
| POST | `/auth/login` | Login with email/password |
| POST | `/auth/social` | Login with social provider |
| POST | `/auth/forgot-password` | Request password reset |
| POST | `/auth/reset-password` | Reset password with token |
| POST | `/auth/wallet/link` | Link wallet to account |
| POST | `/auth/wallet/disconnect` | Disconnect wallet |
| POST | `/auth/refresh` | Refresh access token |
| GET | `/auth/nonce` | Get SIWE nonce for wallet auth |

---

## Scene Controller Classes

**NEW in v2.8!** Scene/Level navigation for self-sufficient architecture.

### UDeskillzAuthController / AuthSceneController

Scene navigation controller for auth flow.

**Unity (AuthSceneController.cs):**

| Method | Description |
|--------|-------------|
| `Instance` | Get singleton instance |
| `Initialize(config)` | Initialize with DeskillzConfig |
| `GoToAuth()` | Navigate to auth scene |
| `GoToLobby()` | Navigate to lobby scene |
| `GoToGame()` | Navigate to game scene |
| `GoToLoading()` | Navigate to loading scene |
| `ShowLogin()` | Show login UI panel |
| `ShowSignUp()` | Show sign up UI panel |
| `ShowForgotPassword()` | Show forgot password UI panel |
| `GetCurrentScene()` | Get current scene name |

**Unity Example:**
```csharp
using Deskillz;
using UnityEngine;

public class GameBootstrap : MonoBehaviour
{
    [SerializeField] private DeskillzConfig config;
    
    void Start()
    {
        // Initialize SDK
        DeskillzSDK.Instance.Initialize(new DeskillzSDKConfig
        {
            GameId = config.GameId,
            ApiKey = config.ApiKey,
            SelfSufficientMode = true
        });
        
        DeskillzSDK.Instance.OnInitialized += OnSDKReady;
    }
    
    void OnSDKReady()
    {
        // Initialize auth
        DeskillzAuth.Instance.Initialize();
        
        // Initialize scene controller
        AuthSceneController.Instance.Initialize(config);
        
        // Navigate based on auth state
        if (DeskillzAuth.Instance.IsAuthenticated)
        {
            AuthSceneController.Instance.GoToLobby();
        }
        else
        {
            AuthSceneController.Instance.GoToAuth();
        }
    }
}
```

**Unreal (DeskillzAuthController.h):**

```cpp
UCLASS()
class DESKILLZSDK_API UDeskillzAuthController : public UObject
{
    GENERATED_BODY()
    
public:
    static UDeskillzAuthController* Get();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Initialize();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void GoToAuth();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void GoToLobby();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void GoToGame();
    
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void GoToLoading();
    
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    FString GetCurrentLevel() const;
};
```

**Unreal Example:**
```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize auth controller
    UDeskillzAuthController::Get()->Initialize();
    
    // Navigate based on auth state
    if (UDeskillzAuth::Get()->IsAuthenticated())
    {
        UDeskillzAuthController::Get()->GoToLobby();
    }
    else
    {
        UDeskillzAuthController::Get()->GoToAuth();
    }
}
```

### Scene Configuration (NEW in v2.8)

Configure scene names in your DeskillzConfig:

**Unity:**
```csharp
[CreateAssetMenu(fileName = "DeskillzConfig", menuName = "Deskillz/Config")]
public class DeskillzConfig : ScriptableObject
{
    // ... existing fields ...
    
    [Header("Self-Sufficient Architecture (NEW in v2.8)")]
    public bool SelfSufficientMode = true;
    public string AuthSceneName = "AuthScene";
    public string LobbySceneName = "LobbyScene";
    public string GameSceneName = "GameScene";
    public string LoadingSceneName = "LoadingScene";
}
```

**Unreal:**
```cpp
// In UDeskillzConfigAsset
UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
bool bSelfSufficientMode = true;

UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
FString AuthLevelName = TEXT("/Game/Maps/AuthLevel");

UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
FString LobbyLevelName = TEXT("/Game/Maps/LobbyLevel");

UPROPERTY(EditAnywhere, Category = "Self-Sufficient")
FString GameLevelName = TEXT("/Game/Maps/GameLevel");
```

---

## Core Classes

### UDeskillzSDK / DeskillzSDK
Main SDK singleton.

| Method | Description |
|--------|-------------|
| `Get()` / `Instance` | Get singleton instance |
| `Initialize(Config)` | Initialize SDK with configuration |
| `Shutdown()` | Clean up SDK resources |
| `IsInitialized()` | Check if SDK is ready |
| `GetGameId()` | Get current game ID |
| `GetSDKVersion()` | Get SDK version string |

### FDeskillzConfig / DeskillzConfig
Configuration struct.

| Property | Type | Description |
|----------|------|-------------|
| `GameId` | string | Your game's unique ID |
| `ApiKey` | string | Your API key |
| `Environment` | EDeskillzEnvironment | Sandbox or Production |
| `BaseUrl` | string | API base URL (optional) |
| `ApiTimeout` | float | Request timeout in seconds |
| `bEnableLogging` | bool | Enable debug logging |
| `bEnableAnalytics` | bool | Enable analytics tracking |
| `bEnableAntiCheat` | bool | Enable anti-cheat protection |

### UDeskillzEvents / DeskillzEvents
Event dispatcher.

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnSDKInitialized` | None | SDK ready to use |
| `OnSDKError` | string Error | SDK error occurred |
| `OnAuthStateChanged` | bool bAuthenticated | Auth state changed |
| `OnConnectionStateChanged` | bool bConnected | Connection state changed |
| `OnWalletUpdated` | Array WalletBalance | Wallet balances updated |

---

## Match Classes

### UDeskillzMatchmaking / DeskillzMatchmaking
Matchmaking manager.

| Method | Description |
|--------|-------------|
| `StartMatchmaking(TournamentId)` | Start searching for match |
| `CancelMatchmaking()` | Cancel matchmaking |
| `IsMatchmaking()` | Check if currently matchmaking |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnMatchFound` | FDeskillzMatch | Match found |
| `OnMatchmakingFailed` | string Reason | Matchmaking failed |
| `OnMatchmakingCancelled` | None | Matchmaking cancelled |

### UDeskillzMatchManager / DeskillzMatchManager
Match lifecycle manager.

| Method | Description |
|--------|-------------|
| `StartMatch(MatchId)` | Begin match gameplay |
| `EndMatch(Result)` | End match with result |
| `PauseMatch()` | Pause current match |
| `ResumeMatch()` | Resume paused match |
| `GetCurrentMatch()` | Get current match info |
| `IsMatchActive()` | Check if match is active |

---

## Security Classes

### UDeskillzScoreEncryption / DeskillzScoreEncryption
Score encryption utilities.

| Method | Description |
|--------|-------------|
| `EncryptScore(Score, MatchId, UserId)` | Encrypt score for submission |
| `DecryptScore(EncryptedData)` | Decrypt score data |
| `GenerateHMAC(Data)` | Generate HMAC signature |
| `VerifyHMAC(Data, HMAC)` | Verify HMAC signature |

### UDeskillzSecureSubmitter / DeskillzSecureSubmitter
Secure score submission.

| Method | Description |
|--------|-------------|
| `SubmitScore(Score, Duration)` | Submit encrypted score |
| `SubmitScoreWithMetadata(Score, Duration, Metadata)` | Submit with extra data |
| `IsSubmitting()` | Check if submission in progress |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnScoreSubmitted` | None | Score submitted successfully |
| `OnScoreValidated` | bool bValid | Score validation result |

### UDeskillzAntiCheat / DeskillzAntiCheat
Anti-cheat protection.

| Method | Description |
|--------|-------------|
| `ValidateScore(Score, Duration)` | Validate score locally |
| `IsSpeedHackDetected()` | Check for speed hacks |
| `CheckMemoryIntegrity()` | Verify memory integrity |
| `GenerateSubmissionId()` | Generate unique submission ID |

---

## Network Classes

### UDeskillzHttpClient / DeskillzHttpClient
HTTP client for API calls.

| Method | Description |
|--------|-------------|
| `Get(Endpoint, Callback)` | HTTP GET request |
| `Post(Endpoint, Body, Callback)` | HTTP POST request |
| `Put(Endpoint, Body, Callback)` | HTTP PUT request |
| `Delete(Endpoint, Callback)` | HTTP DELETE request |
| `SetAuthToken(Token)` | Set authentication token |

### UDeskillzWebSocket / DeskillzWebSocket
WebSocket for real-time communication.

| Method | Description |
|--------|-------------|
| `Connect(Url)` | Connect to WebSocket server |
| `Disconnect()` | Disconnect from server |
| `IsConnected()` | Check connection status |
| `JoinRoom(RoomId)` | Join a room |
| `LeaveRoom(RoomId)` | Leave a room |
| `SendMessage(Event, Data)` | Send message to server |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnConnected` | None | WebSocket connected |
| `OnDisconnected` | None | WebSocket disconnected |
| `OnMessageReceived` | string Event, JsonObject | Message received |
| `OnError` | string Error | WebSocket error |

### UDeskillzApiService / DeskillzApiService
High-level API service.

| Method | Description |
|--------|-------------|
| `Login(WalletAddress, Signature)` | Login with wallet |
| `Register(WalletAddress, Username)` | Register new user |
| `Logout()` | Logout current user |
| `GetTournaments(GameId, Callback)` | Get available tournaments |
| `EnterTournament(TournamentId, Callback)` | Enter tournament |
| `GetWalletBalance(Callback)` | Get wallet balances |
| `GetLeaderboard(TournamentId, Callback)` | Get leaderboard |

---

## Deep Link Classes

### UDeskillzDeepLinkHandler / DeepLinkHandler
Deep link handler for lobby integration (SDK 2.0+).

| Method | Description |
|--------|-------------|
| `Initialize()` | Initialize deep link handling |
| `HasPendingLaunch()` | Check for pending match launch |
| `ProcessPendingLaunch()` | Process pending deep link |
| `GetLaunchData()` | Get current launch data |
| `SimulateDeepLink(Url)` | Simulate deep link (testing) |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnMatchReady` | FDeskillzMatchLaunchData | Match is ready to play |
| `OnValidationFailed` | string Reason, FDeskillzMatchLaunchData | Launch validation failed |
| `OnNavigationReceived` | EDeskillzNavigationAction, Map Parameters | Navigation deep link received |
| `OnMatchLaunchReceived` | string MatchId, string AuthToken | Simplified match launch event |

### UDeskillzBridge / DeskillzBridge
Bridge for communication with main Deskillz app.

| Method | Description |
|--------|-------------|
| `SubmitScore(Score, Callback)` | Submit score and handle return |
| `ReturnToMainApp(Destination)` | Return to main app |
| `GetMatchLaunchData()` | Get match launch data |
| `IsLaunchedFromMainApp()` | Check if launched via deep link |

---

## Private Rooms Classes

### UDeskillzRooms / DeskillzRooms
Private rooms manager for creating and joining custom rooms.

| Method | Description |
|--------|-------------|
| `CreateRoom(Config, OnSuccess, OnError)` | Create a new private room |
| `JoinRoom(RoomCode, OnSuccess, OnError)` | Join room by code |
| `GetPublicRooms(GameId, OnSuccess, OnError)` | Get list of public rooms |
| `SetReady(bReady)` | Set player ready status |
| `LeaveRoom()` | Leave current room |
| `SendChat(Message)` | Send chat message in room |
| `StartMatch()` | Start match (host only) |
| `KickPlayer(PlayerId)` | Kick player from room (host only) |
| `CancelRoom()` | Cancel and close room (host only) |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnRoomJoined` | FPrivateRoom Room | Successfully joined room |
| `OnPlayerJoined` | FRoomPlayer Player | Player joined room |
| `OnPlayerLeft` | string PlayerId | Player left room |
| `OnPlayerReadyChanged` | string PlayerId, bool bReady | Player ready status changed |
| `OnMatchLaunching` | string MatchId | Match is starting |
| `OnChatReceived` | string PlayerId, string Message | Chat message received |
| `OnKicked` | string Reason | You were kicked from room |
| `OnRoomCancelled` | None | Room was cancelled by host |

---

## Host System Classes (NEW in v2.6)

### UDeskillzHostManager / HostManager
Host profile and room management for verified hosts.

| Method | Description |
|--------|-------------|
| `Initialize(UserId)` | Initialize host manager with user ID |
| `RegisterAsHost(OnSuccess, OnError)` | Register current user as host |
| `GetHostProfile(OnSuccess, OnError)` | Get current host profile |
| `GetHostTier()` | Get current host tier |
| `GetHostLevel()` | Get current host level |
| `GetTotalEarnings()` | Get lifetime earnings |
| `GetPendingEarnings()` | Get pending (unsettled) earnings |
| `GetHostedRooms(OnSuccess, OnError)` | Get list of rooms hosted |
| `GetActiveRooms()` | Get currently active rooms |
| `CreateHostRoom(Config, OnSuccess, OnError)` | Create room as host |
| `CloseRoom(RoomId, OnSuccess, OnError)` | Close a hosted room |
| `WithdrawEarnings(Amount, OnSuccess, OnError)` | Withdraw host earnings |
| `GetBadges(OnSuccess, OnError)` | Get earned badges |
| `GetTierProgress()` | Get progress to next tier |
| `GetLevelProgress()` | Get progress to next level |
| `IsVerifiedHost()` | Check if user is verified host |
| `VerifyAge(BirthDate, OnSuccess, OnError)` | Submit age verification |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnHostProfileUpdated` | FHostProfile Profile | Host profile changed |
| `OnTierChanged` | EHostTier OldTier, EHostTier NewTier | Host tier upgraded/downgraded |
| `OnLevelUp` | int32 NewLevel | Host leveled up |
| `OnBadgeEarned` | FHostBadge Badge | New badge earned |
| `OnEarningsUpdated` | float TotalEarnings, float PendingEarnings | Earnings changed |
| `OnRoomCreated` | FPrivateRoom Room | Room created successfully |
| `OnRoomClosed` | string RoomId | Room closed |
| `OnPlayerJoinedHostedRoom` | string RoomId, FRoomPlayer Player | Player joined your room |
| `OnPlayerLeftHostedRoom` | string RoomId, string PlayerId | Player left your room |

**Unity Example:**
```csharp
using Deskillz.Host;

public class MyHostManager : MonoBehaviour
{
    void Start()
    {
        // Initialize host manager
        HostManager.Instance.Initialize(currentUserId);
        
        // Subscribe to events
        HostManager.Instance.OnTierChanged += HandleTierChange;
        HostManager.Instance.OnEarningsUpdated += HandleEarnings;
        
        // Check if user is a host
        if (HostManager.Instance.IsVerifiedHost())
        {
            Debug.Log($"Host Tier: {HostManager.Instance.GetHostTier()}");
            Debug.Log($"Earnings: ${HostManager.Instance.GetTotalEarnings()}");
        }
    }
    
    void RegisterAsHost()
    {
        HostManager.Instance.RegisterAsHost(
            profile => Debug.Log($"Registered as host: {profile.Tier}"),
            error => Debug.LogError($"Registration failed: {error}")
        );
    }
    
    void CreateRoom()
    {
        var config = new CreateHostRoomConfig
        {
            Name = "My Pro Room",
            GameCategory = GameCategory.Esports,
            EntryFee = 10.0m,
            EntryCurrency = "USDT",
            MaxPlayers = 8,
            Mode = RoomMode.Sync,
            Visibility = RoomVisibility.PublicListed
        };
        
        HostManager.Instance.CreateHostRoom(config,
            room => Debug.Log($"Room created: {room.RoomCode}"),
            error => Debug.LogError(error)
        );
    }
    
    void HandleTierChange(HostTier oldTier, HostTier newTier)
    {
        Debug.Log($"Tier changed: {oldTier} -> {newTier}");
    }
    
    void HandleEarnings(float total, float pending)
    {
        Debug.Log($"Total: ${total}, Pending: ${pending}");
    }
}
```

**Unreal Example:**
```cpp
#include "Host/DeskillzHostManager.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize host manager
    UDeskillzHostManager* HostManager = UDeskillzHostManager::Get();
    HostManager->Initialize(CurrentUserId);
    
    // Subscribe to events
    HostManager->OnTierChanged.AddDynamic(this, &AMyGameMode::HandleTierChange);
    HostManager->OnEarningsUpdated.AddDynamic(this, &AMyGameMode::HandleEarnings);
    
    // Check if user is a host
    if (HostManager->IsVerifiedHost())
    {
        UE_LOG(LogTemp, Log, TEXT("Host Tier: %d"), (int32)HostManager->GetHostTier());
        UE_LOG(LogTemp, Log, TEXT("Earnings: $%.2f"), HostManager->GetTotalEarnings());
    }
}

void AMyGameMode::RegisterAsHost()
{
    UDeskillzHostManager::Get()->RegisterAsHost(
        FOnHostProfileResult::CreateLambda([](const FHostProfile& Profile) {
            UE_LOG(LogTemp, Log, TEXT("Registered as host: %s"), *Profile.TierName);
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("Registration failed: %s"), *Error);
        })
    );
}

void AMyGameMode::CreateRoom()
{
    FCreateHostRoomConfig Config;
    Config.Name = TEXT("My Pro Room");
    Config.GameCategory = EGameCategory::Esports;
    Config.EntryFee = 10.0f;
    Config.EntryCurrency = TEXT("USDT");
    Config.MaxPlayers = 8;
    Config.Mode = ERoomMode::Sync;
    Config.Visibility = ERoomVisibility::PublicListed;
    
    UDeskillzHostManager::Get()->CreateHostRoom(Config,
        FOnRoomCreated::CreateLambda([](const FPrivateRoom& Room) {
            UE_LOG(LogTemp, Log, TEXT("Room created: %s"), *Room.RoomCode);
        }),
        FOnHostError::CreateLambda([](const FString& Error) {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
        })
    );
}

void AMyGameMode::HandleTierChange(EHostTier OldTier, EHostTier NewTier)
{
    UE_LOG(LogTemp, Log, TEXT("Tier changed: %d -> %d"), (int32)OldTier, (int32)NewTier);
}

void AMyGameMode::HandleEarnings(float Total, float Pending)
{
    UE_LOG(LogTemp, Log, TEXT("Total: $%.2f, Pending: $%.2f"), Total, Pending);
}
```

---

## Social Game Classes (NEW in v2.6)

### UDeskillzSocialGameManager / SocialGameManager
Social game session management with rake mechanics.

| Method | Description |
|--------|-------------|
| `Initialize(RoomId)` | Initialize for a social room |
| `StartSession(OnSuccess, OnError)` | Start social game session |
| `EndSession(OnSuccess, OnError)` | End current session |
| `GetSessionState()` | Get current session state |
| `GetCurrentRound()` | Get current round number |
| `StartRound()` | Start a new round |
| `EndRound(WinnerId, PotAmount)` | End round with winner |
| `RecordAction(PlayerId, Action, Amount)` | Record player action |
| `GetPlayerBalance(PlayerId)` | Get player's point balance |
| `GetTotalPot()` | Get current pot size |
| `GetRakeCollected()` | Get rake collected this session |
| `RequestPause(Reason)` | Request game pause |
| `VotePause(Approve)` | Vote on pause request |
| `ResumePause()` | Resume from pause |
| `IsPaused()` | Check if game is paused |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnSessionStarted` | FSocialGameSession Session | Session started |
| `OnSessionEnded` | FSocialGameSession Session | Session ended |
| `OnRoundStarted` | int32 RoundNumber | New round started |
| `OnRoundEnded` | FRoundResult Result | Round ended with result |
| `OnPlayerAction` | string PlayerId, string Action, float Amount | Player performed action |
| `OnRakeCollected` | float Amount, float Total | Rake collected from round |
| `OnPauseRequested` | string RequesterId, string Reason | Pause requested |
| `OnPauseVoted` | string VoterId, bool Approved | Player voted on pause |
| `OnGamePaused` | DateTime ResumeTime | Game paused |
| `OnGameResumed` | None | Game resumed |

### UDeskillzRakeCalculator / RakeCalculator
Rake calculation utilities.

| Method | Description |
|--------|-------------|
| `CalculateRake(PotAmount, RakePercentage)` | Calculate rake for pot |
| `CalculateRakeWithCap(PotAmount, RakePercentage, RakeCap)` | Calculate with cap |
| `GetEffectiveRake(PotAmount, RoomSettings)` | Get effective rake amount |
| `PreviewRakeDistribution(RakeAmount, HostTier)` | Preview how rake is split |
| `GetHostShare(RakeAmount, HostTier)` | Get host's share of rake |
| `GetPlatformShare(RakeAmount, HostTier)` | Get platform's share |
| `GetDeveloperShare(RakeAmount, HostTier)` | Get developer's share |

**Rake Distribution by Host Tier:**

| Tier | Host Share | Platform Share | Developer Share |
|------|------------|----------------|-----------------|
| Starter | 50% | 25% | 25% |
| Bronze | 55% | 22.5% | 22.5% |
| Silver | 60% | 20% | 20% |
| Gold | 65% | 17.5% | 17.5% |
| Platinum | 70% | 15% | 15% |
| Diamond | 75% | 12.5% | 12.5% |

### UDeskillzBuyInManager / BuyInManager
Buy-in, rebuy, and cash-out management.

| Method | Description |
|--------|-------------|
| `Initialize(RoomId)` | Initialize for a room |
| `ProcessBuyIn(PlayerId, Amount, OnSuccess, OnError)` | Process initial buy-in |
| `ProcessRebuy(PlayerId, Amount, OnSuccess, OnError)` | Process rebuy |
| `ProcessCashOut(PlayerId, OnSuccess, OnError)` | Process cash out |
| `GetPlayerBalance(PlayerId)` | Get player's current balance |
| `GetMinBuyIn()` | Get minimum buy-in amount |
| `GetMaxBuyIn()` | Get maximum buy-in amount |
| `GetDefaultBuyIn()` | Get default buy-in amount |
| `IsRebuyRequired(PlayerId)` | Check if player needs rebuy |
| `GetRebuyTimeoutSeconds()` | Get rebuy timeout duration |
| `CanCashOut(PlayerId)` | Check if player can cash out |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnBuyInProcessed` | string PlayerId, float Amount, float NewBalance | Buy-in successful |
| `OnRebuyProcessed` | string PlayerId, float Amount, float NewBalance | Rebuy successful |
| `OnCashOutProcessed` | string PlayerId, float Amount | Cash out successful |
| `OnRebuyRequired` | string PlayerId, float TimeoutSeconds | Player needs rebuy |
| `OnRebuyTimeout` | string PlayerId | Player timed out on rebuy |
| `OnLowBalanceWarning` | string PlayerId, float CurrentBalance, float Threshold | Balance below threshold |

**Unity Example:**
```csharp
using Deskillz.Social;

public class MySocialGame : MonoBehaviour
{
    void Start()
    {
        // Initialize managers
        SocialGameManager.Instance.Initialize(roomId);
        BuyInManager.Instance.Initialize(roomId);
        
        // Subscribe to events
        SocialGameManager.Instance.OnRoundEnded += HandleRoundEnd;
        BuyInManager.Instance.OnRebuyRequired += HandleRebuyRequired;
        BuyInManager.Instance.OnLowBalanceWarning += HandleLowBalance;
    }
    
    void StartGame()
    {
        SocialGameManager.Instance.StartSession(
            session => Debug.Log($"Session started: {session.Id}"),
            error => Debug.LogError(error)
        );
    }
    
    void EndRound(string winnerId, float potAmount)
    {
        SocialGameManager.Instance.EndRound(winnerId, potAmount);
        
        // Preview rake distribution
        float rake = RakeCalculator.CalculateRakeWithCap(
            potAmount, 
            rakePercentage, 
            rakeCap
        );
        
        var distribution = RakeCalculator.PreviewRakeDistribution(rake, hostTier);
        Debug.Log($"Host gets: ${distribution.HostShare}");
    }
    
    void HandleRoundEnd(RoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber} won by {result.WinnerId}");
        Debug.Log($"Pot: ${result.PotAmount}, Rake: ${result.RakeAmount}");
    }
    
    void HandleRebuyRequired(string playerId, float timeout)
    {
        // Show rebuy modal
        RebuyModal.Instance.Show(playerId, timeout);
    }
    
    void HandleLowBalance(string playerId, float balance, float threshold)
    {
        Debug.LogWarning($"Player {playerId} balance (${balance}) below threshold (${threshold})");
    }
}
```

**Unreal Example:**
```cpp
#include "Social/DeskillzSocialGameManager.h"
#include "Social/DeskillzRakeCalculator.h"
#include "Social/DeskillzBuyInManager.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize managers
    UDeskillzSocialGameManager::Get()->Initialize(RoomId);
    UDeskillzBuyInManager::Get()->Initialize(RoomId);
    
    // Subscribe to events
    UDeskillzSocialGameManager::Get()->OnRoundEnded.AddDynamic(this, &AMyGameMode::HandleRoundEnd);
    UDeskillzBuyInManager::Get()->OnRebuyRequired.AddDynamic(this, &AMyGameMode::HandleRebuyRequired);
}

void AMyGameMode::StartGame()
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

void AMyGameMode::EndRound(const FString& WinnerId, float PotAmount)
{
    UDeskillzSocialGameManager::Get()->EndRound(WinnerId, PotAmount);
    
    // Preview rake distribution
    float Rake = UDeskillzRakeCalculator::CalculateRakeWithCap(
        PotAmount, 
        RakePercentage, 
        RakeCap
    );
    
    FRakeDistribution Distribution = UDeskillzRakeCalculator::PreviewRakeDistribution(Rake, HostTier);
    UE_LOG(LogTemp, Log, TEXT("Host gets: $%.2f"), Distribution.HostShare);
}

void AMyGameMode::HandleRoundEnd(const FRoundResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("Round %d won by %s"), Result.RoundNumber, *Result.WinnerId);
    UE_LOG(LogTemp, Log, TEXT("Pot: $%.2f, Rake: $%.2f"), Result.PotAmount, Result.RakeAmount);
}

void AMyGameMode::HandleRebuyRequired(const FString& PlayerId, float Timeout)
{
    // Show rebuy widget
    RebuyWidget->Show(PlayerId, Timeout);
}
```

---

## Spectator Classes (NEW in v2.6)

### UDeskillzSpectatorManager / SpectatorManager
Spectator mode for watching live games.

| Method | Description |
|--------|-------------|
| `Initialize()` | Initialize spectator manager |
| `FetchSpectatorRooms(Filter, OnSuccess, OnError)` | Get rooms available for spectating |
| `JoinAsSpectator(RoomId, OnSuccess, OnError)` | Join room as spectator |
| `LeaveSpectator()` | Leave spectator mode |
| `GetCurrentRoom()` | Get room being spectated |
| `GetSpectatorState()` | Get current spectator state |
| `GetPlayers()` | Get visible player information |
| `GetCurrentRound()` | Get current round number |
| `GetScores()` | Get current scores |
| `SetViewMode(Mode)` | Set spectator view mode |
| `GetViewMode()` | Get current view mode |
| `FollowPlayer(PlayerId)` | Focus view on specific player |
| `GetFollowedPlayer()` | Get currently followed player |
| `SwitchRoom(RoomId)` | Switch to different room |
| `GetSpectatorCount()` | Get number of spectators |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnJoinedAsSpectator` | FSpectatorState State | Joined room as spectator |
| `OnLeftSpectator` | None | Left spectator mode |
| `OnGameStateUpdated` | FSpectatorState State | Game state changed |
| `OnRoundStarted` | int32 RoundNumber | New round started |
| `OnRoundEnded` | FSpectatorRoundResult Result | Round ended |
| `OnPlayerAction` | FSpectatorAction Action | Player performed action |
| `OnScoresUpdated` | Array FSpectatorScore | Scores updated |
| `OnSpectatorCountChanged` | int32 Count | Spectator count changed |
| `OnRoomSwitched` | FSpectatorState NewState | Switched to new room |
| `OnKickedFromSpectator` | string Reason | Kicked from spectating |

**Unity Example:**
```csharp
using Deskillz.Spectator;

public class MySpectatorView : MonoBehaviour
{
    void Start()
    {
        SpectatorManager.Instance.Initialize();
        
        // Subscribe to events
        SpectatorManager.Instance.OnGameStateUpdated += HandleStateUpdate;
        SpectatorManager.Instance.OnRoundEnded += HandleRoundEnd;
        SpectatorManager.Instance.OnPlayerAction += HandleAction;
    }
    
    void FetchRooms()
    {
        var filter = new SpectatorRoomFilter
        {
            GameId = currentGameId,
            GameCategory = GameCategory.Social,
            MinPlayers = 2
        };
        
        SpectatorManager.Instance.FetchSpectatorRooms(filter,
            rooms => DisplayRoomList(rooms),
            error => Debug.LogError(error)
        );
    }
    
    void JoinRoom(string roomId)
    {
        SpectatorManager.Instance.JoinAsSpectator(roomId,
            state => {
                Debug.Log($"Spectating room: {state.RoomName}");
                Debug.Log($"Players: {state.Players.Count}");
                UpdateUI(state);
            },
            error => Debug.LogError(error)
        );
    }
    
    void HandleStateUpdate(SpectatorState state)
    {
        // Update game board display
        UpdateGameBoard(state);
        UpdateScorePanel(state.Scores);
    }
    
    void HandleRoundEnd(SpectatorRoundResult result)
    {
        Debug.Log($"Round {result.RoundNumber} winner: {result.WinnerUsername}");
        ShowRoundSummary(result);
    }
    
    void HandleAction(SpectatorAction action)
    {
        Debug.Log($"{action.PlayerUsername}: {action.ActionType}");
        AnimateAction(action);
    }
}
```

**Unreal Example:**
```cpp
#include "Spectator/DeskillzSpectatorManager.h"

void AMySpectatorMode::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzSpectatorManager::Get()->Initialize();
    
    // Subscribe to events
    UDeskillzSpectatorManager::Get()->OnGameStateUpdated.AddDynamic(this, &AMySpectatorMode::HandleStateUpdate);
    UDeskillzSpectatorManager::Get()->OnRoundEnded.AddDynamic(this, &AMySpectatorMode::HandleRoundEnd);
    UDeskillzSpectatorManager::Get()->OnPlayerAction.AddDynamic(this, &AMySpectatorMode::HandleAction);
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

void AMySpectatorMode::JoinRoom(const FString& RoomId)
{
    UDeskillzSpectatorManager::Get()->JoinAsSpectator(RoomId,
        FOnJoinedSpectator::CreateLambda([this](const FSpectatorState& State) {
            UE_LOG(LogTemp, Log, TEXT("Spectating room: %s"), *State.RoomName);
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
    UE_LOG(LogTemp, Log, TEXT("Round %d winner: %s"), Result.RoundNumber, *Result.WinnerUsername);
    ShowRoundSummary(Result);
}

void AMySpectatorMode::HandleAction(const FSpectatorAction& Action)
{
    UE_LOG(LogTemp, Log, TEXT("%s: %s"), *Action.PlayerUsername, *Action.ActionType);
    AnimateAction(Action);
}
```

---

## Auto-Updater Classes

### UDeskillzUpdater / DeskillzUpdater
Automatic update checking and management.

| Method | Description |
|--------|-------------|
| `CheckForUpdates()` | Check for available updates |
| `GetUpdateInfo()` | Get current update information |
| `IsUpdateAvailable()` | Check if update is available |
| `IsForcedUpdate()` | Check if update is mandatory |
| `SkipVersion(Version)` | Skip specific version |
| `OpenUpdateUrl()` | Open download URL |
| `GetCurrentVersion()` | Get installed version |
| `GetLatestVersion()` | Get available version |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnUpdateAvailable` | FUpdateInfo Info | Optional update available |
| `OnForcedUpdateRequired` | FUpdateInfo Info | Mandatory update required |
| `OnNoUpdateAvailable` | None | App is up to date |
| `OnUpdateCheckFailed` | string Error | Update check failed |

---

## Analytics Classes

### UDeskillzAnalytics / DeskillzAnalytics
Analytics tracking.

| Method | Description |
|--------|-------------|
| `TrackEvent(Name, Category, Params)` | Track custom event |
| `TrackScreen(ScreenName)` | Track screen view |
| `TrackTiming(Category, Variable, TimeMs)` | Track timing |
| `SetUserId(UserId)` | Set user ID for analytics |
| `SetUserProperty(Name, Value)` | Set user property |

### UDeskillzEventTracker / DeskillzEventTracker
Extended event tracking.

| Method | Description |
|--------|-------------|
| `StartTimedEvent(Name)` | Start timed event |
| `EndTimedEvent(Name)` | End timed event |
| `TrackEntryFee(TournamentId, Currency, Amount)` | Track entry fee |
| `TrackPrizeWon(TournamentId, Currency, Amount)` | Track prize won |
| `IncrementCounter(Name)` | Increment counter |
| `GetCounter(Name)` | Get counter value |

---

## Platform Classes

### UDeskillzPlatform / DeskillzPlatform
Platform detection and info.

| Method | Description |
|--------|-------------|
| `IsMobile()` | Check if mobile platform |
| `IsDesktop()` | Check if desktop platform |
| `IsIOS()` | Check if iOS |
| `IsAndroid()` | Check if Android |
| `GetDeviceInfo()` | Get device information |
| `GetNetworkInfo()` | Get network information |
| `GetDeviceTier()` | Get device performance tier |

### UDeskillzDeepLink / DeskillzDeepLink
Deep link utilities.

| Method | Description |
|--------|-------------|
| `SetURLScheme(Scheme)` | Set URL scheme |
| `HandleDeepLink(URL)` | Handle incoming deep link |
| `GenerateTournamentLink(TournamentId)` | Generate tournament share link |
| `HasPendingDeepLink()` | Check for pending deep link |

### UDeskillzPushNotifications / DeskillzPushNotifications
Push notification handling.

| Method | Description |
|--------|-------------|
| `RequestPermission(Callback)` | Request notification permission |
| `GetPermissionStatus()` | Get permission status |
| `ScheduleLocalNotification(Title, Body, Delay)` | Schedule local notification |
| `CancelNotification(Id)` | Cancel scheduled notification |
| `SetBadgeCount(Count)` | Set app badge count |

---

## UI Classes

### UDeskillzUIManager / DeskillzUIManager
UI manager for built-in screens.

| Method | Description |
|--------|-------------|
| `ShowTournamentList()` | Show tournament list (deprecated in SDK 2.0) |
| `ShowMatchmaking()` | Show matchmaking (deprecated in SDK 2.0) |
| `ShowResults()` | Show results (deprecated in SDK 2.0) |
| `ShowWallet()` | Show wallet screen |
| `ShowLeaderboard(TournamentId)` | Show leaderboard |
| `ShowPopup(Title, Message)` | Show popup dialog |
| `ShowLoading(Message)` | Show loading overlay |

> **Note:** In SDK 2.0+, tournament browsing and matchmaking are handled by the main Deskillz app. Games receive match data via deep links.

### Host UI Components (NEW in v2.6)

| Component | Description |
|-----------|-------------|
| `HostDashboardUI` | Main host dashboard with tabs |
| `HostProfileCard` | Host profile display |
| `HostTierProgress` | Tier progression visualization |
| `HostBadgeGrid` | Badge collection display |
| `HostEarningsChart` | Earnings visualization |

### Social Game UI Components (NEW in v2.6)

| Component | Description |
|-----------|-------------|
| `BuyInModal` | Initial buy-in dialog |
| `RebuyModal` | Rebuy prompt with timer |
| `CashOutModal` | Cash out confirmation |
| `SocialGameSettings` | Point value and rake config |
| `TurnTimer` | Visual turn countdown |
| `PauseRequestModal` | Pause request and voting |

### Spectator UI Components (NEW in v2.6)

| Component | Description |
|-----------|-------------|
| `SpectatorView` | Main spectator container |
| `SpectatorGameBoard` | Game state visualization |
| `SpectatorScorePanel` | Live score display |
| `RoomSwitcher` | Multi-room navigation |

---

## Key Enums

### EDeskillzEnvironment
```cpp
enum class EDeskillzEnvironment : uint8
{
    Sandbox,      // Testing environment
    Production    // Live environment
};
```

### EDeskillzMatchStatus
```cpp
enum class EDeskillzMatchStatus : uint8
{
    Pending,      // Waiting to start
    Ready,        // Ready to play
    InProgress,   // Currently playing
    Completed,    // Match finished
    Cancelled     // Match cancelled
};
```

### EDeskillzTournamentStatus
```cpp
enum class EDeskillzTournamentStatus : uint8
{
    Upcoming,     // Not started yet
    Active,       // Currently running
    Completed,    // Finished
    Cancelled     // Cancelled
};
```

### EDeskillzMatchResult
```cpp
enum class EDeskillzMatchResult : uint8
{
    Win,          // Player won
    Loss,         // Player lost
    Draw,         // Tied
    Completed,    // Generic completion
    Forfeited     // Player forfeited
};
```

### EDeskillzNavigationAction (SDK 2.0+)
```cpp
enum class EDeskillzNavigationAction : uint8
{
    Tournaments,  // Show tournaments
    Wallet,       // Show wallet
    Profile,      // Show profile
    Game,         // Show specific game
    Settings      // Show settings
};
```

### ERoomMode
```cpp
enum class ERoomMode : uint8
{
    Sync,         // Synchronous (real-time) gameplay
    Async         // Asynchronous (turn-based) gameplay
};
```

### ERoomVisibility
```cpp
enum class ERoomVisibility : uint8
{
    PublicListed, // Visible in public room list
    PrivateCode   // Join by code only
};
```

### ERoomStatus
```cpp
enum class ERoomStatus : uint8
{
    Waiting,      // Waiting for players
    Ready,        // All players ready
    Starting,     // Match starting
    InProgress,   // Match in progress
    Completed,    // Room completed
    Cancelled     // Room cancelled
};
```

### EHostTier (NEW in v2.6)
```cpp
enum class EHostTier : uint8
{
    Starter,      // New host (50% revenue share)
    Bronze,       // 10+ rooms, $100+ revenue (55%)
    Silver,       // 25+ rooms, $500+ revenue (60%)
    Gold,         // 50+ rooms, $2,000+ revenue (65%)
    Platinum,     // 100+ rooms, $5,000+ revenue (70%)
    Diamond       // 250+ rooms, $15,000+ revenue (75%)
};
```

### EGameCategory (NEW in v2.6)
```cpp
enum class EGameCategory : uint8
{
    Esports,      // Tournament-style (entry fee)
    Social        // Cash game style (rake-based)
};
```

### ESettlementTrigger (NEW in v2.6)
```cpp
enum class ESettlementTrigger : uint8
{
    Threshold,    // Rake amount reached threshold
    Time,         // Time interval (30 min)
    Rounds,       // Round count (10 rounds)
    PlayerLeft,   // Player cashed out
    Manual,       // Host triggered
    SessionEnd    // Session ended
};
```

### EPlayerStatus (NEW in v2.6)
```cpp
enum class EPlayerStatus : uint8
{
    Active,       // Playing normally
    Busted,       // Out of chips, needs rebuy
    CashedOut,    // Left with winnings
    Disconnected  // Connection lost
};
```

### ESpectatorViewMode (NEW in v2.6)
```cpp
enum class ESpectatorViewMode : uint8
{
    Overview,     // See all players
    FollowPlayer, // Focus on one player
    Scoreboard    // Show scores only
};
```

---


---

## Tournament Manager (NEW in v3.5.2)

### UDeskillzTournamentManager_v352

Singleton. Access via `UDeskillzTournamentManager_v352::Get()`.

| Method | Description | Endpoint |
|--------|-------------|----------|
| `GetTournaments(Filters, OnSuccess, OnError)` | List tournaments with filters | GET /tournaments |
| `GetActiveTournaments(GameId, OnSuccess, OnError)` | Active tournaments for a game | GET /tournaments/game/:id/active |
| `Register(TournamentId, OnSuccess, OnError)` | Register for tournament | POST /tournaments/:id/register |
| `CheckIn(TournamentId, OnSuccess, OnError)` | Check in to tournament | POST /tournaments/:id/checkin |
| `Leave(TournamentId, OnSuccess, OnError)` | Leave/unregister | DELETE /tournaments/:id/leave |
| `GetEnrollmentStatus(TournamentId, OnSuccess, OnError)` | Get enrollment state | GET /tournaments/:id/my-status |
| `GetMyRegistrations(OnSuccess, OnError)` | All my registrations | GET /tournaments/my-registrations |
| `GetSchedule(TournamentId, OnSuccess, OnError)` | Bracket schedule | GET /tournaments/:id/schedule |
| `GetMyTableAssignment(TournamentId, OnSuccess, OnError)` | My table/seat | GET /tournaments/:id/my-seat |

**Delegates:** `OnTournamentStarted(FString)`, `OnTournamentLeft(FString)`

---

## Quick Play Manager (NEW in v3.5.2)

### UDeskillzQuickPlayManager_v352

Singleton. Access via `UDeskillzQuickPlayManager_v352::Get()`.

| Method | Description | Endpoint |
|--------|-------------|----------|
| `JoinQueue(Params, OnSuccess, OnError)` | Join matchmaking queue | POST /lobby/quick-play/join |
| `LeaveQueue(OnSuccess, OnError)` | Leave queue | POST /lobby/quick-play/leave |
| `GetConfig(GameId, OnSuccess, OnError)` | Get Quick Play config | GET /quick-play/games/:id |
| `LaunchMatch(MatchSessionId, OnSuccess, OnError)` | Launch matched game | POST /lobby/quick-play/match/launch |
| `SubmitScore(MatchId, Score, OnSuccess, OnError)` | Submit match score | POST /lobby/quick-play/match/:id/score |
| `GetMatchResults(MatchId, OnSuccess, OnError)` | Get final results | GET /lobby/quick-play/match/:id/results |
| `ForceCompleteMatch(MatchId, OnSuccess, OnError)` | Force completion | POST /lobby/quick-play/match/:id/complete |
| `CreateSocialRoom(...)` | Create social Quick Play room | POST /lobby/quick-play/social/create |
| `SubmitSocialRound(RoomId, PayloadJson, ...)` | Submit round result | POST /lobby/quick-play/social/:id/round |
| `SocialRebuy(RoomId, Amount, ...)` | Rebuy chips | POST /lobby/quick-play/social/:id/rebuy |
| `SocialCashOut(RoomId, ...)` | Cash out | POST /lobby/quick-play/social/:id/cashout |
| `EndSocialGame(RoomId, ...)` | End social game | POST /lobby/quick-play/social/:id/end |

**Properties:** `IsInQueue()`, `GetCurrentMatch()`
**Delegates:** `OnQueueMatched(FString)`, `OnQueueTimeout()`, `OnMatchCompleted(FString)`

---

## Dispute Manager (NEW in v3.5.2)

### UDeskillzDisputeManager_v352

Singleton. Access via `UDeskillzDisputeManager_v352::Get()`.

| Method | Description | Endpoint |
|--------|-------------|----------|
| `FileDispute(Params, OnSuccess, OnError)` | File a new dispute | POST /disputes |
| `GetMyDisputes(StatusFilter, OnSuccess, OnError)` | List my disputes | GET /disputes/me |
| `GetDisputeDetails(DisputeId, OnSuccess, OnError)` | Get dispute details | GET /disputes/:id |
| `AddEvidence(DisputeId, Evidence, OnSuccess, OnError)` | Add evidence | POST /disputes/:id/evidence |
| `GetRecentMatches(OnSuccess, OnError)` | Recent matches for context | GET /matches/history/me |
| `PersistLastMatch(Context)` | Save last match locally | Local file |
| `GetLastMatch()` | Load last match (7-day expiry) | Local file |
| `ClearLastMatch()` | Delete saved match | Local file |

---

## Room Extensions (NEW in v3.5.2)

### UDeskillzRoomExtensions_v352

Singleton. Access via `UDeskillzRoomExtensions_v352::Get()`.

| Method | Description | Endpoint |
|--------|-------------|----------|
| `CreateEsportRoom(Opts, OnSuccess, OnError)` | Create esport room with HostRole | POST /private-rooms/create |
| `CreateSocialRoom(Opts, OnSuccess, OnError)` | Create social room with win condition | POST /private-rooms/create |
| `BuyIn(RoomId, Amount, Currency, ...)` | Buy chips | POST /private-rooms/:id/buy-in |
| `CashOut(RoomId, ...)` | Cash out chips | POST /private-rooms/:id/cash-out |
| `Rebuy(RoomId, Amount, Currency, ...)` | Rebuy chips | POST /private-rooms/:id/rebuy |
| `SubmitRound(RoomId, PayloadJson, ...)` | Submit round | POST /private-rooms/:id/round |
| `TriggerSettlement(RoomId, ...)` | Trigger rake settlement | POST /private-rooms/:id/settle |
| `InvitePlayer(RoomId, Target, Message, ...)` | Invite player | POST /private-rooms/:id/invite |
| `GetMyInvites(OnSuccess, OnError)` | My pending invites | GET /private-rooms/invites/my |
| `RespondToInvite(InviteId, bAccept, ...)` | Accept/decline invite | POST /private-rooms/invites/:id/respond |

**Delegates:** `OnInviteReceived(FDeskillzRoomInvite)`, `OnBuyInComplete(double)`, `OnCashOutComplete(double)`

---

## Wallet Manager (NEW in v3.5.2)

### UDeskillzWalletManager_v352

Singleton. Access via `UDeskillzWalletManager_v352::Get()`.

| Method | Description | Endpoint |
|--------|-------------|----------|
| `GetBalance(OnSuccess, OnError)` | All currency balances | GET /wallet/balance |
| `GetBalanceForCurrency(Currency, OnSuccess, OnError)` | Single currency balance | GET /wallet/balance/:currency |
| `Deposit(Currency, Amount, ...)` | Initiate deposit | POST /wallet/deposit |
| `Withdraw(Currency, Amount, WalletAddress, ...)` | Initiate withdrawal | POST /wallet/withdraw |
| `GetPlayerStats(OnSuccess, OnError)` | Player statistics | GET /users/me |
| `GetMatchHistory(Page, Limit, OnSuccess, OnError)` | Match history | GET /matches/history/me |
| `GetGameLeaderboard(GameId, Period, Limit, ...)` | Game leaderboard | GET /leaderboard/:gameId |
| `GetTransactions(Limit, Offset, Type, Currency, ...)` | Transaction history | GET /wallet/transactions |

---

## Host Extensions (NEW in v3.5.2)

### UDeskillzHostExtensions_v352

Singleton. Access via `UDeskillzHostExtensions_v352::Get()`.

| Method | Description | Endpoint |
|--------|-------------|----------|
| `GetDashboard(OnSuccess, OnError)` | Composite dashboard | GET /host/dashboard |
| `WithdrawAllEarnings(OnSuccess, OnError)` | Withdraw all | POST /host/withdraw |
| `RequestWithdrawal(Amount, Currency, WalletAddress, ...)` | Parameterized withdrawal | POST /host/withdraw |
| `CheckAgeVerified(OnSuccess, OnError)` | Age verification status | GET /host/verify-age/status |
| `GetGameCapabilities(GameId, OnSuccess, OnError)` | Game capabilities | GET /games/:id/capabilities |

---

## Session Manager (NEW in v3.5.2)

### UDeskillzSessionManager_v352

Singleton. Access via `UDeskillzSessionManager_v352::Get()`.

| Method | Description | Endpoint |
|--------|-------------|----------|
| `ConsumeSSOToken()` | Consume SSO token from launch URL | GET /users/me |
| `CheckForActiveSession(OnResult)` | Check for active room/match | GET /private-rooms/my-active |
| `GetActiveSession()` | Get cached active session | Cached |
| `HasActiveSession()` | Whether active session exists | Cached |
| `EnableGuestMode()` | Enable guest mode | Local state |
| `DisableGuestMode()` | Disable guest mode | Local state |
| `IsGuest()` | Check guest status | Local state |
| `CanPerformAction(ActionName)` | Check if action allowed | Local state |

**Delegates:** `OnSSOAuthenticated(FDeskillzAuthUser)`, `OnSessionResumed(FDeskillzActiveSessionPayload)`, `OnGuestModeActivated()`

## Key Structs

### FDeskillzMatch
```cpp
struct FDeskillzMatch
{
    FString MatchId;
    FString TournamentId;
    EDeskillzMatchStatus Status;
    TArray<FDeskillzPlayerInfo> Players;
    float EntryFee;
    FString Currency;
};
```

### FDeskillzTournament
```cpp
struct FDeskillzTournament
{
    FString TournamentId;
    FString Name;
    float EntryFee;
    float PrizePool;
    EDeskillzTournamentStatus Status;
    int32 PlayerCount;
    int32 MaxPlayers;
};
```

### FDeskillzPlayerInfo
```cpp
struct FDeskillzPlayerInfo
{
    FString UserId;
    FString Username;
    int32 SkillRating;
    int32 Wins;
    int32 Losses;
    FString AvatarUrl;
};
```

### FDeskillzWalletBalance
```cpp
struct FDeskillzWalletBalance
{
    FString Currency;
    float Available;
    float Pending;
    float Total;
};
```

### FDeskillzMatchLaunchData (SDK 2.0+)
```cpp
struct FDeskillzMatchLaunchData
{
    FString MatchId;
    FString TournamentId;
    FString AuthToken;
    float EntryFee;
    FString Currency;
    EDeskillzMatchType MatchType;
    TArray<FDeskillzPlayerInfo> Opponents;
    int32 RandomSeed;
};
```

### FCreateRoomConfig
```cpp
struct FCreateRoomConfig
{
    FString Name;           // Room display name
    float EntryFee;         // Entry fee amount
    FString EntryCurrency;  // Currency (USDT, BNB, etc.)
    int32 MaxPlayers;       // Maximum players (2-8)
    int32 MinPlayers;       // Minimum to start (default: 2)
    ERoomMode Mode;         // Sync or Async
    ERoomVisibility Visibility; // Public or PrivateCode
};
```

### FPrivateRoom
```cpp
struct FPrivateRoom
{
    FString Id;             // Room unique ID
    FString RoomCode;       // Join code (DSKZ-XXXX)
    FString Name;           // Room display name
    FString HostId;         // Host player ID
    TArray<FRoomPlayer> Players; // Current players
    float EntryFee;         // Entry fee
    FString EntryCurrency;  // Currency
    ERoomStatus Status;     // Current status
    ERoomMode Mode;         // Game mode
    ERoomVisibility Visibility; // Visibility setting
    FDateTime CreatedAt;    // Creation timestamp
};
```

### FRoomPlayer
```cpp
struct FRoomPlayer
{
    FString Id;             // Player unique ID
    FString Username;       // Display name
    FString AvatarUrl;      // Avatar image URL
    bool bIsHost;           // Is room host
    bool bIsReady;          // Ready status
    FDateTime JoinedAt;     // Join timestamp
};
```

### FHostProfile (NEW in v2.6)
```cpp
struct FHostProfile
{
    FString UserId;         // User ID
    EHostTier Tier;         // Current tier
    FString TierName;       // Tier display name
    int32 Level;            // Host level (1-10)
    int32 XP;               // Experience points
    int32 XPToNextLevel;    // XP needed for next level
    float TotalEarnings;    // Lifetime earnings
    float PendingEarnings;  // Unsettled earnings
    int32 TotalRoomsHosted; // Rooms hosted count
    float AverageRating;    // Average player rating
    int32 TotalPlayersHosted; // Players hosted count
    bool bIsVerified;       // Age verified
    TArray<FHostBadge> Badges; // Earned badges
    FDateTime CreatedAt;    // Registration date
};
```

### FHostBadge (NEW in v2.6)
```cpp
struct FHostBadge
{
    FString Id;             // Badge ID
    FString Name;           // Badge name
    FString Description;    // Badge description
    FString IconUrl;        // Badge icon URL
    EBadgeCategory Category; // Achievement, Performance, Exclusive
    EBadgeRarity Rarity;    // Common to Legendary
    FDateTime EarnedAt;     // When earned
};
```

### FSocialGameSession (NEW in v2.6)
```cpp
struct FSocialGameSession
{
    FString Id;             // Session ID
    FString RoomId;         // Room ID
    int32 CurrentRound;     // Current round number
    float TotalPot;         // Current pot size
    float TotalRakeCollected; // Rake collected
    bool bIsPaused;         // Is game paused
    TArray<FSocialPlayer> Players; // Active players
    FDateTime StartedAt;    // Session start time
};
```

### FRakeDistribution (NEW in v2.6)
```cpp
struct FRakeDistribution
{
    float TotalRake;        // Total rake amount
    float HostShare;        // Host's share
    float PlatformShare;    // Platform's share
    float DeveloperShare;   // Developer's share
    float HostPercentage;   // Host percentage
};
```

### FSpectatorState (NEW in v2.6)
```cpp
struct FSpectatorState
{
    FString RoomId;         // Room being spectated
    FString RoomCode;       // Room code
    FString RoomName;       // Room name
    EGameCategory GameCategory; // Esports or Social
    int32 CurrentRound;     // Current round
    bool bIsPaused;         // Is game paused
    TArray<FSpectatorPlayer> Players; // Player info
    float TotalPot;         // Current pot (social)
    int32 SpectatorCount;   // Number of spectators
    FDateTime StartedAt;    // Game start time
};
```

### FSpectatorAction (NEW in v2.6)
```cpp
struct FSpectatorAction
{
    FString PlayerId;       // Player ID
    FString PlayerUsername; // Player name
    FString ActionType;     // Action type
    float Amount;           // Amount (if applicable)
    FDateTime Timestamp;    // When action occurred
};
```

---


### FDeskillzTournamentListing (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `Id` | FString | Tournament ID |
| `Name` | FString | Tournament name |
| `Status` | FString | Current status |
| `EntryFee` | double | Entry fee amount |
| `Currency` | FString | Fee currency |
| `PrizePool` | double | Total prize pool |
| `MaxPlayers` | int32 | Maximum participants |
| `CurrentPlayers` | int32 | Current participants |
| `SocialGameType` | FString | Social game type (if applicable) |
| `ScheduledStart` | FDateTime | Scheduled start time |

### FDeskillzQuickPlayConfig (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `GameId` | FString | Game identifier |
| `bEnabled` | bool | Whether Quick Play is enabled |
| `EntryFee` | double | Default entry fee |
| `MatchDurationSeconds` | int32 | Match duration |
| `SocialWinCondition` | FString | Win condition for social mode |
| `SocialPointTargets` | TArray<int32> | Available point targets |

### FDeskillzDisputeRecord (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `Id` | FString | Dispute ID |
| `MatchId` | FString | Disputed match |
| `Reason` | FString | Dispute reason |
| `Description` | FString | Detailed description |
| `Status` | FString | OPEN, UNDER_REVIEW, RESOLVED, REJECTED |
| `Evidence` | TArray<FString> | Evidence URLs |

### FDeskillzActiveSessionPayload (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `bHasActiveSession` | bool | Whether session exists |
| `Type` | FString | room, tournament, quickplay |
| `RoomId` | FString | Active room ID |
| `RoomCode` | FString | Room code for reconnect |

### FDeskillzGameCapabilities (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `bSupports1v1` | bool | 1v1 support |
| `bSupportsFFA` | bool | Free-for-all support |
| `MaxTournamentSize` | int32 | Max tournament players |
| `MinMatchDurationSeconds` | int32 | Min match duration |
| `MaxMatchDurationSeconds` | int32 | Max match duration |

### FDeskillzWalletBalanceEntry (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `Currency` | FString | Currency code (USDT_BSC, BNB, etc.) |
| `Symbol` | FString | Display symbol (USDT, BNB) |
| `Amount` | double | Available balance |
| `UsdValue` | double | USD equivalent |
| `Network` | FString | Blockchain network |

### FDeskillzHostDashboard (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `TotalEarnings` | double | All-time earnings |
| `EsportsEarnings` | double | Esports revenue |
| `SocialEarnings` | double | Social game revenue |
| `EsportsTier` | FString | Current esports tier |
| `SocialTier` | FString | Current social tier |
| `HostLevel` | int32 | Host level |
| `ActiveRoomCount` | int32 | Currently active rooms |

### FDeskillzRoomInvite (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| `Id` | FString | Invite ID |
| `RoomCode` | FString | Room code |
| `RoomName` | FString | Room name |
| `SenderUsername` | FString | Who sent the invite |
| `EntryFee` | double | Room entry fee |
| `CurrentPlayers` | int32 | Players in room |
| `MaxPlayers` | int32 | Max room capacity |

## Version History

### v3.5.2 (April 2026)

- Added UDeskillzTournamentManager_v352 (register, check-in, bracket, table assignment)
- Added UDeskillzQuickPlayManager_v352 (esport queue + social cash games)
- Added UDeskillzDisputeManager_v352 (file disputes, add evidence, last match)
- Added UDeskillzRoomExtensions_v352 (buy-in, cash-out, invites, settlement, host role)
- Added UDeskillzWalletManager_v352 (multi-currency balance, deposit, withdraw, stats, history)
- Added UDeskillzHostExtensions_v352 (composite dashboard, withdrawal, age verify, capabilities)
- Added UDeskillzSessionManager_v352 (SSO token, active session resume, guest mode)
- Added DeskillzTypes_v352.h (8 enums, 21 structs, currency helper, all v3.5.2 endpoints)
- Updated Deskillz.uplugin to version 3.5.2
- Corrected host revenue shares to 15-28% (was incorrectly 50-75%)
- Corrected supported currencies to BNB, USDT, USDC on BSC/TRON only

| Version | Date | Changes |
|---------|------|---------|
| 2.8.0 | Jan 2026 | Self-Sufficient Authentication (DeskillzAuth, AuthSceneController), email/password login, social login, optional wallet, scene navigation |
| 2.7.0 | Jan 2026 | Self-Sufficient Architecture introduction |
| 2.6.0 | Jan 2026 | Added Host System, Social Games, Spectator Mode |
| 2.5.0 | Jan 2025 | Added Auto-Updater (DeskillzUpdater, DeskillzUpdaterUI) |
| 2.2.0 | Dec 2024 | Added Private Rooms (UDeskillzRooms, UDeskillzPrivateRoomUI) |
| 2.1.0 | Dec 2024 | Navigation deep links, improved lobby integration |
| 2.0.0 | Nov 2024 | Centralized lobby architecture, deep link handler |
| 1.x | Legacy | SDK-based matchmaking (deprecated) |

---

## Resources

- [Integration Guide](INTEGRATION_GUIDE.md) - Detailed integration examples
- [Quick Start](QUICKSTART.md) - Getting started guide
- [Unity SDK Repository](https://github.com/Deskillz-Games-Development/unity-sdk)
- [Unreal SDK Repository](https://github.com/Deskillz-Games-Development/unreal-sdk)
- [Sample Projects](https://github.com/Deskillz-Games-Development)
- [Developer Portal](https://developer.deskillz.games)
- [Documentation](https://docs.deskillz.games)