// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAttachEditorCommands.h"

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorCommands::RegisterCommands()
{
	UI_COMMAND(ShowGASAttachEditorViewer, /*"查看角色携带GA"*/"Debug Gameplay Ability System", "Open the Debug Gameplay Ability System tab", EUserInterfaceActionType::Check, FInputChord());
#if WITH_EDITOR
	UI_COMMAND(ShowGASTagLookAssetViewer, /*"查看可Tag调用的GA"*/"View CallByTag Abilities", "Open the GASTagLookAsset tab", EUserInterfaceActionType::Check, FInputChord());
#endif
}

#undef LOCTEXT_NAMESPACE
