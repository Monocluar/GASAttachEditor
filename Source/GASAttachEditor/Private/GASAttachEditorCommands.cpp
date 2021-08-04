// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAttachEditorCommands.h"

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorCommands::RegisterCommands()
{
	UI_COMMAND(ShowGASAttachEditorViewer, /*"查看角色携带GA"*/"View GA Carried By Role", "Bring up GASAttachEditor window", EUserInterfaceActionType::Check, FInputChord());
#if WITH_EDITOR
	UI_COMMAND(ShowGASTagLookAssetViewer, /*"查看可Tag调用的GA"*/"View The GA That Can Be Called By Tag", "Bring up GASTagLookAsset window", EUserInterfaceActionType::Check, FInputChord());
#endif
}

#undef LOCTEXT_NAMESPACE
