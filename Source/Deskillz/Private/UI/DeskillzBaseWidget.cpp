// Copyright Deskillz Games. All Rights Reserved.

#include "UI/DeskillzBaseWidget.h"
#include "Deskillz.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"

UDeskillzBaseWidget::UDeskillzBaseWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default theme
	CurrentTheme = FDeskillzUITheme();
}

void UDeskillzBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Get UI Manager reference
	UIManager = UDeskillzUIManager::Get(this);
	
	// Apply current theme
	if (UIManager)
	{
		ApplyTheme(UIManager->Theme);
	}
}

FReply UDeskillzBaseWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle escape/back
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Android_Back)
	{
		if (HandleBackButton())
		{
			return FReply::Handled();
		}
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ============================================================================
// Theme
// ============================================================================

void UDeskillzBaseWidget::ApplyTheme_Implementation(const FDeskillzUITheme& Theme)
{
	CurrentTheme = Theme;
	
	// Base implementation - override in child classes to apply to specific elements
	UE_LOG(LogDeskillz, Verbose, TEXT("Applied theme to widget: %s"), *GetName());
}

// ============================================================================
// Animations
// ============================================================================

void UDeskillzBaseWidget::PlayShowAnimation_Implementation()
{
	// Try to play named animation
	UWidgetAnimation* ShowAnim = nullptr;
	
	// Find animation by name
	for (UWidgetAnimation* Anim : GetAnimations())
	{
		if (Anim && Anim->GetFName() == ShowAnimationName)
		{
			ShowAnim = Anim;
			break;
		}
	}
	
	if (ShowAnim)
	{
		PlayAnimation(ShowAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, CurrentTheme.AnimationSpeed);
	}
	else
	{
		// Fallback: simple fade in
		SetRenderOpacity(0.0f);
		
		// Animate opacity (in full implementation, use timer or animation)
		SetRenderOpacity(1.0f);
	}
	
	OnShowAnimationComplete();
}

void UDeskillzBaseWidget::PlayHideAnimation_Implementation()
{
	// Try to play named animation
	UWidgetAnimation* HideAnim = nullptr;
	
	for (UWidgetAnimation* Anim : GetAnimations())
	{
		if (Anim && Anim->GetFName() == HideAnimationName)
		{
			HideAnim = Anim;
			break;
		}
	}
	
	if (HideAnim)
	{
		PlayAnimation(HideAnim, 0.0f, 1, EUMGSequencePlayMode::Forward, CurrentTheme.AnimationSpeed);
	}
	else
	{
		// Fallback: simple fade out
		SetRenderOpacity(0.0f);
	}
	
	OnHideAnimationComplete();
}

void UDeskillzBaseWidget::OnShowAnimationComplete_Implementation()
{
	// Override in child classes
}

void UDeskillzBaseWidget::OnHideAnimationComplete_Implementation()
{
	// Override in child classes
}

// ============================================================================
// Navigation
// ============================================================================

bool UDeskillzBaseWidget::HandleBackButton_Implementation()
{
	Close();
	return true;
}

void UDeskillzBaseWidget::Close()
{
	PlayHideAnimation();
	RemoveFromParent();
}

// ============================================================================
// Sound
// ============================================================================

void UDeskillzBaseWidget::PlayClickSound()
{
	if (!UIManager || !UIManager->bEnableSounds)
	{
		return;
	}
	
	// In full implementation, play sound from config
	// UGameplayStatics::PlaySound2D(this, ClickSound);
}

void UDeskillzBaseWidget::PlaySuccessSound()
{
	if (!UIManager || !UIManager->bEnableSounds)
	{
		return;
	}
	
	// UGameplayStatics::PlaySound2D(this, SuccessSound);
}

void UDeskillzBaseWidget::PlayErrorSound()
{
	if (!UIManager || !UIManager->bEnableSounds)
	{
		return;
	}
	
	// UGameplayStatics::PlaySound2D(this, ErrorSound);
}

// ============================================================================
// Utility
// ============================================================================

FString UDeskillzBaseWidget::FormatNumber(int64 Number)
{
	FString NumStr = FString::Printf(TEXT("%lld"), FMath::Abs(Number));
	FString Result;
	
	int32 Count = 0;
	for (int32 i = NumStr.Len() - 1; i >= 0; i--)
	{
		if (Count > 0 && Count % 3 == 0)
		{
			Result = TEXT(",") + Result;
		}
		Result = NumStr[i] + Result;
		Count++;
	}
	
	if (Number < 0)
	{
		Result = TEXT("-") + Result;
	}
	
	return Result;
}

FString UDeskillzBaseWidget::FormatCurrency(double Amount, const FString& Symbol, int32 Decimals)
{
	// Format with specified decimals
	FString Format = FString::Printf(TEXT("%%.%df"), Decimals);
	FString AmountStr = FString::Printf(*Format, Amount);
	
	// Remove trailing zeros
	while (AmountStr.EndsWith(TEXT("0")) && !AmountStr.EndsWith(TEXT(".0")))
	{
		AmountStr = AmountStr.LeftChop(1);
	}
	
	return AmountStr + TEXT(" ") + Symbol;
}

FString UDeskillzBaseWidget::FormatTime(float Seconds)
{
	int32 TotalSeconds = FMath::FloorToInt(Seconds);
	int32 Hours = TotalSeconds / 3600;
	int32 Minutes = (TotalSeconds % 3600) / 60;
	int32 Secs = TotalSeconds % 60;
	
	if (Hours > 0)
	{
		return FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Secs);
	}
	else
	{
		return FString::Printf(TEXT("%d:%02d"), Minutes, Secs);
	}
}

FString UDeskillzBaseWidget::FormatRelativeTime(int64 TimestampMs)
{
	int64 NowMs = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	int64 DiffMs = NowMs - TimestampMs;
	
	bool bPast = DiffMs >= 0;
	DiffMs = FMath::Abs(DiffMs);
	
	int64 Seconds = DiffMs / 1000;
	int64 Minutes = Seconds / 60;
	int64 Hours = Minutes / 60;
	int64 Days = Hours / 24;
	
	FString TimeStr;
	
	if (Days > 0)
	{
		TimeStr = FString::Printf(TEXT("%lld day%s"), Days, Days == 1 ? TEXT("") : TEXT("s"));
	}
	else if (Hours > 0)
	{
		TimeStr = FString::Printf(TEXT("%lld hour%s"), Hours, Hours == 1 ? TEXT("") : TEXT("s"));
	}
	else if (Minutes > 0)
	{
		TimeStr = FString::Printf(TEXT("%lld minute%s"), Minutes, Minutes == 1 ? TEXT("") : TEXT("s"));
	}
	else
	{
		TimeStr = TEXT("just now");
		return TimeStr;
	}
	
	return bPast ? (TimeStr + TEXT(" ago")) : (TEXT("in ") + TimeStr);
}
