// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/DeskillzBaseWidget.h"
#include "DeskillzPopupWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UBorder;
class UVerticalBox;
class UOverlay;

/** Delegate for popup actions */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupConfirm);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupCancel);

/**
 * Toast notification data
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzToast
{
	GENERATED_BODY()
	
	/** Toast message */
	UPROPERTY(BlueprintReadWrite, Category = "Toast")
	FString Message;
	
	/** Display duration */
	UPROPERTY(BlueprintReadWrite, Category = "Toast")
	float Duration = 3.0f;
	
	/** Is error toast */
	UPROPERTY(BlueprintReadWrite, Category = "Toast")
	bool bIsError = false;
	
	/** Remaining time */
	float RemainingTime = 0.0f;
};

/**
 * Popup Widget
 * 
 * Handles modal dialogs and toast notifications:
 * - Confirm/Cancel dialogs
 * - Info popups
 * - Toast notifications
 * - Loading indicators
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzPopupWidget : public UDeskillzBaseWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzPopupWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Public Methods
	// ========================================================================
	
	/**
	 * Setup and show popup
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void Setup(const FString& Title, const FString& Message, bool bShowCancel = false,
		const FString& ConfirmText = TEXT("OK"), const FString& CancelText = TEXT("Cancel"));
	
	/**
	 * Show a toast notification
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowToast(const FString& Message, float Duration = 3.0f, bool bIsError = false);
	
	/**
	 * Clear all toasts
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ClearToasts();
	
	/**
	 * Show loading overlay
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void ShowLoading(const FString& Message);
	
	/**
	 * Hide loading overlay
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void HideLoading();
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when confirm is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnPopupConfirm OnConfirm;
	
	/** Called when cancel is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|UI")
	FOnPopupCancel OnCancel;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme) override;
	virtual bool HandleBackButton_Implementation() override;
	
	// ========================================================================
	// UI Bindings - Popup
	// ========================================================================
	
	/** Popup container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* PopupContainer;
	
	/** Background overlay (dims screen) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* BackgroundOverlay;
	
	/** Title text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* TitleText;
	
	/** Message text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* MessageText;
	
	/** Confirm button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* ConfirmButton;
	
	/** Confirm button text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* ConfirmButtonText;
	
	/** Cancel button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UButton* CancelButton;
	
	/** Cancel button text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* CancelButtonText;
	
	/** Icon image */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UImage* IconImage;
	
	// ========================================================================
	// UI Bindings - Toast
	// ========================================================================
	
	/** Toast container */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UVerticalBox* ToastContainer;
	
	// ========================================================================
	// UI Bindings - Loading
	// ========================================================================
	
	/** Loading overlay */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UWidget* LoadingOverlay;
	
	/** Loading message text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|UI")
	UTextBlock* LoadingText;
	
	// ========================================================================
	// Configuration
	// ========================================================================
	
	/** Max toasts to show */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	int32 MaxToasts = 3;
	
	/** Toast widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	TSubclassOf<UUserWidget> ToastWidgetClass;
	
	// ========================================================================
	// State
	// ========================================================================
	
	/** Active toasts */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	TArray<FDeskillzToast> ActiveToasts;
	
	/** Is popup showing */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bIsPopupVisible = false;
	
	/** Is loading showing */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bIsLoadingVisible = false;
	
	/** Has cancel button */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	bool bHasCancelButton = false;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Show popup UI */
	void ShowPopupUI();
	
	/** Hide popup UI */
	void HidePopupUI();
	
	/** Update toast display */
	void UpdateToasts();
	
	/** Create toast widget */
	UUserWidget* CreateToastWidget(const FDeskillzToast& Toast);
	
	/** Handle confirm clicked */
	UFUNCTION()
	void OnConfirmClicked();
	
	/** Handle cancel clicked */
	UFUNCTION()
	void OnCancelClicked();
	
	/** Handle background clicked */
	UFUNCTION()
	void OnBackgroundClicked();
};
