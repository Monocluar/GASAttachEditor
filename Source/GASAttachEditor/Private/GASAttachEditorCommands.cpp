// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAttachEditorCommands.h"

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorCommands::RegisterCommands()
{
	//UI_COMMAND(ShowGASAttachEditorViewer, "查看角色携带GA", "Bring up GASAttachEditor window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(ShowGASAttachEditorViewer, "查看角色携带GA", "Bring up GASAttachEditor window",  EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(ShowGASTagLookAssetViewer, "查看可Tag调用的GA", "Bring up GASTagLookAsset window",  EUserInterfaceActionType::Check, FInputChord());
}

#undef LOCTEXT_NAMESPACE
