# Deskillz SDK - Quick Start Guide

Get your Unreal Engine game integrated with Deskillz tournaments in 15 minutes.

## Prerequisites

Before starting, ensure you have:

- [ ] Unreal Engine 4.27+ or 5.x installed
- [ ] A Deskillz developer account ([Sign up here](https://developer.deskillz.games))
- [ ] Your Game ID and API Key from the developer portal
- [ ] A game ready for tournament integration

## Step 1: Install the SDK (2 minutes)

### Option A: Copy to Plugins Folder

1. Download the Deskillz SDK
2. Extract to your project's `Plugins` folder:

```
YourProject/
├── Content/
├── Source/
└── Plugins/
    └── Deskillz/           ← Extract here
        ├── Deskillz.uplugin
        └── Source/
```

3. Regenerate project files (right-click `.uproject` → "Generate Visual Studio project files")

### Option B: Add to Engine Plugins

For engine-wide availability, extract to:
- **Windows**: `C:/Program Files/Epic Games/UE_5.X/Engine/Plugins/`
- **Mac**: `/Users/Shared/Epic Games/UE_5.X/Engine/Plugins/`

## Step 2: Enable the Plugin (1 minute)

1. Open your project in Unreal Editor
2. Go to **Edit → Plugins**
3. Search for "Deskillz"
4. Check **Enabled**
5. Restart the editor when prompted

Alternatively, add to your `.uproject` file:

```json
{
  "Plugins": [
    {
      "Name": "Deskillz",
      "Enabled": true
    }
  ]
}
```

## Step 3: Add Module Dependencies (1 minute)

In your game's `Build.cs` file:

```csharp
public class YourGame : ModuleRules
{
    public YourGame(ReadOnlyTargetRules Target) : base(Target)
    {
        // ... existing code ...
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Deskillz"
        });
    }
}
```

## Step 4: Initialize the SDK (3 minutes)

### C++ Method

In your GameMode or GameInstance:

```cpp
// YourGameMode.h
#pragma once
#include "Core/DeskillzSDK.h"
#include "GameFramework/GameModeBase.h"
#include "YourGameMode.generated.h"

UCLASS()
class AYourGameMode : public AGameModeBase
{
    GENERATED_BODY()
    
public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

// YourGameMode.cpp
#include "YourGameMode.h"

void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure SDK
    FDeskillzConfig Config;
    Config.GameId = TEXT("your_game_id_here");      // From developer portal
    Config.ApiKey = TEXT("your_api_key_here");      // From developer portal
    Config.Environment = EDeskillzEnvironment::Sandbox;  // Use Production for release
    Config.bEnableLogging = true;                   // Disable in production
    Config.bEnableAnalytics = true;
    Config.bEnableAntiCheat = true;
    
    // Initialize
    UDeskillzSDK* SDK = UDeskillzSDK::Get();
    SDK->Initialize(Config);
    
    if (SDK->IsInitialized())
    {
        UE_LOG(LogTemp, Log, TEXT("Deskillz SDK initialized successfully!"));
    }
}

void AYourGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UDeskillzSDK::Get()->Shutdown();
    Super::EndPlay(EndPlayReason);
}
```

### Blueprint Method

1. Add **Deskillz Manager** actor to your level
2. In the Details panel, set:
   - Game ID: `your_game_id_here`
   - API Key: `your_api_key_here`
   - Environment: Sandbox
3. The manager auto-initializes on BeginPlay

Or use the Blueprint Function Library:

![Blueprint Init](https://docs.deskillz.games/images/bp_init.png)

## Step 5: Add Tournament Entry Flow (3 minutes)

### Show Tournament List

```cpp
#include "UI/DeskillzUIManager.h"

void AYourGameMode::ShowTournaments()
{
    // Show pre-built tournament list UI
    UDeskillzUIManager::Get()->ShowTournamentList();
}
```

### Enter Tournament Programmatically

```cpp
#include "Match/DeskillzMatchmaking.h"
#include "Match/DeskillzMatchManager.h"

void AYourGameMode::EnterTournament(const FString& TournamentId)
{
    UDeskillzMatchmaking* Matchmaking = UDeskillzMatchmaking::Get();
    
    // Bind callback
    Matchmaking->OnMatchFound.AddDynamic(this, &AYourGameMode::OnMatchFound);
    Matchmaking->OnMatchmakingFailed.AddDynamic(this, &AYourGameMode::OnMatchFailed);
    
    // Start matchmaking
    Matchmaking->StartMatchmaking(TournamentId);
}

void AYourGameMode::OnMatchFound(const FDeskillzMatch& Match)
{
    UE_LOG(LogTemp, Log, TEXT("Match found! ID: %s"), *Match.MatchId);
    
    // Store match info
    CurrentMatchId = Match.MatchId;
    
    // Start your game
    StartGameplay();
    
    // Notify SDK that match has started
    UDeskillzMatchManager::Get()->StartMatch(Match.MatchId);
}

void AYourGameMode::OnMatchFailed(const FString& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("Matchmaking failed: %s"), *Reason);
    // Show error to player
}
```

## Step 6: Submit Score (3 minutes)

When gameplay ends, submit the player's score:

```cpp
#include "Security/DeskillzSecureSubmitter.h"

void AYourGameMode::OnGameplayComplete(int64 FinalScore, float PlayDuration)
{
    UDeskillzSecureSubmitter* Submitter = UDeskillzSecureSubmitter::Get();
    
    // Bind callback
    Submitter->OnScoreSubmitted.AddDynamic(this, &AYourGameMode::OnScoreSubmitted);
    
    // Submit encrypted score
    Submitter->SubmitScore(FinalScore, PlayDuration);
}

void AYourGameMode::OnScoreSubmitted(bool bSuccess, const FString& Message)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Score submitted successfully!"));
        
        // End the match
        UDeskillzMatchManager::Get()->EndMatch(EDeskillzMatchResult::Completed);
        
        // Show results UI
        UDeskillzUIManager::Get()->ShowResults();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Score submission failed: %s"), *Message);
    }
}
```

## Step 7: Test Your Integration (2 minutes)

### In Editor

1. Run PIE (Play In Editor)
2. Check Output Log for "Deskillz SDK initialized successfully!"
3. Open tournament list
4. Enter a sandbox tournament
5. Play your game
6. Verify score submission

### Sandbox Testing

The Sandbox environment:
- Uses test cryptocurrency (no real money)
- Provides instant matchmaking
- Allows testing all features safely

## Complete Minimal Example

Here's a complete minimal integration:

```cpp
// MinimalDeskillzGame.h
#pragma once

#include "CoreMinimal.h"
#include "Core/DeskillzSDK.h"
#include "Match/DeskillzMatchManager.h"
#include "Match/DeskillzMatchmaking.h"
#include "Security/DeskillzSecureSubmitter.h"
#include "GameFramework/GameModeBase.h"
#include "MinimalDeskillzGame.generated.h"

UCLASS()
class AMinimalDeskillzGame : public AGameModeBase
{
    GENERATED_BODY()
    
public:
    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable)
    void EnterTournament(const FString& TournamentId);
    
    UFUNCTION(BlueprintCallable)
    void SubmitGameScore(int64 Score);
    
private:
    UFUNCTION()
    void OnMatchFound(const FDeskillzMatch& Match);
    
    UFUNCTION()
    void OnScoreSubmitted(bool bSuccess, const FString& Message);
    
    FString CurrentMatchId;
    float MatchStartTime;
};

// MinimalDeskillzGame.cpp
#include "MinimalDeskillzGame.h"

void AMinimalDeskillzGame::BeginPlay()
{
    Super::BeginPlay();
    
    FDeskillzConfig Config;
    Config.GameId = TEXT("your_game_id");
    Config.ApiKey = TEXT("your_api_key");
    Config.Environment = EDeskillzEnvironment::Sandbox;
    
    UDeskillzSDK::Get()->Initialize(Config);
}

void AMinimalDeskillzGame::EnterTournament(const FString& TournamentId)
{
    UDeskillzMatchmaking* Matchmaking = UDeskillzMatchmaking::Get();
    Matchmaking->OnMatchFound.AddDynamic(this, &AMinimalDeskillzGame::OnMatchFound);
    Matchmaking->StartMatchmaking(TournamentId);
}

void AMinimalDeskillzGame::OnMatchFound(const FDeskillzMatch& Match)
{
    CurrentMatchId = Match.MatchId;
    MatchStartTime = GetWorld()->GetTimeSeconds();
    
    UDeskillzMatchManager::Get()->StartMatch(Match.MatchId);
    
    // TODO: Start your actual gameplay here
}

void AMinimalDeskillzGame::SubmitGameScore(int64 Score)
{
    float Duration = GetWorld()->GetTimeSeconds() - MatchStartTime;
    
    UDeskillzSecureSubmitter* Submitter = UDeskillzSecureSubmitter::Get();
    Submitter->OnScoreSubmitted.AddDynamic(this, &AMinimalDeskillzGame::OnScoreSubmitted);
    Submitter->SubmitScore(Score, Duration);
}

void AMinimalDeskillzGame::OnScoreSubmitted(bool bSuccess, const FString& Message)
{
    if (bSuccess)
    {
        UDeskillzMatchManager::Get()->EndMatch(EDeskillzMatchResult::Completed);
    }
}
```

## Next Steps

Now that you have basic integration working:

1. **Add UI** - Implement tournament browser, wallet display
2. **Add Analytics** - Track player events for insights
3. **Configure Anti-Cheat** - Protect your game from cheaters
4. **Test Thoroughly** - Use sandbox for comprehensive testing
5. **Go Live** - Switch to Production and launch!

## Resources

- [Full API Reference](API_REFERENCE.md)
- [Integration Guide](INTEGRATION_GUIDE.md)
- [Sample Project](https://github.com/deskillz/unreal-sample)
- [Developer Portal](https://developer.deskillz.games)

## Troubleshooting

### SDK Not Initializing

```cpp
// Check initialization status
if (!UDeskillzSDK::Get()->IsInitialized())
{
    // Verify credentials
    // Check network connectivity
    // Enable logging for details
}
```

### Module Not Found

Ensure `"Deskillz"` is in your `Build.cs` PublicDependencyModuleNames.

### Blueprint Nodes Missing

Restart the editor after enabling the plugin.

---

Need help? Contact sdk-support@deskillz.games
