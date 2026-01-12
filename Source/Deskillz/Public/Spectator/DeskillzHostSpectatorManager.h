// =============================================================================
// Deskillz SDK for Unreal Engine - Host Spectator Manager
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Only room hosts can spectate their own private social rooms.
// General public spectating is NOT available.
// Hosts can see game board and scores but NOT player hands (anti-cheat).
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeskillzHostSpectatorTypes.h"
#include "DeskillzHostSpectatorEvents.h"
#include "DeskillzHostSpectatorManager.generated.h"

class UDeskillzSDK;

/**
 * Host Spectator Manager for the Deskillz SDK.
 * Allows HOSTS to monitor their own private social rooms in real-time.
 * 
 * IMPORTANT RESTRICTIONS:
 * - Only the HOST who created a room can spectate it
 * - Only works for PRIVATE SOCIAL rooms (not esports)
 * - Host can see: game board, scores, turn indicator, chat, round results
 * - Host CANNOT see: player hands, hidden cards, private player info
 * - This is for room management, NOT public viewing
 * 
 * Features:
 * - Monitor your own rooms in real-time
 * - Multi-room hosting (watch up to 4 of your rooms)
 * - Real-time game state updates (board/scores only)
 * - Score tracking and round results
 * - Room switching for multi-room management
 * 
 * Usage:
 * 1. Get via GetGameInstance()->GetSubsystem<UDeskillzHostSpectatorManager>()
 * 2. Call Initialize() after host authentication
 * 3. Call FetchHostRooms() to get YOUR rooms
 * 4. Call SpectateRoom(RoomId) to start monitoring
 * 5. Subscribe to events for real-time updates
 * 6. Call StopSpectating() when done
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzHostSpectatorManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDeskillzHostSpectatorManager();

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
     * Get the Host Spectator Manager instance
     * @param WorldContextObject Any UObject to get world context from
     * @return The Host Spectator Manager instance or nullptr
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Host Spectator Manager"))
    static UDeskillzHostSpectatorManager* Get(const UObject* WorldContextObject);

    // ========================================================================
    // Initialization
    // ========================================================================

    /**
     * Initialize the Host Spectator Manager.
     * Host must be authenticated before calling this.
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void InitializeHostSpectator();

    /**
     * Shutdown the Host Spectator Manager.
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void ShutdownHostSpectator();

    /**
     * Check if initialized
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    bool IsInitialized() const { return bIsInitialized; }

    // ========================================================================
    // Room Discovery (YOUR Rooms Only)
    // ========================================================================

    /**
     * Fetch YOUR rooms available for spectating.
     * Only returns private social rooms that YOU created.
     * @param Filter Optional filter criteria
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void FetchHostRooms(const FDeskillzHostRoomFilter& Filter);

    /**
     * Get cached list of your rooms
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostRoom> GetCachedRooms() const { return CachedRooms; }

    // ========================================================================
    // Spectating (HOST-ONLY)
    // ========================================================================

    /**
     * Spectate YOUR room.
     * Host can see game board and scores but NOT player hands.
     * @param RoomId YOUR room ID to spectate
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void SpectateRoom(const FString& RoomId);

    /**
     * Stop spectating current room
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void StopSpectating();

    /**
     * Stop spectating a specific room (multi-room mode)
     * @param RoomId Room to stop spectating
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void StopSpectatingRoom(const FString& RoomId);

    /**
     * Stop spectating all rooms
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void StopSpectatingAll();

    /**
     * Switch to spectating a different one of YOUR rooms.
     * For multi-room hosting management.
     * @param RoomId YOUR other room ID
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void SwitchRoom(const FString& RoomId);

    /**
     * Check if currently spectating
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    bool IsSpectating() const { return bIsSpectating; }

    /**
     * Get current session
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    FDeskillzHostSpectatorSession GetCurrentSession() const { return CurrentSession; }

    /**
     * Get all watched rooms (multi-room mode)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostSpectatorSession> GetWatchedRooms() const { return WatchedRooms; }

    /**
     * Get connection state
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    EDeskillzHostSpectatorState GetConnectionState() const { return ConnectionState; }

    // ========================================================================
    // Game State (Board/Scores Only - NO Hands)
    // ========================================================================

    /**
     * Get current game state snapshot.
     * Note: Player hands are NOT included (anti-cheat).
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    FDeskillzHostGameState GetCurrentGameState() const { return CurrentSession.CurrentState; }

    /**
     * Get player list (public info only, no hands)
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostPlayerInfo> GetPlayers() const { return CurrentSession.Players; }

    /**
     * Get player scores
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    TArray<FDeskillzHostPlayerScore> GetScores() const { return CurrentSession.CurrentState.Scores; }

    /**
     * Refresh game state (manual sync)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator")
    void RefreshState();

    /**
     * Get remaining turn time
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator")
    float GetRemainingTurnTime() const { return CurrentSession.CurrentState.TurnTimeRemaining; }

    // ========================================================================
    // Events / Delegates
    // ========================================================================

    /** Connected to your room as host spectator */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostSpectatorConnected OnConnected;

    /** Disconnected from session */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostSpectatorDisconnected OnDisconnected;

    /** Connection state changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostSpectatorStateChanged OnStateChanged;

    /** Your rooms fetched */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostRoomsFetched OnRoomsFetched;

    /** Game state updated (board/scores, NOT hands) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostGameStateUpdated OnGameStateUpdated;

    /** Score updated */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostScoreUpdated OnScoreUpdated;

    /** Turn changed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostTurnChanged OnTurnChanged;

    /** Round started */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostRoundStarted OnRoundStarted;

    /** Round ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostRoundEnded OnRoundEnded;

    /** Chat message received */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostChatReceived OnChatReceived;

    /** Game paused */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostGamePaused OnGamePaused;

    /** Game resumed */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostGameResumed OnGameResumed;

    /** Game ended */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostGameEnded OnGameEnded;

    /** Room switched (multi-room hosting) */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostRoomSwitched OnRoomSwitched;

    /** Error occurred */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|Events")
    FOnDeskillzHostSpectatorError OnError;

    // ========================================================================
    // Constants
    // ========================================================================

    /** Maximum rooms that can be watched simultaneously */
    static constexpr int32 MAX_WATCHED_ROOMS = 4;

    /** Maximum reconnect attempts */
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

    /** Whether initialized */
    bool bIsInitialized = false;

    /** Whether currently spectating */
    bool bIsSpectating = false;

    /** Connection state */
    EDeskillzHostSpectatorState ConnectionState = EDeskillzHostSpectatorState::Disconnected;

    /** Current session */
    FDeskillzHostSpectatorSession CurrentSession;

    /** All watched rooms (multi-room mode) */
    TArray<FDeskillzHostSpectatorSession> WatchedRooms;

    /** Cached room list */
    TArray<FDeskillzHostRoom> CachedRooms;

    /** Reconnect attempt counter */
    int32 ReconnectAttempts = 0;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Set connection state and broadcast event */
    void SetConnectionState(EDeskillzHostSpectatorState NewState);

    /** Connect WebSocket for real-time updates */
    void ConnectWebSocket(const FString& RoomId);

    /** Disconnect WebSocket */
    void DisconnectWebSocket();

    /** Handle WebSocket message */
    void HandleWebSocketMessage(const FString& Message);

    /** Remove a room from watched list */
    void RemoveWatchedRoom(const FString& RoomId);

    /** Handle reconnection */
    void AttemptReconnect();
};