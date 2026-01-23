// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#pragma once

#include "CoreMinimal.h"
#include "DeskillzAuthTypes.h"
#include "DeskillzAuth.generated.h"

// Forward declarations
class UDeskillzAuthService;

/**
 * Main Authentication Manager for Deskillz SDK
 * 
 * Handles user login, registration, and session management.
 * 
 * Features:
 * - Email/password login and registration
 * - Social login (Google, Apple, Facebook)
 * - Wallet connection (optional, for paid tournaments)
 * - Automatic token refresh
 * - Persistent login (Remember Me)
 * 
 * Usage:
 * - Use UDeskillzAuth::Get() to access the singleton
 * - Bind to events for async callbacks
 * - Call Login(), SignUp(), etc.
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzAuth : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzAuth();

    // ========================================================================
    // Singleton Access
    // ========================================================================
    
    /** Get the singleton auth manager instance */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Deskillz Auth"))
    static UDeskillzAuth* Get(const UObject* WorldContextObject = nullptr);

    // ========================================================================
    // Events (Blueprint Bindable)
    // ========================================================================
    
    /** Fired when login succeeds */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzLoginSuccess OnLoginSuccess;
    
    /** Fired when logout occurs */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzLogout OnLogout;
    
    /** Fired when authentication error occurs */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzAuthError OnAuthError;
    
    /** Fired when wallet is connected to account */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzWalletConnected OnWalletConnected;
    
    /** Fired when wallet is disconnected from account */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzWalletDisconnected OnWalletDisconnected;
    
    /** Fired when sign up succeeds */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzSignUpSuccess OnSignUpSuccess;
    
    /** Fired when password reset email is sent */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzPasswordResetSent OnPasswordResetSent;
    
    /** Fired when auth state changes */
    UPROPERTY(BlueprintAssignable, Category = "Deskillz|Auth|Events")
    FOnDeskillzAuthStateChangedNew OnAuthStateChanged;

    // ========================================================================
    // State Properties (Blueprint Readable)
    // ========================================================================
    
    /** Get current authenticated user */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    FDeskillzAuthUser GetCurrentUser() const { return CurrentUser; }
    
    /** Check if a user is currently authenticated */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    bool IsAuthenticated() const { return bIsAuthenticated && !AccessToken.IsEmpty(); }
    
    /** Get current authentication state */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    EDeskillzAuthState GetAuthState() const { return AuthState; }
    
    /** Check if user has a connected wallet */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    bool HasWallet() const { return CurrentUser.bHasWallet; }
    
    /** Get connected wallet address (empty if none) */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    FString GetWalletAddress() const { return CurrentUser.WalletAddress; }
    
    /** Get current access token for API calls */
    UFUNCTION(BlueprintPure, Category = "Deskillz|Auth")
    FString GetAccessToken() const { return AccessToken; }

    // ========================================================================
    // Initialization
    // ========================================================================
    
    /**
     * Initialize the auth system. Called automatically by SDK.
     * Restores previous session if "Remember Me" was enabled.
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Initialize();

    // ========================================================================
    // Login Methods
    // ========================================================================
    
    /**
     * Login with email and password
     * @param Email User's email address
     * @param Password User's password
     * @param bRememberMe Whether to persist login across sessions
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Login(const FString& Email, const FString& Password, bool bRememberMe = false);
    
    /**
     * Login with social provider (Google, Apple, Facebook)
     * @param Provider Provider type
     * @param IdToken ID token from the social provider
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void SocialLogin(EDeskillzAuthProvider Provider, const FString& IdToken);

    // ========================================================================
    // Sign Up Methods
    // ========================================================================
    
    /**
     * Register a new user with email and password
     * @param Email User's email address
     * @param Password Password (min 8 characters)
     * @param Username Desired username (3-20 characters)
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void SignUp(const FString& Email, const FString& Password, const FString& Username);

    // ========================================================================
    // Logout
    // ========================================================================
    
    /**
     * Logout the current user
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void Logout();

    // ========================================================================
    // Password Reset
    // ========================================================================
    
    /**
     * Request a password reset email
     * @param Email User's email address
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void ForgotPassword(const FString& Email);

    // ========================================================================
    // Wallet Management
    // ========================================================================
    
    /**
     * Link a wallet to the current account
     * Required for paid tournaments.
     * @param Request Wallet link request data
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void LinkWallet(const FDeskillzWalletLinkRequest& Request);
    
    /**
     * Disconnect the wallet from the current account
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void DisconnectWallet();
    
    /**
     * Check if wallet is required for an action
     * @param ActionDescription Description of the action requiring wallet
     * @return True if wallet already connected
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    bool RequireWallet(const FString& ActionDescription);

    // ========================================================================
    // Token Management
    // ========================================================================
    
    /**
     * Refresh the access token if needed
     * Called automatically before token expiry
     */
    UFUNCTION(BlueprintCallable, Category = "Deskillz|Auth")
    void RefreshAccessToken();

private:
    /** Singleton instance */
    static UDeskillzAuth* Instance;
    
    /** Auth service for API calls */
    UPROPERTY()
    UDeskillzAuthService* AuthService;
    
    // ========================================================================
    // State
    // ========================================================================
    
    /** Current authenticated user */
    FDeskillzAuthUser CurrentUser;
    
    /** Current access token */
    FString AccessToken;
    
    /** Current refresh token */
    FString RefreshToken;
    
    /** Token expiry time */
    FDateTime TokenExpiry;
    
    /** Whether user is authenticated */
    bool bIsAuthenticated = false;
    
    /** Whether auth system is initialized */
    bool bIsInitialized = false;
    
    /** Current auth state */
    EDeskillzAuthState AuthState = EDeskillzAuthState::NotAuthenticated;
    
    /** Whether "Remember Me" is enabled */
    bool bRememberMe = false;

    // ========================================================================
    // Internal Helpers
    // ========================================================================
    
    /** Check if saved session exists */
    bool HasSavedSession() const;
    
    /** Try to restore previous session */
    void TryRestoreSession();
    
    /** Handle successful authentication */
    void HandleAuthSuccess(const FDeskillzAuthResponse& Response, bool bShouldRemember);
    
    /** Handle authentication error */
    void HandleAuthError(const FString& ErrorMessage);
    
    /** Set authentication state */
    void SetState(EDeskillzAuthState NewState);
    
    /** Save session to persistent storage */
    void SaveSession();
    
    /** Save tokens to persistent storage */
    void SaveTokens();
    
    /** Clear saved session */
    void ClearSavedSession();
    
    /** Check if token is expiring soon */
    bool IsTokenExpiringSoon() const;
    
    // ========================================================================
    // Validation Helpers
    // ========================================================================
    
    /** Validate email format */
    bool ValidateEmail(const FString& Email, FString& OutError) const;
    
    /** Validate password requirements */
    bool ValidatePassword(const FString& Password, FString& OutError) const;
    
    /** Validate username requirements */
    bool ValidateUsername(const FString& Username, FString& OutError) const;
    
    // ========================================================================
    // Persistent Storage Keys
    // ========================================================================
    
    static const FString PREF_ACCESS_TOKEN;
    static const FString PREF_REFRESH_TOKEN;
    static const FString PREF_USER_ID;
    static const FString PREF_REMEMBER_ME;
    static const FString PREF_TOKEN_EXPIRY;
    
    /** Token refresh threshold (5 minutes before expiry) */
    static constexpr int32 TOKEN_REFRESH_THRESHOLD_SECONDS = 300;
};