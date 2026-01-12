// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeskillzSocialTypes.h"
#include "DeskillzTypes.h"
#include "DeskillzSocialGameManager.generated.h"

class UDeskillzSDK;
class UDeskillzRakeCalculator;
class UDeskillzBuyInManager;

/**
 * Deskillz Social Game Manager
 * 
 * Manages social game sessions with real-money stakes, including buy-ins,
 * rake collection, turn timers, pause system, and settlements.
 * 
 * Features:
 * - Session lifecycle management
 * - Buy-in/rebuy/cash-out flow
 * - Turn-based timer system
 * - Pause voting system
 * - Real-time balance tracking
 * - Rake calculation and settlement
 * 
 * Usage:
 * 1. Get via GetGameInstance()->GetSubsystem<UDeskillzSocialGameManager>()
 * 2. Call JoinSession() with buy-in amount
 * 3. Use game actions (bet, fold, etc.) via your game logic
 * 4. Handle events for UI updates
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSocialGameManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDeskillzSocialGameManager();

    // ========================================================================
    // Subsystem Lifecycle
    // ========================================================================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // ========================================================================
    // Static Accessors
    // ========================================================================

    /**
     * Get the Social Game Manager instance
     * @param WorldContextObject Any UObject to get world context from
     * @return The Social Game Manager instance or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Social Game Manager"))
    static UDeskillzSocialGameManager* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Session Management
    // ========================================================================

    /**
     * Join a social game session
     * @param RoomId Room to join
     * @param BuyInAmount Initial buy-in amount in dollars
     * @param Currency Currency to use
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void JoinSession(const FString& RoomId, double BuyInAmount, EDeskillzCurrency Currency = EDeskillzCurrency::USDT_BSC);

    /**
     * Leave current session (cash out)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void LeaveSession();

    /**
     * Get current session
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzSocialSession GetCurrentSession() const { return CurrentSession; }

    /**
     * Check if in a session
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    bool IsInSession() const { return bInSession; }

    /**
     * Get current session state
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    EDeskillzSocialSessionState GetSessionState() const { return CurrentSession.State; }

    /**
     * Check if local player is host
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    bool IsHost() const;

    // ========================================================================
    // Buy-In / Rebuy / Cash Out
    // ========================================================================

    /**
     * Request a rebuy
     * @param Amount Amount to rebuy in dollars
     * @param Currency Currency to use
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void RequestRebuy(double Amount, EDeskillzCurrency Currency = EDeskillzCurrency::USDT_BSC);

    /**
     * Request cash out
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void RequestCashOut();

    /**
     * Get buy-in options for current room
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzBuyInOptions GetBuyInOptions() const;

    /**
     * Validate a buy-in amount
     * @param Amount Amount to validate
     * @return Validation result
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzBuyInValidation ValidateBuyIn(double Amount) const;

    /**
     * Check if rebuys are available
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    bool CanRebuy() const;

    /**
     * Get remaining rebuys
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    int32 GetRemainingRebuys() const;

    // ========================================================================
    // Player State
    // ========================================================================

    /**
     * Get local player info
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzSocialPlayer GetLocalPlayer() const;

    /**
     * Get local player balance (in points)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    int32 GetLocalBalance() const;

    /**
     * Get local player balance in dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    double GetLocalBalanceDollars() const;

    /**
     * Get player by ID
     * @param PlayerId Player ID to find
     * @return Player info (empty if not found)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzSocialPlayer GetPlayer(const FString& PlayerId) const;

    /**
     * Get all players in session
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    TArray<FDeskillzSocialPlayer> GetPlayers() const { return CurrentSession.Players; }

    /**
     * Get active player count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    int32 GetActivePlayerCount() const { return CurrentSession.GetActivePlayerCount(); }

    /**
     * Mark local player as ready
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void SetReady(bool bReady = true);

    /**
     * Sit out (temporarily leave active play)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void SitOut();

    /**
     * Return from sitting out
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void SitIn();

    // ========================================================================
    // Turn System
    // ========================================================================

    /**
     * Check if it's local player's turn
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    bool IsMyTurn() const;

    /**
     * Get current turn player ID
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FString GetCurrentTurnPlayerId() const { return CurrentSession.CurrentTurnPlayerId; }

    /**
     * Get remaining turn time in seconds
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    float GetRemainingTurnTime() const;

    /**
     * Get turn timer duration
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    int32 GetTurnTimerDuration() const { return CurrentSession.Settings.TurnTimerSeconds; }

    /**
     * Submit turn action (generic - implement game-specific actions)
     * @param ActionType Action type string
     * @param ActionData Action data as JSON string
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void SubmitTurnAction(const FString& ActionType, const FString& ActionData = TEXT(""));

    // ========================================================================
    // Round & Pot
    // ========================================================================

    /**
     * Get current round number
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    int32 GetCurrentRound() const { return CurrentSession.CurrentRound; }

    /**
     * Get current pot
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzSocialPot GetCurrentPot() const { return CurrentSession.CurrentPot; }

    /**
     * Get pot value in dollars
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    double GetPotDollars() const;

    /**
     * Get total rake collected this session
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    double GetTotalRakeCollected() const { return CurrentSession.TotalRakeCollected; }

    // ========================================================================
    // Pause System
    // ========================================================================

    /**
     * Request a pause
     * @param Reason Optional reason for pause
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void RequestPause(const FString& Reason = TEXT(""));

    /**
     * Vote on pause request
     * @param bApprove Whether to approve the pause
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void VoteOnPause(bool bApprove);

    /**
     * Resume from pause (host only)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Social")
    void ResumeGame();

    /**
     * Check if game is paused
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    bool IsPaused() const { return CurrentSession.bIsPaused; }

    /**
     * Get active pause info
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzActivePause GetActivePause() const { return CurrentSession.ActivePause; }

    /**
     * Get remaining pause time
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    float GetRemainingPauseTime() const;

    /**
     * Check if local player can request pause
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    bool CanRequestPause() const;

    /**
     * Get remaining pauses for local player
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    int32 GetRemainingPauses() const;

    // ========================================================================
    // Game Settings
    // ========================================================================

    /**
     * Get current game settings
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    FDeskillzSocialGameSettings GetSettings() const { return CurrentSession.Settings; }

    /**
     * Get point value
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    double GetPointValue() const { return CurrentSession.Settings.PointValue; }

    /**
     * Get rake percentage
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    float GetRakePercent() const { return CurrentSession.Settings.RakePercent; }

    /**
     * Convert points to dollars
     * @param Points Point amount
     * @return Dollar amount
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    double PointsToDollars(int32 Points) const;

    /**
     * Convert dollars to points
     * @param Dollars Dollar amount
     * @return Point amount
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    int32 DollarsToPoints(double Dollars) const;

    // ========================================================================
    // Rake Calculator Access
    // ========================================================================

    /**
     * Get the rake calculator
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    UDeskillzRakeCalculator* GetRakeCalculator() const { return RakeCalculator; }

    /**
     * Get the buy-in manager
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Social")
    UDeskillzBuyInManager* GetBuyInManager() const { return BuyInManager; }

    // ========================================================================
    // Events / Delegates
    // ========================================================================

    /** Session state changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialSessionStateChanged OnSessionStateChanged;

    /** Player joined */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialPlayerJoined OnPlayerJoined;

    /** Player left */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialPlayerLeft OnPlayerLeft;

    /** Balance updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialBalanceUpdated OnBalanceUpdated;

    /** Turn started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialTurnStarted OnTurnStarted;

    /** Turn timer tick */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialTurnTimerTick OnTurnTimerTick;

    /** Turn timer expired */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialTurnTimerExpired OnTurnTimerExpired;

    /** Round started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialRoundStarted OnRoundStarted;

    /** Round ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialRoundEnded OnRoundEnded;

    /** Pot updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialPotUpdated OnPotUpdated;

    /** Pause requested */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialPauseRequested OnPauseRequested;

    /** Pause vote updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialPauseVoteUpdated OnPauseVoteUpdated;

    /** Game paused */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialGamePaused OnGamePaused;

    /** Game resumed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialGameResumed OnGameResumed;

    /** Buy-in completed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialBuyInCompleted OnBuyInCompleted;

    /** Cash out completed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialCashOutCompleted OnCashOutCompleted;

    /** Session settlement */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Social|Events")
    FOnDeskillzSocialSessionSettlement OnSessionSettlement;

protected:
    // ========================================================================
    // Internal State
    // ========================================================================

    /** Reference to main SDK */
    UPROPERTY()
    UDeskillzSDK* SDK = nullptr;

    /** Rake calculator */
    UPROPERTY()
    UDeskillzRakeCalculator* RakeCalculator = nullptr;

    /** Buy-in manager */
    UPROPERTY()
    UDeskillzBuyInManager* BuyInManager = nullptr;

    /** Current session */
    UPROPERTY()
    FDeskillzSocialSession CurrentSession;

    /** Local player ID */
    FString LocalPlayerId;

    /** Whether in a session */
    bool bInSession = false;

    /** Turn timer handle */
    FTimerHandle TurnTimerHandle;

    /** Remaining turn time */
    float RemainingTurnTime = 0.0f;

    /** Pause cooldown end time */
    FDateTime PauseCooldownEnd;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Handle WebSocket messages */
    void HandleWebSocketMessage(const FString& Message);

    /** Update session from JSON */
    void UpdateSessionFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Parse player from JSON */
    FDeskillzSocialPlayer ParsePlayerFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Start turn timer */
    void StartTurnTimer(float Duration);

    /** Stop turn timer */
    void StopTurnTimer();

    /** Turn timer tick */
    void OnTurnTimerTick();

    /** Handle state transition */
    void HandleStateTransition(EDeskillzSocialSessionState NewState);

    /** Find local player index */
    int32 FindLocalPlayerIndex() const;

    /** Send WebSocket message */
    void SendWebSocketMessage(const FString& EventType, const TSharedPtr<FJsonObject>& Data);
};