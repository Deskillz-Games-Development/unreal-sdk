// =============================================================================
// Deskillz SDK for Unreal Engine - Host Spectator View Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================
// HOST-ONLY FEATURE: Main spectator view widget for hosts monitoring rooms.
// Only the room creator can use this widget.
// Shows board state and scores but NOT player hands (anti-cheat protection).
// =============================================================================

#include "UI/Widgets/Spectator/DeskillzHostSpectatorViewWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"

UDeskillzHostSpectatorViewWidget::UDeskillzHostSpectatorViewWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UDeskillzHostSpectatorViewWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (ExitButton)
    {
        ExitButton->OnClicked.AddDynamic(this, &UDeskillzHostSpectatorViewWidget::OnExitButtonClicked);
    }
    if (ToggleChatButton)
    {
        ToggleChatButton->OnClicked.AddDynamic(this, &UDeskillzHostSpectatorViewWidget::OnToggleChatClicked);
    }
    if (ToggleScoresButton)
    {
        ToggleScoresButton->OnClicked.AddDynamic(this, &UDeskillzHostSpectatorViewWidget::OnToggleScoresClicked);
    }

    // Set up anti-cheat notice
    if (AntiCheatText)
    {
        AntiCheatText->SetText(AntiCheatNoticeText);
    }

    // Set up host badge
    if (HostBadgeText)
    {
        HostBadgeText->SetText(FText::FromString(TEXT("HOST MONITORING")));
    }

    // Initialize visibility
    bChatVisible = bShowChatByDefault;
    bScorePanelVisible = bShowScorePanelByDefault;

    // Hide pause overlay initially
    if (PauseOverlay)
    {
        PauseOverlay->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UDeskillzHostSpectatorViewWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update pause timer if paused
    if (bIsPaused && PauseRemainingTime > 0.0f)
    {
        PauseRemainingTime -= InDeltaTime;
        if (PauseText)
        {
            PauseText->SetText(FText::FromString(
                FString::Printf(TEXT("PAUSED (%.0fs)"), FMath::Max(0.0f, PauseRemainingTime))
            ));
        }

        if (PauseRemainingTime <= 0.0f)
        {
            HidePauseState();
        }
    }
}

// ============================================================================
// Data Binding
// ============================================================================

void UDeskillzHostSpectatorViewWidget::SetSession(const FDeskillzHostSpectatorSession& Session)
{
    CurrentSession = Session;
    UpdateRoomInfoDisplay();
    UpdateGameInfoDisplay();
    UpdatePlayers(Session.Players);
}

void UDeskillzHostSpectatorViewWidget::UpdateGameState(const FDeskillzHostGameState& State)
{
    // Update current session state
    CurrentSession.CurrentState = State;
    UpdateGameInfoDisplay();

    // Update board state display
    // NOTE: Player hands are NOT included - this is intentional for anti-cheat
    if (BoardStateText && !State.BoardState.IsEmpty())
    {
        BoardStateText->SetText(FText::FromString(State.BoardState));
    }

    // Handle pause state
    if (State.bIsPaused && !bIsPaused)
    {
        ShowPauseState(0.0f); // Duration unknown, will be updated
    }
    else if (!State.bIsPaused && bIsPaused)
    {
        HidePauseState();
    }
}

void UDeskillzHostSpectatorViewWidget::UpdatePlayers(const TArray<FDeskillzHostPlayerInfo>& Players)
{
    CurrentSession.Players = Players;
    
    // Update score panel if it exists
    // The score panel component would be updated here
    // NOTE: Player hands/cards are NOT visible - anti-cheat protection
}

void UDeskillzHostSpectatorViewWidget::AddChatMessage(const FDeskillzHostChatMessage& Message)
{
    if (!ChatMessagesContainer)
    {
        return;
    }

    UWidget* MessageWidget = CreateChatMessageWidget(Message);
    if (MessageWidget)
    {
        ChatMessagesContainer->AddChild(MessageWidget);

        // Scroll to bottom
        if (ChatScrollBox)
        {
            ChatScrollBox->ScrollToEnd();
        }
    }
}

void UDeskillzHostSpectatorViewWidget::ShowRoundResult(const FDeskillzHostRoundResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorView] Round %d ended. Winner: %s, Pot: $%.2f"),
        Result.RoundNumber, *Result.WinnerUsername, Result.PotWon);

    // Show round result overlay/animation
    // Implementation would show a temporary overlay with the result
}

void UDeskillzHostSpectatorViewWidget::ShowGameEndResult(const FDeskillzHostGameEndResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("[HostSpectatorView] Game ended. Winner: %s, Your Rake: $%.2f"),
        *Result.WinnerUsername, Result.RakeEarnings);

    // Show game end overlay with your earnings
    // Implementation would show final results including host's rake earnings
}

void UDeskillzHostSpectatorViewWidget::SetAvailableRooms(const TArray<FDeskillzHostRoom>& Rooms, const FString& CurrentRoomId)
{
    // Update room switcher component
    // This would update the HostRoomSwitcherWidget if attached
}

// ============================================================================
// UI State
// ============================================================================

void UDeskillzHostSpectatorViewWidget::SetChatVisible(bool bVisible)
{
    bChatVisible = bVisible;

    if (ChatScrollBox)
    {
        ChatScrollBox->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UDeskillzHostSpectatorViewWidget::SetScorePanelVisible(bool bVisible)
{
    bScorePanelVisible = bVisible;

    // Update score panel visibility
    // The score panel component would be toggled here
}

void UDeskillzHostSpectatorViewWidget::ShowPauseState(float Duration)
{
    bIsPaused = true;
    PauseRemainingTime = Duration;

    if (PauseOverlay)
    {
        PauseOverlay->SetVisibility(ESlateVisibility::Visible);
    }

    if (PauseText)
    {
        if (Duration > 0.0f)
        {
            PauseText->SetText(FText::FromString(
                FString::Printf(TEXT("PAUSED (%.0fs)"), Duration)
            ));
        }
        else
        {
            PauseText->SetText(FText::FromString(TEXT("PAUSED")));
        }
    }
}

void UDeskillzHostSpectatorViewWidget::HidePauseState()
{
    bIsPaused = false;
    PauseRemainingTime = 0.0f;

    if (PauseOverlay)
    {
        PauseOverlay->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzHostSpectatorViewWidget::UpdateRoomInfoDisplay()
{
    if (RoomNameText)
    {
        RoomNameText->SetText(FText::FromString(CurrentSession.RoomName));
    }

    if (RoomCodeText)
    {
        RoomCodeText->SetText(FText::FromString(CurrentSession.RoomCode));
    }

    if (GameNameText)
    {
        GameNameText->SetText(FText::FromString(CurrentSession.GameName));
    }
}

void UDeskillzHostSpectatorViewWidget::UpdateGameInfoDisplay()
{
    const FDeskillzHostGameState& State = CurrentSession.CurrentState;

    if (RoundText)
    {
        RoundText->SetText(FText::FromString(
            FString::Printf(TEXT("Round %d/%d"), State.CurrentRound, State.TotalRounds)
        ));
    }

    if (PotText)
    {
        PotText->SetText(FText::FromString(
            FString::Printf(TEXT("Pot: $%.2f"), State.CurrentPot)
        ));
    }

    if (PhaseText)
    {
        PhaseText->SetText(FText::FromString(
            FString::Printf(TEXT("Phase: %s"), *State.Phase)
        ));
    }

    if (TurnTimerText)
    {
        if (State.TurnTimeRemaining > 0.0f)
        {
            TurnTimerText->SetText(FText::FromString(
                FString::Printf(TEXT("Turn: %.0fs"), State.TurnTimeRemaining)
            ));
        }
        else
        {
            TurnTimerText->SetText(FText::FromString(TEXT("Turn: --")));
        }
    }
}

UWidget* UDeskillzHostSpectatorViewWidget::CreateChatMessageWidget(const FDeskillzHostChatMessage& Message)
{
    // Create a simple text block for the chat message
    UTextBlock* MessageText = NewObject<UTextBlock>(this);
    if (MessageText)
    {
        MessageText->SetText(FText::FromString(
            FString::Printf(TEXT("[%s]: %s"), *Message.SenderUsername, *Message.Content)
        ));
        return MessageText;
    }
    return nullptr;
}

// ============================================================================
// Event Handlers
// ============================================================================

void UDeskillzHostSpectatorViewWidget::OnExitButtonClicked()
{
    OnExitRequested.Broadcast();
}

void UDeskillzHostSpectatorViewWidget::OnToggleChatClicked()
{
    SetChatVisible(!bChatVisible);
}

void UDeskillzHostSpectatorViewWidget::OnToggleScoresClicked()
{
    SetScorePanelVisible(!bScorePanelVisible);
}