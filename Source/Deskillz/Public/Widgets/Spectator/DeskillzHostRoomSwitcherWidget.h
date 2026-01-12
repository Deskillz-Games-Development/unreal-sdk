// =============================================================================
// Deskillz SDK for Unreal Engine - Host Room Switcher Widget
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Room switcher for hosts monitoring multiple rooms.
// Allows hosts to switch between their active private social rooms.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostSpectatorTypes.h"
#include "DeskillzHostRoomSwitcherWidget.generated.h"

class UHorizontalBox;
class UScrollBox;
class UTextBlock;
class UButton;
class UBorder;
class UImage;

/**
 * Deskillz Host Room Switcher Widget
 * 
 * IMPORTANT: This is a HOST-ONLY feature.
 * - Only shows YOUR rooms (rooms you created)
 * - For multi-room hosting management
 * - Maximum 4 rooms can be watched simultaneously
 * 
 * Multi-room host tab switcher with:
 * - Horizontal scrollable tabs for YOUR rooms
 * - Room code and game name
 * - Live/paused status indicator
 * - Player count per room
 * - Close room button per tab
 * - Add room button (if under max)
 * 
 * Usage:
 * 1. Add to host spectator view
 * 2. Call AddRoom() for each of YOUR watched rooms
 * 3. Handle OnRoomSelected for switching
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostRoomSwitcherWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostRoomSwitcherWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Room Management
    // ========================================================================

    /**
     * Add one of YOUR rooms as a tab.
     * @param Room Your room info
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void AddRoom(const FDeskillzHostRoom& Room);

    /**
     * Remove a room tab.
     * @param RoomId Room to remove
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void RemoveRoom(const FString& RoomId);

    /**
     * Update room info.
     * @param Room Updated room info
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdateRoom(const FDeskillzHostRoom& Room);

    /**
     * Set active room (highlighted tab).
     * @param RoomId Room to set as active
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetActiveRoom(const FString& RoomId);

    /**
     * Get active room ID.
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator|UI")
    FString GetActiveRoomId() const { return ActiveRoomId; }

    /**
     * Get room count.
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator|UI")
    int32 GetRoomCount() const { return WatchedRooms.Num(); }

    /**
     * Check if can add more rooms.
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Host|Spectator|UI")
    bool CanAddRoom() const { return WatchedRooms.Num() < MaxRooms; }

    /**
     * Clear all rooms.
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void ClearRooms();

    // ========================================================================
    // Status Updates
    // ========================================================================

    /**
     * Set room paused state.
     * @param RoomId Room ID
     * @param bPaused Whether paused
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetRoomPaused(const FString& RoomId, bool bPaused);

    /**
     * Update room player count.
     * @param RoomId Room ID
     * @param Count Player count
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdatePlayerCount(const FString& RoomId, int32 Count);

    /**
     * Update room round.
     * @param RoomId Room ID
     * @param RoundNumber Current round
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdateRoomRound(const FString& RoomId, int32 RoundNumber);

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Maximum rooms allowed (default 4) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    int32 MaxRooms = 4;

    /** Tab width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    float TabWidth = 180.0f;

    /** Show add room button */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowAddButton = true;

    /** Show close button on tabs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowCloseButton = true;

    /** Show room count indicator */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowRoomCount = true;

    /** Tab widget class (optional custom) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    TSubclassOf<UUserWidget> RoomTabWidgetClass;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when room tab is selected */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomSelected, const FString&, RoomId);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|UI")
    FOnRoomSelected OnRoomSelected;

    /** Called when room tab close is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCloseClicked, const FString&, RoomId);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|UI")
    FOnRoomCloseClicked OnRoomCloseClicked;

    /** Called when add room is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAddRoomClicked);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|UI")
    FOnAddRoomClicked OnAddRoomClicked;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Switcher container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UBorder* SwitcherContainer;

    /** Host label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* HostLabelText;

    /** Room count label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* RoomCountText;

    /** Tabs scroll box */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UScrollBox* TabsScrollBox;

    /** Tabs container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UHorizontalBox* TabsContainer;

    /** Add room button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UButton* AddRoomButton;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Watched rooms (YOUR rooms only) */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    TArray<FDeskillzHostRoom> WatchedRooms;

    /** Active room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    FString ActiveRoomId;

    /** Tab widgets map */
    UPROPERTY()
    TMap<FString, UUserWidget*> TabWidgets;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Create room tab widget */
    UUserWidget* CreateRoomTab(const FDeskillzHostRoom& Room);

    /** Update room tab widget */
    void UpdateRoomTab(UUserWidget* TabWidget, const FDeskillzHostRoom& Room);

    /** Update tab active states */
    void UpdateTabActiveStates();

    /** Update room count display */
    void UpdateRoomCountDisplay();

    /** Update add button state */
    void UpdateAddButtonState();

    /** Find room index */
    int32 FindRoomIndex(const FString& RoomId) const;

    /** Scroll to active tab */
    void ScrollToActiveTab();

    // Event handlers
    UFUNCTION()
    void OnTabClicked(const FString& RoomId);

    UFUNCTION()
    void OnTabCloseClicked(const FString& RoomId);

    UFUNCTION()
    void OnAddButtonClicked();
};