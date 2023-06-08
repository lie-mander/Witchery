// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class WitcheryEditorTarget : TargetRules
{
	public WitcheryEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "Witchery" } );
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_2; // 
    }
}
