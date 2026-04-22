<#
.SYNOPSIS
    Merges v3.5.2 API reference content into the existing API_REFERENCE.md
.DESCRIPTION
    Inserts new manager class docs before the "## Key Structs" section,
    inserts new enums before the existing enum section ends,
    inserts new structs at the end of the Key Structs section,
    and updates the version history.
.USAGE
    Run from D:\NewDeskillzGames\deskillz-unreal-sdk\
    .\merge-api-reference.ps1
#>

$ApiRefPath = ".\Docs\API_REFERENCE.md"
$V352Path = ".\Docs\API_REFERENCE_v352.md"

if (-not (Test-Path $ApiRefPath)) {
    Write-Error "API_REFERENCE.md not found at $ApiRefPath"
    exit 1
}

if (-not (Test-Path $V352Path)) {
    Write-Error "API_REFERENCE_v352.md not found at $V352Path"
    exit 1
}

# Backup original
$BackupPath = ".\Docs\API_REFERENCE.md.bak"
Copy-Item $ApiRefPath $BackupPath -Force
Write-Host "[OK] Backed up to $BackupPath" -ForegroundColor Green

$content = Get-Content $ApiRefPath -Raw

# ============================================================================
# 1. INSERT NEW MANAGER CLASSES BEFORE "## Key Structs"
# ============================================================================

$newManagers = @"

---

## Tournament Manager (NEW in v3.5.2)

### UDeskillzTournamentManager_v352

Singleton. Access via ``UDeskillzTournamentManager_v352::Get()``.

| Method | Description | Endpoint |
|--------|-------------|----------|
| ``GetTournaments(Filters, OnSuccess, OnError)`` | List tournaments with filters | GET /tournaments |
| ``GetActiveTournaments(GameId, OnSuccess, OnError)`` | Active tournaments for a game | GET /tournaments/game/:id/active |
| ``Register(TournamentId, OnSuccess, OnError)`` | Register for tournament | POST /tournaments/:id/register |
| ``CheckIn(TournamentId, OnSuccess, OnError)`` | Check in to tournament | POST /tournaments/:id/checkin |
| ``Leave(TournamentId, OnSuccess, OnError)`` | Leave/unregister | DELETE /tournaments/:id/leave |
| ``GetEnrollmentStatus(TournamentId, OnSuccess, OnError)`` | Get enrollment state | GET /tournaments/:id/my-status |
| ``GetMyRegistrations(OnSuccess, OnError)`` | All my registrations | GET /tournaments/my-registrations |
| ``GetSchedule(TournamentId, OnSuccess, OnError)`` | Bracket schedule | GET /tournaments/:id/schedule |
| ``GetMyTableAssignment(TournamentId, OnSuccess, OnError)`` | My table/seat | GET /tournaments/:id/my-seat |

**Delegates:** ``OnTournamentStarted(FString)``, ``OnTournamentLeft(FString)``

---

## Quick Play Manager (NEW in v3.5.2)

### UDeskillzQuickPlayManager_v352

Singleton. Access via ``UDeskillzQuickPlayManager_v352::Get()``.

| Method | Description | Endpoint |
|--------|-------------|----------|
| ``JoinQueue(Params, OnSuccess, OnError)`` | Join matchmaking queue | POST /lobby/quick-play/join |
| ``LeaveQueue(OnSuccess, OnError)`` | Leave queue | POST /lobby/quick-play/leave |
| ``GetConfig(GameId, OnSuccess, OnError)`` | Get Quick Play config | GET /quick-play/games/:id |
| ``LaunchMatch(MatchSessionId, OnSuccess, OnError)`` | Launch matched game | POST /lobby/quick-play/match/launch |
| ``SubmitScore(MatchId, Score, OnSuccess, OnError)`` | Submit match score | POST /lobby/quick-play/match/:id/score |
| ``GetMatchResults(MatchId, OnSuccess, OnError)`` | Get final results | GET /lobby/quick-play/match/:id/results |
| ``ForceCompleteMatch(MatchId, OnSuccess, OnError)`` | Force completion | POST /lobby/quick-play/match/:id/complete |
| ``CreateSocialRoom(...)`` | Create social Quick Play room | POST /lobby/quick-play/social/create |
| ``SubmitSocialRound(RoomId, PayloadJson, ...)`` | Submit round result | POST /lobby/quick-play/social/:id/round |
| ``SocialRebuy(RoomId, Amount, ...)`` | Rebuy chips | POST /lobby/quick-play/social/:id/rebuy |
| ``SocialCashOut(RoomId, ...)`` | Cash out | POST /lobby/quick-play/social/:id/cashout |
| ``EndSocialGame(RoomId, ...)`` | End social game | POST /lobby/quick-play/social/:id/end |

**Properties:** ``IsInQueue()``, ``GetCurrentMatch()``
**Delegates:** ``OnQueueMatched(FString)``, ``OnQueueTimeout()``, ``OnMatchCompleted(FString)``

---

## Dispute Manager (NEW in v3.5.2)

### UDeskillzDisputeManager_v352

Singleton. Access via ``UDeskillzDisputeManager_v352::Get()``.

| Method | Description | Endpoint |
|--------|-------------|----------|
| ``FileDispute(Params, OnSuccess, OnError)`` | File a new dispute | POST /disputes |
| ``GetMyDisputes(StatusFilter, OnSuccess, OnError)`` | List my disputes | GET /disputes/me |
| ``GetDisputeDetails(DisputeId, OnSuccess, OnError)`` | Get dispute details | GET /disputes/:id |
| ``AddEvidence(DisputeId, Evidence, OnSuccess, OnError)`` | Add evidence | POST /disputes/:id/evidence |
| ``GetRecentMatches(OnSuccess, OnError)`` | Recent matches for context | GET /matches/history/me |
| ``PersistLastMatch(Context)`` | Save last match locally | Local file |
| ``GetLastMatch()`` | Load last match (7-day expiry) | Local file |
| ``ClearLastMatch()`` | Delete saved match | Local file |

---

## Room Extensions (NEW in v3.5.2)

### UDeskillzRoomExtensions_v352

Singleton. Access via ``UDeskillzRoomExtensions_v352::Get()``.

| Method | Description | Endpoint |
|--------|-------------|----------|
| ``CreateEsportRoom(Opts, OnSuccess, OnError)`` | Create esport room with HostRole | POST /private-rooms/create |
| ``CreateSocialRoom(Opts, OnSuccess, OnError)`` | Create social room with win condition | POST /private-rooms/create |
| ``BuyIn(RoomId, Amount, Currency, ...)`` | Buy chips | POST /private-rooms/:id/buy-in |
| ``CashOut(RoomId, ...)`` | Cash out chips | POST /private-rooms/:id/cash-out |
| ``Rebuy(RoomId, Amount, Currency, ...)`` | Rebuy chips | POST /private-rooms/:id/rebuy |
| ``SubmitRound(RoomId, PayloadJson, ...)`` | Submit round | POST /private-rooms/:id/round |
| ``TriggerSettlement(RoomId, ...)`` | Trigger rake settlement | POST /private-rooms/:id/settle |
| ``InvitePlayer(RoomId, Target, Message, ...)`` | Invite player | POST /private-rooms/:id/invite |
| ``GetMyInvites(OnSuccess, OnError)`` | My pending invites | GET /private-rooms/invites/my |
| ``RespondToInvite(InviteId, bAccept, ...)`` | Accept/decline invite | POST /private-rooms/invites/:id/respond |

**Delegates:** ``OnInviteReceived(FDeskillzRoomInvite)``, ``OnBuyInComplete(double)``, ``OnCashOutComplete(double)``

---

## Wallet Manager (NEW in v3.5.2)

### UDeskillzWalletManager_v352

Singleton. Access via ``UDeskillzWalletManager_v352::Get()``.

| Method | Description | Endpoint |
|--------|-------------|----------|
| ``GetBalance(OnSuccess, OnError)`` | All currency balances | GET /wallet/balance |
| ``GetBalanceForCurrency(Currency, OnSuccess, OnError)`` | Single currency balance | GET /wallet/balance/:currency |
| ``Deposit(Currency, Amount, ...)`` | Initiate deposit | POST /wallet/deposit |
| ``Withdraw(Currency, Amount, WalletAddress, ...)`` | Initiate withdrawal | POST /wallet/withdraw |
| ``GetPlayerStats(OnSuccess, OnError)`` | Player statistics | GET /users/me |
| ``GetMatchHistory(Page, Limit, OnSuccess, OnError)`` | Match history | GET /matches/history/me |
| ``GetGameLeaderboard(GameId, Period, Limit, ...)`` | Game leaderboard | GET /leaderboard/:gameId |
| ``GetTransactions(Limit, Offset, Type, Currency, ...)`` | Transaction history | GET /wallet/transactions |

---

## Host Extensions (NEW in v3.5.2)

### UDeskillzHostExtensions_v352

Singleton. Access via ``UDeskillzHostExtensions_v352::Get()``.

| Method | Description | Endpoint |
|--------|-------------|----------|
| ``GetDashboard(OnSuccess, OnError)`` | Composite dashboard | GET /host/dashboard |
| ``WithdrawAllEarnings(OnSuccess, OnError)`` | Withdraw all | POST /host/withdraw |
| ``RequestWithdrawal(Amount, Currency, WalletAddress, ...)`` | Parameterized withdrawal | POST /host/withdraw |
| ``CheckAgeVerified(OnSuccess, OnError)`` | Age verification status | GET /host/verify-age/status |
| ``GetGameCapabilities(GameId, OnSuccess, OnError)`` | Game capabilities | GET /games/:id/capabilities |

---

## Session Manager (NEW in v3.5.2)

### UDeskillzSessionManager_v352

Singleton. Access via ``UDeskillzSessionManager_v352::Get()``.

| Method | Description | Endpoint |
|--------|-------------|----------|
| ``ConsumeSSOToken()`` | Consume SSO token from launch URL | GET /users/me |
| ``CheckForActiveSession(OnResult)`` | Check for active room/match | GET /private-rooms/my-active |
| ``GetActiveSession()`` | Get cached active session | Cached |
| ``HasActiveSession()`` | Whether active session exists | Cached |
| ``EnableGuestMode()`` | Enable guest mode | Local state |
| ``DisableGuestMode()`` | Disable guest mode | Local state |
| ``IsGuest()`` | Check guest status | Local state |
| ``CanPerformAction(ActionName)`` | Check if action allowed | Local state |

**Delegates:** ``OnSSOAuthenticated(FDeskillzAuthUser)``, ``OnSessionResumed(FDeskillzActiveSessionPayload)``, ``OnGuestModeActivated()``

"@

# Find "## Key Structs" and insert before it
if ($content -match "## Key Structs") {
    $content = $content -replace "## Key Structs", "$newManagers`n## Key Structs"
    Write-Host "[OK] Inserted 7 new manager class docs before Key Structs" -ForegroundColor Green
} else {
    # Fallback: append before Version History
    $content = $content -replace "## Version History", "$newManagers`n## Version History"
    Write-Host "[OK] Inserted 7 new manager class docs before Version History" -ForegroundColor Yellow
}

# ============================================================================
# 2. INSERT NEW ENUMS AFTER EXISTING ENUM SECTION
# ============================================================================

$newEnums = @"

### EDeskillzSocialWinCondition (NEW in v3.5.2)

| Value | Description |
|-------|-------------|
| ``FIRST_TO_POINTS`` | First player to reach target points |
| ``FIXED_ROUNDS`` | Play a fixed number of rounds |
| ``TIMED_SESSION`` | Play for a time duration |
| ``SINGLE_GAME`` | One game only |
| ``OPEN_ENDED`` | Continue until players stop |

### EDeskillzEnrollmentStatus (NEW in v3.5.2)

| Value | Description |
|-------|-------------|
| ``NOT_REGISTERED`` | Not enrolled |
| ``REGISTERED`` | Registered, awaiting check-in |
| ``CHECKIN_OPEN`` | Check-in window open |
| ``CHECKED_IN`` | Checked in |
| ``SEATED`` | Assigned to table |
| ``PLAYING`` | In active match |
| ``WON`` | Won the tournament |
| ``ELIMINATED`` | Eliminated from bracket |
| ``DQ_NO_SHOW`` | Disqualified for no-show |
| ``DQ_DISCONNECT`` | Disqualified for disconnect |
| ``STANDBY`` | On standby list |
| ``SUBBED_IN`` | Substituted into table |

### EDeskillzHostRole (NEW in v3.5.2)

| Value | Description |
|-------|-------------|
| ``PLAYER`` | Host participates as a player |
| ``SPECTATOR`` | Host watches but does not play |

### EDeskillzEsportMatchMode (NEW in v3.5.2)

| Value | Description |
|-------|-------------|
| ``SINGLE_MATCH`` | One match |
| ``BEST_OF_3`` | Best of 3 matches |
| ``BEST_OF_5`` | Best of 5 matches |

### EDeskillzSocialGameType (NEW in v3.5.2)

| Value | Description |
|-------|-------------|
| ``BIG_TWO`` | Big 2 card game |
| ``MAHJONG`` | Mahjong |
| ``CHINESE_POKER_13`` | Thirteen Cards |
| ``DOU_DIZHU`` | Dou Dizhu (Fight the Landlord) |

### EDeskillzQuickPlayQueueState (NEW in v3.5.2)

| Value | Description |
|-------|-------------|
| ``IDLE`` | Not in queue |
| ``QUEUED`` | Searching for match |
| ``FOUND`` | Match found |
| ``READY`` | Ready to launch |
| ``PLAYING`` | In match |

"@

# Insert after the last existing enum section (ESpectatorViewMode)
if ($content -match "### ESpectatorViewMode") {
    # Find the end of ESpectatorViewMode section (next ## heading)
    $content = $content -replace "(### ESpectatorViewMode[^#]+?)(\r?\n## Key)", "`$1$newEnums`$2"
    Write-Host "[OK] Inserted 6 new enum docs after ESpectatorViewMode" -ForegroundColor Green
} else {
    Write-Host "[WARN] Could not find ESpectatorViewMode section, appending enums before Key Structs" -ForegroundColor Yellow
    $content = $content -replace "## Key Structs", "$newEnums`n## Key Structs"
}

# ============================================================================
# 3. INSERT NEW STRUCTS AT END OF KEY STRUCTS SECTION
# ============================================================================

$newStructs = @"

### FDeskillzTournamentListing (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``Id`` | FString | Tournament ID |
| ``Name`` | FString | Tournament name |
| ``Status`` | FString | Current status |
| ``EntryFee`` | double | Entry fee amount |
| ``Currency`` | FString | Fee currency |
| ``PrizePool`` | double | Total prize pool |
| ``MaxPlayers`` | int32 | Maximum participants |
| ``CurrentPlayers`` | int32 | Current participants |
| ``SocialGameType`` | FString | Social game type (if applicable) |
| ``ScheduledStart`` | FDateTime | Scheduled start time |

### FDeskillzQuickPlayConfig (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``GameId`` | FString | Game identifier |
| ``bEnabled`` | bool | Whether Quick Play is enabled |
| ``EntryFee`` | double | Default entry fee |
| ``MatchDurationSeconds`` | int32 | Match duration |
| ``SocialWinCondition`` | FString | Win condition for social mode |
| ``SocialPointTargets`` | TArray<int32> | Available point targets |

### FDeskillzDisputeRecord (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``Id`` | FString | Dispute ID |
| ``MatchId`` | FString | Disputed match |
| ``Reason`` | FString | Dispute reason |
| ``Description`` | FString | Detailed description |
| ``Status`` | FString | OPEN, UNDER_REVIEW, RESOLVED, REJECTED |
| ``Evidence`` | TArray<FString> | Evidence URLs |

### FDeskillzActiveSessionPayload (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``bHasActiveSession`` | bool | Whether session exists |
| ``Type`` | FString | room, tournament, quickplay |
| ``RoomId`` | FString | Active room ID |
| ``RoomCode`` | FString | Room code for reconnect |

### FDeskillzGameCapabilities (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``bSupports1v1`` | bool | 1v1 support |
| ``bSupportsFFA`` | bool | Free-for-all support |
| ``MaxTournamentSize`` | int32 | Max tournament players |
| ``MinMatchDurationSeconds`` | int32 | Min match duration |
| ``MaxMatchDurationSeconds`` | int32 | Max match duration |

### FDeskillzWalletBalanceEntry (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``Currency`` | FString | Currency code (USDT_BSC, BNB, etc.) |
| ``Symbol`` | FString | Display symbol (USDT, BNB) |
| ``Amount`` | double | Available balance |
| ``UsdValue`` | double | USD equivalent |
| ``Network`` | FString | Blockchain network |

### FDeskillzHostDashboard (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``TotalEarnings`` | double | All-time earnings |
| ``EsportsEarnings`` | double | Esports revenue |
| ``SocialEarnings`` | double | Social game revenue |
| ``EsportsTier`` | FString | Current esports tier |
| ``SocialTier`` | FString | Current social tier |
| ``HostLevel`` | int32 | Host level |
| ``ActiveRoomCount`` | int32 | Currently active rooms |

### FDeskillzRoomInvite (NEW in v3.5.2)

| Property | Type | Description |
|----------|------|-------------|
| ``Id`` | FString | Invite ID |
| ``RoomCode`` | FString | Room code |
| ``RoomName`` | FString | Room name |
| ``SenderUsername`` | FString | Who sent the invite |
| ``EntryFee`` | double | Room entry fee |
| ``CurrentPlayers`` | int32 | Players in room |
| ``MaxPlayers`` | int32 | Max room capacity |

"@

# Insert before Version History
if ($content -match "## Version History") {
    $content = $content -replace "## Version History", "$newStructs`n## Version History"
    Write-Host "[OK] Inserted 8 new struct docs before Version History" -ForegroundColor Green
} else {
    $content += $newStructs
    Write-Host "[OK] Appended 8 new struct docs to end" -ForegroundColor Yellow
}

# ============================================================================
# 4. UPDATE VERSION HISTORY
# ============================================================================

$versionEntry = @"
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

"@

if ($content -match "## Version History") {
    $content = $content -replace "(## Version History\r?\n)", "`$1`n$versionEntry"
    Write-Host "[OK] Added v3.5.2 to Version History" -ForegroundColor Green
}

# ============================================================================
# 5. WRITE MERGED FILE
# ============================================================================

Set-Content -Path $ApiRefPath -Value $content -NoNewline
Write-Host ""
Write-Host "[DONE] API_REFERENCE.md merged with v3.5.2 content" -ForegroundColor Cyan
Write-Host "  Backup: $BackupPath" -ForegroundColor Gray
Write-Host "  Original line count: $((Get-Content $BackupPath).Count)" -ForegroundColor Gray
Write-Host "  New line count: $((Get-Content $ApiRefPath).Count)" -ForegroundColor Gray