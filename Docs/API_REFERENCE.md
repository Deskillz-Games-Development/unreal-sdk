# Deskillz SDK - API Reference

Complete API documentation for the Deskillz Unreal Engine SDK.

## Table of Contents

- [Core Classes](#core-classes)
- [Match Classes](#match-classes)
- [Security Classes](#security-classes)
- [Network Classes](#network-classes)
- [Deep Link Classes](#deep-link-classes) ← NEW
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
| `EncryptScore(Score, MatchId, UserId)` | Encrypt score data |
| `DecryptScore(...)` | Decrypt score data |
| `GenerateHMAC(Data)` | Generate HMAC signature |
| `VerifyHMAC(Data, HMAC)` | Verify HMAC signature |

### UDeskillzSecureSubmitter
Secure score submission.

| Method | Description |
|--------|-------------|
| `SubmitScore(Score, Duration)` | Submit encrypted score |
| `SubmitScoreWithMetadata(...)` | Submit with extra data |
| `IsSubmitting()` | Check if submission in progress |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnScoreSubmitted` | bool bSuccess, FString Message | Submission complete |
| `OnScoreValidated` | bool bValid | Server validation result |

### UDeskillzAntiCheat
Anti-cheat protection.

| Method | Description |
|--------|-------------|
| `ValidateScore(Score, Duration)` | Validate score locally |
| `IsSpeedHackDetected()` | Check for speed hacks |
| `CheckMemoryIntegrity()` | Check memory tampering |
| `GenerateSubmissionId()` | Generate unique submission ID |

---

## Network Classes

### UDeskillzHttpClient
HTTP request client.

| Method | Description |
|--------|-------------|
| `Get(Endpoint, Callback)` | Make GET request |
| `Post(Endpoint, Body, Callback)` | Make POST request |
| `Put(Endpoint, Body, Callback)` | Make PUT request |
| `Delete(Endpoint, Callback)` | Make DELETE request |
| `SetAuthToken(Token)` | Set authorization token |

### UDeskillzWebSocket
WebSocket client for real-time communication.

| Method | Description |
|--------|-------------|
| `Connect(Url)` | Connect to WebSocket server |
| `Disconnect()` | Disconnect from server |
| `IsConnected()` | Check connection status |
| `JoinRoom(RoomId)` | Join a room |
| `LeaveRoom(RoomId)` | Leave a room |
| `SendMessage(Event, Data)` | Send message to room |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnConnected` | None | Connected to server |
| `OnDisconnected` | None | Disconnected from server |
| `OnMessageReceived` | FString Event, TSharedPtr<FJsonObject> | Message received |
| `OnError` | FString Error | Error occurred |

### UDeskillzApiService
High-level API service.

| Method | Description |
|--------|-------------|
| `Login(...)` | Authenticate user |
| `Register(...)` | Register new user |
| `Logout()` | Log out user |
| `GetTournaments(...)` | Fetch tournament list |
| `EnterTournament(...)` | Enter a tournament |
| `GetWalletBalance(...)` | Get wallet balances |
| `GetLeaderboard(...)` | Fetch leaderboard |
| `GetMatchDetails(MatchId, ...)` | Get match details |

---

## Deep Link Classes (NEW in v2.0)

### UDeskillzDeepLinkHandler
Handles deep link navigation from the Deskillz platform.

| Method | Description |
|--------|-------------|
| `Get()` | Get singleton instance |
| `Initialize()` | Initialize deep link handler |
| `HandleDeepLink(Url)` | Process a deep link URL |
| `ProcessPendingDeepLinks()` | Process any pending deep links |
| `SimulateDeepLink(Url)` | Simulate a deep link (testing) |
| `IsNavigationDeepLink(Url)` | Check if URL is navigation type |
| `IsMatchLaunchDeepLink(Url)` | Check if URL is match launch type |
| `ParseNavigationLink(Url)` | Parse navigation parameters |
| `ParseMatchLaunchLink(Url)` | Parse match launch parameters |

| Delegate | Parameters | Description |
|----------|------------|-------------|
| `OnNavigationReceived` | EDeskillzNavigationAction, TMap<FString,FString> | Navigation request received |
| `OnMatchLaunchReceived` | FString MatchId, FString AuthToken | Match launch request received |

**Example Usage:**

```cpp
// Subscribe to events
UDeskillzDeepLinkHandler* Handler = UDeskillzDeepLinkHandler::Get();
Handler->OnNavigationReceived.AddDynamic(this, &AMyGame::HandleNavigation);
Handler->OnMatchLaunchReceived.AddDynamic(this, &AMyGame::HandleMatchLaunch);

// Handle navigation
void AMyGame::HandleNavigation(EDeskillzNavigationAction Action, const TMap<FString, FString>& Params)
{
    switch (Action)
    {
        case EDeskillzNavigationAction::Tournaments:
            ShowTournaments();
            break;
        case EDeskillzNavigationAction::Wallet:
            ShowWallet();
            break;
        // ... etc
    }
}

// Handle match launch
void AMyGame::HandleMatchLaunch(const FString& MatchId, const FString& Token)
{
    UDeskillzApiService::Get()->SetAuthToken(Token);
    LoadMatch(MatchId);
}
```

### Unity Equivalent: DeepLinkHandler

```csharp
// Subscribe to events
DeepLinkHandler.OnNavigationReceived += HandleNavigation;
DeepLinkHandler.OnMatchLaunchReceived += HandleMatchLaunch;

// Handle navigation
void HandleNavigation(NavigationAction action, Dictionary<string, string> parameters)
{
    switch (action)
    {
        case NavigationAction.Tournaments:
            ShowTournaments();
            break;
        case NavigationAction.Wallet:
            ShowWallet();
            break;
        // ... etc
    }
}

// Handle match launch  
void HandleMatchLaunch(string matchId, string token)
{
    DeskillzApi.SetAuthToken(token);
    LoadMatch(matchId);
}
```

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
| `StopMonitoring()` | Stop performance monitoring |
| `RecordMetric(Type, Name, Value)` | Record custom metric |
| `GetCurrentFPS()` | Get current FPS |
| `GetMemoryUsage()` | Get memory usage |

### UDeskillzEventTracker
Event tracking utilities.

| Method | Description |
|--------|-------------|
| `StartTimedEvent(Name)` | Start timed event |
| `EndTimedEvent(Name)` | End timed event |
| `TrackEntryFee(...)` | Track tournament entry |
| `TrackPrizeWon(...)` | Track prize won |
| `IncrementCounter(Name)` | Increment counter |
| `GetCounter(Name)` | Get counter value |

---

## Platform Classes

### UDeskillzPlatform
Platform utilities.

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
Deep link utilities (legacy - see UDeskillzDeepLinkHandler for v2.0).

| Method | Description |
|--------|-------------|
| `SetURLScheme(Scheme)` | Set URL scheme |
| `HandleDeepLink(URL)` | Handle incoming deep link |
| `GenerateTournamentLink(TournamentId)` | Generate shareable link |
| `HasPendingDeepLink()` | Check for pending links |

### UDeskillzPushNotifications
Push notification management.

| Method | Description |
|--------|-------------|
| `RequestPermission(Callback)` | Request notification permission |
| `GetPermissionStatus()` | Get current permission status |
| `ScheduleLocalNotification(...)` | Schedule local notification |
| `CancelNotification(Id)` | Cancel notification |
| `SetBadgeCount(Count)` | Set app badge count |

---

## UI Classes

### UDeskillzUIManager
UI management.

| Method | Description |
|--------|-------------|
| `ShowTournamentList()` | Show tournament browser |
| `ShowMatchmaking()` | Show matchmaking screen |
| `ShowResults()` | Show match results |
| `ShowWallet()` | Show wallet screen |
| `ShowProfile()` | Show user profile |
| `ShowLeaderboard(TournamentId)` | Show leaderboard |
| `ShowPopup(Title, Message)` | Show popup dialog |
| `ShowLoading(Message)` | Show loading indicator |
| `HideLoading()` | Hide loading indicator |

---

## Key Enums

### EDeskillzEnvironment
| Value | Description |
|-------|-------------|
| `Sandbox` | Test environment (no real money) |
| `Production` | Live environment |

### EDeskillzMatchStatus
| Value | Description |
|-------|-------------|
| `Pending` | Match not started |
| `Ready` | Match ready to start |
| `InProgress` | Match in progress |
| `Completed` | Match completed |
| `Cancelled` | Match cancelled |

### EDeskillzTournamentStatus
| Value | Description |
|-------|-------------|
| `Upcoming` | Tournament not started |
| `Active` | Tournament active |
| `Completed` | Tournament completed |
| `Cancelled` | Tournament cancelled |

### EDeskillzMatchResult
| Value | Description |
|-------|-------------|
| `Win` | Player won |
| `Loss` | Player lost |
| `Draw` | Match was a draw |
| `Completed` | Match completed (async) |
| `Forfeited` | Player forfeited |

### EDeskillzNavigationAction (NEW)
| Value | Description |
|-------|-------------|
| `None` | No action |
| `Tournaments` | Navigate to tournaments |
| `Wallet` | Navigate to wallet |
| `Profile` | Navigate to profile |
| `Game` | Navigate to game details |
| `Settings` | Navigate to settings |

---

## Key Structs

### FDeskillzMatch
| Property | Type | Description |
|----------|------|-------------|
| `MatchId` | FString | Unique match ID |
| `TournamentId` | FString | Parent tournament ID |
| `Status` | EDeskillzMatchStatus | Current status |
| `Players` | TArray<FDeskillzPlayerInfo> | Match participants |
| `EntryFee` | float | Entry fee amount |
| `Currency` | FString | Currency code |

### FDeskillzTournament
| Property | Type | Description |
|----------|------|-------------|
| `TournamentId` | FString | Unique tournament ID |
| `Name` | FString | Tournament name |
| `EntryFee` | float | Entry fee |
| `PrizePool` | float | Total prize pool |
| `Status` | EDeskillzTournamentStatus | Current status |

### FDeskillzPlayerInfo
| Property | Type | Description |
|----------|------|-------------|
| `UserId` | FString | User ID |
| `Username` | FString | Display name |
| `SkillRating` | int32 | ELO rating |
| `Wins` | int32 | Total wins |
| `Losses` | int32 | Total losses |

### FDeskillzWalletBalance
| Property | Type | Description |
|----------|------|-------------|
| `Currency` | FString | Currency code |
| `Available` | float | Available balance |
| `Pending` | float | Pending balance |
| `Total` | float | Total balance |

### FDeskillzNavigationParams (NEW)
| Property | Type | Description |
|----------|------|-------------|
| `Action` | EDeskillzNavigationAction | Navigation action |
| `GameId` | FString | Target game ID (if applicable) |
| `TournamentId` | FString | Target tournament ID (if applicable) |
| `Parameters` | TMap<FString,FString> | Additional parameters |

---

See [Integration Guide](INTEGRATION_GUIDE.md) for detailed examples.