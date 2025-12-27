// Copyright Deskillz Games. All Rights Reserved.

#include "Widgets/Rooms/DeskillzRoomLobbyWidget.h"
#include "Widgets/Rooms/DeskillzRoomPlayerCard.h"
#include "Rooms/DeskillzRooms.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "Components/EditableTextBox.h"
#include "Components/CanvasPanel.h"
#include "HAL/PlatformApplicationMisc.h"

UDeskillzRoomLobbyWidget::UDeskillzRoomLobbyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bEnableChat(true)
	, bIsHost(false)
	, bIsReady(false)
	, CountdownSeconds(0)
	, bIsVisible(false)
{
}

void UDeskillzRoomLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeUI();
	SubscribeToRoomEvents();
}

void UDeskillzRoomLobbyWidget::NativeDestruct()
{
	UnsubscribeFromRoomEvents();
	ClearPlayerCards();
	Super::NativeDestruct();
}

void UDeskillzRoomLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Update countdown if active
	if (CountdownSeconds > 0 && CountdownOverlay && CountdownOverlay->GetVisibility() == ESlateVisibility::Visible)
	{
		// Countdown is updated via HandleCountdownTick events
	}
}

void UDeskillzRoomLobbyWidget::InitializeUI()
{
	// Header buttons
	if (CopyCodeButton)
	{
		CopyCodeButton->OnClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnCopyCodeButtonClicked);
	}
	
	if (ShareButton)
	{
		ShareButton->OnClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnShareButtonClicked);
	}
	
	// Action buttons
	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnLeaveButtonClicked);
	}
	
	if (CancelRoomButton)
	{
		CancelRoomButton->OnClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnCancelRoomButtonClicked);
	}
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnReadyButtonClicked);
	}
	
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnStartButtonClicked);
	}
	
	// Chat
	if (SendChatButton)
	{
		SendChatButton->OnClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnSendChatButtonClicked);
	}
	
	if (ChatInput)
	{
		ChatInput->OnTextCommitted.AddDynamic(this, &UDeskillzRoomLobbyWidget::OnChatInputCommitted);
	}
	
	// Chat visibility
	if (ChatContainer)
	{
		ChatContainer->SetVisibility(bEnableChat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	// Hide countdown initially
	HideCountdown();
}

void UDeskillzRoomLobbyWidget::SubscribeToRoomEvents()
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	Rooms->OnPlayerJoined.AddDynamic(this, &UDeskillzRoomLobbyWidget::HandlePlayerJoined);
	Rooms->OnPlayerLeft.AddDynamic(this, &UDeskillzRoomLobbyWidget::HandlePlayerLeft);
	Rooms->OnPlayerReadyChanged.AddDynamic(this, &UDeskillzRoomLobbyWidget::HandlePlayerReadyChanged);
	Rooms->OnCountdownStarted.AddDynamic(this, &UDeskillzRoomLobbyWidget::HandleCountdownStarted);
	Rooms->OnCountdownTick.AddDynamic(this, &UDeskillzRoomLobbyWidget::HandleCountdownTick);
	Rooms->OnChatReceived.AddDynamic(this, &UDeskillzRoomLobbyWidget::HandleChatReceived);
}

void UDeskillzRoomLobbyWidget::UnsubscribeFromRoomEvents()
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	Rooms->OnPlayerJoined.RemoveDynamic(this, &UDeskillzRoomLobbyWidget::HandlePlayerJoined);
	Rooms->OnPlayerLeft.RemoveDynamic(this, &UDeskillzRoomLobbyWidget::HandlePlayerLeft);
	Rooms->OnPlayerReadyChanged.RemoveDynamic(this, &UDeskillzRoomLobbyWidget::HandlePlayerReadyChanged);
	Rooms->OnCountdownStarted.RemoveDynamic(this, &UDeskillzRoomLobbyWidget::HandleCountdownStarted);
	Rooms->OnCountdownTick.RemoveDynamic(this, &UDeskillzRoomLobbyWidget::HandleCountdownTick);
	Rooms->OnChatReceived.RemoveDynamic(this, &UDeskillzRoomLobbyWidget::HandleChatReceived);
}

// =============================================================================
// Public Methods
// =============================================================================

void UDeskillzRoomLobbyWidget::SetRoom(const FPrivateRoom& Room)
{
	CurrentRoom = Room;
	bIsHost = Room.IsCurrentUserHost();
	
	// Find current user's ready state
	const FRoomPlayer* CurrentPlayer = Room.FindPlayer(Room.Host.Id); // This should be GetCurrentPlayer
	if (CurrentPlayer)
	{
		bIsReady = CurrentPlayer->bIsReady;
	}
	
	UpdateDisplay();
	UpdatePlayerList();
	UpdateButtons();
}

void UDeskillzRoomLobbyWidget::UpdateRoom(const FPrivateRoom& Room)
{
	CurrentRoom = Room;
	bIsHost = Room.IsCurrentUserHost();
	
	UpdateDisplay();
	UpdatePlayerList();
	UpdateButtons();
}

void UDeskillzRoomLobbyWidget::Show()
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);
}

void UDeskillzRoomLobbyWidget::Hide()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

// =============================================================================
// Display Updates
// =============================================================================

void UDeskillzRoomLobbyWidget::UpdateDisplay()
{
	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(CurrentRoom.Name));
	}
	
	if (RoomCodeText)
	{
		RoomCodeText->SetText(FText::FromString(CurrentRoom.RoomCode));
	}
	
	if (EntryFeeText)
	{
		EntryFeeText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f %s"), 
			CurrentRoom.EntryFee, *CurrentRoom.EntryCurrency)));
	}
	
	if (PrizePoolText)
	{
		PrizePoolText->SetText(FText::FromString(FString::Printf(TEXT("$%.2f"), CurrentRoom.PrizePool)));
	}
	
	if (ModeText)
	{
		FString ModeStr = CurrentRoom.Mode == ERoomMode::Sync ? TEXT("Real-time") : TEXT("Turn-based");
		ModeText->SetText(FText::FromString(ModeStr));
	}
	
	if (StatusText)
	{
		FString StatusStr;
		switch (CurrentRoom.Status)
		{
		case ERoomStatus::Waiting: StatusStr = TEXT("Waiting"); break;
		case ERoomStatus::ReadyCheck: StatusStr = TEXT("Ready Check"); break;
		case ERoomStatus::Countdown: StatusStr = TEXT("Starting"); break;
		case ERoomStatus::Launching: StatusStr = TEXT("Launching"); break;
		case ERoomStatus::InProgress: StatusStr = TEXT("In Progress"); break;
		default: StatusStr = TEXT("Unknown"); break;
		}
		StatusText->SetText(FText::FromString(StatusStr));
	}
	
	if (PlayerCountText)
	{
		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("Players (%d/%d)"), 
			CurrentRoom.CurrentPlayers, CurrentRoom.MaxPlayers)));
	}
	
	// Update ready progress
	int32 ReadyCount = CurrentRoom.GetReadyPlayerCount();
	int32 TotalPlayers = CurrentRoom.CurrentPlayers;
	
	if (ReadyStatusText)
	{
		ReadyStatusText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d Ready"), 
			ReadyCount, TotalPlayers)));
	}
	
	if (ReadyProgressBar)
	{
		float Progress = TotalPlayers > 0 ? (float)ReadyCount / TotalPlayers : 0.0f;
		ReadyProgressBar->SetPercent(Progress);
	}
}

void UDeskillzRoomLobbyWidget::UpdatePlayerList()
{
	ClearPlayerCards();
	
	if (!PlayerListContent)
	{
		return;
	}
	
	for (const FRoomPlayer& Player : CurrentRoom.Players)
	{
		UDeskillzRoomPlayerCard* Card = CreatePlayerCard(Player);
		if (Card)
		{
			PlayerCards.Add(Card);
			PlayerListContent->AddChild(Card);
		}
	}
}

UDeskillzRoomPlayerCard* UDeskillzRoomLobbyWidget::CreatePlayerCard(const FRoomPlayer& Player)
{
	if (!PlayerCardClass)
	{
		PlayerCardClass = UDeskillzRoomPlayerCard::StaticClass();
	}
	
	UDeskillzRoomPlayerCard* Card = CreateWidget<UDeskillzRoomPlayerCard>(this, PlayerCardClass);
	if (Card)
	{
		bool bCanKick = bIsHost && !Player.bIsCurrentUser;
		Card->SetPlayer(Player, bCanKick);
		Card->OnKickClicked.AddDynamic(this, &UDeskillzRoomLobbyWidget::HandleKickPlayer);
	}
	
	return Card;
}

void UDeskillzRoomLobbyWidget::ClearPlayerCards()
{
	for (UDeskillzRoomPlayerCard* Card : PlayerCards)
	{
		if (Card)
		{
			Card->RemoveFromParent();
		}
	}
	PlayerCards.Empty();
}

void UDeskillzRoomLobbyWidget::UpdateButtons()
{
	// Host-only buttons
	if (CancelRoomButton)
	{
		CancelRoomButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (StartButton)
	{
		StartButton->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		StartButton->SetIsEnabled(CurrentRoom.AreAllPlayersReady() && CurrentRoom.CurrentPlayers >= CurrentRoom.MinPlayers);
	}
	
	// Ready button
	if (ReadyButton && ReadyButtonText)
	{
		ReadyButtonText->SetText(FText::FromString(bIsReady ? TEXT("Not Ready") : TEXT("Ready")));
	}
}

// =============================================================================
// Countdown
// =============================================================================

void UDeskillzRoomLobbyWidget::ShowCountdown(int32 Seconds)
{
	CountdownSeconds = Seconds;
	
	if (CountdownOverlay)
	{
		CountdownOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (CountdownText)
	{
		CountdownText->SetText(FText::FromString(FString::FromInt(Seconds)));
	}
}

void UDeskillzRoomLobbyWidget::HideCountdown()
{
	CountdownSeconds = 0;
	
	if (CountdownOverlay)
	{
		CountdownOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// =============================================================================
// Chat
// =============================================================================

void UDeskillzRoomLobbyWidget::AddChatMessage(const FString& Username, const FString& Message)
{
	if (!ChatContent)
	{
		return;
	}
	
	// Create simple text block for chat message
	UTextBlock* MsgText = NewObject<UTextBlock>(this);
	if (MsgText)
	{
		MsgText->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Username, *Message)));
		MsgText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		ChatContent->AddChild(MsgText);
		
		// Scroll to bottom
		if (ChatScrollBox)
		{
			ChatScrollBox->ScrollToEnd();
		}
	}
}

// =============================================================================
// Actions
// =============================================================================

void UDeskillzRoomLobbyWidget::CopyRoomCode()
{
	FPlatformApplicationMisc::ClipboardCopy(*CurrentRoom.RoomCode);
	UE_LOG(LogTemp, Log, TEXT("[RoomLobbyWidget] Copied room code: %s"), *CurrentRoom.RoomCode);
}

void UDeskillzRoomLobbyWidget::ShareRoom()
{
	FString ShareUrl = FString::Printf(TEXT("https://deskillz.games/room/%s"), *CurrentRoom.RoomCode);
	FPlatformApplicationMisc::ClipboardCopy(*ShareUrl);
	UE_LOG(LogTemp, Log, TEXT("[RoomLobbyWidget] Copied share link: %s"), *ShareUrl);
}

// =============================================================================
// UI Callbacks
// =============================================================================

void UDeskillzRoomLobbyWidget::OnLeaveButtonClicked()
{
	OnLeaveClicked.Broadcast();
}

void UDeskillzRoomLobbyWidget::OnCancelRoomButtonClicked()
{
	if (!bIsHost)
	{
		return;
	}
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	FOnRoomActionSuccess OnSuccess;
	OnSuccess.BindLambda([this]()
	{
		UE_LOG(LogTemp, Log, TEXT("[RoomLobbyWidget] Room cancelled"));
		OnRoomCancelled.Broadcast();
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([](const FRoomError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomLobbyWidget] Failed to cancel: %s"), *Error.Message);
	});
	
	Rooms->CancelRoom(OnSuccess, OnError);
}

void UDeskillzRoomLobbyWidget::OnReadyButtonClicked()
{
	bIsReady = !bIsReady;
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (Rooms)
	{
		Rooms->SetReady(bIsReady);
	}
	
	UpdateButtons();
}

void UDeskillzRoomLobbyWidget::OnStartButtonClicked()
{
	if (!bIsHost)
	{
		return;
	}
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	FOnRoomActionSuccess OnSuccess;
	OnSuccess.BindLambda([]()
	{
		UE_LOG(LogTemp, Log, TEXT("[RoomLobbyWidget] Start match requested"));
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([](const FRoomError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomLobbyWidget] Failed to start: %s"), *Error.Message);
	});
	
	Rooms->StartMatch(OnSuccess, OnError);
}

void UDeskillzRoomLobbyWidget::OnCopyCodeButtonClicked()
{
	CopyRoomCode();
}

void UDeskillzRoomLobbyWidget::OnShareButtonClicked()
{
	ShareRoom();
}

void UDeskillzRoomLobbyWidget::OnSendChatButtonClicked()
{
	if (!ChatInput)
	{
		return;
	}
	
	FString Message = ChatInput->GetText().ToString().TrimStartAndEnd();
	if (Message.IsEmpty())
	{
		return;
	}
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (Rooms)
	{
		Rooms->SendChat(Message);
	}
	
	ChatInput->SetText(FText::GetEmpty());
}

void UDeskillzRoomLobbyWidget::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		OnSendChatButtonClicked();
	}
}

// =============================================================================
// Room Event Handlers
// =============================================================================

void UDeskillzRoomLobbyWidget::HandlePlayerJoined(const FRoomPlayer& Player)
{
	if (!bIsVisible)
	{
		return;
	}
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (Rooms)
	{
		UpdateRoom(Rooms->GetCurrentRoom());
	}
}

void UDeskillzRoomLobbyWidget::HandlePlayerLeft(const FString& PlayerId)
{
	if (!bIsVisible)
	{
		return;
	}
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (Rooms)
	{
		UpdateRoom(Rooms->GetCurrentRoom());
	}
}

void UDeskillzRoomLobbyWidget::HandlePlayerReadyChanged(const FString& PlayerId, bool bReady)
{
	if (!bIsVisible)
	{
		return;
	}
	
	// Update own ready state if it's us
	// In a real implementation, check if PlayerId matches current user
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (Rooms)
	{
		UpdateRoom(Rooms->GetCurrentRoom());
	}
}

void UDeskillzRoomLobbyWidget::HandleCountdownStarted(int32 Seconds)
{
	ShowCountdown(Seconds);
}

void UDeskillzRoomLobbyWidget::HandleCountdownTick(int32 Seconds)
{
	CountdownSeconds = Seconds;
	
	if (CountdownText)
	{
		CountdownText->SetText(FText::FromString(FString::FromInt(Seconds)));
	}
	
	if (Seconds <= 0)
	{
		HideCountdown();
	}
}

void UDeskillzRoomLobbyWidget::HandleChatReceived(const FString& SenderId, const FString& Username, const FString& Message)
{
	if (!bIsVisible)
	{
		return;
	}
	
	AddChatMessage(Username, Message);
}

void UDeskillzRoomLobbyWidget::HandleKickPlayer(const FString& PlayerId)
{
	if (!bIsHost)
	{
		return;
	}
	
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	FOnRoomActionSuccess OnSuccess;
	OnSuccess.BindLambda([PlayerId]()
	{
		UE_LOG(LogTemp, Log, TEXT("[RoomLobbyWidget] Kicked player %s"), *PlayerId);
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([](const FRoomError& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("[RoomLobbyWidget] Failed to kick: %s"), *Error.Message);
	});
	
	Rooms->KickPlayer(PlayerId, OnSuccess, OnError);
}