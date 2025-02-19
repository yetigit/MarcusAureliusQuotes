// Copyright Baidhir Hidair, 2025. All Rights Reserved.

using UnrealBuildTool;

public class MarcusAureliusQuotes : ModuleRules
{
	public MarcusAureliusQuotes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
        "ApplicationCore",
        "Core",
        "CoreUObject",
        "Engine",
        "HTTP",
        "Json",
        "JsonUtilities",
        "UnrealEd",
        "LevelEditor",
        "Slate",
        "SlateCore", 
        "Projects",
        "DeveloperSettings",
        "Settings",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
