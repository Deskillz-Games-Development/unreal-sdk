// Copyright Deskillz Games. All Rights Reserved.

#include "Widgets/Rooms/DeskillzJoinRoomWidget.h"
#include "Rooms/DeskillzRooms.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Throbber.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"

UDeskillzJoinRoomWidget::UDeskillzJoinRoomWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bHasPreview(false)
	, bIsLoading(false)
	, bIsVisible(false)
{
}

void UDeskillzJoinRoomWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeUI();
}

void UDeskillzJoinRoomWidget::InitializeUI()
{
	// Bind buttons
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDeskillzJoinRoomWidget::OnBackButtonClicked);
	}
	
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UDeskillzJoinRoomWidget::OnCancelButtonClicked);
	}
	
	if (LookupButton)
	{
		LookupButton->OnClicked.AddDynamic(this, &UDeskillzJoinRoomWidget::OnLookupButtonClicked);
	}
	
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UDeskillzJoinRoomWidget::OnJoinButtonClicked);
	}
	
	// Bind code input
	if (CodeInput)
	{
		CodeInput->OnTextChanged.AddDynamic(this, &UDeskillzJoinRoomWidget::OnCodeInputChanged);
		CodeInput->OnTextCommitted.AddDynamic(this, &UDeskillzJoinRoomWidget::OnCodeInputCommitted);
	}
	
	// Hide loading and preview
	SetLoading(false);
	HidePreview();
	ClearError();
}

// =============================================================================
// Public Methods
// =============================================================================

void UDeskillzJoinRoomWidget::Show()
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);
	ResetForm();
	
	// Focus on code input
	if (CodeInput)
	{
		CodeInput->SetKeyboardFocus();
	}
}

void UDeskillzJoinRoomWidget::ShowWithCode(const FString& RoomCode)
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);
	ResetForm();
	
	if (!RoomCode.IsEmpty() && CodeInput)
	{
		CodeInput->SetText(FText::FromString(RoomCode.ToUpper()));
		LookupRoom();
	}
}

void UDeskillzJoinRoomWidget::Hide()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDeskillzJoinRoomWidget::ResetForm()
{
	if (CodeInput)
	{
		CodeInput->SetText(FText::GetEmpty());
	}
	
	bHasPreview = false;
	PreviewRoom = FPrivateRoom();
	HidePreview();
	ClearError();
	UpdateJoinButton();
}

// =============================================================================
// Room Lookup
// =============================================================================

void UDeskillzJoinRoomWidget::LookupRoom()
{
	FString Code = CodeInput ? CodeInput->GetText().ToString().TrimStartAndEnd().ToUpper() : TEXT("");
	
	if (Code.IsEmpty() || Code.Len() < 8)
	{
		return;
	}
	
	SetLoading(true);
	
	UDeskillzRooms* RoomSystem = UDeskillzRooms::Get(this);
	if (!RoomSystem)
	{
		SetLoading(false);
		ShowError(TEXT("Room service not available"));
		return;
	}
	
	FOnRoomSuccess OnSuccess;
	OnSuccess.BindLambda([this](const FPrivateRoom& Room)
	{
		SetLoading(false);
		ShowPreview(Room);
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([this](const FRoomError& Error)
	{
		SetLoading(false);
		ShowError(Error.Message);
		HidePreview();
	});
	
	RoomSystem->GetRoomByCode(Code, OnSuccess, OnError);
}

void UDeskillzJoinRoomWidget::ShowPreview(const FPrivateRoom& Room)
{
	PreviewRoom = Room;
	bHasPreview = true;
	
	if (PreviewSection)
	{
		PreviewSection->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(Room.Name));
	}
	
	if (HostText)
	{
		HostText->SetText(FText::FromString(FString::Printf(TEXT("Host: %s"), *Room.Host.Username)));
	}
	
	if (PlayersText)
	{
		PlayersText->SetText(FText::FromString(FString::Printf(TEXT("Players: %d/%d"), 
			Room.CurrentPlayers, Room.MaxPlayers)));
	}
	
	if (EntryFeeText)
	{
		EntryFeeText->SetText(FText::FromString(FString::Printf(TEXT("Entry Fee: $%.2f %s"), 
			Room.EntryFee, *Room.EntryCurrency)));
	}
	
	if (ModeText)
	{
		FString ModeStr = Room.Mode == ERoomMode::Sync ? TEXT("Real-time") : TEXT("Turn-based");
		ModeText->SetText(FText::FromString(FString::Printf(TEXT("Mode: %s"), *ModeStr)));
	}
	
	UpdateJoinButton();
	
	// Show warnings if can't join
	if (!Room.CanJoin())
	{
		if (Room.IsFull())
		{
			ShowError(TEXT("Room is full"));
		}
		else if (Room.Status != ERoomStatus::Waiting)
		{
			ShowError(TEXT("Room is no longer accepting players"));
		}
	}
	else
	{
		ClearError();
	}
}

void UDeskillzJoinRoomWidget::HidePreview()
{
	bHasPreview = false;
	
	if (PreviewSection)
	{
		PreviewSection->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	UpdateJoinButton();
}

void UDeskillzJoinRoomWidget::JoinRoom()
{
	if (bIsLoading || !bHasPreview)
	{
		return;
	}
	
	SetLoading(true);
	
	UDeskillzRooms* RoomSystem = UDeskillzRooms::Get(this);
	if (!RoomSystem)
	{
		SetLoading(false);
		ShowError(TEXT("Room service not available"));
		return;
	}
	
	FOnRoomSuccess OnSuccess;
	OnSuccess.BindLambda([this](const FPrivateRoom& Room)
	{
		SetLoading(false);
		OnRoomJoined.Broadcast(Room);
		UE_LOG(LogTemp, Log, TEXT("[JoinRoomWidget] Joined room: %s"), *Room.RoomCode);
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([this](const FRoomError& Error)
	{
		SetLoading(false);
		ShowError(Error.Message);
	});
	
	RoomSystem->JoinRoom(PreviewRoom.RoomCode, OnSuccess, OnError);
}

// =============================================================================
// Helpers
// =============================================================================

FString UDeskillzJoinRoomWidget::FormatCode(const FString& Input) const
{
	FString Formatted = Input.ToUpper();
	
	// Remove any non-alphanumeric except hyphen
	FString Clean;
	for (TCHAR Char : Formatted)
	{
		if (FChar::IsAlnum(Char) || Char == '-')
		{
			Clean.AppendChar(Char);
		}
	}
	
	// Auto-insert hyphen after DSKZ
	if (Clean.Len() == 4 && !Clean.Contains(TEXT("-")))
	{
		Clean += TEXT("-");
	}
	
	return Clean;
}

void UDeskillzJoinRoomWidget::ShowError(const FString& Message)
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(Message));
		ErrorText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDeskillzJoinRoomWidget::ClearError()
{
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDeskillzJoinRoomWidget::SetLoading(bool bLoading)
{
	bIsLoading = bLoading;
	
	if (LoadingOverlay)
	{
		LoadingOverlay->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (LookupButton)
	{
		LookupButton->SetIsEnabled(!bLoading);
	}
	
	if (CodeInput)
	{
		CodeInput->SetIsEnabled(!bLoading);
	}
	
	if (CancelButton)
	{
		CancelButton->SetIsEnabled(!bLoading);
	}
	
	if (BackButton)
	{
		BackButton->SetIsEnabled(!bLoading);
	}
	
	UpdateJoinButton();
}

void UDeskillzJoinRoomWidget::UpdateJoinButton()
{
	if (JoinButton)
	{
		bool bCanJoin = !bIsLoading && bHasPreview && PreviewRoom.CanJoin();
		JoinButton->SetIsEnabled(bCanJoin);
	}
}

// =============================================================================
// UI Callbacks
// =============================================================================

void UDeskillzJoinRoomWidget::OnBackButtonClicked()
{
	OnBackClicked.Broadcast();
}

void UDeskillzJoinRoomWidget::OnCancelButtonClicked()
{
	OnBackClicked.Broadcast();
}

void UDeskillzJoinRoomWidget::OnLookupButtonClicked()
{
	LookupRoom();
}

void UDeskillzJoinRoomWidget::OnJoinButtonClicked()
{
	JoinRoom();
}

void UDeskillzJoinRoomWidget::OnCodeInputChanged(const FText& Text)
{
	FString CurrentText = Text.ToString();
	FString FormattedText = FormatCode(CurrentText);
	
	// Update if formatting changed
	if (CurrentText != FormattedText && CodeInput)
	{
		CodeInput->SetText(FText::FromString(FormattedText));
	}
	
	// Clear preview if code changed
	if (bHasPreview && PreviewRoom.RoomCode != FormattedText)
	{
		HidePreview();
	}
	
	ClearError();
}

void UDeskillzJoinRoomWidget::OnCodeInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		LookupRoom();
	}
}