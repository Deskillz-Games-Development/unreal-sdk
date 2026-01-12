// =============================================================================
// Deskillz SDK for Unreal Engine - Host Score Panel Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Score panel widget for hosts monitoring their rooms.
// Shows player scores and turn indicators but NOT player hands (anti-cheat).
// =============================================================================

#include "UI/Widgets/Spectator/DeskillzHostScorePanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"

UDeskillzHostScorePanelWidget::UDeskillzHostScorePanelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UDeskillzHostScorePanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Set title
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(TEXT("PLAYER SCORES")));
    }

    // Set anti-cheat note
    if (AntiCheatNoteText)
    {
        AntiCheatNoteText->SetText(FText::FromString(TEXT("Hands hidden for fair play")));
    }
}

// ============================================================================
// Data Binding
// ============================================================================

void UDeskillzHostScorePanelWidget::SetPlayers(const TArray<FDeskillzHostPlayerInfo>& InPlayers)
{
    Players = InPlayers;

    if (bSortByScore)
    {
        SortPlayers();
    }

    RebuildPlayerList();
}

void UDeskillzHostScorePanelWidget::UpdateScores(const TArray<FDeskillzHostPlayerScore>& Scores)
{
    for (const FDeskillzHostPlayerScore& Score : Scores)
    {
        UpdatePlayerScore(Score.PlayerId, Score.Score);
    }
}

void UDeskillzHostScorePanelWidget::UpdatePlayerScore(const FString& PlayerId, int32 NewScore)
{
    // Find and update player
    for (FDeskillzHostPlayerInfo& Player : Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            int32 OldScore = Player.Score;
            Player.Score = NewScore;

            // Update row widget
            if (UUserWidget** RowWidgetPtr = PlayerRowWidgets.Find(PlayerId))
            {
                UpdatePlayerRow(*RowWidgetPtr, Player);

                // Animate score change
                if (OldScore != NewScore)
                {
                    AnimateScoreChange(*RowWidgetPtr, NewScore - OldScore);
                }
            }
            break;
        }
    }

    // Re-sort if needed
    if (bSortByScore)
    {
        SortPlayers();
        RebuildPlayerList();
    }
}

void UDeskillzHostScorePanelWidget::UpdatePlayerChipStack(const FString& PlayerId, double NewChipStack)
{
    for (FDeskillzHostPlayerInfo& Player : Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            Player.ChipStack = NewChipStack;

            if (UUserWidget** RowWidgetPtr = PlayerRowWidgets.Find(PlayerId))
            {
                UpdatePlayerRow(*RowWidgetPtr, Player);
            }
            break;
        }
    }
}

void UDeskillzHostScorePanelWidget::AddPlayer(const FDeskillzHostPlayerInfo& Player)
{
    // Check if already exists
    for (const FDeskillzHostPlayerInfo& Existing : Players)
    {
        if (Existing.PlayerId == Player.PlayerId)
        {
            return;
        }
    }

    Players.Add(Player);

    if (bSortByScore)
    {
        SortPlayers();
        RebuildPlayerList();
    }
    else
    {
        // Just add the new row
        UUserWidget* RowWidget = CreatePlayerRow(Player);
        if (RowWidget && PlayerListContainer)
        {
            PlayerListContainer->AddChild(RowWidget);
            PlayerRowWidgets.Add(Player.PlayerId, RowWidget);
        }
    }
}

void UDeskillzHostScorePanelWidget::RemovePlayer(const FString& PlayerId)
{
    // Remove from array
    Players.RemoveAll([&PlayerId](const FDeskillzHostPlayerInfo& Player) {
        return Player.PlayerId == PlayerId;
    });

    // Remove widget
    if (UUserWidget** RowWidgetPtr = PlayerRowWidgets.Find(PlayerId))
    {
        if (*RowWidgetPtr)
        {
            (*RowWidgetPtr)->RemoveFromParent();
        }
        PlayerRowWidgets.Remove(PlayerId);
    }
}

void UDeskillzHostScorePanelWidget::HighlightPlayer(const FString& PlayerId)
{
    FString OldTurnPlayer = CurrentTurnPlayerId;
    CurrentTurnPlayerId = PlayerId;

    // Update old player row (remove highlight)
    if (!OldTurnPlayer.IsEmpty() && OldTurnPlayer != PlayerId)
    {
        if (UUserWidget** OldRowPtr = PlayerRowWidgets.Find(OldTurnPlayer))
        {
            // Remove highlight styling
            // Implementation would update visual state
        }
    }

    // Update new player row (add highlight)
    if (!PlayerId.IsEmpty())
    {
        if (UUserWidget** NewRowPtr = PlayerRowWidgets.Find(PlayerId))
        {
            // Add highlight styling
            // Implementation would update visual state
        }
    }

    // Update player data
    for (FDeskillzHostPlayerInfo& Player : Players)
    {
        Player.bIsCurrentTurn = (Player.PlayerId == PlayerId);
    }
}

void UDeskillzHostScorePanelWidget::SetPlayerActive(const FString& PlayerId, bool bActive)
{
    for (FDeskillzHostPlayerInfo& Player : Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            Player.bIsActive = bActive;

            if (UUserWidget** RowWidgetPtr = PlayerRowWidgets.Find(PlayerId))
            {
                UpdatePlayerRow(*RowWidgetPtr, Player);
            }
            break;
        }
    }
}

void UDeskillzHostScorePanelWidget::ClearPlayers()
{
    Players.Empty();

    // Remove all widgets
    for (auto& Pair : PlayerRowWidgets)
    {
        if (Pair.Value)
        {
            Pair.Value->RemoveFromParent();
        }
    }
    PlayerRowWidgets.Empty();

    CurrentTurnPlayerId.Empty();
    SelectedPlayerId.Empty();
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzHostScorePanelWidget::RebuildPlayerList()
{
    if (!PlayerListContainer)
    {
        return;
    }

    // Clear existing widgets
    PlayerListContainer->ClearChildren();
    PlayerRowWidgets.Empty();

    // Create new rows
    for (const FDeskillzHostPlayerInfo& Player : Players)
    {
        UUserWidget* RowWidget = CreatePlayerRow(Player);
        if (RowWidget)
        {
            PlayerListContainer->AddChild(RowWidget);
            PlayerRowWidgets.Add(Player.PlayerId, RowWidget);
        }
    }
}

UUserWidget* UDeskillzHostScorePanelWidget::CreatePlayerRow(const FDeskillzHostPlayerInfo& Player)
{
    // Create default row if no custom class specified
    // In a full implementation, this would create a proper widget

    UTextBlock* RowText = NewObject<UTextBlock>(this);
    if (RowText)
    {
        FString RowString = FString::Printf(TEXT("%s%s: %d"),
            Player.bIsCurrentTurn ? TEXT("[>] ") : TEXT("    "),
            *Player.Username,
            Player.Score);

        if (bShowChipStack)
        {
            RowString += FString::Printf(TEXT(" ($%.2f)"), Player.ChipStack);
        }

        if (bShowActiveStatus && !Player.bIsActive)
        {
            RowString += TEXT(" [FOLDED]");
        }

        RowText->SetText(FText::FromString(RowString));
        RowText->SetColorAndOpacity(Player.bIsActive ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor::Gray));

        // NOTE: Player hands/cards are NOT displayed - anti-cheat protection
    }

    return nullptr; // Would return actual widget in full implementation
}

void UDeskillzHostScorePanelWidget::UpdatePlayerRow(UUserWidget* RowWidget, const FDeskillzHostPlayerInfo& Player)
{
    if (!RowWidget)
    {
        return;
    }

    // Update row widget with new player data
    // Implementation would update all visual elements
    // NOTE: Player hands/cards are NEVER displayed - anti-cheat protection
}

int32 UDeskillzHostScorePanelWidget::FindPlayerIndex(const FString& PlayerId) const
{
    for (int32 i = 0; i < Players.Num(); ++i)
    {
        if (Players[i].PlayerId == PlayerId)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void UDeskillzHostScorePanelWidget::SortPlayers()
{
    Players.Sort([](const FDeskillzHostPlayerInfo& A, const FDeskillzHostPlayerInfo& B) {
        return A.Score > B.Score; // Descending by score
    });
}

void UDeskillzHostScorePanelWidget::OnRowClicked(const FString& PlayerId)
{
    SelectedPlayerId = PlayerId;
    OnPlayerClicked.Broadcast(PlayerId);
}

void UDeskillzHostScorePanelWidget::AnimateScoreChange(UUserWidget* RowWidget, int32 Delta)
{
    if (!RowWidget || Delta == 0)
    {
        return;
    }

    // Play score change animation
    // Green flash for positive, red flash for negative
    UE_LOG(LogTemp, Log, TEXT("[HostScorePanel] Score changed by %d"), Delta);
}