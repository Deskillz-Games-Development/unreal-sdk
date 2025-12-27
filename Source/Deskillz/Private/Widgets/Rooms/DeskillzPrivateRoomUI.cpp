// Copyright Deskillz Games. All Rights Reserved.

#include "Widgets/Rooms/DeskillzPrivateRoomUI.h"
#include "Widgets/Rooms/DeskillzRoomListWidget.h"
#include "Widgets/Rooms/DeskillzCreateRoomWidget.h"
#include "Widgets/Rooms/DeskillzJoinRoomWidget.h"
#include "Widgets/Rooms/DeskillzRoomLobbyWidget.h"
#include "Rooms/DeskillzRooms.h"
#include "Components/CanvasPanel.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

TWeakObjectPtr<UDeskillzPrivateRoomUI> UDeskillzPrivateRoomUI::Instance = nullptr;

UDeskillzPrivateRoomUI::UDeskillzPrivateRoomUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bAutoShowLobbyOnJoin(true)
	, bAutoHideOnMatchStart(true)
	, ActivePanelIndex(-1)
{
}

void UDeskillzPrivateRoomUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	Instance = this;
	InitializeUI();
	SubscribeToRoomEvents();
}

void UDeskillzPrivateRoomUI::NativeDestruct()
{
	UnsubscribeFromRoomEvents();
	
	if (Instance.Get() == this)
	{
		Instance = nullptr;
	}
	
	Super::NativeDestruct();
}

// =============================================================================
// Static Access
// =============================================================================

UDeskillzPrivateRoomUI* UDeskillzPrivateRoomUI::GetOrCreate(UObject* WorldContextObject)
{
	if (Instance.IsValid())
	{
		return Instance.Get();
	}
	
	if (!WorldContextObject)
	{
		return nullptr;
	}
	
	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}
	
	// Create widget - in production, you'd use a proper widget class
	UDeskillzPrivateRoomUI* NewWidget = CreateWidget<UDeskillzPrivateRoomUI>(World);
	if (NewWidget)
	{
		NewWidget->AddToViewport(100);
		Instance = NewWidget;
	}
	
	return NewWidget;
}

UDeskillzPrivateRoomUI* UDeskillzPrivateRoomUI::GetInstance()
{
	return Instance.Get();
}

// =============================================================================
// Initialization
// =============================================================================

void UDeskillzPrivateRoomUI::InitializeUI()
{
	// Create child widgets if not bound in Blueprint
	CreateChildWidgets();
	
	// Wire up navigation events
	if (RoomListPanel)
	{
		RoomListPanel->OnCreateRoomClicked.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleCreateRoomClicked);
		RoomListPanel->OnJoinByCodeClicked.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleJoinByCodeClicked);
		RoomListPanel->OnRoomSelected.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomSelected);
		RoomListPanel->OnBackClicked.AddDynamic(this, &UDeskillzPrivateRoomUI::HideAll);
	}
	
	if (CreateRoomPanel)
	{
		CreateRoomPanel->OnRoomCreated.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomCreated);
		CreateRoomPanel->OnBackClicked.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleBackToList);
	}
	
	if (JoinRoomPanel)
	{
		JoinRoomPanel->OnRoomJoined.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomJoinedFromPanel);
		JoinRoomPanel->OnBackClicked.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleBackToList);
	}
	
	if (RoomLobbyPanel)
	{
		RoomLobbyPanel->OnLeaveClicked.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleLeaveLobby);
	}
	
	// Initially hide all panels
	HideAllPanels();
}

void UDeskillzPrivateRoomUI::CreateChildWidgets()
{
	// Child widgets are typically created in Blueprint UMG designer
	// This method is for programmatic creation if needed
	
	// Note: In production, these would be created from Blueprint classes
	// specified in properties, not directly constructed
}

void UDeskillzPrivateRoomUI::SubscribeToRoomEvents()
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	Rooms->OnRoomJoined.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomJoined);
	Rooms->OnRoomUpdated.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomUpdated);
	Rooms->OnMatchLaunching.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleMatchLaunching);
	Rooms->OnRoomCancelled.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomCancelled);
	Rooms->OnKickedFromRoom.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleKicked);
	Rooms->OnRoomLeft.AddDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomLeft);
}

void UDeskillzPrivateRoomUI::UnsubscribeFromRoomEvents()
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	Rooms->OnRoomJoined.RemoveDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomJoined);
	Rooms->OnRoomUpdated.RemoveDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomUpdated);
	Rooms->OnMatchLaunching.RemoveDynamic(this, &UDeskillzPrivateRoomUI::HandleMatchLaunching);
	Rooms->OnRoomCancelled.RemoveDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomCancelled);
	Rooms->OnKickedFromRoom.RemoveDynamic(this, &UDeskillzPrivateRoomUI::HandleKicked);
	Rooms->OnRoomLeft.RemoveDynamic(this, &UDeskillzPrivateRoomUI::HandleRoomLeft);
}

// =============================================================================
// Show UI Panels
// =============================================================================

void UDeskillzPrivateRoomUI::ShowRoomList()
{
	HideAllPanels();
	
	if (RoomListPanel)
	{
		RoomListPanel->Show();
		ActivePanelIndex = 0;
	}
	
	SetVisibility(ESlateVisibility::Visible);
	OnPanelShown.Broadcast(FPrivateRoom());
}

void UDeskillzPrivateRoomUI::ShowCreateRoom()
{
	HideAllPanels();
	
	if (CreateRoomPanel)
	{
		CreateRoomPanel->Show();
		ActivePanelIndex = 1;
	}
	
	SetVisibility(ESlateVisibility::Visible);
	OnPanelShown.Broadcast(FPrivateRoom());
}

void UDeskillzPrivateRoomUI::ShowJoinRoom()
{
	HideAllPanels();
	
	if (JoinRoomPanel)
	{
		JoinRoomPanel->Show();
		ActivePanelIndex = 2;
	}
	
	SetVisibility(ESlateVisibility::Visible);
	OnPanelShown.Broadcast(FPrivateRoom());
}

void UDeskillzPrivateRoomUI::ShowJoinRoomWithCode(const FString& RoomCode)
{
	HideAllPanels();
	
	if (JoinRoomPanel)
	{
		JoinRoomPanel->ShowWithCode(RoomCode);
		ActivePanelIndex = 2;
	}
	
	SetVisibility(ESlateVisibility::Visible);
	OnPanelShown.Broadcast(FPrivateRoom());
}

void UDeskillzPrivateRoomUI::ShowRoomLobby()
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms || !Rooms->IsInRoom())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PrivateRoomUI] Cannot show lobby: not in a room"));
		return;
	}
	
	ShowRoomLobbyWithRoom(Rooms->GetCurrentRoom());
}

void UDeskillzPrivateRoomUI::ShowRoomLobbyWithRoom(const FPrivateRoom& Room)
{
	HideAllPanels();
	
	if (RoomLobbyPanel)
	{
		RoomLobbyPanel->SetRoom(Room);
		RoomLobbyPanel->Show();
		ActivePanelIndex = 3;
	}
	
	SetVisibility(ESlateVisibility::Visible);
	OnPanelShown.Broadcast(Room);
}

void UDeskillzPrivateRoomUI::HideAll()
{
	HideAllPanels();
	SetVisibility(ESlateVisibility::Collapsed);
	ActivePanelIndex = -1;
	OnAllHidden.Broadcast();
}

void UDeskillzPrivateRoomUI::Close()
{
	HideAll();
	RemoveFromParent();
}

void UDeskillzPrivateRoomUI::HideAllPanels()
{
	if (RoomListPanel)
	{
		RoomListPanel->Hide();
	}
	if (CreateRoomPanel)
	{
		CreateRoomPanel->Hide();
	}
	if (JoinRoomPanel)
	{
		JoinRoomPanel->Hide();
	}
	if (RoomLobbyPanel)
	{
		RoomLobbyPanel->Hide();
	}
}

// =============================================================================
// Quick Actions
// =============================================================================

void UDeskillzPrivateRoomUI::QuickCreateRoom(const FString& RoomName, float EntryFee)
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		ShowNotification(TEXT("Room service not available"), true);
		return;
	}
	
	FOnRoomSuccess OnSuccess;
	OnSuccess.BindLambda([this](const FPrivateRoom& Room)
	{
		ShowRoomLobbyWithRoom(Room);
		OnRoomCreatedFromUI.Broadcast(Room);
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([this](const FRoomError& Error)
	{
		ShowNotification(FString::Printf(TEXT("Failed to create room: %s"), *Error.Message), true);
	});
	
	Rooms->QuickCreateRoom(RoomName, EntryFee, OnSuccess, OnError);
}

void UDeskillzPrivateRoomUI::QuickJoinRoom(const FString& RoomCode)
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		ShowNotification(TEXT("Room service not available"), true);
		return;
	}
	
	FOnRoomSuccess OnSuccess;
	OnSuccess.BindLambda([this](const FPrivateRoom& Room)
	{
		ShowRoomLobbyWithRoom(Room);
		OnRoomJoinedFromUI.Broadcast(Room);
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([this](const FRoomError& Error)
	{
		ShowNotification(FString::Printf(TEXT("Failed to join room: %s"), *Error.Message), true);
	});
	
	Rooms->JoinRoom(RoomCode, OnSuccess, OnError);
}

// =============================================================================
// Properties
// =============================================================================

bool UDeskillzPrivateRoomUI::IsAnyPanelVisible() const
{
	return (RoomListPanel && RoomListPanel->IsWidgetVisible()) ||
	       (CreateRoomPanel && CreateRoomPanel->IsWidgetVisible()) ||
	       (JoinRoomPanel && JoinRoomPanel->IsWidgetVisible()) ||
	       (RoomLobbyPanel && RoomLobbyPanel->IsWidgetVisible());
}

FString UDeskillzPrivateRoomUI::GetActivePanelName() const
{
	switch (ActivePanelIndex)
	{
	case 0: return TEXT("RoomList");
	case 1: return TEXT("CreateRoom");
	case 2: return TEXT("JoinRoom");
	case 3: return TEXT("RoomLobby");
	default: return TEXT("None");
	}
}

// =============================================================================
// Event Handlers
// =============================================================================

void UDeskillzPrivateRoomUI::HandleRoomJoined(const FPrivateRoom& Room)
{
	if (bAutoShowLobbyOnJoin && !RoomLobbyPanel->IsWidgetVisible())
	{
		ShowRoomLobbyWithRoom(Room);
	}
}

void UDeskillzPrivateRoomUI::HandleRoomUpdated(const FPrivateRoom& Room)
{
	if (RoomLobbyPanel && RoomLobbyPanel->IsWidgetVisible())
	{
		RoomLobbyPanel->UpdateRoom(Room);
	}
}

void UDeskillzPrivateRoomUI::HandleMatchLaunching(const FMatchLaunchData& LaunchData)
{
	if (bAutoHideOnMatchStart)
	{
		HideAll();
	}
	
	ShowNotification(TEXT("Match starting!"), false);
	UE_LOG(LogTemp, Log, TEXT("[PrivateRoomUI] Match launching: %s"), *LaunchData.MatchId);
}

void UDeskillzPrivateRoomUI::HandleRoomCancelled(const FString& Reason)
{
	HideAll();
	ShowNotification(FString::Printf(TEXT("Room cancelled: %s"), *Reason), true);
}

void UDeskillzPrivateRoomUI::HandleKicked(const FString& Reason)
{
	HideAll();
	ShowNotification(FString::Printf(TEXT("You were removed from the room: %s"), *Reason), true);
}

void UDeskillzPrivateRoomUI::HandleRoomLeft()
{
	if (RoomLobbyPanel && RoomLobbyPanel->IsWidgetVisible())
	{
		ShowRoomList();
	}
}

// =============================================================================
// Panel Navigation Handlers
// =============================================================================

void UDeskillzPrivateRoomUI::HandleCreateRoomClicked()
{
	ShowCreateRoom();
}

void UDeskillzPrivateRoomUI::HandleJoinByCodeClicked()
{
	ShowJoinRoom();
}

void UDeskillzPrivateRoomUI::HandleRoomSelected(const FPrivateRoom& Room)
{
	QuickJoinRoom(Room.RoomCode);
}

void UDeskillzPrivateRoomUI::HandleBackToList()
{
	ShowRoomList();
}

void UDeskillzPrivateRoomUI::HandleRoomCreated(const FPrivateRoom& Room)
{
	if (bAutoShowLobbyOnJoin)
	{
		ShowRoomLobbyWithRoom(Room);
	}
	OnRoomCreatedFromUI.Broadcast(Room);
}

void UDeskillzPrivateRoomUI::HandleRoomJoinedFromPanel(const FPrivateRoom& Room)
{
	if (bAutoShowLobbyOnJoin)
	{
		ShowRoomLobbyWithRoom(Room);
	}
	OnRoomJoinedFromUI.Broadcast(Room);
}

void UDeskillzPrivateRoomUI::HandleLeaveLobby()
{
	UDeskillzRooms* Rooms = UDeskillzRooms::Get(this);
	if (!Rooms)
	{
		return;
	}
	
	FOnRoomActionSuccess OnSuccess;
	OnSuccess.BindLambda([this]()
	{
		ShowRoomList();
	});
	
	FOnRoomError OnError;
	OnError.BindLambda([this](const FRoomError& Error)
	{
		ShowNotification(FString::Printf(TEXT("Failed to leave: %s"), *Error.Message), true);
	});
	
	Rooms->LeaveRoom(OnSuccess, OnError);
}

// =============================================================================
// Notifications
// =============================================================================

void UDeskillzPrivateRoomUI::ShowNotification(const FString& Message, bool bIsError)
{
	// In production, integrate with your notification system
	UE_LOG(LogTemp, Log, TEXT("[PrivateRoomUI] %s: %s"), 
		bIsError ? TEXT("Error") : TEXT("Info"), *Message);
}