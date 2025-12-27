// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rooms/DeskillzRoomTypes.h"
#include "DeskillzPrivateRoomUI.generated.h"

class UDeskillzRoomListWidget;
class UDeskillzCreateRoomWidget;
class UDeskillzJoinRoomWidget;
class UDeskillzRoomLobbyWidget;
class UWidgetSwitcher;
class UCanvasPanel;

/**
 * Main manager for all Private Room UI components.
 * Provides one-call methods to show room-related screens.
 * 
 * Usage (Blueprint):
 * 1. Create widget from class
 * 2. Add to viewport
 * 3. Call ShowRoomList(), ShowCreateRoom(), etc.
 * 
 * Usage (C++):
 * @code
 * UDeskillzPrivateRoomUI* RoomUI = CreateWidget<UDeskillzPrivateRoomUI>(GetWorld(), RoomUIClass);
 * RoomUI->AddToViewport();
 * RoomUI->ShowRoomList();
 * @endcode
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzPrivateRoomUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeskillzPrivateRoomUI(const FObjectInitializer& ObjectInitializer);

	// =========================================================================
	// Widget Lifecycle
	// =========================================================================

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// =========================================================================
	// Static Access
	// =========================================================================

	/**
	 * Get or create the Private Room UI instance
	 * @param WorldContextObject World context
	 * @return The UI instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI", meta = (WorldContext = "WorldContextObject"))
	static UDeskillzPrivateRoomUI* GetOrCreate(UObject* WorldContextObject);

	/**
	 * Get existing instance if any
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	static UDeskillzPrivateRoomUI* GetInstance();

	// =========================================================================
	// Show UI Panels
	// =========================================================================

	/**
	 * Show the room list (browse public rooms)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ShowRoomList();

	/**
	 * Show the create room form
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ShowCreateRoom();

	/**
	 * Show the join room dialog (enter code)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ShowJoinRoom();

	/**
	 * Show the join room dialog with a pre-filled code
	 * @param RoomCode Pre-filled room code
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ShowJoinRoomWithCode(const FString& RoomCode);

	/**
	 * Show the room lobby (waiting room)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ShowRoomLobby();

	/**
	 * Show the room lobby with a specific room
	 * @param Room Room data to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ShowRoomLobbyWithRoom(const FPrivateRoom& Room);

	/**
	 * Hide all private room UI panels
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void HideAll();

	/**
	 * Close and remove from viewport
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void Close();

	// =========================================================================
	// Quick Actions
	// =========================================================================

	/**
	 * Quick create a room and show lobby
	 * @param RoomName Room display name
	 * @param EntryFee Entry fee amount
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void QuickCreateRoom(const FString& RoomName, float EntryFee);

	/**
	 * Quick join a room by code and show lobby
	 * @param RoomCode Room code to join
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void QuickJoinRoom(const FString& RoomCode);

	// =========================================================================
	// Properties
	// =========================================================================

	/** Whether any room UI panel is visible */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	bool IsAnyPanelVisible() const;

	/** Get current active panel name */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	FString GetActivePanelName() const;

	// =========================================================================
	// Events
	// =========================================================================

	/** Called when any room UI panel is shown */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomJoined OnPanelShown;

	/** Called when room is created from UI */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomJoined OnRoomCreatedFromUI;

	/** Called when room is joined from UI */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomJoined OnRoomJoinedFromUI;

	/** Called when all UI is hidden */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomLeft OnAllHidden;

protected:
	// =========================================================================
	// UI Components
	// =========================================================================

	/** Root canvas panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Deskillz|Rooms|UI")
	UCanvasPanel* RootPanel;

	/** Widget switcher for panels */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Deskillz|Rooms|UI")
	UWidgetSwitcher* PanelSwitcher;

	/** Room list panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Deskillz|Rooms|UI")
	UDeskillzRoomListWidget* RoomListPanel;

	/** Create room panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Deskillz|Rooms|UI")
	UDeskillzCreateRoomWidget* CreateRoomPanel;

	/** Join room panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Deskillz|Rooms|UI")
	UDeskillzJoinRoomWidget* JoinRoomPanel;

	/** Room lobby panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Deskillz|Rooms|UI")
	UDeskillzRoomLobbyWidget* RoomLobbyPanel;

	// =========================================================================
	// Settings
	// =========================================================================

	/** Auto-show lobby when joining a room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	bool bAutoShowLobbyOnJoin;

	/** Auto-hide UI when match starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	bool bAutoHideOnMatchStart;

	// =========================================================================
	// Internal
	// =========================================================================

	/** Current active panel index */
	int32 ActivePanelIndex;

	/** Static instance reference */
	static TWeakObjectPtr<UDeskillzPrivateRoomUI> Instance;

	/** Initialize UI components */
	void InitializeUI();

	/** Create child widgets programmatically if not bound */
	void CreateChildWidgets();

	/** Subscribe to room events */
	void SubscribeToRoomEvents();

	/** Unsubscribe from room events */
	void UnsubscribeFromRoomEvents();

	/** Hide all panels */
	void HideAllPanels();

	// =========================================================================
	// Event Handlers
	// =========================================================================

	UFUNCTION()
	void HandleRoomJoined(const FPrivateRoom& Room);

	UFUNCTION()
	void HandleRoomUpdated(const FPrivateRoom& Room);

	UFUNCTION()
	void HandleMatchLaunching(const FMatchLaunchData& LaunchData);

	UFUNCTION()
	void HandleRoomCancelled(const FString& Reason);

	UFUNCTION()
	void HandleKicked(const FString& Reason);

	UFUNCTION()
	void HandleRoomLeft();

	// =========================================================================
	// Panel Navigation Handlers
	// =========================================================================

	UFUNCTION()
	void HandleCreateRoomClicked();

	UFUNCTION()
	void HandleJoinByCodeClicked();

	UFUNCTION()
	void HandleRoomSelected(const FPrivateRoom& Room);

	UFUNCTION()
	void HandleBackToList();

	UFUNCTION()
	void HandleRoomCreated(const FPrivateRoom& Room);

	UFUNCTION()
	void HandleRoomJoinedFromPanel(const FPrivateRoom& Room);

	UFUNCTION()
	void HandleLeaveLobby();

	// =========================================================================
	// Notifications
	// =========================================================================

	/** Show a notification message */
	void ShowNotification(const FString& Message, bool bIsError = false);
};