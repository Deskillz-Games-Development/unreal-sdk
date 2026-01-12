// =============================================================================
// Deskillz SDK for Unreal Engine - Host Spectator View Widget
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Main spectator view widget for hosts monitoring rooms.
// Only the room creator can use this widget.
// Shows board state and scores but NOT player hands (anti-cheat protection).
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostSpectatorTypes.h"
#include "DeskillzHostSpectatorViewWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UBorder;
class USlider;
class UWidgetSwitcher;
class UVerticalBox;
class UScrollBox;

/**
 * Deskillz Host Spectator View Widget
 * 
 * IMPORTANT: This is a HOST-ONLY feature.
 * - Only the room creator can use this widget
 * - Shows: game board, scores, turn indicator, chat
 * - Does NOT show: player hands, hidden cards (anti-cheat)
 * 
 * Main UI for hosts monitoring their private social rooms:
 * - Room info header with host badge
 * - Anti-cheat notice banner
 * - Game board display (no hands visible)
 * - Score panel integration
 * - Room switcher for multi-room hosting
 * - Chat panel
 * 
 * Usage:
 * 1. Create widget and add to viewport
 * 2. Call SetSession() with host spectator session
 * 3. Subscribe to OnExitRequested for cleanup
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostSpectatorViewWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostSpectatorViewWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ========================================================================
    // Data Binding
    // ========================================================================

    /**
     * Set the host spectator session data.
     * @param Session Your room's spectator session
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetSession(const FDeskillzHostSpectatorSession& Session);

    /**
     * Update game state display.
     * Note: Player hands are NOT included (anti-cheat).
     * @param State Current game state snapshot
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdateGameState(const FDeskillzHostGameState& State);

    /**
     * Update player list.
     * Note: Player hands/cards are NOT visible.
     * @param Players List of players with public info only
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdatePlayers(const TArray<FDeskillzHostPlayerInfo>& Players);

    /**
     * Add chat message to display.
     * @param Message Chat message
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void AddChatMessage(const FDeskillzHostChatMessage& Message);

    /**
     * Show round result overlay.
     * @param Result Round result data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void ShowRoundResult(const FDeskillzHostRoundResult& Result);

    /**
     * Show game end result.
     * @param Result Game end data including your rake earnings
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void ShowGameEndResult(const FDeskillzHostGameEndResult& Result);

    /**
     * Set available rooms for room switcher.
     * @param Rooms Your other active rooms
     * @param CurrentRoomId Current room being watched
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetAvailableRooms(const TArray<FDeskillzHostRoom>& Rooms, const FString& CurrentRoomId);

    // ========================================================================
    // UI State
    // ========================================================================

    /**
     * Set chat panel visibility.
     * @param bVisible Whether to show chat
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetChatVisible(bool bVisible);

    /**
     * Set score panel visibility.
     * @param bVisible Whether to show scores
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetScorePanelVisible(bool bVisible);

    /**
     * Show pause state.
     * @param Duration Pause duration in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void ShowPauseState(float Duration);

    /**
     * Hide pause state.
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void HidePauseState();

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Whether to show chat panel by default */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowChatByDefault = true;

    /** Whether to show score panel by default */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowScorePanelByDefault = true;

    /** Whether to show room switcher (multi-room hosting) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowRoomSwitcher = true;

    /** Anti-cheat notice text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    FText AntiCheatNoticeText = NSLOCTEXT("Deskillz", "AntiCheatNotice", 
        "HOST VIEW: You can see scores and board state. Player hands are HIDDEN for fair play.");

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when exit button is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitRequested);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|UI")
    FOnExitRequested OnExitRequested;

    /** Called when room switch is requested */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomSwitchRequested, const FString&, RoomId);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|UI")
    FOnRoomSwitchRequested OnRoomSwitchRequested;

    /** Called when chat message is sent */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageSent, const FString&, Message);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|UI")
    FOnChatMessageSent OnChatMessageSent;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Header container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UBorder* HeaderContainer;

    /** Host badge text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* HostBadgeText;

    /** Room name text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* RoomNameText;

    /** Room code text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* RoomCodeText;

    /** Exit button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UButton* ExitButton;

    /** Anti-cheat notice banner */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UBorder* AntiCheatBanner;

    /** Anti-cheat notice text block */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* AntiCheatText;

    /** Game info bar */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UBorder* GameInfoBar;

    /** Game name text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* GameNameText;

    /** Round text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* RoundText;

    /** Pot text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* PotText;

    /** Phase text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* PhaseText;

    /** Turn timer text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* TurnTimerText;

    /** Game board container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UBorder* GameBoardContainer;

    /** Board state text (placeholder for game-specific rendering) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* BoardStateText;

    /** Chat scroll box */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UScrollBox* ChatScrollBox;

    /** Chat messages container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UVerticalBox* ChatMessagesContainer;

    /** Toggle chat button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UButton* ToggleChatButton;

    /** Toggle scores button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UButton* ToggleScoresButton;

    /** Pause overlay */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UBorder* PauseOverlay;

    /** Pause text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* PauseText;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Current session data */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    FDeskillzHostSpectatorSession CurrentSession;

    /** Whether chat is visible */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    bool bChatVisible = true;

    /** Whether score panel is visible */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    bool bScorePanelVisible = true;

    /** Whether paused */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    bool bIsPaused = false;

    /** Pause remaining time */
    float PauseRemainingTime = 0.0f;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Update room info display */
    void UpdateRoomInfoDisplay();

    /** Update game info display */
    void UpdateGameInfoDisplay();

    /** Create chat message widget */
    UWidget* CreateChatMessageWidget(const FDeskillzHostChatMessage& Message);

    // Event handlers
    UFUNCTION()
    void OnExitButtonClicked();

    UFUNCTION()
    void OnToggleChatClicked();

    UFUNCTION()
    void OnToggleScoresClicked();
};