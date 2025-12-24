// Copyright Deskillz Games. All Rights Reserved.

using UnrealBuildTool;

public class Deskillz : ModuleRules
{
	public Deskillz(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// C++17 for modern features
		CppStandard = CppStandardVersion.Cpp17;
		
		// Include paths
		PublicIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Public"
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Private"
			}
		);

		// Public dependencies (exposed to other modules)
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Json",
				"JsonUtilities",
				"HTTP",
				"WebSockets",
				"Sockets",
				"Networking",
				"UMG",
				"Slate",
				"SlateCore"
			}
		);

		// Private dependencies (internal only)
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"DeveloperSettings",
				"RenderCore"
			}
		);
		
		// Platform-specific dependencies
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("AndroidPermission");
			PrivateDependencyModuleNames.Add("Launch");
		}
		
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PublicFrameworks.AddRange(
				new string[]
				{
					"Security",
					"CommonCrypto"
				}
			);
		}

		// Encryption library for score security (AES-256-GCM + HMAC-SHA256)
		PublicDefinitions.Add("WITH_SSL=1");
		
		// Add OpenSSL for encryption (if available)
		AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");
		
		// Enable exceptions for error handling
		bEnableExceptions = true;
		
		// RTTI for dynamic casting
		bUseRTTI = true;
	}
}
