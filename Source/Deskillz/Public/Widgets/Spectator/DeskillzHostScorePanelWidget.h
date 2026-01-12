// =============================================================================
// Deskillz SDK for Unreal Engine - Host Score Panel Widget
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Score panel widget for hosts monitoring their rooms.
// Shows player scores and turn indicators but NOT player hands (anti-cheat).
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "Host/DeskillzHostSpectatorTypes.h"
#include "DeskillzHostScorePanelWidget.generated.h"

class UVerticalBox;
class UScrollBox;
class UTextBlock;
class UBorder;
class UButton;

/**
 * Score panel layout options
 */
UENUM(BlueprintType)
enum class EDeskillzHostScorePanelLayout : uint8
{
    Vertical    UMETA(DisplayName = "Vertical List"),
    Horizontal  UMETA(DisplayName = "Horizontal Bar"),
    Compact     UMETA(DisplayName = "Compact")
};

/**
 * Deskillz Host Score Panel Widget
 * 
 * IMPORTANT: This is a HOST-ONLY feature.
 * - Shows: Player names, scores, chip stacks, turn indicator
 * - Does NOT show: Player hands, hidden cards (anti-cheat)
 * 
 * Displays player scores in host spectator mode:
 * - Player avatars and names
 * - Current scores/chip stacks
 * - Turn indicator (highlighted row)
 * - Active/folded status
 * - Clickable rows for selection
 * 
 * Usage:
 * 1. Add to host spectator view
 * 2. Call SetPlayers() with player array
 * 3. Call UpdateScore() for score changes
 * 4. Call HighlightPlayer() for turn changes
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzHostScorePanelWidget : public UDeskillzBaseWidget
{
    GENERATED_BODY()

public:
    UDeskillzHostScorePanelWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    // ========================================================================
    // Data Binding
    // ========================================================================

    /**
     * Set all players.
     * Note: Player hands/cards are NOT visible - anti-cheat protection.
     * @param Players Player info array (public info only)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetPlayers(const TArray<FDeskillzHostPlayerInfo>& Players);

    /**
     * Update scores from state snapshot.
     * @param Scores Score data array
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdateScores(const TArray<FDeskillzHostPlayerScore>& Scores);

    /**
     * Update single player's score.
     * @param PlayerId Player ID
     * @param NewScore New score value
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdatePlayerScore(const FString& PlayerId, int32 NewScore);

    /**
     * Update single player's chip stack.
     * @param PlayerId Player ID
     * @param NewChipStack New chip stack value
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void UpdatePlayerChipStack(const FString& PlayerId, double NewChipStack);

    /**
     * Add a player to the panel.
     * @param Player Player to add
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void AddPlayer(const FDeskillzHostPlayerInfo& Player);

    /**
     * Remove a player from the panel.
     * @param PlayerId Player to remove
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void RemovePlayer(const FString& PlayerId);

    /**
     * Highlight the current turn player.
     * @param PlayerId Player whose turn it is
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void HighlightPlayer(const FString& PlayerId);

    /**
     * Set player active/folded status.
     * @param PlayerId Player ID
     * @param bActive Whether player is still active
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void SetPlayerActive(const FString& PlayerId, bool bActive);

    /**
     * Clear all players.
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Host|Spectator|UI")
    void ClearPlayers();

    // ========================================================================
    // Configuration
    // ========================================================================

    /** Panel layout */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    EDeskillzHostScorePanelLayout Layout = EDeskillzHostScorePanelLayout::Vertical;

    /** Whether to sort by score */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bSortByScore = false;

    /** Whether rows are clickable */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bClickableRows = true;

    /** Show chip stack (for poker-style games) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowChipStack = true;

    /** Show active/folded indicator */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    bool bShowActiveStatus = true;

    /** Player row widget class (optional custom class) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Host|Spectator|UI")
    TSubclassOf<UUserWidget> PlayerRowWidgetClass;

    // ========================================================================
    // Events
    // ========================================================================

    /** Called when player row is clicked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerClicked, const FString&, PlayerId);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Host|Spectator|UI")
    FOnPlayerClicked OnPlayerClicked;

protected:
    // ========================================================================
    // UI Components
    // ========================================================================

    /** Panel container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UBorder* PanelContainer;

    /** Title text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* TitleText;

    /** Anti-cheat note text */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UTextBlock* AntiCheatNoteText;

    /** Player list scroll box */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UScrollBox* PlayerListScroll;

    /** Player list container */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Deskillz|Host|Spectator|UI")
    UVerticalBox* PlayerListContainer;

    // ========================================================================
    // Internal State
    // ========================================================================

    /** All players */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    TArray<FDeskillzHostPlayerInfo> Players;

    /** Current turn player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
    FString CurrentTurnPlayerId;

    /** Selected player ID */
    UPROPERTY(BlueprintReadOnly, Category = "Deskillz|Host|Spectator|UI")
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
    UUserWidget* CreatePlayerRow(const FDeskillzHostPlayerInfo& Player);

    /** Update player row widget */
    void UpdatePlayerRow(UUserWidget* RowWidget, const FDeskillzHostPlayerInfo& Player);

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