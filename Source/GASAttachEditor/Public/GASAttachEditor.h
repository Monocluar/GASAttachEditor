// <Copyright (C) Monocluar. 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"


class FToolBarBuilder;
class FMenuBuilder;
class SWidget;
class SDockTab;

class FGASAttachEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<SWidget> CreateGASCheckTool();

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	TWeakPtr<SDockTab> GameplayCheckEditorTab;

	// <所有Tab控件管理
	// Manage all tab controls
	TSharedPtr<FTabManager> GASEditorTabManager;

	// <所有Tab层级管理
	// All tab level management
	TSharedPtr<FTabManager::FLayout> GASEditorTabLayout;
};
