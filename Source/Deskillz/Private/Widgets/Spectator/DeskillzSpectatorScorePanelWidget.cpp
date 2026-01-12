// =============================================================================
// Deskillz SDK for Unreal Engine - Spectator Score Panel Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Spectator/DeskillzSpectatorScorePanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"

void UDeskillzSpectatorScorePanelWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UDeskillzSpectatorScorePanelWidget::SetScoreData(const TArray<FDeskillzPlayerScoreData>& InScoreData)
{
    ScoreData = InScoreData;

    // Sort by score if enabled
    if (bSortByScore)
    {
        ScoreData.Sort([](const FDeskillzPlayerScoreData& A, const FDeskillzPlayerScoreData& B)
        {
            return A.Score > B.Score;
        });
    }

    // Find highest score for percentage calculations
    HighestScore = 0.0f;
    for (const FDeskillzPlayerScoreData& Data : ScoreData)
    {
        HighestScore = FMath::Max(HighestScore, Data.Score);
    }

    RebuildScoreList();
    UpdateLeaderDisplay();
}

void UDeskillzSpectatorScorePanelWidget::UpdatePlayerScore(const FString& PlayerId, float NewScore, int32 ScoreDelta)
{
    for (FDeskillzPlayerScoreData& Data : ScoreData)
    {
        if (Data.PlayerId == PlayerId)
        {
            Data.Score = NewScore;
            Data.LastScoreDelta = ScoreDelta;
            break;
        }
    }

    // Resort if needed
    if (bSortByScore)
    {
        ScoreData.Sort([](const FDeskillzPlayerScoreData& A, const FDeskillzPlayerScoreData& B)
        {
            return A.Score > B.Score;
        });
    }

    // Update highest score
    HighestScore = 0.0f;
    for (const FDeskillzPlayerScoreData& Data : ScoreData)
    {
        HighestScore = FMath::Max(HighestScore, Data.Score);
    }

    RebuildScoreList();
    UpdateLeaderDisplay();

    // Broadcast score change
    OnScoreUpdated.Broadcast(PlayerId, NewScore, ScoreDelta);
}

void UDeskillzSpectatorScorePanelWidget::SetHighlightedPlayer(const FString& PlayerId)
{
    HighlightedPlayerId = PlayerId;
    RebuildScoreList();
}

void UDeskillzSpectatorScorePanelWidget::SetCompactMode(bool bCompact)
{
    bCompactMode = bCompact;
    RebuildScoreList();
}

void UDeskillzSpectatorScorePanelWidget::SetSortByScore(bool bSort)
{
    bSortByScore = bSort;

    if (bSort)
    {
        ScoreData.Sort([](const FDeskillzPlayerScoreData& A, const FDeskillzPlayerScoreData& B)
        {
            return A.Score > B.Score;
        });
    }

    RebuildScoreList();
}

void UDeskillzSpectatorScorePanelWidget::RebuildScoreList()
{
    if (!ScoreListContainer) return;

    ScoreListContainer->ClearChildren();

    for (int32 i = 0; i < ScoreData.Num(); ++i)
    {
        const FDeskillzPlayerScoreData& Data = ScoreData[i];
        UWidget* ScoreRow = CreateScoreRow(Data, i + 1);
        if (ScoreRow)
        {
            ScoreListContainer->AddChild(ScoreRow);
        }
    }
}

void UDeskillzSpectatorScorePanelWidget::UpdateLeaderDisplay()
{
    if (ScoreData.Num() == 0) return;

    // Find leader (first after sorting, or highest score)
    const FDeskillzPlayerScoreData& Leader = ScoreData[0];

    if (LeaderNameText)
    {
        LeaderNameText->SetText(FText::FromString(Leader.PlayerName));
    }
    if (LeaderScoreText)
    {
        LeaderScoreText->SetText(FText::FromString(FormatScore(Leader.Score)));
    }
    if (LeaderAvatar && Leader.AvatarTexture)
    {
        LeaderAvatar->SetBrushFromTexture(Leader.AvatarTexture);
    }

    // Update lead amount if there's a second player
    if (ScoreData.Num() > 1 && LeadAmountText)
    {
        float Lead = Leader.Score - ScoreData[1].Score;
        LeadAmountText->SetText(FText::FromString(FString::Printf(TEXT("+%s lead"), *FormatScore(Lead))));
    }

    // Check for lead change
    if (Leader.PlayerId != PreviousLeaderId && !PreviousLeaderId.IsEmpty())
    {
        OnLeaderChanged.Broadcast(Leader.PlayerId, Leader.PlayerName);
    }
    PreviousLeaderId = Leader.PlayerId;
}

UWidget* UDeskillzSpectatorScorePanelWidget::CreateScoreRow(const FDeskillzPlayerScoreData& Data, int32 Rank)
{
    UBorder* RowContainer = NewObject<UBorder>(this);
    if (!RowContainer) return nullptr;

    // Highlight if this is the selected player
    bool bIsHighlighted = (Data.PlayerId == HighlightedPlayerId);
    
    if (bIsHighlighted)
    {
        RowContainer->SetBrushColor(HighlightColor);
    }
    else if (Data.bIsActive)
    {
        RowContainer->SetBrushColor(FLinearColor(0.1f, 0.12f, 0.15f, 1.0f));
    }
    else
    {
        RowContainer->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.08f, 0.6f));
    }

    return RowContainer;
}

void UDeskillzSpectatorScorePanelWidget::PlayScoreChangeAnimation(const FString& PlayerId, int32 Delta)
{
    if (!bEnableAnimations) return;

    // Find the row for this player and play animation
    // Animation would show +/- score floating up
}

FLinearColor UDeskillzSpectatorScorePanelWidget::GetRankColor(int32 Rank) const
{
    switch (Rank)
    {
        case 1: return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);      // Gold
        case 2: return FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);    // Silver
        case 3: return FLinearColor(0.8f, 0.5f, 0.2f, 1.0f);       // Bronze
        default: return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);      // Gray
    }
}

FString UDeskillzSpectatorScorePanelWidget::FormatScore(float Score) const
{
    if (Score >= 1000000.0f)
    {
        return FString::Printf(TEXT("%.1fM"), Score / 1000000.0f);
    }
    else if (Score >= 1000.0f)
    {
        return FString::Printf(TEXT("%.1fK"), Score / 1000.0f);
    }
    else if (FMath::Frac(Score) > 0.001f)
    {
        return FString::Printf(TEXT("%.1f"), Score);
    }
    else
    {
        return FString::Printf(TEXT("%d"), FMath::RoundToInt(Score));
    }
}

FString UDeskillzSpectatorScorePanelWidget::FormatScoreDelta(int32 Delta) const
{
    if (Delta > 0)
    {
        return FString::Printf(TEXT("+%d"), Delta);
    }
    else if (Delta < 0)
    {
        return FString::Printf(TEXT("%d"), Delta);
    }
    return TEXT("");
}