// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#include "DeskillzLoginWidget.h"
#include "DeskillzAuth.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/Image.h"

UDeskillzLoginWidget::UDeskillzLoginWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UDeskillzLoginWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Bind button click events
    if (LoginButton)
    {
        LoginButton->OnClicked.AddDynamic(this, &UDeskillzLoginWidget::OnLoginButtonClicked);
    }
    
    if (ForgotPasswordButton)
    {
        ForgotPasswordButton->OnClicked.AddDynamic(this, &UDeskillzLoginWidget::OnForgotPasswordButtonClicked);
    }
    
    if (SignUpButton)
    {
        SignUpButton->OnClicked.AddDynamic(this, &UDeskillzLoginWidget::OnSignUpButtonClicked);
    }
    
    if (GoogleButton)
    {
        GoogleButton->OnClicked.AddDynamic(this, &UDeskillzLoginWidget::OnGoogleButtonClicked);
    }
    
    if (AppleButton)
    {
        AppleButton->OnClicked.AddDynamic(this, &UDeskillzLoginWidget::OnAppleButtonClicked);
    }
    
    if (FacebookButton)
    {
        FacebookButton->OnClicked.AddDynamic(this, &UDeskillzLoginWidget::OnFacebookButtonClicked);
    }
    
    // Subscribe to auth events
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->OnLoginSuccess.AddDynamic(this, &UDeskillzLoginWidget::HandleLoginSuccess);
        Auth->OnAuthError.AddDynamic(this, &UDeskillzLoginWidget::HandleAuthError);
        Auth->OnAuthStateChanged.AddDynamic(this, &UDeskillzLoginWidget::HandleAuthStateChanged);
    }
    
    // Set password input to password mode
    if (PasswordInput)
    {
        PasswordInput->SetIsPassword(true);
    }
    
    // Clear any previous state
    ClearError();
    SetLoading(false);
    
    // Check if already authenticated
    if (Auth && Auth->IsAuthenticated())
    {
        OnLoginSuccess.Broadcast(Auth->GetCurrentUser());
    }
}

void UDeskillzLoginWidget::NativeDestruct()
{
    // Unsubscribe from auth events
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->OnLoginSuccess.RemoveDynamic(this, &UDeskillzLoginWidget::HandleLoginSuccess);
        Auth->OnAuthError.RemoveDynamic(this, &UDeskillzLoginWidget::HandleAuthError);
        Auth->OnAuthStateChanged.RemoveDynamic(this, &UDeskillzLoginWidget::HandleAuthStateChanged);
    }
    
    Super::NativeDestruct();
}

// ============================================================================
// Button Handlers
// ============================================================================

void UDeskillzLoginWidget::OnLoginButtonClicked()
{
    ClearError();
    
    FString Email = EmailInput ? EmailInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
    FString Password = PasswordInput ? PasswordInput->GetText().ToString() : TEXT("");
    bool bRememberMe = RememberMeCheckBox ? RememberMeCheckBox->IsChecked() : false;
    
    // Client-side validation
    if (Email.IsEmpty())
    {
        ShowError(TEXT("Please enter your email address"));
        return;
    }
    
    if (!ValidateEmail(Email))
    {
        ShowError(TEXT("Please enter a valid email address"));
        return;
    }
    
    if (Password.IsEmpty())
    {
        ShowError(TEXT("Please enter your password"));
        return;
    }
    
    SetLoading(true);
    
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->Login(Email, Password, bRememberMe);
    }
}

void UDeskillzLoginWidget::OnForgotPasswordButtonClicked()
{
    OnShowForgotPassword.Broadcast();
}

void UDeskillzLoginWidget::OnSignUpButtonClicked()
{
    OnShowSignUp.Broadcast();
}

void UDeskillzLoginWidget::OnGoogleButtonClicked()
{
    ClearError();
    ShowError(TEXT("Google login coming soon. Use email for now."));
    
    // In a real implementation:
    // 1. Call Google Sign-In SDK
    // 2. Get ID token
    // 3. Call UDeskillzAuth::Get()->SocialLogin(EDeskillzAuthProvider::Google, IdToken);
}

void UDeskillzLoginWidget::OnAppleButtonClicked()
{
    ClearError();
    ShowError(TEXT("Apple login coming soon. Use email for now."));
}

void UDeskillzLoginWidget::OnFacebookButtonClicked()
{
    ClearError();
    ShowError(TEXT("Facebook login coming soon. Use email for now."));
}

// ============================================================================
// Auth Event Handlers
// ============================================================================

void UDeskillzLoginWidget::HandleLoginSuccess(const FDeskillzAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzLoginWidget] Login successful: %s"), *User.Username);
    SetLoading(false);
    OnLoginSuccess.Broadcast(User);
}

void UDeskillzLoginWidget::HandleAuthError(const FString& ErrorMessage)
{
    SetLoading(false);
    ShowError(GetFriendlyErrorMessage(ErrorMessage));
}

void UDeskillzLoginWidget::HandleAuthStateChanged(EDeskillzAuthState NewState, const FDeskillzAuthUser& User)
{
    switch (NewState)
    {
        case EDeskillzAuthState::Authenticating:
            SetLoading(true);
            break;
        case EDeskillzAuthState::Authenticated:
        case EDeskillzAuthState::Error:
        case EDeskillzAuthState::NotAuthenticated:
            SetLoading(false);
            break;
    }
}

// ============================================================================
// Public Methods
// ============================================================================

void UDeskillzLoginWidget::SetEmail(const FString& Email)
{
    if (EmailInput)
    {
        EmailInput->SetText(FText::FromString(Email));
    }
}

void UDeskillzLoginWidget::ClearInputs()
{
    if (EmailInput)
    {
        EmailInput->SetText(FText::GetEmpty());
    }
    
    if (PasswordInput)
    {
        PasswordInput->SetText(FText::GetEmpty());
    }
}

void UDeskillzLoginWidget::ShowError(const FString& Message)
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::FromString(Message));
        ErrorText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UDeskillzLoginWidget::ClearError()
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::GetEmpty());
        ErrorText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// ============================================================================
// UI Helpers
// ============================================================================

void UDeskillzLoginWidget::SetLoading(bool bNewLoading)
{
    bIsLoading = bNewLoading;
    
    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(bIsLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    
    if (LoginButton)
    {
        LoginButton->SetIsEnabled(!bIsLoading);
    }
    
    if (GoogleButton)
    {
        GoogleButton->SetIsEnabled(!bIsLoading);
    }
    
    if (AppleButton)
    {
        AppleButton->SetIsEnabled(!bIsLoading);
    }
    
    if (FacebookButton)
    {
        FacebookButton->SetIsEnabled(!bIsLoading);
    }
}

FString UDeskillzLoginWidget::GetFriendlyErrorMessage(const FString& TechnicalMessage) const
{
    if (TechnicalMessage.Contains(TEXT("401")) || TechnicalMessage.Contains(TEXT("unauthorized"), ESearchCase::IgnoreCase))
    {
        return TEXT("Invalid email or password. Please try again.");
    }
    
    if (TechnicalMessage.Contains(TEXT("404")))
    {
        return TEXT("Account not found. Please sign up first.");
    }
    
    if (TechnicalMessage.Contains(TEXT("network"), ESearchCase::IgnoreCase) || 
        TechnicalMessage.Contains(TEXT("connection"), ESearchCase::IgnoreCase))
    {
        return TEXT("Connection error. Please check your internet and try again.");
    }
    
    return TechnicalMessage;
}

bool UDeskillzLoginWidget::ValidateEmail(const FString& Email) const
{
    return Email.Contains(TEXT("@")) && Email.Contains(TEXT("."));
}