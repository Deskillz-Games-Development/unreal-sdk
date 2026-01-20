# Deskillz SDK - API Reference

**SDK Version: 2.6.0** | Complete API documentation for the Deskillz Unity and Unreal Engine SDKs.

## Table of Contents

- [Getting Started](#getting-started) - NEW in v2.6
- [Developer Portal REST API](#developer-portal-rest-api) - NEW in v2.6
- [Core Classes](#core-classes)
- [Match Classes](#match-classes)
- [Security Classes](#security-classes)
- [Network Classes](#network-classes)
- [Deep Link Classes](#deep-link-classes)
- [Private Rooms Classes](#private-rooms-classes)
- [Host System Classes](#host-system-classes) - NEW in v2.6
- [Social Game Classes](#social-game-classes) - NEW in v2.6
- [Spectator Classes](#spectator-classes) - NEW in v2.6
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

## Version History

| Version | Date | Changes |
|---------|------|---------|
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