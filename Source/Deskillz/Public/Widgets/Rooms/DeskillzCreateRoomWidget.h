// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rooms/DeskillzRoomTypes.h"
#include "DeskillzCreateRoomWidget.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;
class UMultiLineEditableTextBox;
class UComboBoxString;
class USlider;
class UCheckBox;
class UThrobber;
class UCanvasPanel;

/** Delegate for room created */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCreatedDelegate, const FPrivateRoom&, Room);

/** Delegate for navigation */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateRoomNavigationDelegate);

/**
 * Widget for creating a new private room.
 * Allows configuration of room name, entry fee, player count, etc.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzCreateRoomWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDeskillzCreateRoomWidget(const FObjectInitializer& ObjectInitializer);

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
	 * Hide the widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Rooms|UI")
	void Hide();

	/**
	 * Reset the form to default values
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

	/** Called when room is successfully created */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnRoomCreatedDelegate OnRoomCreated;

	/** Called when back button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Rooms|UI|Events")
	FOnCreateRoomNavigationDelegate OnBackClicked;

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
	// UI Components - Form Fields
	// =========================================================================

	/** Room name input */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UEditableTextBox* NameInput;

	/** Room description input */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UMultiLineEditableTextBox* DescriptionInput;

	/** Entry fee input */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UEditableTextBox* EntryFeeInput;

	/** Currency dropdown */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UComboBoxString* CurrencyDropdown;

	/** Min players slider */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	USlider* MinPlayersSlider;

	/** Min players value text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* MinPlayersText;

	/** Max players slider */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	USlider* MaxPlayersSlider;

	/** Max players value text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* MaxPlayersText;

	/** Visibility dropdown */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UComboBoxString* VisibilityDropdown;

	/** Game mode dropdown */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UComboBoxString* ModeDropdown;

	/** Invite required checkbox */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UCheckBox* InviteRequiredCheckbox;

	// =========================================================================
	// UI Components - Actions
	// =========================================================================

	/** Error text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UTextBlock* ErrorText;

	/** Cancel button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* CancelButton;

	/** Create button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UButton* CreateButton;

	/** Loading overlay */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UCanvasPanel* LoadingOverlay;

	/** Loading indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "UI")
	UThrobber* LoadingIndicator;

	// =========================================================================
	// Settings
	// =========================================================================

	/** Available currencies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	TArray<FString> AvailableCurrencies;

	/** Default currency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	FString DefaultCurrency;

	/** Min players limit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	int32 MinPlayersLimit;

	/** Max players limit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|Rooms|UI|Settings")
	int32 MaxPlayersLimit;

	// =========================================================================
	// State
	// =========================================================================

	/** Whether currently creating */
	bool bIsCreating;

	/** Whether widget is visible */
	bool bIsVisible;

	// =========================================================================
	// Internal Methods
	// =========================================================================

	/** Initialize UI bindings */
	void InitializeUI();

	/** Populate dropdown options */
	void PopulateDropdowns();

	/** Validate form input */
	bool ValidateForm();

	/** Get visibility from dropdown */
	ERoomVisibility GetSelectedVisibility() const;

	/** Get mode from dropdown */
	ERoomMode GetSelectedMode() const;

	/** Build room config from form */
	FCreateRoomConfig BuildRoomConfig() const;

	/** Show error message */
	void ShowError(const FString& Message);

	/** Clear error message */
	void ClearError();

	/** Set loading state */
	void SetLoading(bool bLoading);

	// =========================================================================
	// UI Callbacks
	// =========================================================================

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	UFUNCTION()
	void OnCreateButtonClicked();

	UFUNCTION()
	void OnMinPlayersSliderChanged(float Value);

	UFUNCTION()
	void OnMaxPlayersSliderChanged(float Value);

	UFUNCTION()
	void OnNameInputChanged(const FText& Text);
};