// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeskillzAuthTypes.h"
#include "DeskillzSignUpWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class UButton;
class UCheckBox;
class UImage;
class UProgressBar;

/**
 * Sign Up Widget for Deskillz SDK
 * 
 * Provides email/password registration UI with social sign up options.
 * Can be used directly in Blueprints or extended in C++.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzSignUpWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UDeskillzSignUpWidget(const FObjectInitializer& ObjectInitializer);

    // ========================================================================
    // Blueprint Events
    // ========================================================================
    
    /** Called when sign up succeeds */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|SignUp|Events")
    FOnDeskillzSignUpSuccess OnSignUpSuccess;
    
    /** Called when an error occurs */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|SignUp|Events")
    FOnDeskillzAuthError OnSignUpError;
    
    /** Called when user requests to switch to login */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowLogin);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|SignUp|Events")
    FOnShowLogin OnShowLogin;

    // ========================================================================
    // Widget Bindings
    // ========================================================================
    
    /** Username input field */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|SignUp")
    UEditableTextBox* UsernameInput;
    
    /** Email input field */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|SignUp")
    UEditableTextBox* EmailInput;
    
    /** Password input field */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|SignUp")
    UEditableTextBox* PasswordInput;
    
    /** Confirm password input field */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|SignUp")
    UEditableTextBox* ConfirmPasswordInput;
    
    /** Sign up button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|SignUp")
    UButton* SignUpButton;
    
    /** Terms agreement checkbox */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UCheckBox* TermsCheckBox;
    
    /** Login button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UButton* LoginButton;
    
    /** Google sign up button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UButton* GoogleButton;
    
    /** Apple sign up button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UButton* AppleButton;
    
    /** Facebook sign up button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UButton* FacebookButton;
    
    /** Error message text */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UTextBlock* ErrorText;
    
    /** Loading indicator */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UImage* LoadingIndicator;
    
    /** Password strength progress bar */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UProgressBar* PasswordStrengthBar;
    
    /** Password strength text */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|SignUp")
    UTextBlock* PasswordStrengthText;

    // ========================================================================
    // Configuration
    // ========================================================================
    
    /** URL for Terms of Service */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|SignUp|Config")
    FString TermsUrl = TEXT("https://deskillz.games/terms");
    
    /** URL for Privacy Policy */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deskillz|SignUp|Config")
    FString PrivacyUrl = TEXT("https://deskillz.games/privacy");

    // ========================================================================
    // Public Methods
    // ========================================================================
    
    /** Clear all input fields */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|SignUp")
    void ClearInputs();
    
    /** Show error message */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|SignUp")
    void ShowError(const FString& Message);
    
    /** Clear error message */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|SignUp")
    void ClearError();

protected:
    // ========================================================================
    // UUserWidget Interface
    // ========================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ========================================================================
    // Button Handlers
    // ========================================================================
    
    UFUNCTION()
    void OnSignUpButtonClicked();
    
    UFUNCTION()
    void OnLoginButtonClicked();
    
    UFUNCTION()
    void OnGoogleButtonClicked();
    
    UFUNCTION()
    void OnAppleButtonClicked();
    
    UFUNCTION()
    void OnFacebookButtonClicked();
    
    UFUNCTION()
    void OnTermsCheckBoxChanged(bool bIsChecked);
    
    UFUNCTION()
    void OnPasswordChanged(const FText& Text);

    // ========================================================================
    // Auth Event Handlers
    // ========================================================================
    
    UFUNCTION()
    void HandleSignUpSuccess(const FDeskillzAuthUser& User);
    
    UFUNCTION()
    void HandleAuthError(const FString& ErrorMessage);
    
    UFUNCTION()
    void HandleAuthStateChanged(EDeskillzAuthState NewState, const FDeskillzAuthUser& User);

    // ========================================================================
    // Validation
    // ========================================================================
    
    /** Validate all inputs */
    bool ValidateInputs(FString& OutError) const;
    
    /** Validate username */
    bool ValidateUsername(const FString& Username, FString& OutError) const;
    
    /** Validate email */
    bool ValidateEmail(const FString& Email, FString& OutError) const;
    
    /** Validate password */
    bool ValidatePassword(const FString& Password, FString& OutError) const;
    
    /** Calculate password strength (0-4) */
    int32 CalculatePasswordStrength(const FString& Password) const;
    
    /** Get password strength label */
    FString GetPasswordStrengthLabel(int32 Strength) const;
    
    /** Get password strength color */
    FLinearColor GetPasswordStrengthColor(int32 Strength) const;

    // ========================================================================
    // UI Helpers
    // ========================================================================
    
    /** Set loading state */
    void SetLoading(bool bIsLoading);
    
    /** Update sign up button enabled state */
    void UpdateSignUpButtonState();
    
    /** Update password strength indicator */
    void UpdatePasswordStrength(const FString& Password);
    
    /** Get user-friendly error message */
    FString GetFriendlyErrorMessage(const FString& TechnicalMessage) const;

private:
    /** Whether currently loading */
    bool bIsLoading = false;
};