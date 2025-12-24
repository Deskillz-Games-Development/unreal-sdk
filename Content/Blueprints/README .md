# Blueprints Folder

This folder contains Blueprint assets for the Deskillz SDK.

## What Goes Here

Blueprint assets (`.uasset` files) that provide easy-to-use, no-code integration options for developers who prefer visual scripting.

### Expected Contents:

| Blueprint | Description |
|-----------|-------------|
| `BP_DeskillzManager.uasset` | Actor Blueprint for easy SDK setup - drag into level |
| `BP_TournamentEntry.uasset` | Example Blueprint showing tournament entry flow |
| `BP_ScoreSubmission.uasset` | Example Blueprint for submitting scores |
| `BP_WalletDisplay.uasset` | Example Blueprint for showing wallet balance |

## How to Create These Blueprints

### BP_DeskillzManager

1. In Unreal Editor: Right-click in Content Browser
2. Select **Blueprint Class**
3. Choose **ADeskillzManager** as parent class
4. Name it `BP_DeskillzManager`
5. Open and configure:
   - Set `Game Id` property
   - Set `Api Key` property
   - Set `Environment` (Sandbox/Production)
6. Save to this folder

### Example Tournament Entry Blueprint

1. Create new Blueprint (Actor or Widget)
2. Add these nodes:
   ```
   Event BeginPlay
       |
       v
   Get DeskillzSDK --> Initialize
       |
       v
   Bind to OnSDKInitialized
       |
       v
   Get DeskillzMatchmaking --> StartMatchmaking
       |
       v
   Bind to OnMatchFound --> Start Your Game
   ```

## Blueprint Function Library

The C++ class `UDeskillzBlueprintLibrary` exposes these Blueprint nodes:

### Initialization
- `Initialize Deskillz` - Initialize SDK with config
- `Is Deskillz Initialized` - Check if ready
- `Shutdown Deskillz` - Clean shutdown

### Tournaments
- `Get Tournaments` - Fetch available tournaments
- `Enter Tournament` - Enter with entry fee
- `Leave Tournament` - Exit tournament

### Matchmaking
- `Start Matchmaking` - Begin finding opponent
- `Cancel Matchmaking` - Stop searching
- `Is Matchmaking` - Check status

### Match
- `Start Match` - Begin gameplay
- `End Match` - Complete match
- `Pause Match` - Pause gameplay
- `Resume Match` - Continue gameplay
- `Get Match Duration` - Time elapsed

### Score
- `Submit Score` - Submit encrypted score
- `Add Score` - Increment score during play
- `Get Current Score` - Read current score

### Wallet
- `Get Wallet Balance` - Check balance by currency
- `Get All Balances` - Get all currency balances

### UI
- `Show Tournament List` - Display tournament browser
- `Show Matchmaking UI` - Display matchmaking screen
- `Show Results` - Display match results
- `Show Wallet` - Display wallet screen
- `Show Popup` - Display message popup

## Notes

- Blueprint assets must be created in Unreal Editor (not code files)
- All Blueprints should extend from the C++ base classes in `Source/Deskillz/`
- Test Blueprints in Sandbox environment before Production
- See `Docs/QUICKSTART.md` for integration guide
