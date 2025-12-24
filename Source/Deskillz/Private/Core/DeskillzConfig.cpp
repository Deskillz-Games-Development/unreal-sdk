// Copyright Deskillz Games. All Rights Reserved.

#include "Core/DeskillzConfig.h"
#include "Deskillz.h"

#define LOCTEXT_NAMESPACE "DeskillzConfig"

UDeskillzConfig::UDeskillzConfig()
{
	// Default configuration values are set in header
	// This constructor can be used for any runtime initialization
}

UDeskillzConfig* UDeskillzConfig::Get()
{
	return GetMutableDefault<UDeskillzConfig>();
}

FDeskillzEndpoints UDeskillzConfig::GetActiveEndpoints() const
{
	if (bUseCustomEndpoints)
	{
		return CustomEndpoints;
	}
	
	return FDeskillzEndpoints::ForEnvironment(Environment);
}

bool UDeskillzConfig::IsValid() const
{
	// Check required fields
	if (APIKey.IsEmpty())
	{
		return false;
	}
	
	if (GameId.IsEmpty())
	{
		return false;
	}
	
	// API key format validation (should be 32+ characters)
	if (APIKey.Len() < 32)
	{
		return false;
	}
	
	// Game ID format validation (should be UUID-like or alphanumeric)
	if (GameId.Len() < 8)
	{
		return false;
	}
	
	// Score range validation
	if (MinScore >= MaxScore)
	{
		return false;
	}
	
	return true;
}

FString UDeskillzConfig::GetValidationError() const
{
	if (APIKey.IsEmpty())
	{
		return TEXT("API Key is required. Get your key from the Deskillz Developer Portal.");
	}
	
	if (APIKey.Len() < 32)
	{
		return TEXT("API Key appears to be invalid. Please check your key from the Developer Portal.");
	}
	
	if (GameId.IsEmpty())
	{
		return TEXT("Game ID is required. Get your Game ID from the Deskillz Developer Portal.");
	}
	
	if (GameId.Len() < 8)
	{
		return TEXT("Game ID appears to be invalid. Please check your Game ID from the Developer Portal.");
	}
	
	if (MinScore >= MaxScore)
	{
		return TEXT("Minimum score must be less than maximum score.");
	}
	
	return TEXT("");
}

#if WITH_EDITOR

FText UDeskillzConfig::GetSectionText() const
{
	return LOCTEXT("SectionText", "Deskillz SDK");
}

FText UDeskillzConfig::GetSectionDescription() const
{
	return LOCTEXT("SectionDescription", "Configure the Deskillz competitive gaming SDK for tournament play, cryptocurrency prizes, and real-time multiplayer.");
}

void UDeskillzConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) 
		? PropertyChangedEvent.Property->GetFName() 
		: NAME_None;
	
	// Log config changes in editor
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UDeskillzConfig, Environment))
	{
		FString EnvName;
		switch (Environment)
		{
			case EDeskillzEnvironment::Production: EnvName = TEXT("Production"); break;
			case EDeskillzEnvironment::Sandbox: EnvName = TEXT("Sandbox"); break;
			case EDeskillzEnvironment::Development: EnvName = TEXT("Development"); break;
		}
		UE_LOG(LogDeskillz, Log, TEXT("Environment changed to: %s"), *EnvName);
		
		// Auto-update endpoints when environment changes
		if (!bUseCustomEndpoints)
		{
			CustomEndpoints = FDeskillzEndpoints::ForEnvironment(Environment);
		}
	}
	
	// Validate configuration
	if (!IsValid())
	{
		UE_LOG(LogDeskillz, Warning, TEXT("Deskillz Config Validation: %s"), *GetValidationError());
	}
}

#endif

#undef LOCTEXT_NAMESPACE
