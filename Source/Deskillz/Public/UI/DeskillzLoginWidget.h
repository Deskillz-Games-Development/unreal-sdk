// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeskillzAuthTypes.h"
#include "DeskillzLoginWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class UButton;
class UCheckBox;
class UImage;

/**
 * Login Widget for Deskillz SDK
 * 
 * Provides email/password login UI with social login options.
 * Can be used directly in Blueprints or extended in C++.
 */
UCLASS(BlueprintType, Blueprintable)
class DESKILLZ_API UDeskillzLoginWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UDeskillzLoginWidget(const FObjectInitializer& ObjectInitializer);

    // ========================================================================
    // Blueprint Events
    // ========================================================================
    
    /** Called when login succeeds */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Login|Events")
    FOnDeskillzLoginSuccess OnLoginSuccess;
    
    /** Called when an error occurs */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Login|Events")
    FOnDeskillzAuthError OnLoginError;
    
    /** Called when user requests to switch to sign up */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowSignUp);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Login|Events")
    FOnShowSignUp OnShowSignUp;
    
    /** Called when user requests password reset */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowForgotPassword);
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Login|Events")
    FOnShowForgotPassword OnShowForgotPassword;

    // ========================================================================
    // Widget Bindings (for Blueprint/C++ binding)
    // ========================================================================
    
    /** Email input field */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|Login")
    UEditableTextBox* EmailInput;
    
    /** Password input field */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|Login")
    UEditableTextBox* PasswordInput;
    
    /** Login button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Deskillz|Login")
    UButton* LoginButton;
    
    /** Remember me checkbox */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UCheckBox* RememberMeCheckBox;
    
    /** Forgot password button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UButton* ForgotPasswordButton;
    
    /** Sign up button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UButton* SignUpButton;
    
    /** Google login button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UButton* GoogleButton;
    
    /** Apple login button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UButton* AppleButton;
    
    /** Facebook login button */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UButton* FacebookButton;
    
    /** Error message text */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UTextBlock* ErrorText;
    
    /** Loading indicator image */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Deskillz|Login")
    UImage* LoadingIndicator;

    // ========================================================================
    // Public Methods
    // ========================================================================
    
    /** Pre-fill the email field */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Login")
    void SetEmail(const FString& Email);
    
    /** Clear all input fields */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Login")
    void ClearInputs();
    
    /** Show error message */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Login")
    void ShowError(const FString& Message);
    
    /** Clear error message */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Login")
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
    void OnLoginButtonClicked();
    
    UFUNCTION()
    void OnForgotPasswordButtonClicked();
    
    UFUNCTION()
    void OnSignUpButtonClicked();
    
    UFUNCTION()
    void OnGoogleButtonClicked();
    
    UFUNCTION()
    void OnAppleButtonClicked();
    
    UFUNCTION()
    void OnFacebookButtonClicked();

    // ========================================================================
    // Auth Event Handlers
    // ========================================================================
    
    UFUNCTION()
    void HandleLoginSuccess(const FDeskillzAuthUser& User);
    
    UFUNCTION()
    void HandleAuthError(const FString& ErrorMessage);
    
    UFUNCTION()
    void HandleAuthStateChanged(EDeskillzAuthState NewState, const FDeskillzAuthUser& User);

    // ========================================================================
    // UI Helpers
    // ========================================================================
    
    /** Set loading state */
    void SetLoading(bool bIsLoading);
    
    /** Get user-friendly error message */
    FString GetFriendlyErrorMessage(const FString& TechnicalMessage) const;
    
    /** Validate email format */
    bool ValidateEmail(const FString& Email) const;

private:
    /** Whether currently loading */
    bool bIsLoading = false;
};