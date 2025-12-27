// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rooms/DeskillzRoomTypes.h"
#include "DeskillzRoomLobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UVerticalBox;
class UProgressBar;
class UEditableTextBox;
class UCanvasPanel;
class UDeskillzRoomPlayerCard;

/** Delegate for lobby navigation */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyNavigationDelegate);

/**
 * Widget for the room lobby (waiting room).
 * Shows player list, ready status, room code, and controls.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzRoomLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeskillzRoomLobbyWidget(const FObjectInitializer& ObjectInitializer);

	// =========================================================================
	// Widget Lifecycle
	// =========================================================================

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// =========================================================================
	// Public Methods
	// =========================================================================

	/**
	 * Set the room to display
	 * @param Room Room data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void SetRoom(const FPrivateRoom& Room);

	/**
	 * Update the room display with new data
	 * @param Room Updated room data
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void UpdateRoom(const FPrivateRoom& Room);

	/**
	 * Show the widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void Show();

	/**
	 * Hide the widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void Hide();

	/**
	 * Check if widget is visible
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	bool IsWidgetVisible() const { return bIsVisible; }

	/**
	 * Get current room
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	FPrivateRoom GetCurrentRoom() const { return CurrentRoom; }

	// =========================================================================
	// Events
	// =========================================================================

	/** Called when leave button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnLobbyNavigationDelegate OnLeaveClicked;

	/** Called when room is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnLobbyNavigationDelegate OnRoomCancelled;

protected:
	// =========================================================================
	// UI Components - Header
	// =========================================================================

	/** Room name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* RoomNameText;

	/** Room code text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* RoomCodeText;

	/** Copy code button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* CopyCodeButton;

	/** Share button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* ShareButton;

	// =========================================================================
	// UI Components - Room Info
	// =========================================================================

	/** Entry fee text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* EntryFeeText;

	/** Prize pool text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* PrizePoolText;

	/** Mode text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* ModeText;

	/** Status text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* StatusText;

	// =========================================================================
	// UI Components - Player List
	// =========================================================================

	/** Player count text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* PlayerCountText;

	/** Player list scroll box */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UScrollBox* PlayerListScrollBox;

	/** Player list content */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UVerticalBox* PlayerListContent;

	// =========================================================================
	// UI Components - Ready Section
	// =========================================================================

	/** Ready status text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* ReadyStatusText;

	/** Ready progress bar */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UProgressBar* ReadyProgressBar;

	// =========================================================================
	// UI Components - Countdown
	// =========================================================================

	/** Countdown overlay */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UCanvasPanel* CountdownOverlay;

	/** Countdown text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* CountdownText;

	// =========================================================================
	// UI Components - Chat (Optional)
	// =========================================================================

	/** Chat container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UVerticalBox* ChatContainer;

	/** Chat scroll box */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UScrollBox* ChatScrollBox;

	/** Chat content */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UVerticalBox* ChatContent;

	/** Chat input */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UEditableTextBox* ChatInput;

	/** Send chat button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* SendChatButton;

	// =========================================================================
	// UI Components - Action Buttons
	// =========================================================================

	/** Leave button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* LeaveButton;

	/** Cancel button (host only) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* CancelRoomButton;

	/** Ready button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* ReadyButton;

	/** Ready button text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* ReadyButtonText;

	/** Start button (host only) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* StartButton;

	// =========================================================================
	// Settings
	// =========================================================================

	/** Player card widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	TSubclassOf<UDeskillzRoomPlayerCard> PlayerCardClass;

	/** Enable chat feature */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	bool bEnableChat;

	// =========================================================================
	// State
	// =========================================================================

	/** Current room data */
	UPROPERTY()
	FPrivateRoom CurrentRoom;

	/** Active player card widgets */
	UPROPERTY()
	TArray<UDeskillzRoomPlayerCard*> PlayerCards;

	/** Whether current user is host */
	bool bIsHost;

	/** Whether current user is ready */
	bool bIsReady;

	/** Current countdown seconds */
	int32 CountdownSeconds;

	/** Whether widget is visible */
	bool bIsVisible;

	// =========================================================================
	// Internal Methods
	// =========================================================================

	/** Initialize UI bindings */
	void InitializeUI();

	/** Subscribe to room events */
	void SubscribeToRoomEvents();

	/** Unsubscribe from room events */
	void UnsubscribeFromRoomEvents();

	/** Update all display elements */
	void UpdateDisplay();

	/** Update player list */
	void UpdatePlayerList();

	/** Create a player card */
	UDeskillzRoomPlayerCard* CreatePlayerCard(const FRoomPlayer& Player);

	/** Clear all player cards */
	void ClearPlayerCards();

	/** Update action buttons visibility/state */
	void UpdateButtons();

	/** Show countdown overlay */
	void ShowCountdown(int32 Seconds);

	/** Hide countdown overlay */
	void HideCountdown();

	/** Add chat message */
	void AddChatMessage(const FString& Username, const FString& Message);

	/** Copy room code to clipboard */
	void CopyRoomCode();

	/** Share room link */
	void ShareRoom();

	// =========================================================================
	// UI Callbacks
	// =========================================================================

	UFUNCTION()
	void OnLeaveButtonClicked();

	UFUNCTION()
	void OnCancelRoomButtonClicked();

	UFUNCTION()
	void OnReadyButtonClicked();

	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnCopyCodeButtonClicked();

	UFUNCTION()
	void OnShareButtonClicked();

	UFUNCTION()
	void OnSendChatButtonClicked();

	UFUNCTION()
	void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// =========================================================================
	// Room Event Handlers
	// =========================================================================

	UFUNCTION()
	void HandlePlayerJoined(const FRoomPlayer& Player);

	UFUNCTION()
	void HandlePlayerLeft(const FString& PlayerId);

	UFUNCTION()
	void HandlePlayerReadyChanged(const FString& PlayerId, bool bReady);

	UFUNCTION()
	void HandleCountdownStarted(int32 Seconds);

	UFUNCTION()
	void HandleCountdownTick(int32 Seconds);

	UFUNCTION()
	void HandleChatReceived(const FString& SenderId, const FString& Username, const FString& Message);

	UFUNCTION()
	void HandleKickPlayer(const FString& PlayerId);
};