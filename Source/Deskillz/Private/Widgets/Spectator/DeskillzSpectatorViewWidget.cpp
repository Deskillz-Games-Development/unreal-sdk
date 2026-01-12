// =============================================================================
// Deskillz SDK for Unreal Engine - Spectator View Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Spectator/DeskillzSpectatorViewWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/Slider.h"
#include "Components/WidgetSwitcher.h"

void UDeskillzSpectatorViewWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind view control buttons
    if (FollowPlayerButton)
    {
        FollowPlayerButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnFollowPlayerClicked);
    }
    if (FreeViewButton)
    {
        FreeViewButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnFreeViewClicked);
    }
    if (TableViewButton)
    {
        TableViewButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnTableViewClicked);
    }

    // Bind playback controls
    if (PlayPauseButton)
    {
        PlayPauseButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnPlayPauseClicked);
    }
    if (SpeedSlider)
    {
        SpeedSlider->OnValueChanged.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnSpeedSliderChanged);
    }
    if (Speed1xButton)
    {
        Speed1xButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnSpeed1xClicked);
    }
    if (Speed2xButton)
    {
        Speed2xButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnSpeed2xClicked);
    }

    // Bind action buttons
    if (LeaveButton)
    {
        LeaveButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnLeaveClicked);
    }
    if (JoinGameButton)
    {
        JoinGameButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnJoinGameClicked);
    }
    if (ToggleStatsButton)
    {
        ToggleStatsButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnToggleStatsClicked);
    }
    if (ToggleChatButton)
    {
        ToggleChatButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnToggleChatClicked);
    }
    if (SwitchRoomButton)
    {
        SwitchRoomButton->OnClicked.AddDynamic(this, &UDeskillzSpectatorViewWidget::OnSwitchRoomClicked);
    }

    // Initialize
    SetViewMode(EDeskillzSpectatorViewMode::TableView);
    SetPlaybackSpeed(1.0f);
}

void UDeskillzSpectatorViewWidget::SetRoomData(const FDeskillzSpectatorRoomData& InRoomData)
{
    RoomData = InRoomData;
    UpdateRoomInfoDisplay();
    UpdatePlayerList();
}

void UDeskillzSpectatorViewWidget::SetViewMode(EDeskillzSpectatorViewMode Mode)
{
    CurrentViewMode = Mode;
    UpdateViewModeDisplay();
    OnViewModeChanged.Broadcast(Mode);
}

void UDeskillzSpectatorViewWidget::SetFollowedPlayer(const FString& PlayerId)
{
    FollowedPlayerId = PlayerId;
    
    if (!PlayerId.IsEmpty())
    {
        SetViewMode(EDeskillzSpectatorViewMode::FollowPlayer);
    }

    UpdateFollowedPlayerDisplay();
    OnPlayerFollowed.Broadcast(PlayerId);
}

void UDeskillzSpectatorViewWidget::SetPlaybackSpeed(float Speed)
{
    PlaybackSpeed = FMath::Clamp(Speed, MinPlaybackSpeed, MaxPlaybackSpeed);

    if (SpeedText)
    {
        SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%.1fx"), PlaybackSpeed)));
    }
    if (SpeedSlider)
    {
        float SliderValue = (PlaybackSpeed - MinPlaybackSpeed) / (MaxPlaybackSpeed - MinPlaybackSpeed);
        SpeedSlider->SetValue(SliderValue);
    }

    OnPlaybackSpeedChanged.Broadcast(PlaybackSpeed);
}

void UDeskillzSpectatorViewWidget::TogglePlayback()
{
    bIsPlaying = !bIsPlaying;
    UpdatePlaybackControls();
    OnPlaybackToggled.Broadcast(bIsPlaying);
}

void UDeskillzSpectatorViewWidget::SetStatsVisible(bool bVisible)
{
    bStatsVisible = bVisible;

    if (StatsPanel)
    {
        StatsPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UDeskillzSpectatorViewWidget::SetChatVisible(bool bVisible)
{
    bChatVisible = bVisible;

    if (ChatPanel)
    {
        ChatPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UDeskillzSpectatorViewWidget::UpdateGameState(const FDeskillzSpectatorGameState& GameState)
{
    CurrentGameState = GameState;
    UpdateGameStateDisplay();
}

void UDeskillzSpectatorViewWidget::OnFollowPlayerClicked()
{
    // Show player selection or follow next player
    if (RoomData.Players.Num() > 0)
    {
        // Find next player to follow
        int32 CurrentIndex = -1;
        for (int32 i = 0; i < RoomData.Players.Num(); ++i)
        {
            if (RoomData.Players[i].PlayerId == FollowedPlayerId)
            {
                CurrentIndex = i;
                break;
            }
        }

        int32 NextIndex = (CurrentIndex + 1) % RoomData.Players.Num();
        SetFollowedPlayer(RoomData.Players[NextIndex].PlayerId);
    }
}

void UDeskillzSpectatorViewWidget::OnFreeViewClicked()
{
    SetViewMode(EDeskillzSpectatorViewMode::FreeView);
    FollowedPlayerId = TEXT("");
}

void UDeskillzSpectatorViewWidget::OnTableViewClicked()
{
    SetViewMode(EDeskillzSpectatorViewMode::TableView);
}

void UDeskillzSpectatorViewWidget::OnPlayPauseClicked()
{
    TogglePlayback();
}

void UDeskillzSpectatorViewWidget::OnSpeedSliderChanged(float Value)
{
    float NewSpeed = FMath::Lerp(MinPlaybackSpeed, MaxPlaybackSpeed, Value);
    SetPlaybackSpeed(NewSpeed);
}

void UDeskillzSpectatorViewWidget::OnSpeed1xClicked()
{
    SetPlaybackSpeed(1.0f);
}

void UDeskillzSpectatorViewWidget::OnSpeed2xClicked()
{
    SetPlaybackSpeed(2.0f);
}

void UDeskillzSpectatorViewWidget::OnLeaveClicked()
{
    OnLeaveRequested.Broadcast();
}

void UDeskillzSpectatorViewWidget::OnJoinGameClicked()
{
    OnJoinGameRequested.Broadcast();
}

void UDeskillzSpectatorViewWidget::OnToggleStatsClicked()
{
    SetStatsVisible(!bStatsVisible);
}

void UDeskillzSpectatorViewWidget::OnToggleChatClicked()
{
    SetChatVisible(!bChatVisible);
}

void UDeskillzSpectatorViewWidget::OnSwitchRoomClicked()
{
    OnSwitchRoomRequested.Broadcast();
}

void UDeskillzSpectatorViewWidget::UpdateRoomInfoDisplay()
{
    if (RoomNameText)
    {
        RoomNameText->SetText(FText::FromString(RoomData.RoomName));
    }
    if (GameNameText)
    {
        GameNameText->SetText(FText::FromString(RoomData.GameName));
    }
    if (PlayerCountText)
    {
        PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d players"), 
            RoomData.CurrentPlayers, RoomData.MaxPlayers)));
    }
    if (SpectatorCountText)
    {
        SpectatorCountText->SetText(FText::FromString(FString::Printf(TEXT("%d watching"), RoomData.SpectatorCount)));
    }
    if (PotSizeText)
    {
        PotSizeText->SetText(FText::FromString(FString::Printf(TEXT("Pot: $%.2f"), RoomData.CurrentPotSize)));
    }

    // Update join button visibility based on available seats
    if (JoinGameButton)
    {
        bool bCanJoin = RoomData.CurrentPlayers < RoomData.MaxPlayers && RoomData.bJoinable;
        JoinGameButton->SetVisibility(bCanJoin ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UDeskillzSpectatorViewWidget::UpdatePlayerList()
{
    if (!PlayerListContainer) return;

    PlayerListContainer->ClearChildren();

    for (const FDeskillzSpectatorPlayerInfo& Player : RoomData.Players)
    {
        UWidget* PlayerWidget = CreatePlayerListItem(Player);
        if (PlayerWidget)
        {
            PlayerListContainer->AddChild(PlayerWidget);
        }
    }
}

void UDeskillzSpectatorViewWidget::UpdateViewModeDisplay()
{
    const FLinearColor ActiveColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);
    const FLinearColor InactiveColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);

    if (FollowPlayerButton)
    {
        FollowPlayerButton->SetColorAndOpacity(CurrentViewMode == EDeskillzSpectatorViewMode::FollowPlayer ? ActiveColor : InactiveColor);
    }
    if (FreeViewButton)
    {
        FreeViewButton->SetColorAndOpacity(CurrentViewMode == EDeskillzSpectatorViewMode::FreeView ? ActiveColor : InactiveColor);
    }
    if (TableViewButton)
    {
        TableViewButton->SetColorAndOpacity(CurrentViewMode == EDeskillzSpectatorViewMode::TableView ? ActiveColor : InactiveColor);
    }

    if (ViewModeText)
    {
        FString ModeStr;
        switch (CurrentViewMode)
        {
            case EDeskillzSpectatorViewMode::FollowPlayer: ModeStr = TEXT("Following Player"); break;
            case EDeskillzSpectatorViewMode::FreeView: ModeStr = TEXT("Free View"); break;
            case EDeskillzSpectatorViewMode::TableView: ModeStr = TEXT("Table View"); break;
        }
        ViewModeText->SetText(FText::FromString(ModeStr));
    }
}

void UDeskillzSpectatorViewWidget::UpdateFollowedPlayerDisplay()
{
    if (FollowedPlayerText)
    {
        if (FollowedPlayerId.IsEmpty())
        {
            FollowedPlayerText->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            // Find player name
            FString PlayerName = FollowedPlayerId;
            for (const FDeskillzSpectatorPlayerInfo& Player : RoomData.Players)
            {
                if (Player.PlayerId == FollowedPlayerId)
                {
                    PlayerName = Player.DisplayName;
                    break;
                }
            }
            FollowedPlayerText->SetText(FText::FromString(FString::Printf(TEXT("Following: %s"), *PlayerName)));
            FollowedPlayerText->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void UDeskillzSpectatorViewWidget::UpdatePlaybackControls()
{
    if (PlayPauseIcon)
    {
        // Would update icon based on bIsPlaying state
    }
    if (PlayPauseText)
    {
        PlayPauseText->SetText(FText::FromString(bIsPlaying ? TEXT("Pause") : TEXT("Play")));
    }
}

void UDeskillzSpectatorViewWidget::UpdateGameStateDisplay()
{
    if (RoundNumberText)
    {
        RoundNumberText->SetText(FText::FromString(FString::Printf(TEXT("Round %d"), CurrentGameState.CurrentRound)));
    }
    if (CurrentTurnText)
    {
        CurrentTurnText->SetText(FText::FromString(FString::Printf(TEXT("Turn: %s"), *CurrentGameState.CurrentPlayerName)));
    }
    if (GamePhaseText)
    {
        GamePhaseText->SetText(FText::FromString(CurrentGameState.GamePhase));
    }
}

UWidget* UDeskillzSpectatorViewWidget::CreatePlayerListItem(const FDeskillzSpectatorPlayerInfo& Player)
{
    UBorder* Container = NewObject<UBorder>(this);
    if (!Container) return nullptr;

    // Style based on player state
    if (Player.bIsActive)
    {
        Container->SetBrushColor(FLinearColor(0.1f, 0.2f, 0.15f, 1.0f));
    }
    else
    {
        Container->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));
    }

    return Container;
}