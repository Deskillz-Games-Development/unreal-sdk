// Copyright Deskillz Games. All Rights Reserved.
// Version: 3.0.0 (Self-Sufficient Architecture)

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "DeskillzAuthTypes.h"
#include "DeskillzAuthService.generated.h"

// Forward declarations
class FJsonObject;

/**
 * Authentication Service
 * 
 * Handles all authentication API calls to the Deskillz backend.
 * This is an internal service used by UDeskillzAuth.
 */
UCLASS()
class DESKILLZ_API UDeskillzAuthService : public UObject
{
    GENERATED_BODY()

public:
    UDeskillzAuthService();

    /** Get singleton instance */
    static UDeskillzAuthService* Get();

    // ========================================================================
    // API Methods - Delegates for async callbacks
    // ========================================================================

    /** Delegate for auth response */
    DECLARE_DELEGATE_TwoParams(FOnAuthResponse, bool /* bSuccess */, const FDeskillzAuthResponse& /* Response */);
    
    /** Delegate for user response */
    DECLARE_DELEGATE_TwoParams(FOnUserResponse, bool /* bSuccess */, const FDeskillzAuthUser& /* User */);
    
    /** Delegate for simple success/failure */
    DECLARE_DELEGATE_TwoParams(FOnSimpleResponse, bool /* bSuccess */, const FString& /* ErrorMessage */);
    
    /** Delegate for nonce response */
    DECLARE_DELEGATE_ThreeParams(FOnNonceResponse, bool /* bSuccess */, const FString& /* Nonce */, const FString& /* Message */);

    // ========================================================================
    // Public API Methods
    // ========================================================================

    /**
     * Register a new user with email and password
     * @param Request Sign up request data
     * @param OnComplete Callback when complete
     */
    void SignUp(const FDeskillzSignUpRequest& Request, FOnAuthResponse OnComplete);

    /**
     * Login with email and password
     * @param Request Login request data
     * @param OnComplete Callback when complete
     */
    void Login(const FDeskillzLoginRequest& Request, FOnAuthResponse OnComplete);

    /**
     * Login with social provider (Google, Apple, Facebook)
     * @param Request Social auth request data
     * @param OnComplete Callback when complete
     */
    void SocialLogin(const FDeskillzSocialAuthRequest& Request, FOnAuthResponse OnComplete);

    /**
     * Refresh access token
     * @param RefreshToken Current refresh token
     * @param OnComplete Callback when complete
     */
    void RefreshToken(const FString& RefreshToken, FOnAuthResponse OnComplete);

    /**
     * Get current authenticated user
     * @param AccessToken Current access token
     * @param OnComplete Callback when complete
     */
    void GetMe(const FString& AccessToken, FOnUserResponse OnComplete);

    /**
     * Request password reset email
     * @param Email User's email
     * @param OnComplete Callback when complete
     */
    void ForgotPassword(const FString& Email, FOnSimpleResponse OnComplete);

    /**
     * Reset password with token
     * @param Token Reset token from email
     * @param NewPassword New password
     * @param OnComplete Callback when complete
     */
    void ResetPassword(const FString& Token, const FString& NewPassword, FOnSimpleResponse OnComplete);

    /**
     * Link wallet to existing account
     * @param Request Wallet link request data
     * @param AccessToken Current access token
     * @param OnComplete Callback when complete
     */
    void LinkWallet(const FDeskillzWalletLinkRequest& Request, const FString& AccessToken, FOnUserResponse OnComplete);

    /**
     * Disconnect wallet from account
     * @param AccessToken Current access token
     * @param OnComplete Callback when complete
     */
    void DisconnectWallet(const FString& AccessToken, FOnUserResponse OnComplete);

    /**
     * Get nonce for wallet signature (SIWE)
     * @param WalletAddress Wallet address
     * @param OnComplete Callback when complete
     */
    void GetWalletNonce(const FString& WalletAddress, FOnNonceResponse OnComplete);

private:
    /** Singleton instance */
    static UDeskillzAuthService* Instance;

    /** Base API URL */
    FString GetBaseUrl() const;

    // ========================================================================
    // Internal HTTP Helpers
    // ========================================================================

    /** Create HTTP request */
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateRequest(const FString& Verb, const FString& Endpoint);

    /** Add auth header to request */
    void AddAuthHeader(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request, const FString& AccessToken);

    /** Parse auth response from JSON */
    bool ParseAuthResponse(const FString& ResponseContent, FDeskillzAuthResponse& OutResponse);

    /** Parse user from JSON */
    bool ParseUser(const TSharedPtr<FJsonObject>& JsonObject, FDeskillzAuthUser& OutUser);

    /** Parse user role from string */
    EDeskillzUserRole ParseUserRole(const FString& RoleString);

    // ========================================================================
    // Request Handlers
    // ========================================================================

    void HandleAuthResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnAuthResponse OnComplete);
    void HandleUserResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnUserResponse OnComplete);
    void HandleSimpleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnSimpleResponse OnComplete);
    void HandleNonceResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnNonceResponse OnComplete);
};