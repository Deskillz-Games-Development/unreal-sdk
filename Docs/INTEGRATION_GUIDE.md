# Deskillz SDK - Integration Guide

Complete guide for integrating Deskillz tournaments into your Unreal Engine game.

## Game Type Selection

### Asynchronous (Score Attack)
Players compete independently. Best for: puzzle games, endless runners, score-attack games.

### Synchronous (Real-time)
Players compete simultaneously. Best for: fighting games, racing, real-time competitive.

---

## Asynchronous Integration

```cpp
// 1. Initialize SDK
FDeskillzConfig Config;
Config.GameId = TEXT("your_game_id");
Config.ApiKey = TEXT("your_api_key");
UDeskillzSDK::Get()->Initialize(Config);

// 2. Enter Tournament
UDeskillzMatchmaking::Get()->OnMatchFound.AddDynamic(this, &OnMatchFound);
UDeskillzMatchmaking::Get()->StartMatchmaking(TournamentId);

// 3. Handle Match Found
void OnMatchFound(const FDeskillzMatch& Match)
{
    UDeskillzMatchManager::Get()->StartMatch(Match.MatchId);
    StartYourGameplay();
}

// 4. Submit Score
void OnGameplayComplete(int64 Score, float Duration)
{
    UDeskillzSecureSubmitter::Get()->SubmitScore(Score, Duration);
}
```

---

## Synchronous Integration

```cpp
// 1. After match found, connect WebSocket
UDeskillzWebSocket* WS = UDeskillzWebSocket::Get();
WS->OnConnected.AddDynamic(this, &OnConnected);
WS->Connect(WebSocketUrl);

// 2. Join Room
void OnConnected() { WS->JoinRoom(MatchId); }

// 3. Send State (in Tick)
TSharedPtr<FJsonObject> State = MakeShareable(new FJsonObject());
State->SetNumberField(TEXT("score"), CurrentScore);
State->SetNumberField(TEXT("x"), Position.X);
WS->SendGameState(State);

// 4. Receive Opponent State
WS->OnMessageReceived.AddLambda([](const FString& Event, TSharedPtr<FJsonObject> Data) {
    if (Event == TEXT("game_state")) {
        // Update opponent visualization
    }
});
```

---

## Wallet Integration

```cpp
// Get Balance
UDeskillzApiService::Get()->GetWalletBalance(
    FOnWalletResult::CreateLambda([](bool bSuccess, const TArray<FDeskillzWalletBalance>& Balances) {
        for (const auto& B : Balances)
            UE_LOG(LogTemp, Log, TEXT("%s: %.4f"), *B.Currency, B.Available);
    })
);

// Show Wallet UI
UDeskillzUIManager::Get()->ShowWallet();
```

---

## Analytics

```cpp
// Track Events
TMap<FString, FString> Params;
Params.Add(TEXT("level"), TEXT("5"));
Params.Add(TEXT("score"), TEXT("15000"));
UDeskillzAnalytics::Get()->TrackEvent(TEXT("level_complete"), EDeskillzEventCategory::Gameplay, Params);

// Track Timed Events
UDeskillzEventTracker::Get()->StartTimedEvent(TEXT("boss_fight"));
// ... gameplay ...
UDeskillzEventTracker::Get()->EndTimedEvent(TEXT("boss_fight"));

// Track Revenue
UDeskillzEventTracker::Get()->TrackEntryFee(TournamentId, TEXT("USDT"), 10.0);
```

---

## UI Customization

```cpp
FDeskillzUITheme Theme;
Theme.PrimaryColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);
Theme.AccentColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
Theme.CornerRadius = 8.0f;
UDeskillzUIManager::Get()->SetTheme(Theme);
```

---

## Platform Setup

### iOS - Info.plist
```xml
<key>CFBundleURLSchemes</key>
<array><string>deskillz-yourgame</string></array>
```

### Android - AndroidManifest.xml
```xml
<data android:scheme="deskillz-yourgame" />
```

---

## Security Best Practices

1. Never hardcode API keys
2. Enable anti-cheat in production
3. Use HTTPS only
4. Validate server responses
5. Log security events

```cpp
#if UE_BUILD_SHIPPING
    Config.Environment = EDeskillzEnvironment::Production;
    Config.bEnableLogging = false;
    Config.bEnableAntiCheat = true;
#endif
```

---

## Testing

```cpp
// Enable verbose logging
Config.bEnableLogging = true;
Config.LogLevel = EDeskillzLogLevel::Verbose;

// Use mock server for testing
FDeskillzMockServer::Get()->Start();
FDeskillzTestScenarios::SetupHappyPath();
```

See [API Reference](API_REFERENCE.md) for complete method documentation.
