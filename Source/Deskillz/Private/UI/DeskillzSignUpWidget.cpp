// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#include "DeskillzSignUpWidget.h"
#include "DeskillzAuth.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

UDeskillzSignUpWidget::UDeskillzSignUpWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UDeskillzSignUpWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Bind button click events
    if (SignUpButton)
    {
        SignUpButton->OnClicked.AddDynamic(this, &UDeskillzSignUpWidget::OnSignUpButtonClicked);
    }
    
    if (LoginButton)
    {
        LoginButton->OnClicked.AddDynamic(this, &UDeskillzSignUpWidget::OnLoginButtonClicked);
    }
    
    if (GoogleButton)
    {
        GoogleButton->OnClicked.AddDynamic(this, &UDeskillzSignUpWidget::OnGoogleButtonClicked);
    }
    
    if (AppleButton)
    {
        AppleButton->OnClicked.AddDynamic(this, &UDeskillzSignUpWidget::OnAppleButtonClicked);
    }
    
    if (FacebookButton)
    {
        FacebookButton->OnClicked.AddDynamic(this, &UDeskillzSignUpWidget::OnFacebookButtonClicked);
    }
    
    if (TermsCheckBox)
    {
        TermsCheckBox->OnCheckStateChanged.AddDynamic(this, &UDeskillzSignUpWidget::OnTermsCheckBoxChanged);
    }
    
    // Bind password input change for strength indicator
    if (PasswordInput)
    {
        PasswordInput->SetIsPassword(true);
        PasswordInput->OnTextChanged.AddDynamic(this, &UDeskillzSignUpWidget::OnPasswordChanged);
    }
    
    if (ConfirmPasswordInput)
    {
        ConfirmPasswordInput->SetIsPassword(true);
    }
    
    // Subscribe to auth events
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->OnSignUpSuccess.AddDynamic(this, &UDeskillzSignUpWidget::HandleSignUpSuccess);
        Auth->OnAuthError.AddDynamic(this, &UDeskillzSignUpWidget::HandleAuthError);
        Auth->OnAuthStateChanged.AddDynamic(this, &UDeskillzSignUpWidget::HandleAuthStateChanged);
    }
    
    // Initialize UI state
    ClearError();
    SetLoading(false);
    UpdateSignUpButtonState();
    UpdatePasswordStrength(TEXT(""));
}

void UDeskillzSignUpWidget::NativeDestruct()
{
    // Unsubscribe from auth events
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->OnSignUpSuccess.RemoveDynamic(this, &UDeskillzSignUpWidget::HandleSignUpSuccess);
        Auth->OnAuthError.RemoveDynamic(this, &UDeskillzSignUpWidget::HandleAuthError);
        Auth->OnAuthStateChanged.RemoveDynamic(this, &UDeskillzSignUpWidget::HandleAuthStateChanged);
    }
    
    Super::NativeDestruct();
}

// ============================================================================
// Button Handlers
// ============================================================================

void UDeskillzSignUpWidget::OnSignUpButtonClicked()
{
    ClearError();
    
    FString ValidationError;
    if (!ValidateInputs(ValidationError))
    {
        ShowError(ValidationError);
        return;
    }
    
    FString Username = UsernameInput ? UsernameInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
    FString Email = EmailInput ? EmailInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
    FString Password = PasswordInput ? PasswordInput->GetText().ToString() : TEXT("");
    
    SetLoading(true);
    
    UDeskillzAuth* Auth = UDeskillzAuth::Get();
    if (Auth)
    {
        Auth->SignUp(Email, Password, Username);
    }
}

void UDeskillzSignUpWidget::OnLoginButtonClicked()
{
    OnShowLogin.Broadcast();
}

void UDeskillzSignUpWidget::OnGoogleButtonClicked()
{
    ClearError();
    ShowError(TEXT("Google sign up coming soon. Use email for now."));
}

void UDeskillzSignUpWidget::OnAppleButtonClicked()
{
    ClearError();
    ShowError(TEXT("Apple sign up coming soon. Use email for now."));
}

void UDeskillzSignUpWidget::OnFacebookButtonClicked()
{
    ClearError();
    ShowError(TEXT("Facebook sign up coming soon. Use email for now."));
}

void UDeskillzSignUpWidget::OnTermsCheckBoxChanged(bool bIsChecked)
{
    UpdateSignUpButtonState();
}

void UDeskillzSignUpWidget::OnPasswordChanged(const FText& Text)
{
    ClearError();
    UpdatePasswordStrength(Text.ToString());
    UpdateSignUpButtonState();
}

// ============================================================================
// Auth Event Handlers
// ============================================================================

void UDeskillzSignUpWidget::HandleSignUpSuccess(const FDeskillzAuthUser& User)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzSignUpWidget] Sign up successful: %s"), *User.Username);
    SetLoading(false);
    OnSignUpSuccess.Broadcast(User);
}

void UDeskillzSignUpWidget::HandleAuthError(const FString& ErrorMessage)
{
    SetLoading(false);
    ShowError(GetFriendlyErrorMessage(ErrorMessage));
}

void UDeskillzSignUpWidget::HandleAuthStateChanged(EDeskillzAuthState NewState, const FDeskillzAuthUser& User)
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

void UDeskillzSignUpWidget::ClearInputs()
{
    if (UsernameInput) UsernameInput->SetText(FText::GetEmpty());
    if (EmailInput) EmailInput->SetText(FText::GetEmpty());
    if (PasswordInput) PasswordInput->SetText(FText::GetEmpty());
    if (ConfirmPasswordInput) ConfirmPasswordInput->SetText(FText::GetEmpty());
    if (TermsCheckBox) TermsCheckBox->SetIsChecked(false);
    
    UpdatePasswordStrength(TEXT(""));
    UpdateSignUpButtonState();
}

void UDeskillzSignUpWidget::ShowError(const FString& Message)
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::FromString(Message));
        ErrorText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UDeskillzSignUpWidget::ClearError()
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::GetEmpty());
        ErrorText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// ============================================================================
// Validation
// ============================================================================

bool UDeskillzSignUpWidget::ValidateInputs(FString& OutError) const
{
    FString Username = UsernameInput ? UsernameInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
    FString Email = EmailInput ? EmailInput->GetText().ToString().TrimStartAndEnd() : TEXT("");
    FString Password = PasswordInput ? PasswordInput->GetText().ToString() : TEXT("");
    FString ConfirmPassword = ConfirmPasswordInput ? ConfirmPasswordInput->GetText().ToString() : TEXT("");
    
    if (!ValidateUsername(Username, OutError)) return false;
    if (!ValidateEmail(Email, OutError)) return false;
    if (!ValidatePassword(Password, OutError)) return false;
    
    if (Password != ConfirmPassword)
    {
        OutError = TEXT("Passwords do not match");
        return false;
    }
    
    if (TermsCheckBox && !TermsCheckBox->IsChecked())
    {
        OutError = TEXT("Please agree to the Terms of Service");
        return false;
    }
    
    return true;
}

bool UDeskillzSignUpWidget::ValidateUsername(const FString& Username, FString& OutError) const
{
    if (Username.IsEmpty())
    {
        OutError = TEXT("Please enter a username");
        return false;
    }
    
    if (Username.Len() < 3)
    {
        OutError = TEXT("Username must be at least 3 characters");
        return false;
    }
    
    if (Username.Len() > 20)
    {
        OutError = TEXT("Username cannot exceed 20 characters");
        return false;
    }
    
    for (const TCHAR& Char : Username)
    {
        if (!FChar::IsAlnum(Char) && Char != TEXT('_'))
        {
            OutError = TEXT("Username can only contain letters, numbers, and underscores");
            return false;
        }
    }
    
    return true;
}

bool UDeskillzSignUpWidget::ValidateEmail(const FString& Email, FString& OutError) const
{
    if (Email.IsEmpty())
    {
        OutError = TEXT("Please enter your email address");
        return false;
    }
    
    if (!Email.Contains(TEXT("@")) || !Email.Contains(TEXT(".")))
    {
        OutError = TEXT("Please enter a valid email address");
        return false;
    }
    
    return true;
}

bool UDeskillzSignUpWidget::ValidatePassword(const FString& Password, FString& OutError) const
{
    if (Password.IsEmpty())
    {
        OutError = TEXT("Please enter a password");
        return false;
    }
    
    if (Password.Len() < 8)
    {
        OutError = TEXT("Password must be at least 8 characters");
        return false;
    }
    
    return true;
}

int32 UDeskillzSignUpWidget::CalculatePasswordStrength(const FString& Password) const
{
    if (Password.IsEmpty()) return 0;
    
    int32 Strength = 0;
    
    // Length checks
    if (Password.Len() >= 8) Strength++;
    if (Password.Len() >= 12) Strength++;
    
    // Complexity checks
    bool bHasLower = false, bHasUpper = false, bHasDigit = false, bHasSpecial = false;
    
    for (const TCHAR& Char : Password)
    {
        if (FChar::IsLower(Char)) bHasLower = true;
        else if (FChar::IsUpper(Char)) bHasUpper = true;
        else if (FChar::IsDigit(Char)) bHasDigit = true;
        else bHasSpecial = true;
    }
    
    if (bHasLower && bHasUpper) Strength++;
    if (bHasDigit) Strength++;
    if (bHasSpecial) Strength++;
    
    return FMath::Clamp(Strength, 0, 4);
}

FString UDeskillzSignUpWidget::GetPasswordStrengthLabel(int32 Strength) const
{
    switch (Strength)
    {
        case 0: return TEXT("Very Weak");
        case 1: return TEXT("Weak");
        case 2: return TEXT("Fair");
        case 3: return TEXT("Strong");
        case 4: return TEXT("Very Strong");
        default: return TEXT("");
    }
}

FLinearColor UDeskillzSignUpWidget::GetPasswordStrengthColor(int32 Strength) const
{
    switch (Strength)
    {
        case 0: return FLinearColor::Red;
        case 1: return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
        case 2: return FLinearColor::Yellow;
        case 3: return FLinearColor(0.5f, 1.0f, 0.0f); // Light green
        case 4: return FLinearColor::Green;
        default: return FLinearColor::White;
    }
}

// ============================================================================
// UI Helpers
// ============================================================================

void UDeskillzSignUpWidget::SetLoading(bool bNewLoading)
{
    bIsLoading = bNewLoading;
    
    if (LoadingIndicator)
    {
        LoadingIndicator->SetVisibility(bIsLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
    
    UpdateSignUpButtonState();
    
    if (GoogleButton) GoogleButton->SetIsEnabled(!bIsLoading);
    if (AppleButton) AppleButton->SetIsEnabled(!bIsLoading);
    if (FacebookButton) FacebookButton->SetIsEnabled(!bIsLoading);
}

void UDeskillzSignUpWidget::UpdateSignUpButtonState()
{
    if (!SignUpButton) return;
    
    bool bHasUsername = UsernameInput && !UsernameInput->GetText().IsEmpty();
    bool bHasEmail = EmailInput && !EmailInput->GetText().IsEmpty();
    bool bHasPassword = PasswordInput && !PasswordInput->GetText().IsEmpty();
    bool bHasConfirm = ConfirmPasswordInput && !ConfirmPasswordInput->GetText().IsEmpty();
    bool bAcceptedTerms = !TermsCheckBox || TermsCheckBox->IsChecked();
    
    SignUpButton->SetIsEnabled(!bIsLoading && bHasUsername && bHasEmail && bHasPassword && bHasConfirm && bAcceptedTerms);
}

void UDeskillzSignUpWidget::UpdatePasswordStrength(const FString& Password)
{
    int32 Strength = CalculatePasswordStrength(Password);
    FString Label = GetPasswordStrengthLabel(Strength);
    FLinearColor Color = GetPasswordStrengthColor(Strength);
    
    if (PasswordStrengthBar)
    {
        PasswordStrengthBar->SetPercent(Strength / 4.0f);
        PasswordStrengthBar->SetFillColorAndOpacity(Color);
    }
    
    if (PasswordStrengthText)
    {
        PasswordStrengthText->SetText(Password.IsEmpty() ? FText::GetEmpty() : FText::FromString(Label));
        PasswordStrengthText->SetColorAndOpacity(FSlateColor(Color));
    }
}

FString UDeskillzSignUpWidget::GetFriendlyErrorMessage(const FString& TechnicalMessage) const
{
    if (TechnicalMessage.Contains(TEXT("409")) || TechnicalMessage.Contains(TEXT("conflict"), ESearchCase::IgnoreCase))
    {
        return TEXT("Email or username already taken. Please try another.");
    }
    
    if (TechnicalMessage.Contains(TEXT("email"), ESearchCase::IgnoreCase) && 
        TechnicalMessage.Contains(TEXT("taken"), ESearchCase::IgnoreCase))
    {
        return TEXT("This email is already registered. Try logging in instead.");
    }
    
    if (TechnicalMessage.Contains(TEXT("username"), ESearchCase::IgnoreCase) && 
        TechnicalMessage.Contains(TEXT("taken"), ESearchCase::IgnoreCase))
    {
        return TEXT("This username is already taken. Please choose another.");
    }
    
    if (TechnicalMessage.Contains(TEXT("network"), ESearchCase::IgnoreCase) || 
        TechnicalMessage.Contains(TEXT("connection"), ESearchCase::IgnoreCase))
    {
        return TEXT("Connection error. Please check your internet and try again.");
    }
    
    return TechnicalMessage;
}