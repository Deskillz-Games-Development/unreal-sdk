// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rooms/DeskillzRoomTypes.h"
#include "DeskillzJoinRoomWidget.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;
class UThrobber;
class UCanvasPanel;
class UVerticalBox;

/** Delegate for room joined from join widget */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomJoinedFromWidgetDelegate, const FPrivateRoom&, Room);

/** Delegate for navigation */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJoinRoomNavigationDelegate);

/**
 * Widget for joining a room by code.
 * Shows a dialog to enter a room code and preview room details.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzJoinRoomWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeskillzJoinRoomWidget(const FObjectInitializer& ObjectInitializer);

	// =========================================================================
	// Widget Lifecycle
	// =========================================================================

	virtual void NativeConstruct() override;

	// =========================================================================
	// Public Methods
	// =========================================================================

	/**
	 * Show the widget and reset form
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void Show();

	/**
	 * Show the widget with a pre-filled room code
	 * @param RoomCode Pre-filled room code
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ShowWithCode(const FString& RoomCode);

	/**
	 * Hide the widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void Hide();

	/**
	 * Reset the form
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void ResetForm();

	/**
	 * Check if widget is visible
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Rooms|UI")
	bool IsWidgetVisible() const { return bIsVisible; }

	// =========================================================================
	// Events
	// =========================================================================

	/** Called when room is successfully joined */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomJoinedFromWidgetDelegate OnRoomJoined;

	/** Called when back button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnJoinRoomNavigationDelegate OnBackClicked;

protected:
	// =========================================================================
	// UI Components - Header
	// =========================================================================

	/** Back button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* BackButton;

	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* TitleText;

	// =========================================================================
	// UI Components - Code Input
	// =========================================================================

	/** Code input field */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UEditableTextBox* CodeInput;

	/** Lookup button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* LookupButton;

	/** Hint text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* HintText;

	// =========================================================================
	// UI Components - Preview Section
	// =========================================================================

	/** Preview section container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UVerticalBox* PreviewSection;

	/** Room name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* RoomNameText;

	/** Host text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* HostText;

	/** Players text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* PlayersText;

	/** Entry fee text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* EntryFeeText;

	/** Mode text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* ModeText;

	// =========================================================================
	// UI Components - Actions
	// =========================================================================

	/** Error text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* ErrorText;

	/** Cancel button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* CancelButton;

	/** Join button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* JoinButton;

	/** Loading overlay */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UCanvasPanel* LoadingOverlay;

	/** Loading indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UThrobber* LoadingIndicator;

	// =========================================================================
	// State
	// =========================================================================

	/** Preview room data */
	UPROPERTY()
	FPrivateRoom PreviewRoom;

	/** Whether we have a valid preview */
	bool bHasPreview;

	/** Whether currently loading */
	bool bIsLoading;

	/** Whether widget is visible */
	bool bIsVisible;

	// =========================================================================
	// Internal Methods
	// =========================================================================

	/** Initialize UI bindings */
	void InitializeUI();

	/** Lookup room by code */
	void LookupRoom();

	/** Show room preview */
	void ShowPreview(const FPrivateRoom& Room);

	/** Hide room preview */
	void HidePreview();

	/** Join the previewed room */
	void JoinRoom();

	/** Format code input (uppercase, add hyphen) */
	FString FormatCode(const FString& Input) const;

	/** Show error message */
	void ShowError(const FString& Message);

	/** Clear error message */
	void ClearError();

	/** Set loading state */
	void SetLoading(bool bLoading);

	/** Update join button state */
	void UpdateJoinButton();

	// =========================================================================
	// UI Callbacks
	// =========================================================================

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	UFUNCTION()
	void OnLookupButtonClicked();

	UFUNCTION()
	void OnJoinButtonClicked();

	UFUNCTION()
	void OnCodeInputChanged(const FText& Text);

	UFUNCTION()
	void OnCodeInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};