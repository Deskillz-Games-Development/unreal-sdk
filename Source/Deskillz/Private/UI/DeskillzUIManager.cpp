// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzUIManager.h"
#include "UI/DeskillzTournamentListWidget.h"
#include "UI/DeskillzMatchmakingWidget.h"
#include "UI/DeskillzResultsWidget.h"
#include "UI/DeskillzWalletWidget.h"
#include "UI/DeskillzHUDWidget.h"
#include "UI/DeskillzPopupWidget.h"
#include "UI/DeskillzLeaderboardWidget.h"
#include "Core/DeskillzTypes.h"
#include "Deskillz.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/Widget.h"

// Static singleton
static UDeskillzUIManager* GUIManager = nullptr;

UDeskillzUIManager::UDeskillzUIManager()
{
	// Default theme (gaming/neon style)
	Theme = FDeskillzUITheme();
}

UDeskillzUIManager* UDeskillzUIManager::Get(const UObject* WorldContextObject)
{
	if (!GUIManager)
	{
		GUIManager = NewObject<UDeskillzUIManager>();
		GUIManager->AddToRoot(); // Prevent garbage collection
	}
	return GUIManager;
}

// ============================================================================
// Tournament List
// ============================================================================

void UDeskillzUIManager::ShowTournamentList()
{
	if (!bUseBuiltInWidgets)
	{
		BroadcastUIEvent(TEXT("ShowTournamentList"));
		return;
	}
	
	if (!TournamentListWidget)
	{
		TournamentListWidget = CreateWidget<UDeskillzTournamentListWidget>(
			TournamentListWidgetClass ? TournamentListWidgetClass : UDeskillzTournamentListWidget::StaticClass()
		);
	}
	
	if (TournamentListWidget)
	{
		ShowWidget(TournamentListWidget, EDeskillzUILayer::Content);
		TournamentListWidget->RefreshTournaments();
	}
	
	BroadcastUIEvent(TEXT("TournamentListShown"));
}

void UDeskillzUIManager::HideTournamentList()
{
	if (TournamentListWidget)
	{
		HideWidget(TournamentListWidget);
	}
	BroadcastUIEvent(TEXT("TournamentListHidden"));
}

bool UDeskillzUIManager::IsTournamentListVisible() const
{
	return TournamentListWidget && TournamentListWidget->IsInViewport();
}

// ============================================================================
// Matchmaking
// ============================================================================

void UDeskillzUIManager::ShowMatchmaking(const FString& TournamentId)
{
	if (!bUseBuiltInWidgets)
	{
		BroadcastUIEvent(TEXT("ShowMatchmaking"));
		return;
	}
	
	// Hide tournament list if visible
	HideTournamentList();
	
	if (!MatchmakingWidget)
	{
		MatchmakingWidget = CreateWidget<UDeskillzMatchmakingWidget>(
			MatchmakingWidgetClass ? MatchmakingWidgetClass : UDeskillzMatchmakingWidget::StaticClass()
		);
	}
	
	if (MatchmakingWidget)
	{
		MatchmakingWidget->SetTournamentId(TournamentId);
		ShowWidget(MatchmakingWidget, EDeskillzUILayer::Content);
		MatchmakingWidget->StartMatchmaking();
	}
	
	BroadcastUIEvent(TEXT("MatchmakingStarted"));
}

void UDeskillzUIManager::HideMatchmaking()
{
	if (MatchmakingWidget)
	{
		MatchmakingWidget->CancelMatchmaking();
		HideWidget(MatchmakingWidget);
	}
	BroadcastUIEvent(TEXT("MatchmakingHidden"));
}

void UDeskillzUIManager::UpdateMatchmakingStatus(const FString& Status, float Progress)
{
	if (MatchmakingWidget)
	{
		MatchmakingWidget->UpdateStatus(Status, Progress);
	}
}

// ============================================================================
// Results
// ============================================================================

void UDeskillzUIManager::ShowResults(const FDeskillzMatchResult& Result)
{
	if (!bUseBuiltInWidgets)
	{
		BroadcastUIEvent(TEXT("ShowResults"));
		return;
	}
	
	// Hide HUD
	HideHUD();
	
	if (!ResultsWidget)
	{
		ResultsWidget = CreateWidget<UDeskillzResultsWidget>(
			ResultsWidgetClass ? ResultsWidgetClass : UDeskillzResultsWidget::StaticClass()
		);
	}
	
	if (ResultsWidget)
	{
		ResultsWidget->SetMatchResult(Result);
		ShowWidget(ResultsWidget, EDeskillzUILayer::Content);
		ResultsWidget->PlayResultAnimation();
	}
	
	BroadcastUIEvent(TEXT("ResultsShown"));
}

void UDeskillzUIManager::HideResults()
{
	if (ResultsWidget)
	{
		HideWidget(ResultsWidget);
	}
	BroadcastUIEvent(TEXT("ResultsHidden"));
}

// ============================================================================
// Wallet
// ============================================================================

void UDeskillzUIManager::ShowWallet()
{
	if (!bUseBuiltInWidgets)
	{
		BroadcastUIEvent(TEXT("ShowWallet"));
		return;
	}
	
	if (!WalletWidget)
	{
		WalletWidget = CreateWidget<UDeskillzWalletWidget>(
			WalletWidgetClass ? WalletWidgetClass : UDeskillzWalletWidget::StaticClass()
		);
	}
	
	if (WalletWidget)
	{
		ShowWidget(WalletWidget, EDeskillzUILayer::Content);
		WalletWidget->RefreshBalances();
	}
	
	BroadcastUIEvent(TEXT("WalletShown"));
}

void UDeskillzUIManager::HideWallet()
{
	if (WalletWidget)
	{
		HideWidget(WalletWidget);
	}
	BroadcastUIEvent(TEXT("WalletHidden"));
}

void UDeskillzUIManager::RefreshWalletBalance()
{
	if (WalletWidget && WalletWidget->IsInViewport())
	{
		WalletWidget->RefreshBalances();
	}
}

// ============================================================================
// HUD
// ============================================================================

void UDeskillzUIManager::ShowHUD()
{
	if (!bUseBuiltInWidgets)
	{
		BroadcastUIEvent(TEXT("ShowHUD"));
		return;
	}
	
	if (!HUDWidget)
	{
		HUDWidget = CreateWidget<UDeskillzHUDWidget>(
			HUDWidgetClass ? HUDWidgetClass : UDeskillzHUDWidget::StaticClass()
		);
	}
	
	if (HUDWidget)
	{
		ShowWidget(HUDWidget, EDeskillzUILayer::Overlay);
	}
	
	BroadcastUIEvent(TEXT("HUDShown"));
}

void UDeskillzUIManager::HideHUD()
{
	if (HUDWidget)
	{
		HideWidget(HUDWidget);
	}
	BroadcastUIEvent(TEXT("HUDHidden"));
}

void UDeskillzUIManager::UpdateHUDScore(int64 Score)
{
	if (HUDWidget)
	{
		HUDWidget->SetScore(Score);
	}
}

void UDeskillzUIManager::UpdateHUDTimer(float RemainingSeconds)
{
	if (HUDWidget)
	{
		HUDWidget->SetTimer(RemainingSeconds);
	}
}

// ============================================================================
// Leaderboard
// ============================================================================

void UDeskillzUIManager::ShowLeaderboard(const FString& TournamentId)
{
	if (!bUseBuiltInWidgets)
	{
		BroadcastUIEvent(TEXT("ShowLeaderboard"));
		return;
	}
	
	if (!LeaderboardWidget)
	{
		LeaderboardWidget = CreateWidget<UDeskillzLeaderboardWidget>(UDeskillzLeaderboardWidget::StaticClass());
	}
	
	if (LeaderboardWidget)
	{
		LeaderboardWidget->SetTournamentId(TournamentId);
		ShowWidget(LeaderboardWidget, EDeskillzUILayer::Content);
		LeaderboardWidget->RefreshLeaderboard();
	}
	
	BroadcastUIEvent(TEXT("LeaderboardShown"));
}

void UDeskillzUIManager::HideLeaderboard()
{
	if (LeaderboardWidget)
	{
		HideWidget(LeaderboardWidget);
	}
	BroadcastUIEvent(TEXT("LeaderboardHidden"));
}

// ============================================================================
// Popups & Toasts
// ============================================================================

void UDeskillzUIManager::ShowPopup(const FString& Title, const FString& Message, bool bShowCancel,
	const FString& ConfirmText, const FString& CancelText)
{
	if (!PopupWidget)
	{
		PopupWidget = CreateWidget<UDeskillzPopupWidget>(UDeskillzPopupWidget::StaticClass());
	}
	
	if (PopupWidget)
	{
		PopupWidget->Setup(Title, Message, bShowCancel, ConfirmText, CancelText);
		ShowWidget(PopupWidget, EDeskillzUILayer::Popup);
	}
}

void UDeskillzUIManager::ShowToast(const FString& Message, float Duration, bool bIsError)
{
	// Create a temporary toast widget
	// In a full implementation, this would use a pooled toast system
	UE_LOG(LogDeskillz, Log, TEXT("Toast: %s"), *Message);
	
	if (PopupWidget)
	{
		PopupWidget->ShowToast(Message, Duration, bIsError);
	}
}

void UDeskillzUIManager::ShowLoading(const FString& Message)
{
	// Simple loading indicator
	// In full implementation, this would be a proper loading widget
	UE_LOG(LogDeskillz, Log, TEXT("Loading: %s"), *Message);
	BroadcastUIEvent(TEXT("LoadingShown"));
}

void UDeskillzUIManager::HideLoading()
{
	BroadcastUIEvent(TEXT("LoadingHidden"));
}

// ============================================================================
// General
// ============================================================================

void UDeskillzUIManager::HideAllUI()
{
	HideTournamentList();
	HideMatchmaking();
	HideResults();
	HideWallet();
	HideHUD();
	HideLeaderboard();
	HideLoading();
	
	if (PopupWidget && PopupWidget->IsInViewport())
	{
		PopupWidget->RemoveFromParent();
	}
	
	ActiveWidget = nullptr;
	
	// Restore game input
	SetUIInputMode(false);
	
	BroadcastUIEvent(TEXT("AllUIHidden"));
}

UUserWidget* UDeskillzUIManager::GetActiveWidget() const
{
	return ActiveWidget;
}

bool UDeskillzUIManager::IsAnyUIVisible() const
{
	return (TournamentListWidget && TournamentListWidget->IsInViewport()) ||
		   (MatchmakingWidget && MatchmakingWidget->IsInViewport()) ||
		   (ResultsWidget && ResultsWidget->IsInViewport()) ||
		   (WalletWidget && WalletWidget->IsInViewport()) ||
		   (LeaderboardWidget && LeaderboardWidget->IsInViewport()) ||
		   (PopupWidget && PopupWidget->IsInViewport());
}

void UDeskillzUIManager::SetUIInputMode(bool bUIOnly)
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}
	
	if (bUIOnly)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

// ============================================================================
// Internal Methods
// ============================================================================

template<typename T>
T* UDeskillzUIManager::CreateWidget(TSubclassOf<T> WidgetClass)
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogDeskillz, Error, TEXT("Cannot create widget: No player controller"));
		return nullptr;
	}
	
	if (!WidgetClass)
	{
		WidgetClass = T::StaticClass();
	}
	
	T* Widget = ::CreateWidget<T>(PC, WidgetClass);
	
	if (Widget)
	{
		// Apply theme
		if (UDeskillzBaseWidget* BaseWidget = Cast<UDeskillzBaseWidget>(Widget))
		{
			BaseWidget->ApplyTheme(Theme);
		}
	}
	
	return Widget;
}

void UDeskillzUIManager::ShowWidget(UUserWidget* Widget, EDeskillzUILayer Layer)
{
	if (!Widget)
	{
		return;
	}
	
	// Determine Z-order based on layer
	int32 ZOrder = 0;
	switch (Layer)
	{
		case EDeskillzUILayer::Background: ZOrder = 0; break;
		case EDeskillzUILayer::Content: ZOrder = 100; break;
		case EDeskillzUILayer::Overlay: ZOrder = 200; break;
		case EDeskillzUILayer::Popup: ZOrder = 300; break;
		case EDeskillzUILayer::Toast: ZOrder = 400; break;
	}
	
	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(ZOrder);
	}
	
	// Track active widget for content layer
	if (Layer == EDeskillzUILayer::Content)
	{
		if (ActiveWidget && ActiveWidget != Widget)
		{
			HideWidget(ActiveWidget);
		}
		ActiveWidget = Widget;
		SetUIInputMode(true);
	}
	
	// Play show animation if supported
	if (bEnableAnimations)
	{
		if (UDeskillzBaseWidget* BaseWidget = Cast<UDeskillzBaseWidget>(Widget))
		{
			BaseWidget->PlayShowAnimation();
		}
	}
}

void UDeskillzUIManager::HideWidget(UUserWidget* Widget)
{
	if (!Widget || !Widget->IsInViewport())
	{
		return;
	}
	
	// Play hide animation if supported
	if (bEnableAnimations)
	{
		if (UDeskillzBaseWidget* BaseWidget = Cast<UDeskillzBaseWidget>(Widget))
		{
			BaseWidget->PlayHideAnimation();
			// In full implementation, remove after animation completes
		}
	}
	
	Widget->RemoveFromParent();
	
	if (Widget == ActiveWidget)
	{
		ActiveWidget = nullptr;
		
		// Restore game input if no other UI visible
		if (!IsAnyUIVisible())
		{
			SetUIInputMode(false);
		}
	}
}

APlayerController* UDeskillzUIManager::GetPlayerController() const
{
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			return World->GetFirstPlayerController();
		}
	}
	return nullptr;
}

void UDeskillzUIManager::BroadcastUIEvent(const FString& EventName)
{
	OnUIEvent.Broadcast(EventName);
	UE_LOG(LogDeskillz, Verbose, TEXT("UI Event: %s"), *EventName);
}
