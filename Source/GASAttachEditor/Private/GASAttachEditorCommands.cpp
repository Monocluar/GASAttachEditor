// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAttachEditorCommands.h"

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "查看角色携带GA", "Bring up GASAttachEditor window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
