// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Spectator/DeskillzSpectatorTypes.h"
#include "DeskillzSpectatorScorePanelWidget.generated.h"

class UVerticalBox;
class UScrollBox;
class UTextBlock;
class UBorder;
class UButton;

/**
 * Score panel layout
 */
UENUM(BlueprintType)
enum class EDeskillzScorePanelLayout : uint8
{
    Vertical    UMETA(DisplayName = "Vertical List"),
    Horizontal  UMETA(DisplayName = "Horizontal Bar"),
    Grid        UMETA(DisplayName = "Grid"),
    Compact     UMETA(DisplayName = "Compact")
};

/**
 * Deskillz Spectator Score Panel Widget
 * 
 * Displays all player scores in spectator mode:
 * - Player avatars and names
 * - Current scores/balances
 * - Turn indicator
 * - Connection status
 * - Clickable for follow mode
 * 
 * Usage:
 * 1. Add to spectator view
 * 2. Call SetPlayers() with player array
 * 3. Update via UpdatePlayer() for changes
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzSpectatorScorePanelWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzSpectatorScorePanelWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Data Binding
    // ========================================================================

    /**
     * Set all players
     * @param Players Player info array
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void SetPlayers(const TArray<FDeskillzSpectatorPlayerInfo>& Players);

    /**
     * Update single player
     * @param Player Updated player info
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void UpdatePlayer(const FDeskillzSpectatorPlayerInfo& Player);

    /**
     * Add player
     * @param Player Player to add
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void AddPlayer(const FDeskillzSpectatorPlayerInfo& Player);

    /**
     * Remove player
     * @param PlayerId Player to remove
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void RemovePlayer(const FString& PlayerId);

    /**
     * Update score for player
     * @param Update Score update data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void UpdateScore(const FDeskillzScoreUpdate& Update);

    /**
     * Set current turn player
     * @param PlayerId Player whose turn it is
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void SetCurrentTurn(const FString& PlayerId);

    /**
     * Set selected/followed player
     * @param PlayerId Player being followed
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void SetSelectedPlayer(const FString& PlayerId);

    /**
     * Clear selection
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Spectator|UI")
    void ClearSelection();

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Panel layout */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    EDeskillzScorePanelLayout Layout = EDeskillzScorePanelLayout::Vertical;

    /** Show avatars */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bShowAvatars = true;

    /** Show score changes (delta) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bShowScoreChanges = true;

    /** Show connection status */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bShowConnectionStatus = true;

    /** Animate score changes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bAnimateScoreChanges = true;

    /** Clickable rows for follow mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    bool bClickableRows = true;

    /** Player row widget class */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Spectator|UI")
    TSubclassOf<UUserWidget> PlayerRowWidgetClass;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when player row is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerClicked, const FString&, PlayerId);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Spectator|UI")
    FOnPlayerClicked OnPlayerClicked;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Panel container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UBorder* PanelContainer;

    /** Title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UTextBlock* TitleText;

    /** Player list scroll box */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UScrollBox* PlayerListScroll;

    /** Player list container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UVerticalBox* PlayerListContainer;

    /** Total pot text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Spectator|UI")
    UTextBlock* TotalPotText;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** All players */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator|UI")
    TArray<FDeskillzSpectatorPlayerInfo> Players;

    /** Current turn player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator|UI")
    FString CurrentTurnPlayerId;

    /** Selected player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Spectator|UI")
    FString SelectedPlayerId;

    /** Player row widgets map */
    UPROPERTY()
    TMap<FString, UUserWidget*> PlayerRowWidgets;

    // ========================================================================
    // Internal Methods
    // ========================================================================

    /** Rebuild entire list */
    void RebuildPlayerList();

    /** Create player row widget */
    UUserWidget* CreatePlayerRow(const FDeskillzSpectatorPlayerInfo& Player);

    /** Update player row widget */
    void UpdatePlayerRow(UUserWidget* RowWidget, const FDeskillzSpectatorPlayerInfo& Player);

    /** Find player index */
    int32 FindPlayerIndex(const FString& PlayerId) const;

    /** Sort players (by score or seat) */
    void SortPlayers();

    /** Handle row clicked */
    UFUNCTION()
    void OnRowClicked(const FString& PlayerId);

    /** Animate score change */
    void AnimateScoreChange(UUserWidget* RowWidget, int32 Delta);
};