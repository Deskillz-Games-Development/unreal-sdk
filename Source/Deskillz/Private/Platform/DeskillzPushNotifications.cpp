// Copyright Deskillz Games. All Rights Reserved.

#include "Platform/DeskillzPushNotifications.h"
#include "Platform/DeskillzPlatform.h"
#include "Platform/DeskillzDeepLink.h"
#include "Network/DeskillzHttpClient.h"
#include "Deskillz.h"
#include "HAL/PlatformMisc.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

// Static singleton
static UDeskillzPushNotifications* GPushNotifications = nullptr;

UDeskillzPushNotifications::UDeskillzPushNotifications()
{
}

UDeskillzPushNotifications* UDeskillzPushNotifications::Get()
{
	if (!GPushNotifications)
	{
		GPushNotifications = NewObject<UDeskillzPushNotifications>();
		GPushNotifications->AddToRoot();
	}
	return GPushNotifications;
}

// ============================================================================
// Initialization
// ============================================================================

void UDeskillzPushNotifications::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}
	
	// Platform-specific initialization
#if PLATFORM_IOS || PLATFORM_ANDROID
	RegisterForRemoteNotifications();
#endif
	
	bIsInitialized = true;
	
	UE_LOG(LogDeskillz, Log, TEXT("Push Notifications initialized"));
}

void UDeskillzPushNotifications::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}
	
	// Unregister token if we have one
	if (HasPushToken())
	{
		UnregisterTokenFromServer();
	}
	
	bIsInitialized = false;
	UE_LOG(LogDeskillz, Log, TEXT("Push Notifications shutdown"));
}

// ============================================================================
// Permissions
// ============================================================================

void UDeskillzPushNotifications::RequestPermission()
{
#if PLATFORM_IOS
	// iOS-specific permission request
	// Would call native iOS code for UNUserNotificationCenter authorization
	UE_LOG(LogDeskillz, Log, TEXT("Requesting iOS notification permission"));
	
	// Simulated async response
	OnPermissionResult(true);
	
#elif PLATFORM_ANDROID
	// Android 13+ requires POST_NOTIFICATIONS permission
	// For older versions, permission is granted by default
	UE_LOG(LogDeskillz, Log, TEXT("Requesting Android notification permission"));
	
	// Android < 13 doesn't require explicit permission
	OnPermissionResult(true);
	
#else
	// Desktop platforms - no permission needed
	PermissionStatus = EDeskillzNotificationPermission::Authorized;
	OnPermissionChanged.Broadcast(PermissionStatus);
#endif
}

void UDeskillzPushNotifications::OpenNotificationSettings()
{
#if PLATFORM_IOS
	// Open iOS Settings app to notification settings
	FPlatformProcess::LaunchURL(TEXT("app-settings:"), nullptr, nullptr);
#elif PLATFORM_ANDROID
	// Open Android notification settings for the app
	// Would require native code to open Settings.ACTION_APP_NOTIFICATION_SETTINGS
	UE_LOG(LogDeskillz, Log, TEXT("Opening notification settings"));
#endif
}

// ============================================================================
// Token Management
// ============================================================================

void UDeskillzPushNotifications::RegisterTokenWithServer()
{
	if (!HasPushToken())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot register - no push token available"));
		return;
	}
	
	UDeskillzPlatform* Platform = UDeskillzPlatform::Get();
	
	// Build registration payload
	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	Payload->SetStringField(TEXT("token"), PushToken);
	Payload->SetStringField(TEXT("platform"), Platform->GetPlatformName());
	Payload->SetStringField(TEXT("device_id"), Platform->GetDeviceId());
	
#if PLATFORM_IOS
	Payload->SetStringField(TEXT("service"), TEXT("apns"));
#elif PLATFORM_ANDROID
	Payload->SetStringField(TEXT("service"), TEXT("fcm"));
#endif
	
	// Send to server
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->PostJson(TEXT("/api/v1/notifications/register"), Payload,
		FOnDeskillzHttpResponse::CreateLambda([](const FDeskillzHttpResponse& Response)
		{
			if (Response.IsOk())
			{
				UE_LOG(LogDeskillz, Log, TEXT("Push token registered with server"));
			}
			else
			{
				UE_LOG(LogDeskillz, Warning, TEXT("Failed to register push token: %s"), *Response.ErrorMessage);
			}
		})
	);
}

void UDeskillzPushNotifications::UnregisterTokenFromServer()
{
	if (!HasPushToken())
	{
		return;
	}
	
	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	Payload->SetStringField(TEXT("token"), PushToken);
	
	UDeskillzHttpClient* Http = UDeskillzHttpClient::Get();
	Http->PostJson(TEXT("/api/v1/notifications/unregister"), Payload,
		FOnDeskillzHttpResponse::CreateLambda([](const FDeskillzHttpResponse& Response)
		{
			UE_LOG(LogDeskillz, Log, TEXT("Push token unregistered"));
		})
	);
}

// ============================================================================
// Local Notifications
// ============================================================================

void UDeskillzPushNotifications::ScheduleLocalNotification(const FDeskillzLocalNotification& Notification)
{
	if (!AreNotificationsEnabled())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Cannot schedule notification - permission not granted"));
		return;
	}
	
#if PLATFORM_IOS || PLATFORM_ANDROID
	// Platform-specific local notification scheduling
	// Would use UNUserNotificationCenter on iOS
	// Would use NotificationManager on Android
	
	UE_LOG(LogDeskillz, Log, TEXT("Scheduling local notification: %s (delay: %.1fs)"), 
		*Notification.Identifier, Notification.DelaySeconds);
	
	PendingNotificationIds.AddUnique(Notification.Identifier);
#else
	UE_LOG(LogDeskillz, Verbose, TEXT("Local notifications not supported on this platform"));
#endif
}

void UDeskillzPushNotifications::CancelLocalNotification(const FString& Identifier)
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	PendingNotificationIds.Remove(Identifier);
	UE_LOG(LogDeskillz, Log, TEXT("Cancelled local notification: %s"), *Identifier);
#endif
}

void UDeskillzPushNotifications::CancelAllLocalNotifications()
{
#if PLATFORM_IOS || PLATFORM_ANDROID
	PendingNotificationIds.Empty();
	UE_LOG(LogDeskillz, Log, TEXT("Cancelled all local notifications"));
#endif
}

TArray<FString> UDeskillzPushNotifications::GetPendingLocalNotifications() const
{
	return PendingNotificationIds;
}

// ============================================================================
// Badge
// ============================================================================

void UDeskillzPushNotifications::SetBadgeCount(int32 Count)
{
	BadgeCount = FMath::Max(0, Count);
	
#if PLATFORM_IOS
	// Set iOS app badge
	// Would call native code: [[UIApplication sharedApplication] setApplicationIconBadgeNumber:Count]
	UE_LOG(LogDeskillz, Verbose, TEXT("Set badge count: %d"), BadgeCount);
#elif PLATFORM_ANDROID
	// Android badge depends on launcher support
	UE_LOG(LogDeskillz, Verbose, TEXT("Set badge count: %d (launcher dependent)"), BadgeCount);
#endif
}

// ============================================================================
// Notification Handling
// ============================================================================

void UDeskillzPushNotifications::HandleNotificationReceived(const FDeskillzNotificationPayload& Notification)
{
	UE_LOG(LogDeskillz, Log, TEXT("Notification received: %s - %s"), 
		*Notification.Title, *Notification.Body);
	
	OnNotificationReceived.Broadcast(Notification);
}

void UDeskillzPushNotifications::HandleNotificationOpened(const FDeskillzNotificationPayload& Notification)
{
	UE_LOG(LogDeskillz, Log, TEXT("Notification opened: %s"), *Notification.NotificationId);
	
	OnNotificationOpened.Broadcast(Notification);
	
	// Handle deep link if present
	if (!Notification.DeepLink.IsEmpty())
	{
		UDeskillzDeepLink* DeepLink = UDeskillzDeepLink::Get();
		DeepLink->HandleDeepLink(Notification.DeepLink);
	}
}

bool UDeskillzPushNotifications::GetLaunchNotification(FDeskillzNotificationPayload& OutNotification)
{
	if (!bHasLaunchNotification)
	{
		return false;
	}
	
	OutNotification = LaunchNotification;
	bHasLaunchNotification = false; // Clear after retrieval
	return true;
}

// ============================================================================
// Topic Subscription
// ============================================================================

void UDeskillzPushNotifications::SubscribeToTopic(const FString& Topic)
{
#if PLATFORM_ANDROID
	// Firebase topic subscription
	UE_LOG(LogDeskillz, Log, TEXT("Subscribing to topic: %s"), *Topic);
	SubscribedTopics.AddUnique(Topic);
#else
	UE_LOG(LogDeskillz, Verbose, TEXT("Topic subscription not available on this platform"));
#endif
}

void UDeskillzPushNotifications::UnsubscribeFromTopic(const FString& Topic)
{
#if PLATFORM_ANDROID
	UE_LOG(LogDeskillz, Log, TEXT("Unsubscribing from topic: %s"), *Topic);
	SubscribedTopics.Remove(Topic);
#endif
}

// ============================================================================
// Internal Methods
// ============================================================================

void UDeskillzPushNotifications::RegisterForRemoteNotifications()
{
#if PLATFORM_IOS
	// iOS remote notification registration
	// Would call: [[UIApplication sharedApplication] registerForRemoteNotifications]
	UE_LOG(LogDeskillz, Log, TEXT("Registering for iOS remote notifications"));
#elif PLATFORM_ANDROID
	// Firebase Cloud Messaging registration
	// Would call: FirebaseMessaging.getInstance().getToken()
	UE_LOG(LogDeskillz, Log, TEXT("Registering for FCM"));
#endif
}

void UDeskillzPushNotifications::OnTokenReceived(const FString& Token)
{
	PushToken = Token;
	
	UE_LOG(LogDeskillz, Log, TEXT("Push token received: %s..."), 
		*Token.Left(FMath::Min(20, Token.Len())));
	
	OnPushTokenReceived.Broadcast(Token);
	
	// Auto-register with server
	RegisterTokenWithServer();
}

void UDeskillzPushNotifications::OnPermissionResult(bool bGranted)
{
	PermissionStatus = bGranted ? 
		EDeskillzNotificationPermission::Authorized : 
		EDeskillzNotificationPermission::Denied;
	
	UE_LOG(LogDeskillz, Log, TEXT("Notification permission: %s"), 
		bGranted ? TEXT("Granted") : TEXT("Denied"));
	
	OnPermissionChanged.Broadcast(PermissionStatus);
}

EDeskillzNotificationType UDeskillzPushNotifications::ParseNotificationType(const TMap<FString, FString>& Data) const
{
	const FString* TypeStr = Data.Find(TEXT("type"));
	if (!TypeStr)
	{
		return EDeskillzNotificationType::Generic;
	}
	
	FString LowerType = TypeStr->ToLower();
	
	if (LowerType == TEXT("tournament_starting"))
	{
		return EDeskillzNotificationType::TournamentStarting;
	}
	if (LowerType == TEXT("tournament_results"))
	{
		return EDeskillzNotificationType::TournamentResults;
	}
	if (LowerType == TEXT("match_found"))
	{
		return EDeskillzNotificationType::MatchFound;
	}
	if (LowerType == TEXT("match_reminder"))
	{
		return EDeskillzNotificationType::MatchReminder;
	}
	if (LowerType == TEXT("friend"))
	{
		return EDeskillzNotificationType::FriendActivity;
	}
	if (LowerType == TEXT("prize"))
	{
		return EDeskillzNotificationType::PrizeWon;
	}
	if (LowerType == TEXT("promo"))
	{
		return EDeskillzNotificationType::Promotional;
	}
	if (LowerType == TEXT("system"))
	{
		return EDeskillzNotificationType::System;
	}
	
	return EDeskillzNotificationType::Generic;
}
