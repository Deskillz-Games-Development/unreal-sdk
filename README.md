# Deskillz Unreal SDK

**Version:** 3.5.2 | **Unreal Engine:** 4.27+ / 5.x | **License:** MIT

---

## Overview

The Deskillz Unreal SDK integrates your Unreal Engine game with the Deskillz competitive gaming platform. Players compete in skill-based tournaments, create private rooms, host social games with rake systems, and win cryptocurrency prizes (BNB, USDT, USDC on BSC and TRON networks).

**Architecture:** Self-sufficient -- everything happens inside your game. Players authenticate, browse tournaments, join matches, and collect prizes without leaving your application.

All public methods use `UFUNCTION(BlueprintCallable)` macros and all data types use `USTRUCT(BlueprintType)` / `UENUM(BlueprintType)`, so the full SDK is accessible from both C++ and Blueprints.

---

## Requirements

- Unreal Engine 4.27+ or 5.x
- C++ project (Blueprint-only projects can use the Blueprint Function Library)
- iOS 13+ / Android 7.0+ (API 24+)
- Deskillz Developer account ([deskillz.games/developer](https://deskillz.games/developer))

---

## Installation

### Git Clone

```bash
cd YourProject/Plugins/
git clone https://github.com/Deskillz-Games/unreal-sdk.git Deskillz
```

### Download ZIP

1. Download from [GitHub Releases](https://github.com/Deskillz-Games/unreal-sdk/releases)
2. Extract to `YourProject/Plugins/Deskillz/`

### Setup

1. Enable the plugin in **Edit > Plugins > Deskillz SDK**
2. Create a `DeskillzConfig` Data Asset: **Content Browser > Miscellaneous > Data Asset > DeskillzConfig**
3. Set **Game ID**, **API Key**, and **Environment**
4. Add `ADeskillzManager` to your persistent level (or use the Blueprint `BP_DeskillzManager`)

---

## Plugin Structure

```
Source/Deskillz/
  Public/
    Core/
      DeskillzSDK.h                      - Core SDK subsystem
      DeskillzConfig.h                   - Configuration Data Asset
      DeskillzTypes.h                    - Base enums and structs
      DeskillzTypes_v352.h               - v3.5.2 types, delegates, endpoints [NEW]
      DeskillzEvents.h                   - Event system
      DeskillzUpdater.h                  - Auto-updater
    DeskillzAuth.h                       - Authentication (email, social, wallet)
    DeskillzAuthController.h             - Scene flow controller
    DeskillzAuthService.h                - Auth HTTP client
    DeskillzSessionManager_v352.h        - SSO, session resume, guest mode [NEW]
    DeskillzTournamentManager_v352.h     - Tournament lifecycle [NEW]
    DeskillzQuickPlayManager_v352.h      - Quick Play matchmaking [NEW]
    DeskillzDisputeManager_v352.h        - Dispute system [NEW]
    DeskillzRoomExtensions_v352.h        - Room buy-in, invites, settlement [NEW]
    DeskillzWalletManager_v352.h         - Multi-currency wallet [NEW]
    DeskillzHostExtensions_v352.h        - Host dashboard, capabilities [NEW]
    Network/
      DeskillzHttpClient.h               - HTTP with auth, retry, caching
      DeskillzWebSocket.h                - Real-time WebSocket
      DeskillzApiEndpoints.h             - REST endpoint strings
      DeskillzApiService.h               - API request service
      DeskillzNetworkManager.h           - Connection management
    Rooms/
      DeskillzRooms.h                    - Room create, join, leave
      DeskillzRoomClient.h               - Room WebSocket client
      DeskillzRoomTypes.h                - Room data types
    Host/
      DeskillzHostManager.h              - Host profile, earnings, badges
      DeskillzHostEvents.h               - Host event delegates
      DeskillzHostTypes.h                - Host data types
    Match/
      DeskillzMatchManager.h             - Match lifecycle
      DeskillzMatchmaking.h              - Matchmaking queue
      DeskillzTournamentManager.h        - Legacy tournament manager
    Security/
      DeskillzScoreEncryption.h          - HMAC-SHA256 score signing
      DeskillzSecureSubmitter.h          - Encrypted score submission
      DeskillzAntiCheat.h                - Anti-cheat detection
      DeskillzSecurityModule.h           - Security subsystem
    Social/
      DeskillzSocialGameManager.h        - Social game sessions
      DeskillzBuyInManager.h             - Buy-in / rebuy flow
      DeskillzRakeCalculator.h           - Rake computation
      DeskillzSocialTypes.h              - Social data types
      DeskillzSocialEvents.h             - Social event delegates
    Spectator/
      DeskillzHostSpectatorManager.h     - Spectator mode
      DeskillzHostSpectatorTypes.h       - Spectator data types
      DeskillzHostSpectatorEvents.h      - Spectator events
    Lobby/
      DeskillzBridge.h                   - Platform bridge
      DeskillzDeepLinkHandler.h          - Deep link parsing
      DeskillzLobbyClient.h              - Lobby API client
      DeskillzLobbyTypes.h               - Lobby data types
    Analytics/
      DeskillzAnalytics.h                - Analytics tracking
      DeskillzEventTracker.h             - Event tracking
      DeskillzTelemetry.h                - Telemetry data
    Platform/
      DeskillzPlatform.h                 - Platform abstraction
      DeskillzDeepLink.h                 - Deep link system
      DeskillzAppLifecycle.h             - App lifecycle hooks
      DeskillzPushNotifications.h        - Push notification support
    UI/                                  - Pre-built UMG widgets
    Blueprints/
      DeskillzBlueprintLibrary.h         - Blueprint function library
      DeskillzManager.h                  - Blueprint manager actor
    Widgets/                             - Widget Blueprints (Host, Rooms, Social, Spectator)
  Private/                               - Implementation files (.cpp)
  Tests/                                 - Integration tests
Content/
  Blueprints/                            - Blueprint assets (create in editor)
  Widgets/                               - Widget Blueprint assets (create in editor)
```

---

## Quick Start

### 1. Initialize

```cpp
#include "Core/DeskillzSDK.h"
#include "DeskillzAuth.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    UDeskillzSDK::Get()->OnInitialized.AddDynamic(this, &AMyGameMode::OnSDKReady);
}

void AMyGameMode::OnSDKReady(bool bSuccess, const FDeskillzError& Error)
{
    if (bSuccess)
    {
        UDeskillzAuth::Get()->OnLoginSuccess.AddDynamic(this, &AMyGameMode::OnLoggedIn);
        if (UDeskillzAuth::Get()->IsAuthenticated())
            UDeskillzAuthController::Get()->GoToLobby();
        else
            UDeskillzAuthController::Get()->GoToAuth();
    }
}
```

### 2. Authentication

```cpp
#include "DeskillzAuth.h"

// Email login
UDeskillzAuth::Get()->Login(FDeskillzLoginRequest(TEXT("player@email.com"), TEXT("password")));

// Social login (Google, Apple, Facebook)
UDeskillzAuth::Get()->SocialLogin(
    FDeskillzSocialAuthRequest(EDeskillzAuthProvider::Google, IdToken));

// Wallet connection (optional, for crypto transactions)
UDeskillzAuth::Get()->LinkWallet(
    FDeskillzWalletLinkRequest(Address, Signature, Message, Nonce));

// Auth events
UDeskillzAuth::Get()->OnLoginSuccess.AddDynamic(this, &AMyHUD::OnLoggedIn);
UDeskillzAuth::Get()->OnSignUpSuccess.AddDynamic(this, &AMyHUD::OnSignedUp);
UDeskillzAuth::Get()->OnLogout.AddDynamic(this, &AMyHUD::OnLoggedOut);
UDeskillzAuth::Get()->OnAuthError.AddDynamic(this, &AMyHUD::OnAuthError);
UDeskillzAuth::Get()->OnWalletLinked.AddDynamic(this, &AMyHUD::OnWalletLinked);
```

### 3. Submit Score

```cpp
#include "Security/DeskillzSecureSubmitter.h"

// Score is encrypted with HMAC-SHA256 before submission
UDeskillzSecureSubmitter::Get()->SubmitScore(PlayerScore,
    FOnDeskillzScoreSubmitted::CreateLambda([](bool bSuccess, const FDeskillzError& Error) {
        if (bSuccess) UE_LOG(LogTemp, Log, TEXT("Score submitted!"));
    })
);
```

---

## Tournaments

Full lifecycle: browse, register, check-in, play bracket, view results.

```cpp
#include "DeskillzTournamentManager_v352.h"

auto* TM = UDeskillzTournamentManager_v352::Get();

// Browse
TM->GetTournaments({},
    FOnTournamentListings::CreateLambda([](const TArray<FDeskillzTournamentListing>& List) {
        for (const auto& T : List)
            UE_LOG(LogTemp, Log, TEXT("%s - %s %s - %d/%d"), *T.Name,
                *FDeskillzCurrencyHelper::Format(T.EntryFee, T.Currency),
                *T.Status, T.CurrentPlayers, T.MaxPlayers);
    }), OnError);

// Three-step enrollment: Register -> Check In -> Play
TM->Register(TournamentId, OnRegistered, OnError);      // Step 1
TM->CheckIn(TournamentId, OnCheckedIn, OnError);        // Step 2 (T-30 to T-10 window)
TM->Leave(TournamentId, OnLeft, OnError);                // Unregister (refunds entry fee)

// Status and bracket
TM->GetEnrollmentStatus(TournamentId, OnStatus, OnError);
TM->GetMyRegistrations(OnRegistrations, OnError);
TM->GetSchedule(TournamentId, OnSchedule, OnError);
TM->GetMyTableAssignment(TournamentId, OnSeat, OnError);

// Socket events
TM->OnTournamentStarted.AddDynamic(this, &AMyHUD::OnStarted);
TM->OnTournamentLeft.AddDynamic(this, &AMyHUD::OnLeft);
```

---

## Quick Play

Instant matchmaking for esport games + social cash game rooms.

```cpp
#include "DeskillzQuickPlayManager_v352.h"

auto* QP = UDeskillzQuickPlayManager_v352::Get();

// Esport: Join queue -> Match found -> Launch -> Score -> Results
FDeskillzQuickPlayJoinParams Params;
Params.GameId = GameId;
Params.EntryFee = 1.0;
Params.Currency = TEXT("USDT_BSC");
QP->JoinQueue(Params, OnJoined, OnError);

QP->OnQueueMatched.AddDynamic(this, &AMyHUD::OnMatchFound);
QP->LaunchMatch(MatchSessionId, OnLaunched, OnError);
QP->SubmitScore(MatchId, 1500, OnScored, OnError);
QP->GetMatchResults(MatchId, OnResults, OnError);

// Social: Create room -> Submit rounds -> Rebuy/CashOut -> End
QP->CreateSocialRoom(GameId, 0.10, TEXT("USDT_BSC"), 4, OnCreated, OnError);
QP->SubmitSocialRound(RoomId, PayloadJson, OnSuccess, OnError);
QP->SocialRebuy(RoomId, 50.0, OnSuccess, OnError);
QP->SocialCashOut(RoomId, OnSuccess, OnError);
QP->EndSocialGame(RoomId, OnSuccess, OnError);
```

---

## Private Rooms

Create rooms, invite friends, manage players, handle buy-in/cash-out.

### Basic Room Operations

```cpp
#include "Rooms/DeskillzRooms.h"

// Create
FCreateRoomConfig Config;
Config.Name = TEXT("My Room");
Config.MaxPlayers = 4;
Config.GameId = UDeskillzSDK::Get()->GetGameId();
UDeskillzRooms::Get()->CreateRoom(Config, OnCreated, OnError);

// Join by code
UDeskillzRooms::Get()->JoinRoom(TEXT("DSKZ-AB3C"), OnJoined, OnError);
```

### Extended Room Operations (v3.5.2)

```cpp
#include "DeskillzRoomExtensions_v352.h"

auto* RE = UDeskillzRoomExtensions_v352::Get();

// Create with host role
FDeskillzCreateEsportRoomOpts Opts;
Opts.Name = TEXT("Tournament Room");
Opts.EntryFee = 10.0;
Opts.HostRole = EDeskillzHostRole::SPECTATOR;
Opts.MatchMode = EDeskillzEsportMatchMode::BEST_OF_3;
RE->CreateEsportRoom(Opts, OnSuccess, OnError);

// Social room with win condition
FDeskillzCreateSocialRoomOpts SOpts;
SOpts.SocialGameType = TEXT("MAHJONG");
SOpts.TableStakes = 1.0;
SOpts.WinCondition = EDeskillzSocialWinCondition::FIXED_ROUNDS;
SOpts.WinConditionTarget = 8;
RE->CreateSocialRoom(SOpts, OnSuccess, OnError);

// Financial operations
RE->BuyIn(RoomId, 100.0, TEXT("USDT_BSC"), OnSuccess, OnError);
RE->CashOut(RoomId, OnSuccess, OnError);
RE->Rebuy(RoomId, 50.0, TEXT("USDT_BSC"), OnSuccess, OnError);
RE->SubmitRound(RoomId, PayloadJson, OnSuccess, OnError);
RE->TriggerSettlement(RoomId, OnSuccess, OnError);

// Invites
RE->InvitePlayer(RoomId, TEXT("friend"), TEXT("Come play!"), OnSuccess, OnError);
RE->RespondToInvite(InviteId, true, OnSuccess, OnError);
RE->OnInviteReceived.AddDynamic(this, &AMyHUD::OnInvite);
```

---

## Wallet

Multi-currency wallet with deposit, withdrawal, stats, and transaction history.

```cpp
#include "DeskillzWalletManager_v352.h"

auto* W = UDeskillzWalletManager_v352::Get();

// Balances
W->GetBalance(FOnWalletBalances::CreateLambda([](const TArray<FDeskillzWalletBalanceEntry>& B) {
    for (const auto& E : B)
        UE_LOG(LogTemp, Log, TEXT("%s: %.4f ($%.2f)"), *E.Symbol, E.Amount, E.UsdValue);
}), OnError);

// Deposit / Withdraw
W->Deposit(TEXT("USDT_BSC"), 100.0, OnSuccess, OnError);
W->Withdraw(TEXT("USDT_BSC"), 50.0, TEXT("0xWallet..."), OnSuccess, OnError);

// Player data
W->GetPlayerStats(OnStats, OnError);
W->GetMatchHistory(1, 20, OnMatches, OnError);
W->GetGameLeaderboard(GameId, TEXT("all"), 50, OnLeaderboard, OnError);
W->GetTransactions(20, 0, TEXT(""), TEXT(""), OnSuccess, OnError);
```

### Currency Formatting

```cpp
FString Label = FDeskillzCurrencyHelper::GetDisplayName(TEXT("USDT_BSC"));  // "USDT (BSC)"
FString Fmt = FDeskillzCurrencyHelper::Format(5.50, TEXT("BNB"));           // "5.50 BNB (BSC)"
```

**Supported currencies:** BNB, USDT (BSC), USDT (TRON), USDC (BSC), USDC (TRON)

---

## Host System

Players become hosts, create rooms, and earn revenue from platform fees and rake.

```cpp
#include "Host/DeskillzHostManager.h"
#include "DeskillzHostExtensions_v352.h"

// Register as host
UDeskillzHostManager::Get()->RegisterAsHost(OnRegistered, OnError);

// Full dashboard (v3.5.2 composite endpoint)
auto* HE = UDeskillzHostExtensions_v352::Get();
HE->GetDashboard(FOnHostDashboard::CreateLambda([](const FDeskillzHostDashboard& D) {
    UE_LOG(LogTemp, Log, TEXT("Level %d | Earnings: $%.2f | Rooms: %d"),
        D.HostLevel, D.TotalEarnings, D.ActiveRoomCount);
}), OnError);

// Withdraw earnings
HE->RequestWithdrawal(100.0, TEXT("USDT_BSC"), TEXT("0xWallet..."), OnWithdrawal, OnError);

// Age verification (required for social games with rake)
HE->CheckAgeVerified(OnAgeStatus, OnError);
```

### Host Tiers and Revenue Share

| Tier | Players Required | Revenue Share |
|------|-----------------|---------------|
| Bronze | 0 | 15% |
| Silver | 50 | 18% |
| Gold | 250 | 20% |
| Platinum | 1,000 | 23% |
| Diamond | 5,000 | 25% |
| Elite | 10,000 | 28% |

---

## Social Games

Rake-based social games (Big 2, Mahjong, Thirteen Cards, Dou Dizhu).

```cpp
#include "Social/DeskillzSocialGameManager.h"
#include "Social/DeskillzRakeCalculator.h"
#include "Social/DeskillzBuyInManager.h"

// Initialize for a room
UDeskillzSocialGameManager::Get()->Initialize(RoomId);
UDeskillzBuyInManager::Get()->Initialize(RoomId);

// Subscribe to events
UDeskillzSocialGameManager::Get()->OnRoundEnded.AddDynamic(this, &AMyGame::HandleRoundEnd);
UDeskillzBuyInManager::Get()->OnRebuyRequired.AddDynamic(this, &AMyGame::HandleRebuy);

// End a round with pot distribution
UDeskillzSocialGameManager::Get()->EndRound(WinnerId, PotAmount);

// Rake calculation
float Rake = UDeskillzRakeCalculator::CalculateRakeWithCap(PotAmount, RakePercent, RakeCap);
FRakeDistribution Dist = UDeskillzRakeCalculator::PreviewRakeDistribution(Rake, HostTier);
```

---

## Spectator Mode

Allow spectators to watch live games.

```cpp
#include "Spectator/DeskillzHostSpectatorManager.h"

auto* Spec = UDeskillzHostSpectatorManager::Get();
Spec->JoinAsSpectator(RoomId,
    FOnSpectatorJoined::CreateLambda([](const FSpectatorState& State) {
        UE_LOG(LogTemp, Log, TEXT("Spectating room: %s"), *State.RoomId);
    }), OnError);

Spec->OnGameStateUpdated.AddDynamic(this, &AMyHUD::OnStateUpdate);
Spec->OnScoreChanged.AddDynamic(this, &AMyHUD::OnScoreChange);
Spec->SwitchRoom(NewRoomId, OnSwitched, OnError);
Spec->LeaveSpectator();
```

---

## Disputes

File disputes against matches with evidence.

```cpp
#include "DeskillzDisputeManager_v352.h"

auto* DM = UDeskillzDisputeManager_v352::Get();

FDeskillzFileDisputeParams Params;
Params.DisputeType = TEXT("TOURNAMENT");
Params.MatchId = MatchId;
Params.Reason = TEXT("Cheating");
Params.Description = TEXT("Opponent used auto-clicker");
DM->FileDispute(Params, OnDispute, OnError);

DM->GetMyDisputes(TEXT(""), OnDisputes, OnError);
DM->AddEvidence(DisputeId, Evidence, OnSuccess, OnError);

// Last match auto-suggest (stored locally for 7 days)
FDeskillzLastMatchContext Last = DM->GetLastMatch();
```

---

## Game Capabilities

Query what your game supports (configured in Developer Portal).

```cpp
#include "DeskillzHostExtensions_v352.h"

auto* HE = UDeskillzHostExtensions_v352::Get();
HE->GetGameCapabilities(GameId,
    FOnGameCapabilities::CreateLambda([](const FDeskillzGameCapabilities& Caps) {
        UE_LOG(LogTemp, Log, TEXT("1v1: %d, FFA: %d, Max tournament: %d"),
            Caps.bSupports1v1, Caps.bSupportsFFA, Caps.MaxTournamentSize);
    }), OnError);

// Cached capabilities always available (defaults before API responds)
FDeskillzGameCapabilities Current = HE->CachedCapabilities;
```

---

## Session Management

SSO token handoff, active session resume, and guest mode.

```cpp
#include "DeskillzSessionManager_v352.h"

auto* SM = UDeskillzSessionManager_v352::Get();

// SSO consumed automatically from launch URL
SM->OnSSOAuthenticated.AddDynamic(this, &AMyHUD::OnSSO);

// Check for active session (reconnect after crash/restart)
SM->CheckForActiveSession(FOnActiveSession::CreateLambda(
    [](const FDeskillzActiveSessionPayload& S) {
        if (S.bHasActiveSession)
            UE_LOG(LogTemp, Log, TEXT("Resuming %s: %s"), *S.Type, *S.RoomCode);
    }));

// Guest mode (browse only, no paid features)
SM->EnableGuestMode();
bool bCan = SM->CanPerformAction(TEXT("joinTournament")); // false
```

---

## Score Encryption

Scores are signed with HMAC-SHA256 to prevent tampering.

```cpp
#include "Security/DeskillzScoreEncryption.h"
#include "Security/DeskillzSecureSubmitter.h"

// Automatic encryption on submit (recommended)
UDeskillzSecureSubmitter::Get()->SubmitScore(Score, OnSubmitted);

// Manual signing if needed
FString Sig = UDeskillzScoreEncryption::SignScore(MatchId, Score, Timestamp);
bool bValid = UDeskillzScoreEncryption::VerifyScore(MatchId, Score, Timestamp, Sig);
```

---

## Blueprint Integration

All SDK features are accessible from Blueprints via `UDeskillzBlueprintLibrary`:

- **Initialize Deskillz** / **Is Deskillz Initialized** / **Shutdown Deskillz**
- **Get Tournaments** / **Enter Tournament** / **Leave Tournament**
- **Start Matchmaking** / **Cancel Matchmaking**
- **Submit Score** / **Add Score** / **Get Current Score**
- **Get Wallet Balance** / **Get All Balances**
- **Show Tournament List** / **Show Matchmaking UI** / **Show Results** / **Show Wallet**

Use `BP_DeskillzManager` Actor Blueprint for drag-and-drop setup in your level.

---

## Pre-Built UI Widgets

The SDK includes UMG widget base classes ready for customization:

| Widget | Description |
|--------|-------------|
| DeskillzTournamentListWidget | Tournament browser with filtering |
| DeskillzMatchmakingWidget | Matchmaking progress display |
| DeskillzResultsWidget | Match results and rankings |
| DeskillzWalletWidget | Multi-currency wallet display |
| DeskillzLeaderboardWidget | Tournament leaderboard |
| DeskillzHUDWidget | In-game score/timer overlay |
| DeskillzLoginWidget | Email/password login form |
| DeskillzSignUpWidget | Registration form |
| DeskillzHostDashboardWidget | Host earnings and tier progress |
| DeskillzPrivateRoomUI | Room lobby with player cards |
| DeskillzBuyInModalWidget | Buy-in confirmation dialog |
| DeskillzCashOutModalWidget | Cash-out confirmation dialog |
| DeskillzRebuyModalWidget | Rebuy prompt dialog |
| DeskillzHostSpectatorViewWidget | Spectator view with room switcher |

Create Widget Blueprints extending these classes in `Content/Widgets/`. All widgets support theming via `UDeskillzUIManager::Get()->SetTheme()`.

---

## Auto-Updater

The SDK includes an auto-update system that checks for new versions.

```cpp
#include "Core/DeskillzUpdater.h"

UDeskillzUpdater::Get()->CheckForUpdates(
    FOnUpdateAvailable::CreateLambda([](const FString& NewVersion, const FString& Changelog) {
        UE_LOG(LogTemp, Log, TEXT("Update available: %s"), *NewVersion);
    })
);
```

---

## Configuration

Create a `DeskillzConfig` Data Asset in the Unreal Editor.

| Property | Description | Default |
|----------|-------------|---------|
| GameId | Your Game ID from Developer Portal | (required) |
| ApiKey | Your API Key from Developer Portal | (required) |
| Environment | Production / Sandbox / Development | Sandbox |
| bAutoInitialize | Initialize on BeginPlay | true |
| bTestMode | Enable test mode (no real money) | false |
| LogLevel | Verbose / Log / Warning / Error | Log |
| bSelfSufficientMode | All features in your app | true |
| AuthSceneName | Level for login UI | "Auth" |
| LobbySceneName | Level for tournament lobby | "Lobby" |
| GameSceneName | Level for gameplay | "Game" |

---

## Platform Setup

### iOS

Add URL scheme to `Info.plist`:

```xml
<key>CFBundleURLTypes</key>
<array>
    <dict>
        <key>CFBundleURLSchemes</key>
        <array><string>deskillz</string></array>
    </dict>
</array>
```

### Android

Add intent filter to `AndroidManifest.xml`:

```xml
<intent-filter>
    <action android:name="android.intent.action.VIEW"/>
    <category android:name="android.intent.category.DEFAULT"/>
    <category android:name="android.intent.category.BROWSABLE"/>
    <data android:scheme="deskillz" android:host="launch"/>
</intent-filter>
```

---

## Test Mode

```cpp
// Set bTestMode = true in DeskillzConfig Data Asset, or:
UDeskillzSDK::Get()->StartTestMatch(EDeskillzMatchType::Asynchronous, 120);
```

Test mode uses local data and does not connect to production servers.

---

## Troubleshooting

**SDK not initializing:** Verify DeskillzConfig Data Asset exists and is assigned. Check Output Log.

**Blueprint nodes not appearing:** Rebuild the project after adding the plugin. Verify plugin is enabled.

**Auth not working:** Check API key and Game ID. Verify network connectivity.

**WebSocket not connecting:** Check WebSocket URL matches environment. Check firewall.

**Score submission failing:** Confirm match is in progress before submitting. Check ScoreType config.

**Host system not initializing:** Ensure host profile is created first with RegisterAsHost.

**Social game session errors:** Verify room is in correct state before submitting rounds.

**Spectator not connecting:** Check that the room allows spectators and WebSocket is connected.

---

## Support

- Developer Portal: [deskillz.games/developer](https://deskillz.games/developer)
- Documentation: [docs.deskillz.games/unreal](https://docs.deskillz.games/unreal)
- Email: sdk@deskillz.games
- Discord: [discord.gg/deskillz](https://discord.gg/deskillz)

---

## License

MIT License. See [LICENSE](LICENSE) for details.