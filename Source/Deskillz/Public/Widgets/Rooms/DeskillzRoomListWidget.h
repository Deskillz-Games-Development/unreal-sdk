// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rooms/DeskillzRoomTypes.h"
#include "DeskillzRoomListWidget.generated.h"

class UScrollBox;
class UEditableTextBox;
class UComboBoxString;
class UButton;
class UTextBlock;
class UDeskillzRoomCardWidget;
class UVerticalBox;
class UThrobber;

/** Sort options for room list */
UENUM(BlueprintType)
enum class ERoomSortOption : uint8
{
	Newest UMETA(DisplayName = "Newest First"),
	EntryFeeAsc UMETA(DisplayName = "Entry Fee: Low to High"),
	EntryFeeDesc UMETA(DisplayName = "Entry Fee: High to Low"),
	PlayersAsc UMETA(DisplayName = "Players: Low to High"),
	PlayersDesc UMETA(DisplayName = "Players: High to Low")
};

/** Delegate for room selection */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomSelectedDelegate, const FPrivateRoom&, Room);

/** Delegate for navigation actions */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoomListNavigationDelegate);

/**
 * Widget for browsing public rooms.
 * Shows a list of available rooms with filtering and sorting options.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzRoomListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeskillzRoomListWidget(const FObjectInitializer& ObjectInitializer);

	// =========================================================================
	// Widget Lifecycle
	// =========================================================================

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// =========================================================================
	// Public Methods
	// =========================================================================

	/**
	 * Refresh the room list from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void RefreshRooms();

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

	// =========================================================================
	// Events
	// =========================================================================

	/** Called when create room button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomListNavigationDelegate OnCreateRoomClicked;

	/** Called when join by code button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomListNavigationDelegate OnJoinByCodeClicked;

	/** Called when a room is selected from the list */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomSelectedDelegate OnRoomSelected;

	/** Called when back button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomListNavigationDelegate OnBackClicked;

protected:
	// =========================================================================
	// UI Components
	// =========================================================================

	/** Back button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* BackButton;

	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* TitleText;

	/** Refresh button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* RefreshButton;

	/** Search input */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UEditableTextBox* SearchInput;

	/** Sort dropdown */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UComboBoxString* SortDropdown;

	/** Room list scroll box */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UScrollBox* RoomListScrollBox;

	/** Room list content container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UVerticalBox* RoomListContent;

	/** Empty state text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* EmptyStateText;

	/** Loading indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UThrobber* LoadingIndicator;

	/** Create room button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* CreateRoomButton;

	/** Join by code button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* JoinCodeButton;

	// =========================================================================
	// Settings
	// =========================================================================

	/** Room card widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	TSubclassOf<UDeskillzRoomCardWidget> RoomCardClass;

	// =========================================================================
	// State
	// =========================================================================

	/** Cached room list */
	UPROPERTY()
	TArray<FPrivateRoom> Rooms;

	/** Active room card widgets */
	UPROPERTY()
	TArray<UDeskillzRoomCardWidget*> RoomCards;

	/** Current search filter */
	FString SearchFilter;

	/** Current sort option */
	ERoomSortOption CurrentSortOption;

	/** Whether currently loading */
	bool bIsLoading;

	/** Whether widget is visible */
	bool bIsVisible;

	// =========================================================================
	// Internal Methods
	// =========================================================================

	/** Initialize UI bindings */
	void InitializeUI();

	/** Apply filters and sort to room list */
	void ApplyFiltersAndSort();

	/** Update room card display */
	void UpdateRoomCards(const TArray<FPrivateRoom>& FilteredRooms);

	/** Create a room card widget */
	UDeskillzRoomCardWidget* CreateRoomCard(const FPrivateRoom& Room);

	/** Clear all room cards */
	void ClearRoomCards();

	/** Set loading state */
	void SetLoading(bool bLoading);

	/** Show/hide empty state */
	void ShowEmptyState(bool bShow);

	// =========================================================================
	// UI Callbacks
	// =========================================================================

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnRefreshButtonClicked();

	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);

	UFUNCTION()
	void OnSortSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnCreateRoomButtonClicked();

	UFUNCTION()
	void OnJoinCodeButtonClicked();

	UFUNCTION()
	void OnRoomCardJoinClicked(const FPrivateRoom& Room);
};

/**
 * Individual room card widget for the list
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzRoomCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeskillzRoomCardWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/**
	 * Set the room data to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void SetRoom(const FPrivateRoom& Room);

	/**
	 * Get the current room
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	FPrivateRoom GetRoom() const { return CurrentRoom; }

	/** Called when join button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomSelectedDelegate OnJoinClicked;

protected:
	/** Room name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* RoomNameText;

	/** Room code text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* RoomCodeText;

	/** Host name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* HostText;

	/** Player count text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* PlayersText;

	/** Entry fee text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* EntryFeeText;

	/** Join button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* JoinButton;

	/** Current room data */
	FPrivateRoom CurrentRoom;

	UFUNCTION()
	void OnJoinButtonClicked();

	void UpdateDisplay();
};