// <Copyright (C) Monocluar. 2021. All Rights Reserved.

#include "GASAttachEditorCommands.h"

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorCommands::RegisterCommands()
{
	UI_COMMAND(ShowGASAttachEditorViewer, "View GA Carried By Role", "Bring up GASAttachEditor window", EUserInterfaceActionType::Check, FInputChord());
#if WITH_EDITOR
	UI_COMMAND(ShowGASTagLookAssetViewer, "View The GA That Can Be Called By Tag", "Bring up GASTagLookAsset window", EUserInterfaceActionType::Check, FInputChord());
#endif
}

#undef LOCTEXT_NAMESPACE
