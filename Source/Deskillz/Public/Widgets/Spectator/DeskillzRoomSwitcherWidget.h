// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Spectator/DeskillzSpectatorTypes.h"
#include "DeskillzRoomSwitcherWidget.generated.h"

class UHorizontalBox;
class UScrollBox;
class UTextBlock;
class UButton;
class UBorder;
class UImage;

/**
 * Deskillz Room Switcher Widget
 * 
 * Multi-room spectator tab switcher with:
 * - Horizontal scrollable tabs
 * - Room code and game name
 * - Live/paused status indicator
 * - Spectator count per room
 * - Add room button
 * - Close room button per tab
 * 
 * Usage:
 * 1. Add to spectator view
 * 2. Call AddRoom() for each watched room
 * 3. Handle OnRoomSelected for switching
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzRoomSwitcherWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzRoomSwitcherWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Room Management
    // ========================================================================

    /**
     * Add a room tab
     * @param Room Room info
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void AddRoom(const FDeskillzWatchedRoom& Room);

    /**
     * Remove a room tab
     * @param RoomId Room to remove
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void RemoveRoom(const FString& RoomId);

    /**
     * Update room info
     * @param Room Updated room info
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void UpdateRoom(const FDeskillzWatchedRoom& Room);

    /**
     * Set active room
     * @param RoomId Room to set as active
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void SetActiveRoom(const FString& RoomId);

    /**
     * Get active room ID
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator|UI")
    FString GetActiveRoomId() const { return ActiveRoomId; }

    /**
     * Get room count
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator|UI")
    int32 GetRoomCount() const { return WatchedRooms.Num(); }

    /**
     * Check if can add more rooms
     */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Spectator|UI")
    bool CanAddRoom() const { return WatchedRooms.Num() < MaxRooms; }

    /**
     * Clear all rooms
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void ClearRooms();

    // ========================================================================
    // Status Updates
    // ========================================================================

    /**
     * Set room paused state
     * @param RoomId Room ID
     * @param bPaused Whether paused
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void SetRoomPaused(const FString& RoomId, bool bPaused);

    /**
     * Update room spectator count
     * @param RoomId Room ID
     * @param Count Spectator count
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void UpdateSpectatorCount(const FString& RoomId, int32 Count);

    /**
     * Update room round
     * @param RoomId Room ID
     * @param RoundNumber Current round
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void UpdateRoomRound(const FString& RoomId, int32 RoundNumber);

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Maximum rooms allowed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    int32 MaxRooms = 4;

    /** Tab width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    float TabWidth = 180.0f;

    /** Show add room button */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bShowAddButton = true;

    /** Show close button on tabs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bShowCloseButton = true;

    /** Show room count */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bShowRoomCount = true;

    /** Tab widget class */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    TSubclassOf<UUserWidget> RoomTabWidgetClass;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when room tab is selected */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomSelected, const FString&, RoomId);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|UI")
    FOnRoomSelected OnRoomSelected;

    /** Called when room tab close is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCloseClicked, const FString&, RoomId);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|UI")
    FOnRoomCloseClicked OnRoomCloseClicked;

    /** Called when add room is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAddRoomClicked);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|UI")
    FOnAddRoomClicked OnAddRoomClicked;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Switcher container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UBorder* SwitcherContainer;

    /** Room count label */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UTextBlock* RoomCountText;

    /** Tabs scroll box */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UScrollBox* TabsScrollBox;

    /** Tabs container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UHorizontalBox* TabsContainer;

    /** Add room button */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UButton* AddRoomButton;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** Watched rooms */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator|UI")
    TArray<FDeskillzWatchedRoom> WatchedRooms;

    /** Active room ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator|UI")
    FString ActiveRoomId;

    /** Tab widgets map */
    UPROPERTY()
    TMap<FString, UUserWidget*> TabWidgets;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Create room tab widget */
    UUserWidget* CreateRoomTab(const FDeskillzWatchedRoom& Room);

    /** Update room tab widget */
    void UpdateRoomTab(UUserWidget* TabWidget, const FDeskillzWatchedRoom& Room);

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