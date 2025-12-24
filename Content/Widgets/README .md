# Widgets Folder

This folder contains UMG Widget Blueprint assets for the Deskillz SDK UI.

## What Goes Here

Widget Blueprint assets (`.uasset` files) that provide pre-built UI components for tournaments, matchmaking, results, and wallet display.

### Expected Contents:

| Widget Blueprint | C++ Parent Class | Description |
|------------------|------------------|-------------|
| `WBP_TournamentList.uasset` | `UDeskillzTournamentListWidget` | Tournament browser with filtering |
| `WBP_Matchmaking.uasset` | `UDeskillzMatchmakingWidget` | Matchmaking progress display |
| `WBP_Results.uasset` | `UDeskillzResultsWidget` | Match results and rankings |
| `WBP_Wallet.uasset` | `UDeskillzWalletWidget` | Multi-currency wallet display |
| `WBP_Leaderboard.uasset` | `UDeskillzLeaderboardWidget` | Tournament leaderboard |
| `WBP_HUD.uasset` | `UDeskillzHUDWidget` | In-game score/timer overlay |
| `WBP_Popup.uasset` | `UDeskillzPopupWidget` | Modal dialog/notification |
| `WBP_Loading.uasset` | `UDeskillzLoadingWidget` | Loading indicator |

## How to Create Widget Blueprints

### Step 1: Create Widget Blueprint

1. In Unreal Editor: Right-click in Content Browser
2. Select **User Interface > Widget Blueprint**
3. When prompted for parent class, choose the C++ widget class (e.g., `UDeskillzTournamentListWidget`)
4. Name it with `WBP_` prefix (e.g., `WBP_TournamentList`)
5. Save to this folder

### Step 2: Design the UI

1. Open the Widget Blueprint
2. In the Designer tab, add UI elements:
   - Use **Vertical Box** for lists
   - Use **Horizontal Box** for rows
   - Add **Text** blocks for labels
   - Add **Buttons** for actions
   - Add **Images** for icons/avatars

### Step 3: Bind to C++ Functions

The C++ parent classes provide these bindable functions and events:

#### Tournament List Widget
```
Functions:
- RefreshTournaments()
- FilterByEntryFee(Min, Max)
- FilterByStatus(Status)
- SelectTournament(TournamentId)

Events:
- OnTournamentsLoaded(Tournaments)
- OnTournamentSelected(Tournament)
- OnError(Message)
```

#### Matchmaking Widget
```
Functions:
- StartMatchmaking(TournamentId)
- CancelMatchmaking()
- GetElapsedTime()

Events:
- OnMatchmakingStarted()
- OnMatchFound(Match)
- OnMatchmakingCancelled()
- OnMatchmakingFailed(Reason)
```

#### Results Widget
```
Functions:
- ShowResults(MatchResult)
- PlayAgain()
- ReturnToLobby()

Events:
- OnPlayAgainClicked()
- OnReturnClicked()
```

#### Wallet Widget
```
Functions:
- RefreshBalances()
- SelectCurrency(Currency)
- ShowDepositDialog()
- ShowWithdrawDialog()

Events:
- OnBalancesLoaded(Balances)
- OnTransactionComplete(Transaction)
```

## Theming

All widgets support theming through `UDeskillzUIManager`:

```cpp
// In C++ or Blueprint
FDeskillzUITheme Theme;
Theme.PrimaryColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);    // Blue
Theme.SecondaryColor = FLinearColor(0.1f, 0.1f, 0.15f, 1.0f); // Dark
Theme.AccentColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);     // Gold
Theme.TextColor = FLinearColor::White;
Theme.CornerRadius = 8.0f;
Theme.Padding = 16.0f;

UDeskillzUIManager::Get()->SetTheme(Theme);
```

## Example: Custom Tournament Card

```
[Horizontal Box]
├── [Image] - Game thumbnail
├── [Vertical Box]
│   ├── [Text] - Tournament name
│   ├── [Text] - Entry fee + currency
│   └── [Text] - Participants (50/100)
└── [Button] - "Enter" 
    └── OnClicked → EnterTournament(TournamentId)
```

## Animation Support

Widget Blueprints can use UMG animations:

1. In Widget Blueprint, go to **Animations** tab
2. Create animations for:
   - `FadeIn` - Widget appears
   - `FadeOut` - Widget disappears
   - `SlideIn` - Slides from edge
   - `Pulse` - Attention animation

The C++ base classes call these animations automatically when showing/hiding.

## Notes

- Widget Blueprints must be created in Unreal Editor
- Always extend from the C++ base widget classes for full functionality
- Test on multiple screen sizes (mobile, tablet, desktop)
- Use anchors for responsive layouts
- See `Source/Deskillz/Public/UI/` for C++ widget implementations
