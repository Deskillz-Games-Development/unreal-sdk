// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzPopupWidget.h"
#include "Deskillz.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/Overlay.h"

UDeskillzPopupWidget::UDeskillzPopupWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDeskillzPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Setup buttons
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UDeskillzPopupWidget::OnConfirmClicked);
	}
	
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UDeskillzPopupWidget::OnCancelClicked);
	}
	
	// Initially hide popup and loading
	if (PopupContainer)
	{
		PopupContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (BackgroundOverlay)
	{
		BackgroundOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (LoadingOverlay)
	{
		LoadingOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDeskillzPopupWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Update toast timers
	bool bNeedsUpdate = false;
	
	for (int32 i = ActiveToasts.Num() - 1; i >= 0; i--)
	{
		ActiveToasts[i].RemainingTime -= InDeltaTime;
		
		if (ActiveToasts[i].RemainingTime <= 0.0f)
		{
			ActiveToasts.RemoveAt(i);
			bNeedsUpdate = true;
		}
	}
	
	if (bNeedsUpdate)
	{
		UpdateToasts();
	}
}

void UDeskillzPopupWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	Super::ApplyTheme_Implementation(Theme);
	
	if (TitleText)
	{
		TitleText->SetColorAndOpacity(Theme.TextColor);
	}
	
	if (MessageText)
	{
		MessageText->SetColorAndOpacity(Theme.MutedTextColor);
	}
	
	if (LoadingText)
	{
		LoadingText->SetColorAndOpacity(Theme.TextColor);
	}
}

bool UDeskillzPopupWidget::HandleBackButton_Implementation()
{
	if (bIsPopupVisible)
	{
		if (bHasCancelButton)
		{
			OnCancelClicked();
		}
		else
		{
			OnConfirmClicked();
		}
		return true;
	}
	
	return false;
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzPopupWidget::Setup(const FString& Title, const FString& Message, bool bShowCancel,
	const FString& ConfirmText, const FString& CancelText)
{
	bHasCancelButton = bShowCancel;
	
	// Set title
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(Title));
	}
	
	// Set message
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(Message));
	}
	
	// Set confirm button text
	if (ConfirmButtonText)
	{
		ConfirmButtonText->SetText(FText::FromString(ConfirmText));
	}
	
	// Set cancel button text and visibility
	if (CancelButton)
	{
		CancelButton->SetVisibility(bShowCancel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	if (CancelButtonText)
	{
		CancelButtonText->SetText(FText::FromString(CancelText));
	}
	
	// Show popup
	ShowPopupUI();
}

void UDeskillzPopupWidget::ShowToast(const FString& Message, float Duration, bool bIsError)
{
	// Remove oldest if at max
	if (ActiveToasts.Num() >= MaxToasts)
	{
		ActiveToasts.RemoveAt(0);
	}
	
	// Add new toast
	FDeskillzToast NewToast;
	NewToast.Message = Message;
	NewToast.Duration = Duration;
	NewToast.bIsError = bIsError;
	NewToast.RemainingTime = Duration;
	
	ActiveToasts.Add(NewToast);
	UpdateToasts();
	
	UE_LOG(LogDeskillz, Log, TEXT("Toast: %s (error: %d)"), *Message, bIsError);
}

void UDeskillzPopupWidget::ClearToasts()
{
	ActiveToasts.Empty();
	UpdateToasts();
}

void UDeskillzPopupWidget::ShowLoading(const FString& Message)
{
	bIsLoadingVisible = true;
	
	if (LoadingText)
	{
		LoadingText->SetText(FText::FromString(Message));
	}
	
	if (LoadingOverlay)
	{
		LoadingOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (BackgroundOverlay)
	{
		BackgroundOverlay->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDeskillzPopupWidget::HideLoading()
{
	bIsLoadingVisible = false;
	
	if (LoadingOverlay)
	{
		LoadingOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Only hide background if popup also not visible
	if (!bIsPopupVisible && BackgroundOverlay)
	{
		BackgroundOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzPopupWidget::ShowPopupUI()
{
	bIsPopupVisible = true;
	
	if (PopupContainer)
	{
		PopupContainer->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (BackgroundOverlay)
	{
		BackgroundOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Play show animation
	PlayShowAnimation();
	
	// Ensure we're in viewport
	if (!IsInViewport())
	{
		AddToViewport(300); // Popup layer
	}
}

void UDeskillzPopupWidget::HidePopupUI()
{
	bIsPopupVisible = false;
	
	if (PopupContainer)
	{
		PopupContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Only hide background if loading also not visible
	if (!bIsLoadingVisible && BackgroundOverlay)
	{
		BackgroundOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Clear delegates
	OnConfirm.Clear();
	OnCancel.Clear();
}

void UDeskillzPopupWidget::UpdateToasts()
{
	if (!ToastContainer)
	{
		return;
	}
	
	// Clear existing
	ToastContainer->ClearChildren();
	
	// Create widgets for active toasts
	for (const FDeskillzToast& Toast : ActiveToasts)
	{
		UUserWidget* ToastWidget = CreateToastWidget(Toast);
		if (ToastWidget)
		{
			ToastContainer->AddChildToVerticalBox(ToastWidget);
		}
	}
}

UUserWidget* UDeskillzPopupWidget::CreateToastWidget(const FDeskillzToast& Toast)
{
	// In full implementation, create from ToastWidgetClass
	// For now, return nullptr (use Blueprint implementation)
	return nullptr;
}

void UDeskillzPopupWidget::OnConfirmClicked()
{
	PlayClickSound();
	OnConfirm.Broadcast();
	HidePopupUI();
}

void UDeskillzPopupWidget::OnCancelClicked()
{
	PlayClickSound();
	OnCancel.Broadcast();
	HidePopupUI();
}

void UDeskillzPopupWidget::OnBackgroundClicked()
{
	// Optional: close on background click
	if (bHasCancelButton)
	{
		OnCancelClicked();
	}
}
