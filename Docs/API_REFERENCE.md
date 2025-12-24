# Deskillz SDK - API Reference

Complete API documentation for the Deskillz Unreal Engine SDK.

## Core Classes

### UDeskillzSDK
Main SDK singleton. Methods: `Get()`, `Initialize(Config)`, `Shutdown()`, `IsInitialized()`, `GetGameId()`, `GetSDKVersion()`

### FDeskillzConfig  
Configuration struct with: `GameId`, `ApiKey`, `Environment`, `BaseUrl`, `ApiTimeout`, `bEnableLogging`, `bEnableAnalytics`, `bEnableAntiCheat`

### UDeskillzEvents
Event dispatcher. Delegates: `OnSDKInitialized`, `OnSDKError`, `OnAuthStateChanged`, `OnConnectionStateChanged`, `OnWalletUpdated`

## Match Classes

### UDeskillzMatchmaking
Methods: `StartMatchmaking(TournamentId)`, `CancelMatchmaking()`, `IsMatchmaking()`
Delegates: `OnMatchFound`, `OnMatchmakingFailed`, `OnMatchmakingCancelled`

### UDeskillzMatchManager
Methods: `StartMatch(MatchId)`, `EndMatch(Result)`, `PauseMatch()`, `ResumeMatch()`, `GetCurrentMatch()`, `IsMatchActive()`

## Security Classes

### UDeskillzScoreEncryption
Methods: `EncryptScore(Score, MatchId, UserId)`, `DecryptScore(...)`, `GenerateHMAC(Data)`, `VerifyHMAC(Data, HMAC)`

### UDeskillzSecureSubmitter
Methods: `SubmitScore(Score, Duration)`, `SubmitScoreWithMetadata(...)`, `IsSubmitting()`
Delegates: `OnScoreSubmitted`, `OnScoreValidated`

### UDeskillzAntiCheat
Methods: `ValidateScore(Score, Duration)`, `IsSpeedHackDetected()`, `CheckMemoryIntegrity()`, `GenerateSubmissionId()`

## Network Classes

### UDeskillzHttpClient
Methods: `Get(Endpoint, Callback)`, `Post(Endpoint, Body, Callback)`, `Put(...)`, `Delete(...)`, `SetAuthToken(Token)`

### UDeskillzWebSocket
Methods: `Connect(Url)`, `Disconnect()`, `IsConnected()`, `JoinRoom(RoomId)`, `LeaveRoom(RoomId)`, `SendMessage(Event, Data)`
Delegates: `OnConnected`, `OnDisconnected`, `OnMessageReceived`, `OnError`

### UDeskillzApiService
Methods: `Login(...)`, `Register(...)`, `Logout()`, `GetTournaments(...)`, `EnterTournament(...)`, `GetWalletBalance(...)`, `GetLeaderboard(...)`

## Analytics Classes

### UDeskillzAnalytics
Methods: `TrackEvent(Name, Category, Params)`, `TrackScreenView(Screen)`, `TrackSessionStart()`, `TrackSessionEnd()`, `SetUserProperty(Key, Value)`, `Flush()`

### UDeskillzTelemetry
Methods: `StartMonitoring()`, `StopMonitoring()`, `RecordMetric(Type, Name, Value)`, `GetCurrentFPS()`, `GetMemoryUsage()`

### UDeskillzEventTracker
Methods: `StartTimedEvent(Name)`, `EndTimedEvent(Name)`, `TrackEntryFee(...)`, `TrackPrizeWon(...)`, `IncrementCounter(Name)`, `GetCounter(Name)`

## Platform Classes

### UDeskillzPlatform
Methods: `IsMobile()`, `IsDesktop()`, `IsIOS()`, `IsAndroid()`, `GetDeviceInfo()`, `GetNetworkInfo()`, `GetDeviceTier()`

### UDeskillzDeepLink
Methods: `SetURLScheme(Scheme)`, `HandleDeepLink(URL)`, `GenerateTournamentLink(TournamentId)`, `HasPendingDeepLink()`

### UDeskillzPushNotifications
Methods: `RequestPermission(Callback)`, `GetPermissionStatus()`, `ScheduleLocalNotification(...)`, `CancelNotification(Id)`, `SetBadgeCount(Count)`

## UI Classes

### UDeskillzUIManager
Methods: `ShowTournamentList()`, `ShowMatchmaking()`, `ShowResults()`, `ShowWallet()`, `ShowLeaderboard(TournamentId)`, `ShowPopup(Title, Message)`, `ShowLoading(Message)`

## Key Enums

- `EDeskillzEnvironment`: Sandbox, Production
- `EDeskillzMatchStatus`: Pending, Ready, InProgress, Completed, Cancelled
- `EDeskillzTournamentStatus`: Upcoming, Active, Completed, Cancelled
- `EDeskillzMatchResult`: Win, Loss, Draw, Completed, Forfeited

## Key Structs

- `FDeskillzMatch`: MatchId, TournamentId, Status, Players, EntryFee, Currency
- `FDeskillzTournament`: TournamentId, Name, EntryFee, PrizePool, Status
- `FDeskillzPlayerInfo`: UserId, Username, SkillRating, Wins, Losses
- `FDeskillzWalletBalance`: Currency, Available, Pending, Total

See [Integration Guide](INTEGRATION_GUIDE.md) for detailed examples.
