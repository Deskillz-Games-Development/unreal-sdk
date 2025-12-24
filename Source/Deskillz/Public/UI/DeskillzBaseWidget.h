// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/DeskillzUIManager.h"
#include "DeskillzBaseWidget.generated.h"

/**
 * Base class for all Deskillz UI widgets
 * 
 * Provides common functionality:
 * - Theme application
 * - Show/Hide animations
 * - Back button handling
 * - Sound effects
 * 
 * Extend this class for custom Deskillz widgets.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzBaseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UDeskillzBaseWidget(const FObjectInitializer& ObjectInitializer);
	
	// ========================================================================
	// Theme
	// ========================================================================
	
	/**
	 * Apply theme to this widget
	 * Override in child classes to apply theme to specific elements
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void ApplyTheme(const FDeskillzUITheme& Theme);
	virtual void ApplyTheme_Implementation(const FDeskillzUITheme& Theme);
	
	/**
	 * Get current theme
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	FDeskillzUITheme GetTheme() const { return CurrentTheme; }
	
	// ========================================================================
	// Animations
	// ========================================================================
	
	/**
	 * Play widget show animation
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void PlayShowAnimation();
	virtual void PlayShowAnimation_Implementation();
	
	/**
	 * Play widget hide animation
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void PlayHideAnimation();
	virtual void PlayHideAnimation_Implementation();
	
	/**
	 * Called when show animation completes
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void OnShowAnimationComplete();
	virtual void OnShowAnimationComplete_Implementation();
	
	/**
	 * Called when hide animation completes
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	void OnHideAnimationComplete();
	virtual void OnHideAnimationComplete_Implementation();
	
	// ========================================================================
	// Navigation
	// ========================================================================
	
	/**
	 * Handle back button/escape press
	 * @return True if handled
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Deskillz|UI")
	bool HandleBackButton();
	virtual bool HandleBackButton_Implementation();
	
	/**
	 * Close this widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void Close();
	
	// ========================================================================
	// Sound
	// ========================================================================
	
	/**
	 * Play UI click sound
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void PlayClickSound();
	
	/**
	 * Play UI success sound
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void PlaySuccessSound();
	
	/**
	 * Play UI error sound
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|UI")
	void PlayErrorSound();
	
	// ========================================================================
	// Utility
	// ========================================================================
	
	/**
	 * Format a number with commas (e.g., 1000000 -> "1,000,000")
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	static FString FormatNumber(int64 Number);
	
	/**
	 * Format currency amount (e.g., 0.00123456 ETH -> "0.0012 ETH")
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	static FString FormatCurrency(double Amount, const FString& Symbol, int32 Decimals = 4);
	
	/**
	 * Format time as MM:SS or HH:MM:SS
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	static FString FormatTime(float Seconds);
	
	/**
	 * Format relative time (e.g., "2 hours ago", "in 5 minutes")
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|UI")
	static FString FormatRelativeTime(int64 TimestampMs);
	
protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
	/** Current theme */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	FDeskillzUITheme CurrentTheme;
	
	/** Show animation name (for UMG animation player) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FName ShowAnimationName = TEXT("ShowAnim");
	
	/** Hide animation name */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Deskillz|UI")
	FName HideAnimationName = TEXT("HideAnim");
	
	/** Reference to UI Manager */
	UPROPERTY(BlueprintReadOnly, Category = "Deskillz|UI")
	UDeskillzUIManager* UIManager;
};
