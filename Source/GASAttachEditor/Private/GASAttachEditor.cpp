// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAttachEditor.h"
#include "GASAttachEditorStyle.h"
#include "GASAttachEditorCommands.h"
//#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "SGASAttachEditor.h"
#if WITH_EDITOR
#include "SGASTagLookAsset.h"
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"
#include "ToolMenus.h"
#endif
#include "Framework/Commands/UICommandList.h"
#include "Templates/SharedPointer.h"
#include "Framework/Docking/LayoutService.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/MenuStack.h"
#include "Widgets/Layout/SBorder.h"
#if WITH_EDITOR
#include "EditorStyleSet.h"
#endif
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName GASAttachEditorTabName("GASAttachEditor");

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

void FGASAttachEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FGASAttachEditorStyle::Initialize();
	FGASAttachEditorStyle::ReloadTextures();

	FGASAttachEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
#if WITH_EDITOR
	const IWorkspaceMenuStructure& MenuStructure =  WorkspaceMenu::GetMenuStructure();
#endif
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GASAttachEditorTabName, FOnSpawnTab::CreateRaw(this, &FGASAttachEditorModule::OnSpawnPluginTab))
		/*.SetDisplayName(LOCTEXT("FGASAttachEditorTabTitle", "查看角色携带GA"))
		.SetTooltipText(LOCTEXT("FGASAttachEditorTooltipText", "打开“查看角色携带GA”选项卡"))*/
		.SetDisplayName(LOCTEXT("FGASAttachEditorTabTitle", "Debug Gameplay Ability System"))
		.SetTooltipText(LOCTEXT("FGASAttachEditorTooltipText", "Open the Debug Gameplay Ability System tab"))
#if WITH_EDITOR
		.SetGroup(MenuStructure.GetDeveloperToolsDebugCategory())
#endif
		.SetIcon(FSlateIcon(FGASAttachEditorStyle::GetStyleSetName(), "GASAttachEditor.OpenPluginWindow"));

}

void FGASAttachEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.


#if WITH_EDITOR
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);
#endif
	FGASAttachEditorStyle::Shutdown();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GASAttachEditorTabName);

	if (GASEditorTabManager.IsValid())
	{
		FGlobalTabmanager::Get()->UnregisterTabSpawner(GASAttachEditorTabName);
		GASEditorTabLayout = TSharedPtr<FTabManager::FLayout>();
		GASEditorTabManager = TSharedPtr<FTabManager>();
	}

	FGASAttachEditorCommands::Unregister();
}

TSharedRef<SDockTab> FGASAttachEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> NomadTab = SAssignNew(GameplayCheckEditorTab, SDockTab)
		.TabRole(ETabRole::NomadTab);

	if (!GASEditorTabManager.IsValid())
	{
		GASEditorTabManager = FGlobalTabmanager::Get()->NewTabManager(NomadTab);

		GASEditorTabManager->SetOnPersistLayout(
			FTabManager::FOnPersistLayout::CreateStatic(
				[](const TSharedRef<FTabManager::FLayout>& InLayout)
				{
					if (InLayout->GetPrimaryArea().Pin().IsValid())
					{
						FLayoutSaveRestore::SaveToConfig(GEditorLayoutIni, InLayout);
					}
				}
			)
		);
	}
	else
	{
		ensure(GASEditorTabLayout.IsValid());
	}

	TWeakPtr<FTabManager> GASEditorTabManagerWeak = GASEditorTabManager;

	const FName GAAttachEditorName = SGASAttachEditor::GetTabName();
#if WITH_EDITOR
	const FName GASTagLookAssetName = SGASTagLookAsset::GetTabName();
#endif

	NomadTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateStatic(
		[](TSharedRef<SDockTab> Self, TWeakPtr<FTabManager> TabManager)
		{
			TSharedPtr<FTabManager> OwningTabManager = TabManager.Pin();
			if (OwningTabManager.IsValid())
			{
				FLayoutSaveRestore::SaveToConfig(GEditorLayoutIni, OwningTabManager->PersistLayout());
				OwningTabManager->CloseAllAreas();
			}
		}
		, GASEditorTabManagerWeak
		));

	if (!GASEditorTabLayout.IsValid())
	{

		SGASAttachEditor::RegisterTabSpawner(*GASEditorTabManager);

#if WITH_EDITOR
		SGASTagLookAsset::RegisterTabSpawner(*GASEditorTabManager);
#endif

		GASEditorTabLayout = FTabManager::NewLayout("Standalone_GASAttachEditor_Layout_v1")
			->AddArea
			(
				FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(.4f)
					->SetHideTabWell(true)
					->AddTab(GAAttachEditorName, ETabState::OpenedTab)
#if WITH_EDITOR
					->AddTab(GASTagLookAssetName, ETabState::OpenedTab)
#endif
					->SetForegroundTab(GAAttachEditorName)
				)
			);
	}

	GASEditorTabLayout = FLayoutSaveRestore::LoadFromConfig(GEditorLayoutIni, GASEditorTabLayout.ToSharedRef());

	TSharedRef<SWidget> TabContents = GASEditorTabManager->RestoreFrom(GASEditorTabLayout.ToSharedRef(), TSharedPtr<SWindow>()).ToSharedRef();


	TWeakPtr<FTabManager> GASEditorManagerWeak = GASEditorTabManager;

	const auto ToggleTabVisibility = [](TWeakPtr<FTabManager> InDebuggingToolsManagerWeak, FName InTabName)
	{
		TSharedPtr<FTabManager> InDebuggingToolsManager = InDebuggingToolsManagerWeak.Pin();
		if (InDebuggingToolsManager.IsValid())
		{
			TSharedPtr<SDockTab> ExistingTab = InDebuggingToolsManager->FindExistingLiveTab(InTabName);
			if (ExistingTab.IsValid())
			{
				ExistingTab->RequestCloseTab();
			}
			else
			{
				InDebuggingToolsManager->TryInvokeTab(InTabName);
			}
		}
	};

	const auto IsTabVisible = [](TWeakPtr<FTabManager> InDebuggingToolsManagerWeak, FName InTabName)
	{
		TSharedPtr<FTabManager> InDebuggingToolsManager = InDebuggingToolsManagerWeak.Pin();
		if (InDebuggingToolsManager.IsValid())
		{
			return InDebuggingToolsManager->FindExistingLiveTab(InTabName).IsValid();
		}
		return false;
	};

	PluginCommands->MapAction(
		FGASAttachEditorCommands::Get().ShowGASAttachEditorViewer,
		FExecuteAction::CreateStatic(
			ToggleTabVisibility,
			GASEditorManagerWeak,
			GAAttachEditorName
		),
		FCanExecuteAction::CreateStatic(
			[]() { return true; }
		),
		FIsActionChecked::CreateStatic(
			IsTabVisible,
			GASEditorManagerWeak,
			GAAttachEditorName
		)
	);

#if WITH_EDITOR
	PluginCommands->MapAction(
		FGASAttachEditorCommands::Get().ShowGASTagLookAssetViewer,
		FExecuteAction::CreateStatic(
			ToggleTabVisibility,
			GASEditorManagerWeak,
			GASTagLookAssetName
		),
		FCanExecuteAction::CreateStatic(
			[]() { return true; }
		),
		FIsActionChecked::CreateStatic(
			IsTabVisible,
			GASEditorManagerWeak,
			GASTagLookAssetName
		)
	);
#endif

	TWeakPtr<SWidget> OwningWidgetWeak = NomadTab;
	TabContents->SetOnMouseButtonUp(
		FPointerEventHandler::CreateStatic(
			[]( /** The geometry of the widget*/
				const FGeometry&,
				/** The Mouse Event that we are processing */
				const FPointerEvent& PointerEvent,
				TWeakPtr<SWidget> InOwnerWeak,
				TSharedPtr<FUICommandList> InCommandList) -> FReply
	{
		if (PointerEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			// if the tab manager is still available then make a context window that allows users to
			// show and hide tabs:
			TSharedPtr<SWidget> InOwner = InOwnerWeak.Pin();
			if (InOwner.IsValid())
			{
				FMenuBuilder MenuBuilder(true, InCommandList);

				MenuBuilder.PushCommandList(InCommandList.ToSharedRef());
				{
					MenuBuilder.AddMenuEntry(FGASAttachEditorCommands::Get().ShowGASAttachEditorViewer);
#if WITH_EDITOR
					MenuBuilder.AddMenuEntry(FGASAttachEditorCommands::Get().ShowGASTagLookAssetViewer);
#endif
				}
				MenuBuilder.PopCommandList();


				FWidgetPath WidgetPath = PointerEvent.GetEventPath() != nullptr ? *PointerEvent.GetEventPath() : FWidgetPath();
				FSlateApplication::Get().PushMenu(InOwner.ToSharedRef(), WidgetPath, MenuBuilder.MakeWidget(), PointerEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

				return FReply::Handled();
			}
		}

		return FReply::Unhandled();
	}
			, OwningWidgetWeak
		, PluginCommands
		)
	);

	NomadTab->SetContent(
		SNew(SBorder)
#if WITH_EDITOR
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
#endif
		.Padding(FMargin(0.f, 2.f))
		[
			TabContents
		]
	);

	return NomadTab;
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
	//TEXT("打开查看角色GA的编辑器"),
	TEXT("Open the editor for viewing character GA"),
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
			Section.AddMenuEntryWithCommandList(FGASAttachEditorCommands::Get().ShowGASAttachEditorViewer, PluginCommands);
		}
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGASAttachEditorModule, GASAttachEditor)