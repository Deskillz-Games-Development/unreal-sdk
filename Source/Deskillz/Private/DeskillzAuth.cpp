// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#include "DeskillzAuth.h"
#include "DeskillzAuthService.h"
#include "DeskillzConfig.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"

// Static instance
UDeskillzAuth* UDeskillzAuth::Instance = nullptr;

// Storage keys
const FString UDeskillzAuth::PREF_ACCESS_TOKEN = TEXT("DeskillzAccessToken");
const FString UDeskillzAuth::PREF_REFRESH_TOKEN = TEXT("DeskillzRefreshToken");
const FString UDeskillzAuth::PREF_USER_ID = TEXT("DeskillzUserId");
const FString UDeskillzAuth::PREF_REMEMBER_ME = TEXT("DeskillzRememberMe");
const FString UDeskillzAuth::PREF_TOKEN_EXPIRY = TEXT("DeskillzTokenExpiry");

UDeskillzAuth::UDeskillzAuth()
{
}

UDeskillzAuth* UDeskillzAuth::Get(const UObject* WorldContextObject)
{
    if (!Instance)
    {
        Instance = NewObject<UDeskillzAuth>();
        Instance->AddToRoot(); // Prevent garbage collection
        Instance->AuthService = UDeskillzAuthService::Get();
    }
    return Instance;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzAuth::Initialize()
{
    if (bIsInitialized)
    {
        return;
    }
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Initializing authentication system"));
    
    // Check for existing session
    if (HasSavedSession())
    {
        UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Found saved session, attempting restore"));
        TryRestoreSession();
    }
    else
    {
        SetState(EDeskillzAuthState::NotAuthenticated);
    }
}

bool UDeskillzAuth::HasSavedSession() const
{
    bool bRemember = false;
    GConfig->GetBool(TEXT("Deskillz"), *PREF_REMEMBER_ME, bRemember, GGameIni);
    
    FString SavedRefreshToken;
    GConfig->GetString(TEXT("Deskillz"), *PREF_REFRESH_TOKEN, SavedRefreshToken, GGameIni);
    
    return bRemember && !SavedRefreshToken.IsEmpty();
}

void UDeskillzAuth::TryRestoreSession()
{
    SetState(EDeskillzAuthState::Authenticating);
    
    GConfig->GetString(TEXT("Deskillz"), *PREF_REFRESH_TOKEN, RefreshToken, GGameIni);
    
    if (RefreshToken.IsEmpty())
    {
        SetState(EDeskillzAuthState::NotAuthenticated);
        return;
    }
    
    // Try to refresh the token
    AuthService->RefreshToken(RefreshToken, UDeskillzAuthService::FOnAuthResponse::CreateLambda(
        [this](bool bSuccess, const FDeskillzAuthResponse& Response)
        {
            if (bSuccess)
            {
                HandleAuthSuccess(Response, true);
                UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Session restored successfully"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[DeskillzAuth] Failed to restore session"));
                ClearSavedSession();
                SetState(EDeskillzAuthState::NotAuthenticated);
            }
        }
    ));
}

// ============================================================================
// Login Methods
// ============================================================================

void UDeskillzAuth::Login(const FString& Email, const FString& Password, bool bShouldRemember)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Logging in: %s"), *Email);
    SetState(EDeskillzAuthState::Authenticating);
    
    FString Error;
    if (!ValidateEmail(Email, Error) || !ValidatePassword(Password, Error))
    {
        HandleAuthError(Error);
        return;
    }
    
    bRememberMe = bShouldRemember;
    
    FDeskillzLoginRequest Request(Email, Password);
    AuthService->Login(Request, UDeskillzAuthService::FOnAuthResponse::CreateLambda(
        [this](bool bSuccess, const FDeskillzAuthResponse& Response)
        {
            if (bSuccess)
            {
                HandleAuthSuccess(Response, bRememberMe);
                UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Login successful: %s"), *CurrentUser.Username);
                OnLoginSuccess.Broadcast(CurrentUser);
            }
            else
            {
                HandleAuthError(TEXT("Login failed. Please check your credentials."));
            }
        }
    ));
}

void UDeskillzAuth::SocialLogin(EDeskillzAuthProvider Provider, const FString& IdToken)
{
    FString ProviderName;
    switch (Provider)
    {
        case EDeskillzAuthProvider::Google: ProviderName = TEXT("google"); break;
        case EDeskillzAuthProvider::Apple: ProviderName = TEXT("apple"); break;
        case EDeskillzAuthProvider::Facebook: ProviderName = TEXT("facebook"); break;
        default:
            HandleAuthError(TEXT("Invalid social provider"));
            return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Social login via: %s"), *ProviderName);
    SetState(EDeskillzAuthState::Authenticating);
    
    if (IdToken.IsEmpty())
    {
        HandleAuthError(TEXT("ID token is required"));
        return;
    }
    
    FDeskillzSocialAuthRequest Request(Provider, IdToken);
    AuthService->SocialLogin(Request, UDeskillzAuthService::FOnAuthResponse::CreateLambda(
        [this](bool bSuccess, const FDeskillzAuthResponse& Response)
        {
            if (bSuccess)
            {
                HandleAuthSuccess(Response, true); // Social logins always remember
                
                if (Response.bIsNewUser)
                {
                    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] New user created: %s"), *CurrentUser.Username);
                    OnSignUpSuccess.Broadcast(CurrentUser);
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Social login successful: %s"), *CurrentUser.Username);
                    OnLoginSuccess.Broadcast(CurrentUser);
                }
            }
            else
            {
                HandleAuthError(TEXT("Social login failed"));
            }
        }
    ));
}

// ============================================================================
// Sign Up
// ============================================================================

void UDeskillzAuth::SignUp(const FString& Email, const FString& Password, const FString& Username)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Registering: %s, username: %s"), *Email, *Username);
    SetState(EDeskillzAuthState::Authenticating);
    
    FString Error;
    if (!ValidateEmail(Email, Error) || !ValidatePassword(Password, Error) || !ValidateUsername(Username, Error))
    {
        HandleAuthError(Error);
        return;
    }
    
    FDeskillzSignUpRequest Request(Email, Password, Username);
    AuthService->SignUp(Request, UDeskillzAuthService::FOnAuthResponse::CreateLambda(
        [this](bool bSuccess, const FDeskillzAuthResponse& Response)
        {
            if (bSuccess)
            {
                HandleAuthSuccess(Response, true); // Auto-login after signup
                UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Registration successful: %s"), *CurrentUser.Username);
                OnSignUpSuccess.Broadcast(CurrentUser);
            }
            else
            {
                HandleAuthError(TEXT("Registration failed. Email or username may already be taken."));
            }
        }
    ));
}

// ============================================================================
// Logout
// ============================================================================

void UDeskillzAuth::Logout()
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Logging out"));
    
    CurrentUser = FDeskillzAuthUser();
    AccessToken.Empty();
    RefreshToken.Empty();
    TokenExpiry = FDateTime();
    bIsAuthenticated = false;
    
    ClearSavedSession();
    SetState(EDeskillzAuthState::NotAuthenticated);
    
    OnLogout.Broadcast();
}

// ============================================================================
// Password Reset
// ============================================================================

void UDeskillzAuth::ForgotPassword(const FString& Email)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Requesting password reset for: %s"), *Email);
    
    FString Error;
    if (!ValidateEmail(Email, Error))
    {
        HandleAuthError(Error);
        return;
    }
    
    AuthService->ForgotPassword(Email, UDeskillzAuthService::FOnSimpleResponse::CreateLambda(
        [this](bool bSuccess, const FString& ErrorMessage)
        {
            if (bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Password reset email sent"));
                OnPasswordResetSent.Broadcast();
            }
            else
            {
                HandleAuthError(ErrorMessage.IsEmpty() ? TEXT("Failed to send password reset email") : ErrorMessage);
            }
        }
    ));
}

// ============================================================================
// Wallet Management
// ============================================================================

void UDeskillzAuth::LinkWallet(const FDeskillzWalletLinkRequest& Request)
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Linking wallet: %s"), *Request.WalletAddress);
    
    if (!IsAuthenticated())
    {
        HandleAuthError(TEXT("Must be logged in to link wallet"));
        return;
    }
    
    AuthService->LinkWallet(Request, AccessToken, UDeskillzAuthService::FOnUserResponse::CreateLambda(
        [this](bool bSuccess, const FDeskillzAuthUser& User)
        {
            if (bSuccess)
            {
                CurrentUser = User;
                UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Wallet linked successfully"));
                OnWalletConnected.Broadcast(CurrentUser.WalletAddress);
            }
            else
            {
                HandleAuthError(TEXT("Failed to link wallet"));
            }
        }
    ));
}

void UDeskillzAuth::DisconnectWallet()
{
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Disconnecting wallet"));
    
    if (!IsAuthenticated())
    {
        HandleAuthError(TEXT("Must be logged in to disconnect wallet"));
        return;
    }
    
    if (!HasWallet())
    {
        UE_LOG(LogTemp, Warning, TEXT("[DeskillzAuth] No wallet connected"));
        return;
    }
    
    AuthService->DisconnectWallet(AccessToken, UDeskillzAuthService::FOnUserResponse::CreateLambda(
        [this](bool bSuccess, const FDeskillzAuthUser& User)
        {
            if (bSuccess)
            {
                CurrentUser = User;
                UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Wallet disconnected successfully"));
                OnWalletDisconnected.Broadcast();
            }
            else
            {
                HandleAuthError(TEXT("Failed to disconnect wallet"));
            }
        }
    ));
}

bool UDeskillzAuth::RequireWallet(const FString& ActionDescription)
{
    if (HasWallet())
    {
        return true;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Wallet required for: %s"), *ActionDescription);
    // Fire event to show wallet connection UI (handled by game code)
    return false;
}

// ============================================================================
// Token Management
// ============================================================================

void UDeskillzAuth::RefreshAccessToken()
{
    if (RefreshToken.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[DeskillzAuth] No refresh token available"));
        Logout();
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Refreshing access token"));
    
    AuthService->RefreshToken(RefreshToken, UDeskillzAuthService::FOnAuthResponse::CreateLambda(
        [this](bool bSuccess, const FDeskillzAuthResponse& Response)
        {
            if (bSuccess)
            {
                AccessToken = Response.AccessToken;
                RefreshToken = Response.RefreshToken;
                TokenExpiry = FDateTime::UtcNow() + FTimespan::FromSeconds(Response.ExpiresIn);
                
                if (bRememberMe)
                {
                    SaveTokens();
                }
                
                UE_LOG(LogTemp, Log, TEXT("[DeskillzAuth] Token refreshed successfully"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[DeskillzAuth] Token refresh failed"));
                Logout();
            }
        }
    ));
}

bool UDeskillzAuth::IsTokenExpiringSoon() const
{
    if (TokenExpiry.GetTicks() == 0)
    {
        return false;
    }
    return (FDateTime::UtcNow() + FTimespan::FromSeconds(TOKEN_REFRESH_THRESHOLD_SECONDS)) >= TokenExpiry;
}

// ============================================================================
// Internal Helpers
// ============================================================================

void UDeskillzAuth::HandleAuthSuccess(const FDeskillzAuthResponse& Response, bool bShouldRemember)
{
    CurrentUser = Response.User;
    AccessToken = Response.AccessToken;
    RefreshToken = Response.RefreshToken;
    TokenExpiry = FDateTime::UtcNow() + FTimespan::FromSeconds(Response.ExpiresIn);
    bIsAuthenticated = true;
    bRememberMe = bShouldRemember;
    
    if (bShouldRemember)
    {
        SaveSession();
    }
    
    SetState(EDeskillzAuthState::Authenticated);
}

void UDeskillzAuth::HandleAuthError(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("[DeskillzAuth] Auth error: %s"), *ErrorMessage);
    SetState(EDeskillzAuthState::Error);
    OnAuthError.Broadcast(ErrorMessage);
}

void UDeskillzAuth::SetState(EDeskillzAuthState NewState)
{
    if (AuthState != NewState)
    {
        AuthState = NewState;
        OnAuthStateChanged.Broadcast(NewState, CurrentUser);
    }
}

void UDeskillzAuth::SaveSession()
{
    GConfig->SetBool(TEXT("Deskillz"), *PREF_REMEMBER_ME, true, GGameIni);
    SaveTokens();
    
    if (CurrentUser.IsValid())
    {
        GConfig->SetString(TEXT("Deskillz"), *PREF_USER_ID, *CurrentUser.Id, GGameIni);
    }
    
    GConfig->Flush(false, GGameIni);
}

void UDeskillzAuth::SaveTokens()
{
    GConfig->SetString(TEXT("Deskillz"), *PREF_ACCESS_TOKEN, *AccessToken, GGameIni);
    GConfig->SetString(TEXT("Deskillz"), *PREF_REFRESH_TOKEN, *RefreshToken, GGameIni);
    GConfig->SetString(TEXT("Deskillz"), *PREF_TOKEN_EXPIRY, *TokenExpiry.ToString(), GGameIni);
    GConfig->Flush(false, GGameIni);
}

void UDeskillzAuth::ClearSavedSession()
{
    GConfig->RemoveKey(TEXT("Deskillz"), *PREF_ACCESS_TOKEN, GGameIni);
    GConfig->RemoveKey(TEXT("Deskillz"), *PREF_REFRESH_TOKEN, GGameIni);
    GConfig->RemoveKey(TEXT("Deskillz"), *PREF_USER_ID, GGameIni);
    GConfig->RemoveKey(TEXT("Deskillz"), *PREF_REMEMBER_ME, GGameIni);
    GConfig->RemoveKey(TEXT("Deskillz"), *PREF_TOKEN_EXPIRY, GGameIni);
    GConfig->Flush(false, GGameIni);
}

// ============================================================================
// Validation Helpers
// ============================================================================

bool UDeskillzAuth::ValidateEmail(const FString& Email, FString& OutError) const
{
    if (Email.IsEmpty())
    {
        OutError = TEXT("Email is required");
        return false;
    }
    
    if (!Email.Contains(TEXT("@")) || !Email.Contains(TEXT(".")))
    {
        OutError = TEXT("Invalid email format");
        return false;
    }
    
    return true;
}

bool UDeskillzAuth::ValidatePassword(const FString& Password, FString& OutError) const
{
    if (Password.IsEmpty())
    {
        OutError = TEXT("Password is required");
        return false;
    }
    
    if (Password.Len() < 8)
    {
        OutError = TEXT("Password must be at least 8 characters");
        return false;
    }
    
    return true;
}

bool UDeskillzAuth::ValidateUsername(const FString& Username, FString& OutError) const
{
    if (Username.IsEmpty())
    {
        OutError = TEXT("Username is required");
        return false;
    }
    
    if (Username.Len() < 3 || Username.Len() > 20)
    {
        OutError = TEXT("Username must be 3-20 characters");
        return false;
    }
    
    // Check for valid characters
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