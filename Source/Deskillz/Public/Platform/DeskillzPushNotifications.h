// Copyright Deskillz Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DeskillzPushNotifications.generated.h"

/**
 * Notification type
 */
UENUM(BlueprintType)
enum class EDeskillzNotificationType : uint8
{
	/** Generic notification */
	Generic,
	
	/** Tournament starting soon */
	TournamentStarting,
	
	/** Tournament results ready */
	TournamentResults,
	
	/** Match found */
	MatchFound,
	
	/** Match reminder */
	MatchReminder,
	
	/** Friend activity */
	FriendActivity,
	
	/** Prize won */
	PrizeWon,
	
	/** Promotional */
	Promotional,
	
	/** System message */
	System
};

/**
 * Notification permission status
 */
UENUM(BlueprintType)
enum class EDeskillzNotificationPermission : uint8
{
	/** Not yet determined */
	NotDetermined,
	
	/** Permission granted */
	Authorized,
	
	/** Permission denied */
	Denied,
	
	/** Provisional (iOS) */
	Provisional
};

/**
 * Push notification payload
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzNotificationPayload
{
	GENERATED_BODY()
	
	/** Notification ID */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	FString NotificationId;
	
	/** Title */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	FString Title;
	
	/** Body text */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	FString Body;
	
	/** Notification type */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	EDeskillzNotificationType Type = EDeskillzNotificationType::Generic;
	
	/** Custom data */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	TMap<FString, FString> Data;
	
	/** Deep link URL */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	FString DeepLink;
	
	/** Image URL */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	FString ImageURL;
	
	/** Badge count */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	int32 BadgeCount = 0;
	
	/** Sound name */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	FString Sound;
	
	/** Was received while app in foreground */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	bool bReceivedInForeground = false;
	
	/** Timestamp */
	UPROPERTY(BlueprintReadOnly, Category = "Notifications")
	int64 Timestamp = 0;
};

/**
 * Local notification schedule
 */
USTRUCT(BlueprintType)
struct DESKILLZ_API FDeskillzLocalNotification
{
	GENERATED_BODY()
	
	/** Unique identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	FString Identifier;
	
	/** Title */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	FString Title;
	
	/** Body */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	FString Body;
	
	/** Delay in seconds (for immediate scheduling) */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	float DelaySeconds = 0.0f;
	
	/** Scheduled time (Unix timestamp, 0 for delay-based) */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	int64 ScheduledTime = 0;
	
	/** Custom data */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	TMap<FString, FString> Data;
	
	/** Badge count to set */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	int32 BadgeCount = -1;
	
	/** Sound name (empty for default) */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	FString Sound;
	
	/** Repeat interval (0 for no repeat) */
	UPROPERTY(BlueprintReadWrite, Category = "Notifications")
	int32 RepeatIntervalMinutes = 0;
};

/** Notification delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPushTokenReceived, const FString&, Token);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationReceived, const FDeskillzNotificationPayload&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationOpened, const FDeskillzNotificationPayload&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPermissionChanged, EDeskillzNotificationPermission, Permission);

/**
 * Deskillz Push Notifications Manager
 * 
 * Handles push notifications:
 * - Remote push notifications (FCM/APNs)
 * - Local notifications
 * - Permission management
 * - Token registration
 * - Notification handling
 * 
 * Usage:
 *   UDeskillzPushNotifications* Push = UDeskillzPushNotifications::Get();
 *   
 *   Push->OnNotificationReceived.AddDynamic(this, &AMyActor::HandleNotification);
 *   Push->RequestPermission();
 */
UCLASS(BlueprintType)
class DESKILLZ_API UDeskillzPushNotifications : public UObject
{
	GENERATED_BODY()
	
public:
	UDeskillzPushNotifications();
	
	// ========================================================================
	// Singleton Access
	// ========================================================================
	
	/**
	 * Get the Push Notifications instance
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Notifications", meta = (DisplayName = "Get Deskillz Push Notifications"))
	static UDeskillzPushNotifications* Get();
	
	// ========================================================================
	// Initialization
	// ========================================================================
	
	/**
	 * Initialize push notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void Initialize();
	
	/**
	 * Shutdown notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void Shutdown();
	
	// ========================================================================
	// Permissions
	// ========================================================================
	
	/**
	 * Request notification permission
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void RequestPermission();
	
	/**
	 * Get current permission status
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Notifications")
	EDeskillzNotificationPermission GetPermissionStatus() const { return PermissionStatus; }
	
	/**
	 * Are notifications enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Notifications")
	bool AreNotificationsEnabled() const { return PermissionStatus == EDeskillzNotificationPermission::Authorized; }
	
	/**
	 * Open system notification settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void OpenNotificationSettings();
	
	// ========================================================================
	// Token Management
	// ========================================================================
	
	/**
	 * Get push token (device token)
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Notifications")
	FString GetPushToken() const { return PushToken; }
	
	/**
	 * Is push token available
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Notifications")
	bool HasPushToken() const { return !PushToken.IsEmpty(); }
	
	/**
	 * Register token with server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void RegisterTokenWithServer();
	
	/**
	 * Unregister token from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void UnregisterTokenFromServer();
	
	// ========================================================================
	// Local Notifications
	// ========================================================================
	
	/**
	 * Schedule local notification
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void ScheduleLocalNotification(const FDeskillzLocalNotification& Notification);
	
	/**
	 * Cancel local notification
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void CancelLocalNotification(const FString& Identifier);
	
	/**
	 * Cancel all local notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void CancelAllLocalNotifications();
	
	/**
	 * Get pending local notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	TArray<FString> GetPendingLocalNotifications() const;
	
	// ========================================================================
	// Badge
	// ========================================================================
	
	/**
	 * Set app badge count
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void SetBadgeCount(int32 Count);
	
	/**
	 * Get app badge count
	 */
	UFUNCTION(BlueprintPure, Category = "Deskillz|Notifications")
	int32 GetBadgeCount() const { return BadgeCount; }
	
	/**
	 * Clear badge
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void ClearBadge() { SetBadgeCount(0); }
	
	// ========================================================================
	// Notification Handling
	// ========================================================================
	
	/**
	 * Handle received notification (called from platform)
	 */
	void HandleNotificationReceived(const FDeskillzNotificationPayload& Notification);
	
	/**
	 * Handle notification opened (called from platform)
	 */
	void HandleNotificationOpened(const FDeskillzNotificationPayload& Notification);
	
	/**
	 * Get launch notification (if app was opened from notification)
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	bool GetLaunchNotification(FDeskillzNotificationPayload& OutNotification);
	
	// ========================================================================
	// Topic Subscription (FCM)
	// ========================================================================
	
	/**
	 * Subscribe to topic
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void SubscribeToTopic(const FString& Topic);
	
	/**
	 * Unsubscribe from topic
	 */
	UFUNCTION(BlueprintCallable, Category = "Deskillz|Notifications")
	void UnsubscribeFromTopic(const FString& Topic);
	
	// ========================================================================
	// Events
	// ========================================================================
	
	/** Called when push token is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Notifications")
	FOnPushTokenReceived OnPushTokenReceived;
	
	/** Called when notification is received */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Notifications")
	FOnNotificationReceived OnNotificationReceived;
	
	/** Called when notification is opened/tapped */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Notifications")
	FOnNotificationOpened OnNotificationOpened;
	
	/** Called when permission status changes */
	UPROPERTY(BlueprintAssignable, Category = "Deskillz|Notifications")
	FOnPermissionChanged OnPermissionChanged;
	
protected:
	/** Is initialized */
	UPROPERTY()
	bool bIsInitialized = false;
	
	/** Current permission status */
	UPROPERTY()
	EDeskillzNotificationPermission PermissionStatus = EDeskillzNotificationPermission::NotDetermined;
	
	/** Push token */
	UPROPERTY()
	FString PushToken;
	
	/** Badge count */
	UPROPERTY()
	int32 BadgeCount = 0;
	
	/** Launch notification */
	UPROPERTY()
	FDeskillzNotificationPayload LaunchNotification;
	
	/** Has launch notification */
	UPROPERTY()
	bool bHasLaunchNotification = false;
	
	/** Subscribed topics */
	UPROPERTY()
	TArray<FString> SubscribedTopics;
	
	/** Pending local notifications */
	TArray<FString> PendingNotificationIds;
	
	// ========================================================================
	// Internal Methods
	// ========================================================================
	
	/** Register for remote notifications */
	void RegisterForRemoteNotifications();
	
	/** Handle token received from platform */
	void OnTokenReceived(const FString& Token);
	
	/** Handle permission result */
	void OnPermissionResult(bool bGranted);
	
	/** Parse notification type from data */
	EDeskillzNotificationType ParseNotificationType(const TMap<FString, FString>& Data) const;
};
