// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAttachEditor.h"
#include "GASAttachEditorStyle.h"
#include "GASAttachEditorCommands.h"
//#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#if WITH_EDITOR
#include "ToolMenus.h"
#endif
#include "SGASAttachEditor.h"

static const FName GASAttachEditorTabName("GASAttachEditor");

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if !UE_SERVER
	FGASAttachEditorStyle::Initialize();
	FGASAttachEditorStyle::ReloadTextures();

	FGASAttachEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FGASAttachEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FGASAttachEditorModule::PluginButtonClicked),
		FCanExecuteAction());
#if WITH_EDITOR
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGASAttachEditorModule::RegisterMenus));
#endif
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GASAttachEditorTabName, FOnSpawnTab::CreateRaw(this, &FGASAttachEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FGASAttachEditorTabTitle", "查看角色携带GA"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
#endif
	
}

void FGASAttachEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if !UE_SERVER
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName(TEXT("GameplayCueApp")));

		if (GameplayCheckEditorTab.IsValid())
		{
			GameplayCheckEditorTab.Pin()->RequestCloseTab();
		}
	}
#if WITH_EDITOR
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);
#endif
	FGASAttachEditorStyle::Shutdown();

	FGASAttachEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GASAttachEditorTabName);
#endif
}

TSharedRef<SDockTab> FGASAttachEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{

	return SAssignNew(GameplayCheckEditorTab, SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			CreateGASCheckTool().ToSharedRef()
		];
}

TSharedPtr<SWidget> FGASAttachEditorModule::CreateGASCheckTool()
{
	TSharedPtr<SWidget> ReturnWidget;
	if (IsInGameThread())
	{
		TSharedPtr<SGASAttachEditor> SharedPtr = SNew(SGASAttachEditor);
		ReturnWidget = SharedPtr;
	}
	return ReturnWidget;
}

static void GASAttachEditorShow(UWorld* InWorld)
{
	FGlobalTabmanager::Get()->TryInvokeTab(GASAttachEditorTabName);
}

FAutoConsoleCommandWithWorld AbilitySystemEditoeDebugNextCategoryCmd(
	TEXT("GASAttachEditorShow"),
	TEXT("打开查看角色GA的编辑器"),
	FConsoleCommandWithWorldDelegate::CreateStatic(GASAttachEditorShow)
);

void FGASAttachEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(GASAttachEditorTabName);
}

void FGASAttachEditorModule::RegisterMenus()
{
#if WITH_EDITOR
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowGlobalTabSpawners");
			Section.AddMenuEntryWithCommandList(FGASAttachEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}
#endif
	/*{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FGASAttachEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}*/
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGASAttachEditorModule, GASAttachEditor)