// <Copyright (C) Monocluar. 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "GASAttachEditorStyle.h"

class FGASAttachEditorCommands : public TCommands<FGASAttachEditorCommands>
{
public:

	FGASAttachEditorCommands()
		: TCommands<FGASAttachEditorCommands>(TEXT("GASAttachEditor"), NSLOCTEXT("Contexts", "GASAttachEditor", /*"查看角色携带GA插件"*/"View GA Plugins Carried By Roles"), NAME_None, FGASAttachEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > ShowGASAttachEditorViewer;

#if WITH_EDITOR
	TSharedPtr< FUICommandInfo > ShowGASTagLookAssetViewer;
#endif

};