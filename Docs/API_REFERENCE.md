# Deskillz SDK - API Reference

**SDK Version: 2.3.0** | Complete API documentation for the Deskillz Unity and Unreal Engine SDKs.

## Table of Contents

- [Core Classes](#core-classes)
- [Match Classes](#match-classes)
- [Security Classes](#security-classes)
- [Network Classes](#network-classes)
- [Deep Link Classes](#deep-link-classes)
- [Private Rooms Classes](#private-rooms-classes)
- [Auto-Updater Classes](#auto-updater-classes) ← NEW in v2.3
- [Analytics Classes](#analytics-classes)
- [Platform Classes](#platform-classes)
- [UI Classes](#ui-classes)
- [Enums](#key-enums)
- [Structs](#key-structs)

---

## Core Classes

### UDeskillzSDK
Main SDK singleton.

| Method | Description |
|--------|-------------|
| `Get()` | Get singleton instance |
| `Initialize(Config)` | Initialize SDK with configuration |
| `Shutdown()` | Clean up SDK resources |
| `IsInitialized()` | Check if SDK is ready |
| `GetGameId()` | Get current game ID |
| `GetSDKVersion()` | Get SDK version string |

### FDeskillzConfig  
Configuration struct.

| Property | Type | Description |
|----------|------|-------------|
| `GameId` | FString | Your game's unique ID |
| `ApiKey` | FString | Your API key |
| `Environment` | EDeskillzEnvironment | Sandbox or Production |
| `BaseUrl` | FString | API base URL (optional) |
| `ApiTimeout` | float | Request timeout in seconds |
| `bEnableLogging` | bool | Enable debug logging |
| `bEnableAnalytics` | bool | Enable analytics tracking |
| `bEnableAntiCheat` | bool | Enable anti-cheat protection |

### UDeskillzEvents
Event dispatcher.

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnSDKInitialized` | None | SDK ready to use |
| `OnSDKError` | FString Error | SDK error occurred |
| `OnAuthStateChanged` | bool bAuthenticated | Auth state changed |
| `OnConnectionStateChanged` | bool bConnected | Connection state changed |
| `OnWalletUpdated` | TArray<FDeskillzWalletBalance> | Wallet balances updated |

---

## Match Classes

### UDeskillzMatchmaking
Matchmaking manager.

| Method | Description |
|--------|-------------|
| `StartMatchmaking(TournamentId)` | Start searching for match |
| `CancelMatchmaking()` | Cancel matchmaking |
| `IsMatchmaking()` | Check if currently matchmaking |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnMatchFound` | FDeskillzMatch | Match found |
| `OnMatchmakingFailed` | FString Reason | Matchmaking failed |
| `OnMatchmakingCancelled` | None | Matchmaking cancelled |

### UDeskillzMatchManager
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

### UDeskillzScoreEncryption
Score encryption utilities.

| Method | Description |
|--------|-------------|
| `EncryptScore(Score, MatchId, UserId)` | Encrypt score for submission |
| `DecryptScore(EncryptedData)` | Decrypt score data |
| `GenerateHMAC(Data)` | Generate HMAC signature |
| `VerifyHMAC(Data, HMAC)` | Verify HMAC signature |

### UDeskillzSecureSubmitter
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

### UDeskillzAntiCheat
Anti-cheat protection.

| Method | Description |
|--------|-------------|
| `ValidateScore(Score, Duration)` | Validate score locally |
| `IsSpeedHackDetected()` | Check for speed hacks |
| `CheckMemoryIntegrity()` | Verify memory integrity |
| `GenerateSubmissionId()` | Generate unique submission ID |

---

## Network Classes

### UDeskillzHttpClient
HTTP client for API calls.

| Method | Description |
|--------|-------------|
| `Get(Endpoint, Callback)` | HTTP GET request |
| `Post(Endpoint, Body, Callback)` | HTTP POST request |
| `Put(Endpoint, Body, Callback)` | HTTP PUT request |
| `Delete(Endpoint, Callback)` | HTTP DELETE request |
| `SetAuthToken(Token)` | Set authentication token |

### UDeskillzWebSocket
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
| `OnMessageReceived` | FString Event, TSharedPtr<FJsonObject> | Message received |
| `OnError` | FString Error | WebSocket error |

### UDeskillzApiService
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

### UDeskillzDeepLinkHandler
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
| `OnValidationFailed` | FString Reason, FDeskillzMatchLaunchData | Launch validation failed |
| `OnNavigationReceived` | EDeskillzNavigationAction, TMap<FString, FString> | Navigation deep link received |
| `OnMatchLaunchReceived` | FString MatchId, FString AuthToken | Simplified match launch event |

### UDeskillzBridge
Bridge for communication with main Deskillz app.

| Method | Description |
|--------|-------------|
| `SubmitScore(Score, Callback)` | Submit score and handle return |
| `ReturnToMainApp(Destination)` | Return to main app |
| `GetMatchLaunchData()` | Get match launch data |
| `IsLaunchedFromMainApp()` | Check if launched via deep link |

---

## Private Rooms Classes (NEW in v2.2)

### UDeskillzRooms
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
| `OnPlayerLeft` | FString PlayerId | Player left room |
| `OnPlayerReadyChanged` | FString PlayerId, bool bReady | Player ready status changed |
| `OnMatchLaunching` | FString MatchId | Match is starting |
| `OnChatReceived` | FString PlayerId, FString Message | Chat message received |
| `OnKicked` | FString Reason | You were kicked from room |
| `OnRoomCancelled` | None | Room was cancelled by host |

**Unity Example:**
```csharp
using Deskillz.Rooms;

// Create a room
var config = new CreateRoomConfig {
    Name = "My Room",
    EntryFee = 5.0m,
    EntryCurrency = "USDT",
    MaxPlayers = 4,
    Mode = RoomMode.Sync,
    Visibility = RoomVisibility.PrivateCode
};

DeskillzRooms.CreateRoom(config, 
    room => Debug.Log($"Room created: {room.RoomCode}"),
    error => Debug.LogError(error)
);

// Subscribe to events
DeskillzRooms.OnMatchLaunching += matchId => {
    Debug.Log($"Match starting: {matchId}");
};
```

**Unreal Example:**
```cpp
#include "Rooms/DeskillzRooms.h"

// Create a room
FCreateRoomConfig Config;
Config.Name = TEXT("My Room");
Config.EntryFee = 5.0f;
Config.EntryCurrency = TEXT("USDT");
Config.MaxPlayers = 4;
Config.Mode = ERoomMode::Sync;
Config.Visibility = ERoomVisibility::PrivateCode;

UDeskillzRooms::Get()->CreateRoom(Config,
    FOnRoomCreated::CreateLambda([](const FPrivateRoom& Room) {
        UE_LOG(LogTemp, Log, TEXT("Room created: %s"), *Room.RoomCode);
    }),
    FOnRoomError::CreateLambda([](const FString& Error) {
        UE_LOG(LogTemp, Error, TEXT("Failed: %s"), *Error);
    })
);

// Subscribe to events
UDeskillzRooms::Get()->OnMatchLaunching.AddDynamic(this, &AMyGameMode::OnMatchStart);
```

### UDeskillzPrivateRoomUI
Pre-built UI components for private rooms.

| Method | Description |
|--------|-------------|
| `ShowRoomList()` | Show room browser/list |
| `ShowCreateRoom()` | Show room creation form |
| `ShowJoinRoom()` | Show join by code dialog |
| `ShowRoomLobby(Room)` | Show room lobby with players |
| `HideAll()` | Hide all room UI |
| `QuickJoinRoom(RoomCode)` | Quickly join room without UI |

**Unity Example:**
```csharp
using Deskillz.Rooms.UI;

// Show room browser
PrivateRoomUI.Instance.ShowRoomList();

// Show create room form
PrivateRoomUI.Instance.ShowCreateRoom();

// Quick join by code
PrivateRoomUI.Instance.QuickJoinRoom("DSKZ-ABCD");
```

**Unreal Example:**
```cpp
#include "Rooms/UI/DeskillzPrivateRoomUI.h"

// Show room browser
UDeskillzPrivateRoomUI::Get()->ShowRoomList();

// Show create room form
UDeskillzPrivateRoomUI::Get()->ShowCreateRoom();

// Quick join by code
UDeskillzPrivateRoomUI::Get()->QuickJoinRoom(TEXT("DSKZ-ABCD"));
```

### UI Widget Files

**Unity (6 files):**
- `Runtime/Rooms/UI/PrivateRoomUI.cs`
- `Runtime/Rooms/UI/RoomListUI.cs`
- `Runtime/Rooms/UI/CreateRoomUI.cs`
- `Runtime/Rooms/UI/JoinRoomUI.cs`
- `Runtime/Rooms/UI/RoomLobbyUI.cs`
- `Runtime/Rooms/UI/RoomPlayerCard.cs`

**Unreal (12 files):**
- `Source/Deskillz/Public/Rooms/UI/DeskillzPrivateRoomUI.h`
- `Source/Deskillz/Public/Rooms/UI/DeskillzRoomListWidget.h`
- `Source/Deskillz/Public/Rooms/UI/DeskillzCreateRoomWidget.h`
- `Source/Deskillz/Public/Rooms/UI/DeskillzJoinRoomWidget.h`
- `Source/Deskillz/Public/Rooms/UI/DeskillzRoomLobbyWidget.h`
- `Source/Deskillz/Public/Rooms/UI/DeskillzRoomPlayerCard.h`
- (Plus corresponding .cpp files in Private/)

---

## Analytics Classes

### UDeskillzAnalytics
Analytics tracking.

| Method | Description |
|--------|-------------|
| `TrackEvent(Name, Category, Params)` | Track custom event |
| `TrackScreenView(Screen)` | Track screen view |
| `TrackSessionStart()` | Track session start |
| `TrackSessionEnd()` | Track session end |
| `SetUserProperty(Key, Value)` | Set user property |
| `Flush()` | Flush pending events |

### UDeskillzTelemetry
Performance telemetry.

| Method | Description |
|--------|-------------|
| `StartMonitoring()` | Start performance monitoring |
| `StopMonitoring()` | Stop monitoring |
| `RecordMetric(Type, Name, Value)` | Record custom metric |
| `GetCurrentFPS()` | Get current FPS |
| `GetMemoryUsage()` | Get memory usage |

### UDeskillzEventTracker
Event tracking utilities.

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

### UDeskillzPlatform
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

### UDeskillzDeepLink
Deep link utilities.

| Method | Description |
|--------|-------------|
| `SetURLScheme(Scheme)` | Set URL scheme |
| `HandleDeepLink(URL)` | Handle incoming deep link |
| `GenerateTournamentLink(TournamentId)` | Generate tournament share link |
| `HasPendingDeepLink()` | Check for pending deep link |

### UDeskillzPushNotifications
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

### UDeskillzUIManager
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

### ERoomMode (NEW in v2.2)
```cpp
enum class ERoomMode : uint8
{
    Sync,         // Synchronous (real-time) gameplay
    Async         // Asynchronous (turn-based) gameplay
};
```

### ERoomVisibility (NEW in v2.2)
```cpp
enum class ERoomVisibility : uint8
{
    PublicListed, // Visible in public room list
    PrivateCode   // Join by code only
};
```

### ERoomStatus (NEW in v2.2)
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

### FCreateRoomConfig (NEW in v2.2)
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

### FPrivateRoom (NEW in v2.2)
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

### FRoomPlayer (NEW in v2.2)
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

## Auto-Updater Classes ← NEW in v2.3

### UDeskillzUpdater (Unreal)
Automatic update checker singleton.

| Method | Description |
|--------|-------------|
| `Get()` | Get singleton instance |
| `SetCurrentVersion(Version, VersionCode)` | Set current app version for comparison |
| `CheckForUpdates()` | Check server for new version |
| `GetLatestVersion()` | Get latest available version string |
| `IsUpdateAvailable()` | Check if update exists |
| `IsUpdateRequired()` | Check if update is forced/mandatory |
| `OpenDownloadPage()` | Open APK download URL in browser |
| `SkipVersion(Version)` | Mark version as skipped (optional only) |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnUpdateCheckStarted` | None | Update check has begun |
| `OnUpdateAvailable` | FUpdateInfo | Optional update available |
| `OnForceUpdateRequired` | FUpdateInfo | Mandatory update required |
| `OnNoUpdateNeeded` | None | App is up to date |
| `OnUpdateCheckFailed` | FString Error | Check failed (network/parse error) |
| `OnUpdateAccepted` | FUpdateInfo | User accepted update |
| `OnUpdateSkipped` | FUpdateInfo | User skipped optional update |

### DeskillzUpdater (Unity)
Unity auto-updater singleton.

| Property | Type | Description |
|----------|------|-------------|
| `Instance` | DeskillzUpdater | Singleton instance |
| `CurrentVersion` | string | Current app version string |
| `CurrentVersionCode` | int | Current app version code |

| Method | Description |
|--------|-------------|
| `CheckForUpdates()` | Check server for new version |
| `StartUpdate()` | Open download page in browser |
| `SkipUpdate()` | Skip current optional update |

| Event | Parameters | Description |
|-------|------------|-------------|
| `OnUpdateCheckStarted` | None | Check started |
| `OnUpdateAvailable` | UpdateInfo | Optional update ready |
| `OnForcedUpdateRequired` | UpdateInfo | Must update to continue |
| `OnNoUpdateNeeded` | None | Already on latest |
| `OnUpdateCheckFailed` | string | Error message |
| `OnUpdateAccepted` | UpdateInfo | User clicked Update |
| `OnUpdateSkipped` | UpdateInfo | User clicked Skip |

### DeskillzUpdaterUI (Unity)
Pre-built update dialog UI.

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `Instance` | DeskillzUpdaterUI | - | Singleton instance |
| `ShowOnUpdateAvailable` | bool | true | Auto-show for optional updates |
| `BlockOnForcedUpdate` | bool | true | Block app for forced updates |
| `UpdateTitle` | string | "Update Available" | Dialog title |
| `UpdateButtonText` | string | "Update Now" | Update button text |
| `SkipButtonText` | string | "Later" | Skip button text |
| `Theme` | UpdateUITheme | Dark | Dialog theme |

| Method | Description |
|--------|-------------|
| `ShowUpdateDialog(info, blocking)` | Manually show update dialog |
| `HideDialog()` | Hide current dialog |

### FUpdateInfo / UpdateInfo
Update information struct.

| Property | Type | Description |
|----------|------|-------------|
| `LatestVersion` | string | Latest version string (e.g., "1.2.0") |
| `VersionCode` | int | Numeric version code (e.g., 10200) |
| `UpdateAvailable` | bool | Whether an update exists |
| `IsForced` | bool | Whether update is mandatory |
| `DownloadUrl` | string | APK download URL |
| `FileSize` | long | File size in bytes |
| `FileSizeFormatted` | string | Human-readable size (e.g., "52.4 MB") |
| `ReleaseNotes` | string | Changelog/release notes text |

### Usage Example (Unreal)
```cpp
#include "Core/DeskillzUpdater.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UDeskillzUpdater* Updater = UDeskillzUpdater::Get();
    Updater->SetCurrentVersion(TEXT("1.0.0"), 10000);
    
    Updater->OnUpdateAvailable.AddDynamic(this, &AMyGameMode::HandleOptionalUpdate);
    Updater->OnForceUpdateRequired.AddDynamic(this, &AMyGameMode::HandleForcedUpdate);
    Updater->OnNoUpdateNeeded.AddDynamic(this, &AMyGameMode::HandleNoUpdate);
    Updater->OnUpdateCheckFailed.AddDynamic(this, &AMyGameMode::HandleCheckFailed);
    
    Updater->CheckForUpdates();
}

void AMyGameMode::HandleOptionalUpdate(const FUpdateInfo& Info)
{
    UE_LOG(LogTemp, Log, TEXT("Update available: %s (%s)"), 
        *Info.LatestVersion, *Info.FileSizeFormatted);
    // Show optional update UI
}

void AMyGameMode::HandleForcedUpdate(const FUpdateInfo& Info)
{
    UE_LOG(LogTemp, Warning, TEXT("Required update: %s"), *Info.LatestVersion);
    // Show blocking update UI - user must update
}
```

### Usage Example (Unity)
```csharp
using Deskillz;

public class UpdateManager : MonoBehaviour
{
    void Start()
    {
        DeskillzUpdater updater = DeskillzUpdater.Instance;
        updater.CurrentVersion = Application.version;
        updater.CurrentVersionCode = GetVersionCode();
        
        DeskillzUpdater.OnUpdateAvailable += HandleOptionalUpdate;
        DeskillzUpdater.OnForcedUpdateRequired += HandleForcedUpdate;
        DeskillzUpdater.OnNoUpdateNeeded += () => Debug.Log("Up to date!");
        
        updater.CheckForUpdates();
    }
    
    void HandleOptionalUpdate(UpdateInfo info)
    {
        Debug.Log($"Update available: {info.LatestVersion} ({info.FileSizeFormatted})");
        // Show optional update dialog
    }
    
    void HandleForcedUpdate(UpdateInfo info)
    {
        Debug.Log($"Required update: {info.LatestVersion}");
        // Show blocking dialog - must update to continue
    }
    
    int GetVersionCode()
    {
        string[] parts = Application.version.Split('.');
        return int.Parse(parts[0]) * 10000 + 
               int.Parse(parts[1]) * 100 + 
               int.Parse(parts[2]);
    }
}
---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 2.3.0 | Jan 2025 | Added Auto-Updater (DeskillzUpdater, DeskillzUpdaterUI) |
| 2.2.0 | Dec 2024 | Added Private Rooms (UDeskillzRooms, UDeskillzPrivateRoomUI) |
| 2.1.0 | Dec 2024 | Navigation deep links, improved lobby integration |
| 2.0.0 | Nov 2024 | Centralized lobby architecture, deep link handler |
| 1.x | Legacy | SDK-based matchmaking (deprecated) |
---

See [Integration Guide](INTEGRATION_GUIDE.md) for detailed examples and [Quick Start](QUICKSTART.md) for getting started.