// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeskillzSpectatorTypes.h"
#include "DeskillzTypes.h"
#include "DeskillzSpectatorManager.generated.h"

class UDeskillzSDK;
class IWebSocket;

/**
 * Deskillz Spectator Manager
 * 
 * Manages spectator mode for watching live games. Supports watching
 * single or multiple rooms simultaneously with real-time updates.
 * 
 * Features:
 * - Join/leave spectator sessions
 * - Real-time game state updates
 * - Action feed streaming
 * - Multi-room spectating
 * - View mode control (overview, follow player)
 * - Score tracking
 * 
 * Usage:
 * 1. Get via GetGameInstance()->GetSubsystem<UDeskillzSpectatorManager>()
 * 2. Call JoinSpectator(RoomId) to start watching
 * 3. Subscribe to events for real-time updates
 * 4. Call LeaveSpectator() when done
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzSpectatorManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDeskillzSpectatorManager();

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
     * Get the Spectator Manager instance
     * @param WorldContextObject Any UObject to get world context from
     * @return The Spectator Manager instance or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Spectator Manager"))
    static UDeskillzSpectatorManager* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Spectator Session
    // ========================================================================

    /**
     * Join a room as spectator
     * @param RoomId Room to spectate
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void JoinSpectator(const FString& RoomId);

    /**
     * Leave current spectator session
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void LeaveSpectator();

    /**
     * Leave a specific room (multi-room mode)
     * @param RoomId Room to leave
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void LeaveRoom(const FString& RoomId);

    /**
     * Check if spectating
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    bool IsSpectating() const { return bIsSpectating; }

    /**
     * Get current session
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    FDeskillzSpectatorSession GetCurrentSession() const { return CurrentSession; }

    /**
     * Get connection state
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    EDeskillzSpectatorConnectionState GetConnectionState() const { return ConnectionState; }

    // ========================================================================
    // Multi-Room Spectating
    // ========================================================================

    /**
     * Add a room to watch (multi-room mode)
     * @param RoomId Room to add
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void AddWatchedRoom(const FString& RoomId);

    /**
     * Remove a watched room
     * @param RoomId Room to remove
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void RemoveWatchedRoom(const FString& RoomId);

    /**
     * Switch to a different watched room
     * @param RoomId Room to switch to
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void SwitchToRoom(const FString& RoomId);

    /**
     * Get all watched rooms
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    TArray<FDeskillzWatchedRoom> GetWatchedRooms() const { return WatchedRooms; }

    /**
     * Get watched room count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    int32 GetWatchedRoomCount() const { return WatchedRooms.Num(); }

    /**
     * Check if watching a specific room
     * @param RoomId Room to check
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    bool IsWatchingRoom(const FString& RoomId) const;

    // ========================================================================
    // Game State
    // ========================================================================

    /**
     * Get current game state snapshot
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    FDeskillzGameStateSnapshot GetCurrentState() const { return CurrentSession.CurrentState; }

    /**
     * Get all players
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    TArray<FDeskillzSpectatorPlayerInfo> GetPlayers() const { return CurrentSession.Players; }

    /**
     * Get player by ID
     * @param PlayerId Player to find
     * @return Player info (empty if not found)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    FDeskillzSpectatorPlayerInfo GetPlayer(const FString& PlayerId) const;

    /**
     * Get current turn player
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    FDeskillzSpectatorPlayerInfo GetCurrentTurnPlayer() const;

    /**
     * Get spectator count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    int32 GetSpectatorCount() const { return CurrentSession.SpectatorCount; }

    /**
     * Get current round
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    int32 GetCurrentRound() const { return CurrentSession.CurrentState.RoundNumber; }

    /**
     * Get current pot
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    double GetCurrentPot() const { return CurrentSession.CurrentState.CurrentPot; }

    /**
     * Check if game is paused
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    bool IsPaused() const { return CurrentSession.CurrentState.bIsPaused; }

    // ========================================================================
    // View Mode
    // ========================================================================

    /**
     * Set view mode
     * @param Mode New view mode
     * @param FollowPlayerId Player to follow (if FollowPlayer mode)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void SetViewMode(EDeskillzSpectatorViewMode Mode, const FString& FollowPlayerId = TEXT(""));

    /**
     * Get current view mode
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    EDeskillzSpectatorViewMode GetViewMode() const { return Settings.ViewMode; }

    /**
     * Get followed player ID
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    FString GetFollowedPlayerId() const { return Settings.FollowedPlayerId; }

    /**
     * Follow a specific player
     * @param PlayerId Player to follow
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void FollowPlayer(const FString& PlayerId);

    /**
     * Stop following and return to overview
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void StopFollowing();

    // ========================================================================
    // Action Feed
    // ========================================================================

    /**
     * Get recent actions
     * @param Count Number of actions to get
     * @return Recent actions
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    TArray<FDeskillzGameAction> GetRecentActions(int32 Count = 20) const;

    /**
     * Clear action feed
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void ClearActionFeed();

    // ========================================================================
    // Settings
    // ========================================================================

    /**
     * Get spectator settings
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    FDeskillzSpectatorSettings GetSettings() const { return Settings; }

    /**
     * Update spectator settings
     * @param NewSettings New settings to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void UpdateSettings(const FDeskillzSpectatorSettings& NewSettings);

    /**
     * Toggle action feed visibility
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void ToggleActionFeed();

    /**
     * Toggle score panel visibility
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void ToggleScores();

    // ========================================================================
    // Utility
    // ========================================================================

    /**
     * Refresh game state (manual sync)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator")
    void RefreshState();

    /**
     * Get remaining turn time
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator")
    float GetRemainingTurnTime() const { return CurrentSession.CurrentState.TurnTimeRemaining; }

    // ========================================================================
    // Events / Delegates
    // ========================================================================

    /** Connected to spectator session */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorConnected OnConnected;

    /** Disconnected from session */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorDisconnected OnDisconnected;

    /** Connection state changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorConnectionStateChanged OnConnectionStateChanged;

    /** Game state updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorGameStateUpdated OnGameStateUpdated;

    /** Score updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorScoreUpdated OnScoreUpdated;

    /** Game action */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorGameAction OnGameAction;

    /** Player updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorPlayerUpdated OnPlayerUpdated;

    /** Spectator count changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorCountChanged OnSpectatorCountChanged;

    /** Game paused */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorGamePaused OnGamePaused;

    /** Game resumed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorGameResumed OnGameResumed;

    /** Round started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorRoundStarted OnRoundStarted;

    /** Round ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorRoundEnded OnRoundEnded;

    /** View mode changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|Events")
    FOnDeskillzSpectatorViewModeChanged OnViewModeChanged;

    // ========================================================================
    // Constants
    // ========================================================================

    /** Maximum watched rooms */
    static constexpr int32 MAX_WATCHED_ROOMS = 4;

    /** Action feed max items */
    static constexpr int32 ACTION_FEED_MAX_ITEMS = 100;

    /** Reconnect max attempts */
    static constexpr int32 RECONNECT_MAX_ATTEMPTS = 5;

    /** Reconnect delay in seconds */
    static constexpr float RECONNECT_DELAY = 2.0f;

protected:
    // ========================================================================
    // Internal State
    // ========================================================================

    /** Reference to main SDK */
    UPROPERTY()
    UDeskillzSDK* SDK = nullptr;

    /** Current session */
    UPROPERTY()
    FDeskillzSpectatorSession CurrentSession;

    /** Watched rooms (multi-room mode) */
    UPROPERTY()
    TArray<FDeskillzWatchedRoom> WatchedRooms;

    /** Spectator settings */
    UPROPERTY()
    FDeskillzSpectatorSettings Settings;

    /** Action feed */
    UPROPERTY()
    TArray<FDeskillzGameAction> ActionFeed;

    /** Connection state */
    EDeskillzSpectatorConnectionState ConnectionState = EDeskillzSpectatorConnectionState::Disconnected;

    /** Whether spectating */
    bool bIsSpectating = false;

    /** WebSocket connection */
    TSharedPtr<IWebSocket> SpectatorWebSocket;

    /** Reconnect attempts */
    int32 ReconnectAttempts = 0;

    /** Reconnect timer */
    FTimerHandle ReconnectTimerHandle;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Connect WebSocket */
    void ConnectWebSocket(const FString& RoomId);

    /** Disconnect WebSocket */
    void DisconnectWebSocket();

    /** Handle WebSocket connected */
    void OnWebSocketConnected();

    /** Handle WebSocket disconnected */
    void OnWebSocketDisconnected(int32 StatusCode, const FString& Reason, bool bWasClean);

    /** Handle WebSocket error */
    void OnWebSocketError(const FString& Error);

    /** Handle WebSocket message */
    void OnWebSocketMessage(const FString& Message);

    /** Attempt reconnect */
    void AttemptReconnect();

    /** Update connection state */
    void SetConnectionState(EDeskillzSpectatorConnectionState NewState);

    /** Parse session from JSON */
    void ParseSessionFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Parse player from JSON */
    FDeskillzSpectatorPlayerInfo ParsePlayerFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Parse action from JSON */
    FDeskillzGameAction ParseActionFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Parse game state from JSON */
    FDeskillzGameStateSnapshot ParseGameStateFromJson(const TSharedPtr<FJsonObject>& JsonObject);

    /** Add action to feed */
    void AddActionToFeed(const FDeskillzGameAction& Action);

    /** Update watched room */
    void UpdateWatchedRoom(const FString& RoomId, const FDeskillzWatchedRoom& RoomData);

    /** Find watched room index */
    int32 FindWatchedRoomIndex(const FString& RoomId) const;
};