#include "SGASAttachEditor.h"
#include "Widgets/SBoxPanel.h"
#include "GASAttachEditor/SGASReflectorNodeBase.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "GameplayAbilitySpec.h"
#include "../Public/GASAttachEditorStyle.h"
#include "Widgets/Views/STreeView.h"
#include "GASAttachEditor/SGASCharacterTagsBase.h"
#include "GASAttachEditor/SGASAttributesNodeBase.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Input/SComboButton.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagsManager.h"
#if WITH_EDITOR
#include "SGameplayTagWidget.h"
#include "EditorStyleSet.h"
#endif
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "GASAttachEditor/SGASGameplayEffectNodeBase.h"
#include "Misc/ConfigCacheIni.h"
#include "Widgets/SWidget.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Input/SCheckBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UIAction.h"
#include "HAL/ExceptionHandling.h"
#include "Widgets/Input/SButton.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/Pawn.h"

#define LOCTEXT_NAMESPACE "SGASAttachEditor"

const FName GAActivationColumnName("GAActivation");

struct FASCDebugTargetInfo
{
	FASCDebugTargetInfo()
	{
		DebugCategoryIndex = 0;
		DebugCategories.Add(TEXT("Attributes"));
		DebugCategories.Add(TEXT("GameplayEffects"));
		DebugCategories.Add(TEXT("Ability"));
	}

	TArray<FName> DebugCategories;
	int32 DebugCategoryIndex;

	TWeakObjectPtr<UWorld>	TargetWorld;
	TWeakObjectPtr<UAbilitySystemComponent>	LastDebugTarget;
};

TArray<FASCDebugTargetInfo>	AbilitySystemAttachDebugInfoList;

FASCDebugTargetInfo* GetASCDebugTargetInfo(UWorld* World)
{
	FASCDebugTargetInfo* TargetInfo = nullptr;
	for (FASCDebugTargetInfo& Info : AbilitySystemAttachDebugInfoList)
	{
		if (Info.TargetWorld.Get() == World)
		{
			TargetInfo = &Info;
			break;
		}
	}
	if (TargetInfo == nullptr)
	{
		TargetInfo = &AbilitySystemAttachDebugInfoList[AbilitySystemAttachDebugInfoList.AddDefaulted()];
		TargetInfo->TargetWorld = World;
	}
	return TargetInfo;
}

TArray<TWeakObjectPtr< UAbilitySystemComponent>> PlayerComp;

void UpDataPlayerComp(UWorld* World)
{
	PlayerComp.Reset();

	if (!World)
	{
		return;
	}

	for (TObjectIterator<UAbilitySystemComponent> It; It; ++It)
	{
		if (UAbilitySystemComponent* ASC = *It)
		{
			if (ASC->GetWorld() != World)
			{
				continue;
			}
			if (!MakeWeakObjectPtr(ASC).Get())
			{
				continue;
			}
			PlayerComp.Add(ASC);
		}
	}
}

AActor* GetGASActor(const TWeakObjectPtr<UAbilitySystemComponent>& InASC)
{
	if (!InASC.IsValid())
	{
		return nullptr;
	}

	;
	if (AActor* LocalAvatarActor = InASC->GetAvatarActor_Direct())
	{
		return LocalAvatarActor;
	}

 	if (AActor* LocalOwnerActor = InASC->GetOwnerActor())
	{
		return LocalOwnerActor;
	}

	return nullptr;
}

UAbilitySystemComponent* GetDebugTarget(FASCDebugTargetInfo* Info, const UAbilitySystemComponent* InSelectComponent, FName& SelectActorName)
{
	// Return target if we already have one
	if (UAbilitySystemComponent* ASC = Info->LastDebugTarget.Get())
	{
		if (ASC == InSelectComponent)
		{
			return ASC;
		}
	}

	// Find one
	bool bIsSelect = false;

	for (TWeakObjectPtr<UAbilitySystemComponent>& ASC : PlayerComp)
	{
		if (!ASC.IsValid())
		{
			continue;
		}

		if (InSelectComponent == ASC.Get() && !bIsSelect)
		{
			SelectActorName = GetGASActor(ASC)->GetFName();
			Info->LastDebugTarget = ASC;
			bIsSelect = true;
			break;
		}
	}

	if (!bIsSelect)
	{
		if (!PlayerComp.IsValidIndex(0))
		{
			// 刷新世界中的ASComp;
			// Refresh ascomp in the world;
			UpDataPlayerComp(Info->TargetWorld.Get());
		}

		// 如果筛选框里面有筛选的名字，则直接用那个名字的角色
		// If there is a filtered name in the filter box, the role with that name will be used directly
		if (SelectActorName.IsNone())
		{
			if (PlayerComp.IsValidIndex(0))
			{
				Info->LastDebugTarget = PlayerComp[0];
				SelectActorName = GetGASActor(Info->LastDebugTarget)->GetFName();
			}
			else
			{
				Info->LastDebugTarget = nullptr;
			}
		}
		else
		{
			bool bIsSelectActorName = false;
			for (TWeakObjectPtr<UAbilitySystemComponent>& ASC : PlayerComp)
			{
				if (!ASC.IsValid()) continue;

				if (GetGASActor(ASC)->GetFName() == SelectActorName)
				{
					Info->LastDebugTarget = ASC;
					bIsSelectActorName = true;
					break;
				}

			}

			if (!bIsSelectActorName)
			{
				if (PlayerComp.IsValidIndex(0) && PlayerComp[0] != nullptr)
				{
					Info->LastDebugTarget = PlayerComp[0];
					SelectActorName = GetGASActor(PlayerComp[0])->GetFName();
				}
				else
				{
					Info->LastDebugTarget = nullptr;
				}
			}
		}

	}

	if (Info->LastDebugTarget.IsValid())
	{
		return Info->LastDebugTarget.Get();
	}

	return nullptr;
}

class SGASAttachEditorImpl : public SGASAttachEditor
{
	typedef STreeView<TSharedRef<FGASAbilitieNodeBase>> SAbilitieTree;
	typedef STreeView<TSharedRef<FGASAttributesNodeBase>> SAttributesTree;
	typedef STreeView<TSharedRef<FGASGameplayEffectNodeBase>> SGameplayEffectTree;

public:
	virtual void Construct(const FArguments& InArgs) override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:
	virtual ~SGASAttachEditorImpl() override;


protected:
	// 当前筛选世界场景
	// Current filter world scene
	TSharedRef<SWidget>	OnGetShowWorldTypeMenu();

	// 选中世界场景
	// Select the world scene
	void HandleShowWorldTypeChange(FWorldContext InWorldContext);

private:
	// 当期选择的世界场景句柄
	// Current selected world scene handle
	FName SelectWorldSceneConetextHandle;

	FText SelectWorldSceneText;

protected:

	FText GenerateToolTipForText(TSharedRef<FGASAbilitieNodeBase> InReflectorNode) const;

protected:
	/** 树视图生成树的回调 */
	/** Callback for tree view spanning tree */
	TSharedRef<ITableRow> OnGenerateWidgetForFilterListView(TSharedRef< FGASAbilitieNodeBase > InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/** 用于获取给定反射树节点的子项的回调 */
	/** Callback to get children of a given reflection tree node */
	void HandleReflectorTreeGetChildren( TSharedRef<FGASAbilitieNodeBase> InReflectorNode, TArray<TSharedRef<FGASAbilitieNodeBase>>& OutChildren );

	/** 当反射树中的选择发生更改时的回调 */
	/** Callback when the selection in the reflection tree changes */
	void HandleReflectorTreeSelectionChanged(TSharedPtr<FGASAbilitieNodeBase>, ESelectInfo::Type /*SelectInfo*/);

	// 提示小部件
	// Prompt widget
	TSharedPtr<IToolTip> GenerateToolTipForReflectorNode( TSharedRef<FGASAbilitieNodeBase> InReflectorNode );

	// 当请求反射器树中的上下文菜单时的回调
	// Callback when a context menu in the reflector tree is requested
	TSharedPtr<SWidget> HandleReflectorTreeContextMenuPtr();

	/** 反射树头列表更改时的回调. */
	/** A callback that reflects changes to the tree header list */
	void HandleReflectorTreeHiddenColumnsListChanged();

	// GA排序
	// sorting
	EColumnSortMode::Type GetColumnSortMode(const FName ColumnId) const;

	// 排序更改响应
	// Sort change response
	void OnColumnSortModeChanged(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type InSortMode);

	// 更改排序
	// Change sort
	void RequestSort();

private:
	// GA排序模式
	// GA sorting mode
	EColumnSortMode::Type SortMode;

protected:
	/** 树视图生成树的回调 */
	/** Callback for tree view spanning tree */
	TSharedRef<ITableRow> HandleAttributesWidgetForFilterListView(TSharedRef< FGASAttributesNodeBase > InItem, const TSharedRef<STableViewBase>& OwnerTable);

	void HandleAttributesTreeGetChildren( TSharedRef<FGASAttributesNodeBase> InReflectorNode, TArray<TSharedRef<FGASAttributesNodeBase>>& OutChildren );

protected:
	// 当前筛选的角色
	// Player currently filtered
	TSharedRef<SWidget>	OnGetShowOverrideTypeMenu();

	// 当前选择的角色
	// Currently Selected Player
	FText GetOverrideTypeDropDownText() const;

	// 选中的GAS组件
	// Selected GAS components
	void HandleOverrideTypeChange(TWeakObjectPtr<UAbilitySystemComponent> InComp);

protected:
	// 是否被选中
	// Is it selected
	ECheckBoxState HandleGetPickingButtonChecked() const;

	// 选中状态更改
	// Check status change
	void HandlePickingModeStateChanged(ECheckBoxState NewValue);

	// 设置状态改变的东东
	// Set the status change
	virtual void SetPickingMode(bool bTick) override;

	// 设置单选框名称
	// Set radio box name
	FText HandleGetPickingModeText() const;

protected:

	/** Called when the user clicks the "Expand All" button; Expands the entire tag tree */
	FReply OnExpandAllClicked();

	/** Called when the user clicks the "Collapse All" button; Collapses the entire tag tree */
	FReply OnCollapseAllClicked();


	void SetGASTreeItemExpansion(bool bExpand);

	bool bGASTreeExpand;

public:

	FORCEINLINE bool HasGASTreeExpand(){ return bGASTreeExpand; }

private:

	void SaveSettings();
	void LoadSettings();

	// 刷新树状表
	// Refresh tree table
	void UpdateGameplayCueListItems();
	FReply UpdateGameplayCueListItemsButtom();

	// 创建查看Categories类型的筛选框
	// Create a filter box to view categories types
	TSharedRef<SWidget> OnGetShowDebugAbilitieCategories();

	// 当前选择的Categories类型
	// Currently selected categories type
	FText GetShowDebugAbilitieCategoriesDropDownText() const;

	// 选中查看的Categories类型
	// Select the categories type to view
	void HandleShowDebugAbilitieCategories(EDebugAbilitieCategories InType);

	FORCEINLINE FName GetAbilitieCategoriesName(EDebugAbilitieCategories InType) const;

	FORCEINLINE FText GetAbilitieCategoriesText(EDebugAbilitieCategories InType) const;

	// 创建Categories查看控件
	// Create categories view control
	void CreateDebugAbilitieCategories(EDebugAbilitieCategories InType);

private:
	// Categories查看Tool的Slot控件
	// Categories view the slot control of the tool
	TSharedPtr<SOverlay> CategoriesToolSlot;

private:
	// 当前选择Categories类型
	// Currently selected categories type
	EDebugAbilitieCategories SelectAbilitieCategories;

	// 按键监听指针
	// Key monitor pointer
	TSharedPtr<IInputProcessor> InputPtr;

protected: 
	
	FORCEINLINE UWorld* GetWorld();

private:

	TSharedPtr<SWrapBox> FilteredOwnedTagsView;

	TSharedPtr<SWrapBox> FilteredBlockedTagsView;

	FGameplayTagContainer OldOwnerTags;

	FGameplayTagContainer OldBlockedTags;

protected:

	TSharedPtr<SWidget> CreateAbilityTagWidget();

protected:
	void OnApplicationPreInputKeyDownListener(const FKeyEvent& InKeyEvent);

#if WITH_EDITOR
	// <tag控件的监听
	// Monitoring of tag control
	FReply OnMouseButtonUpTags(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FName TagsName);

	// <Tag控件选择
	// Tag control selection
	void RefreshTagList(FName TagsName);
#endif

public:
	// 筛选激活状态图表Tree
	// Filter activation status chart tree
	void HandleScreenModeStateChanged(ECheckBoxState NewValue, EScreenGAModeState InState);

protected:
	// 是否被选中
	// Is it selected
	ECheckBoxState HandleGetScreenButtonChecked(EScreenGAModeState InState) const;

	// 创建选中框
	// Create check box
	TSharedPtr<SCheckBox> CreateScreenCheckBox(EScreenGAModeState InState);

	// 设置筛选名称
	// Set filter name
	FText HandleGeScreenModeText(EScreenGAModeState InState) const;

	// 创建Ability查看控件
	// Create ability view control
	TSharedPtr<SWidget> CreateAbilityToolWidget();

protected:

	// 创建Attributes查看控件
	// Create attributes view control
	TSharedPtr<SWidget> CreateAttributesToolWidget();


protected:
	// 创建GameplayEffects查看控件
	// Create gameplay effects view
	TSharedPtr<SWidget> CreateGameplayEffectToolWidget();

	TSharedRef<ITableRow> OneGameplayEffecGenerateWidgetForFilterListView(TSharedRef< FGASGameplayEffectNodeBase > InItem, const TSharedRef<STableViewBase>& OwnerTable);

	void HandleGameplayEffectTreeGetChildren(TSharedRef<FGASGameplayEffectNodeBase> InReflectorNode, TArray<TSharedRef<FGASGameplayEffectNodeBase>>& OutChildren);

	void HandleGameplayEffectTreeHiddenColumnsListChanged();

	void HandleGameplayEffectTreeSelectionChanged(TSharedPtr<FGASGameplayEffectNodeBase>, ESelectInfo::Type /*SelectInfo*/);

private:

	uint8 ScreenModeState;

private:
	TSharedPtr<SAbilitieTree> AbilitieReflectorTree;

	TArray<FString> HiddenReflectorTreeColumns;

	TArray<TSharedRef<FGASAbilitieNodeBase>> SelectedNodes;

	TArray<TSharedRef<FGASAbilitieNodeBase>> AbilitieFilteredTreeRoot;

	TWeakObjectPtr<UAbilitySystemComponent> SelectAbilitySystemComponent;

	FName SelectAbilitySystemComponentForActorName;

	bool bPickingTick;

private:

	TSharedPtr<SAttributesTree> AttributesReflectorTree;

	TArray<TSharedRef<FGASAttributesNodeBase>> AttributesFilteredTreeRoot;

#if WITH_EDITOR
	TArray<SGameplayTagWidget::FEditableGameplayTagContainerDatum> EditableOwnerContainers;
	FGameplayTagContainer OwnweTagContainer;
	FGameplayTagContainer OldOwnweTagContainer;

	TArray<SGameplayTagWidget::FEditableGameplayTagContainerDatum> EditableBlockedContainers;
	FGameplayTagContainer BlockedTagContainer;
	FGameplayTagContainer OldBlockedTagContainer;

#endif

private:
	TSharedPtr<SGameplayEffectTree> GameplayEffectTree;

	TArray<TSharedRef<FGASGameplayEffectNodeBase>> GameplayEffectTreeRoot;

	TArray<FString> HiddenGameplayEffectTreeColumns;
};

TSharedRef<SGASAttachEditor> SGASAttachEditor::New()
{
	return MakeShareable(new SGASAttachEditorImpl());
}

FName SGASAttachEditor::GetTabName()
{
	return "GAAttachEditorApp";
}

void SGASAttachEditor::RegisterTabSpawner(FTabManager& TabManager)
{
	const auto SpawnCallStackViewTab = [](const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::PanelTab)
			//.Label(LOCTEXT("TabTitle", "游戏中GAS查看器"))
			.Label(LOCTEXT("TabTitle", "Runtime Debug"))
			[
				SNew(SBorder)
#if WITH_EDITOR
				.BorderImage(FAppStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
#endif
				.BorderBackgroundColor(FSlateColor(FLinearColor(0.2f,0.2f,0.2f,1.f)))
				[
					SNew(SGASAttachEditor)
				]
			];
	};

	TabManager.RegisterTabSpawner(SGASAttachEditor::GetTabName(), FOnSpawnTab::CreateStatic(SpawnCallStackViewTab))
		//.SetDisplayName(LOCTEXT("TabTitle", "游戏中GAS查看器"));
		.SetDisplayName(LOCTEXT("TabTitle", "Runtime Debug"));
}

void SGASAttachEditorImpl::Construct(const FArguments& InArgs)
{
	bGASTreeExpand = false;
	bPickingTick = false;
	SelectAbilitySystemComponentForActorName = FName();
	SelectAbilitieCategories = Ability;

	ScreenModeState = EScreenGAModeState::Active | EScreenGAModeState::Blocked | EScreenGAModeState::NoActive;

	SortMode = EColumnSortMode::Ascending;

	LoadSettings();


	ChildSlot
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			//.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Left)
					//.Text(LOCTEXT("Refresh", "刷新查看"))
					.Text(LOCTEXT("Refresh", "Update"))
					.OnClicked(this, &SGASAttachEditorImpl::UpdateGameplayCueListItemsButtom)
				]
				
				+ SHorizontalBox::Slot() 
				.FillWidth(1.f)
				.HAlign(HAlign_Right)
				[
					SNew(SComboButton)
					.OnGetMenuContent(this, &SGASAttachEditorImpl::OnGetShowWorldTypeMenu)
					.VAlign(VAlign_Center)
					.ContentPadding(2)
					.ButtonContent()
					[
						SNew(STextBlock)
						//.ToolTipText(LOCTEXT("ShowWorldTypeType", "选择需要查看场景"))
						.ToolTipText(LOCTEXT("ShowWorldTypeType", "Select World Scene"))
						.Text_Lambda([this]{return SelectWorldSceneText;})
					]
				]

			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(2.0f)
				.AutoWidth()
				[
					SNew(SCheckBox)
					.Padding(FMargin(4, 0))
					.IsChecked(this, &SGASAttachEditorImpl::HandleGetPickingButtonChecked)
					.OnCheckStateChanged(this, &SGASAttachEditorImpl::HandlePickingModeStateChanged)
					[
						SNew(SBox)
						.MinDesiredWidth(125)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SGASAttachEditorImpl::HandleGetPickingModeText)
						]

					]
				]
				+ SHorizontalBox::Slot()
				.Padding(2.f, 2.f)
				.AutoWidth()
				[
					SNew(SComboButton)
					.OnGetMenuContent(this, &SGASAttachEditorImpl::OnGetShowOverrideTypeMenu)
					.VAlign( VAlign_Center )
					.ContentPadding(2)
					.ButtonContent()
					[
						SNew( STextBlock )
						//.ToolTipText(LOCTEXT("ShowOverrideType", "选择需要查看的角色的GA"))
						.ToolTipText(LOCTEXT("ShowOverrideType", "Select Player GA"))
						.Text(this, &SGASAttachEditorImpl::GetOverrideTypeDropDownText )
					]
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.Padding(2.f, 2.f)
				.AutoHeight()
				.HAlign(HAlign_Left)
				[
					SNew(SComboButton)
					.OnGetMenuContent(this, &SGASAttachEditorImpl::OnGetShowDebugAbilitieCategories)
					.VAlign(VAlign_Center)
					.ContentPadding(2)
					.ButtonContent()
					[
						SNew(STextBlock)
						//.ToolTipText(LOCTEXT("ShowCharactAbilitieType", "选择需要查看角色身上的效果类型"))
						.ToolTipText(LOCTEXT("ShowCharactAbilitieType", "Select Player Look Up Type"))
						.Text(this, &SGASAttachEditorImpl::GetShowDebugAbilitieCategoriesDropDownText)
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					//CreateAbilityToolWidget()
					SAssignNew(CategoriesToolSlot, SOverlay)
				]
				
			]

		];

	HandleShowDebugAbilitieCategories(Ability);

	UpdateGameplayCueListItemsButtom();
	InputPtr = MakeShareable(new FAttachInputProcessor(this));
	FSlateApplication::Get().RegisterInputPreProcessor(InputPtr);

#if WITH_EDITOR
	//TagKeyDownHandle = FSlateApplication::Get().OnApplicationPreInputKeyDownListener().AddRaw(this, &SGASAttachEditorImpl::OnApplicationPreInputKeyDownListener);

	EditableOwnerContainers.Empty();
	EditableBlockedContainers.Empty();

	EditableOwnerContainers.Add(SGameplayTagWidget::FEditableGameplayTagContainerDatum(nullptr,&OwnweTagContainer));
	EditableBlockedContainers.Add(SGameplayTagWidget::FEditableGameplayTagContainerDatum(nullptr,&BlockedTagContainer));
#endif
}


void SGASAttachEditorImpl::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bPickingTick)
	{
		UpdateGameplayCueListItems();
	}
}


SGASAttachEditorImpl::~SGASAttachEditorImpl()
{
	FSlateApplication::Get().UnregisterInputPreProcessor(InputPtr);
	InputPtr = nullptr;
}

TSharedRef<SWidget> SGASAttachEditorImpl::OnGetShowWorldTypeMenu()
{
	FMenuBuilder MenuBuilder( true, NULL );

	const TIndirectArray<FWorldContext>& WorldList = GEngine->GetWorldContexts();

	for (auto& Item : WorldList)
	{
		if (Item.WorldType == EWorldType::Type::PIE || Item.WorldType == EWorldType::Type::Game)
		{
			FUIAction NoAction( FExecuteAction::CreateSP( this, &SGASAttachEditorImpl::HandleShowWorldTypeChange, Item ) );

			FText ShowName;
			if (Item.RunAsDedicated)
			{
				//ShowName = LOCTEXT("Dedicated","专用服务器");
				ShowName = LOCTEXT("Dedicated","Dedicated");
			}
			else
			{
				if (Item.WorldType == EWorldType::Type::Game)
				{
					ShowName = LOCTEXT("Client",/*"客户端"*/"Client");
				}
				else
				{
					ShowName = FText::Format(FText::FromString("{0}  [{1}]"), LOCTEXT("Client",/*"客户端"*/"Client"), FText::AsNumber(Item.PIEInstance));
				}
				
			}

			MenuBuilder.AddMenuEntry(ShowName, FText(), FSlateIcon(), NoAction);
		}
	}

	return MenuBuilder.MakeWidget();
}


void SGASAttachEditorImpl::HandleShowWorldTypeChange(FWorldContext InWorldContext)
{
	SelectWorldSceneConetextHandle = InWorldContext.ContextHandle;


	if (InWorldContext.RunAsDedicated)
	{
		SelectWorldSceneText = LOCTEXT("Dedicated", "专用服务器");
	}
	else
	{
		if (InWorldContext.WorldType == EWorldType::Type::Game)
		{
			SelectWorldSceneText = LOCTEXT("Client",/*"客户端"*/"Client");
		}
		else
		{
			SelectWorldSceneText = FText::Format(FText::FromString("{0}  [{1}]"), LOCTEXT("Client",/*"客户端"*/"Client"), FText::AsNumber(InWorldContext.PIEInstance));
		}
	}

	UpdateGameplayCueListItemsButtom();
}

FText SGASAttachEditorImpl::GenerateToolTipForText(TSharedRef<FGASAbilitieNodeBase> InReflectorNode) const
{
	return InReflectorNode->GetAbilitieHasTag();
}

TSharedRef<ITableRow> SGASAttachEditorImpl::OnGenerateWidgetForFilterListView(TSharedRef< FGASAbilitieNodeBase > InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGASAbilitieTreeItem, OwnerTable)
		.WidgetInfoToVisualize(InItem)
		.ToolTip(GenerateToolTipForReflectorNode(InItem));
}


void SGASAttachEditorImpl::HandleReflectorTreeGetChildren(TSharedRef<FGASAbilitieNodeBase> InReflectorNode, TArray<TSharedRef<FGASAbilitieNodeBase>>& OutChildren)
{
	OutChildren = InReflectorNode->GetChildNodes();
}

void SGASAttachEditorImpl::HandleReflectorTreeSelectionChanged(TSharedPtr<FGASAbilitieNodeBase>, ESelectInfo::Type /*SelectInfo*/)
{
	SelectedNodes = AbilitieReflectorTree->GetSelectedItems();

}

TSharedPtr<IToolTip> SGASAttachEditorImpl::GenerateToolTipForReflectorNode(TSharedRef<FGASAbilitieNodeBase> InReflectorNode)
{
	return SNew(SToolTip)
			.Text(this, &SGASAttachEditorImpl::GenerateToolTipForText, InReflectorNode);
}

TSharedPtr<SWidget> SGASAttachEditorImpl::HandleReflectorTreeContextMenuPtr()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("NotFunction", "NotFunction"),
		LOCTEXT("NotFunctionTooltip", "Null"),
		FSlateIcon(),
		FUIAction()
	);

	return MenuBuilder.MakeWidget();
}

void SGASAttachEditorImpl::HandleReflectorTreeHiddenColumnsListChanged()
{
#if WITH_EDITOR
	if (AbilitieReflectorTree && AbilitieReflectorTree->GetHeaderRow())
	{
		const TArray<FName> HiddenColumnIds = AbilitieReflectorTree->GetHeaderRow()->GetHiddenColumnIds();
		HiddenReflectorTreeColumns.Reset(HiddenColumnIds.Num());
		for (const FName Id : HiddenColumnIds)
		{
			HiddenReflectorTreeColumns.Add(Id.ToString());
		}
		SaveSettings();
	}
#endif
}

EColumnSortMode::Type SGASAttachEditorImpl::GetColumnSortMode(const FName ColumnId) const
{
	if (NAME_AbilitietName != ColumnId) return EColumnSortMode::None;

	return SortMode;
}

void SGASAttachEditorImpl::OnColumnSortModeChanged(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type InSortMode)
{
	SortMode = InSortMode;
	RequestSort();
}

void SGASAttachEditorImpl::RequestSort()
{
	if (SortMode == EColumnSortMode::Ascending)
	{
		AbilitieFilteredTreeRoot.Sort([](TSharedRef<FGASAbilitieNodeBase> A, TSharedRef<FGASAbilitieNodeBase> B)
		{
			return A->GetGAName().ToString() < B->GetGAName().ToString();
		});
	}
	else if (SortMode == EColumnSortMode::Descending)
	{
		AbilitieFilteredTreeRoot.Sort([](TSharedRef<FGASAbilitieNodeBase> A, TSharedRef<FGASAbilitieNodeBase> B)
		{
			return A->GetGAName().ToString() >= B->GetGAName().ToString();
		});
	}

	AbilitieReflectorTree->RequestTreeRefresh();
}

TSharedRef<ITableRow> SGASAttachEditorImpl::HandleAttributesWidgetForFilterListView(TSharedRef< FGASAttributesNodeBase > InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGASAttributesTreeItem, OwnerTable)
		.WidgetInfoToVisualize(InItem);
}

void SGASAttachEditorImpl::HandleAttributesTreeGetChildren(TSharedRef<FGASAttributesNodeBase> InReflectorNode, TArray<TSharedRef<FGASAttributesNodeBase>>& OutChildren)
{
}

FText GetLocalRoleText(ENetRole InRole)
{
	FText RoleText;
	switch (InRole)
	{
	case ROLE_Authority:
		// RoleText = LOCTEXT("Authority", "权威");
		RoleText = LOCTEXT("Authority", "Authority");
		break;
	case ROLE_AutonomousProxy:
		// RoleText = LOCTEXT("AutonomousProxy", "本地");
		RoleText = LOCTEXT("AutonomousProxy", "AutonomousProxy");
		break;
	case ROLE_SimulatedProxy:
		// RoleText = LOCTEXT("SimulatedProxy", "模拟");
		RoleText = LOCTEXT("SimulatedProxy", "SimulatedProxy");
		break;
	}

	return RoleText;
}

FText GetOverrideTypeDropDownText_Explicit(const TWeakObjectPtr<UAbilitySystemComponent>& InComp)
{
	if (!InComp.IsValid())
	{
		return FText();
	}

	AActor* LocalAvatarActor = InComp->GetAvatarActor_Direct();
	AActor* LocalOwnerActor = InComp->GetOwnerActor();
	APawn* AvatarAsPawn = LocalAvatarActor ? Cast<APawn>(LocalAvatarActor) : nullptr;
	APawn* OwnerAsPawn = LocalOwnerActor ? Cast<APawn>(LocalOwnerActor) : nullptr;

	FText OutName = FText::FromString(LocalAvatarActor == nullptr ? LocalOwnerActor->GetName() : LocalAvatarActor->GetName());
	bool IsPlayer = (IsValid(AvatarAsPawn) ? AvatarAsPawn->IsPlayerControlled() : false) || (IsValid(OwnerAsPawn) ? OwnerAsPawn->IsPlayerControlled() : false);
	bool IsSelected =  (LocalAvatarActor ? LocalAvatarActor->IsSelected() : false) || (LocalOwnerActor ? LocalOwnerActor->IsSelected() : false);

	if (LocalOwnerActor)
	{
		OutName = FText::Format(FText::FromString(TEXT("{0} [{2}] [{1}]{3}{4}")), OutName, GetLocalRoleText(LocalAvatarActor == nullptr ? LocalOwnerActor->GetLocalRole() : LocalAvatarActor->GetLocalRole()), FText::FromString(LocalOwnerActor->GetName()), FText::FromString(IsPlayer ? " (Player)" : ""), FText::FromString(IsSelected ? " (Selected)" : ""));
	}

	return OutName;
}

TSharedRef<SWidget> SGASAttachEditorImpl::OnGetShowOverrideTypeMenu()
{
	FMenuBuilder MenuBuilder( true, NULL );

	for (TWeakObjectPtr<UAbilitySystemComponent>& Comp :PlayerComp)
	{
		FUIAction NoAction( FExecuteAction::CreateSP( this, &SGASAttachEditorImpl::HandleOverrideTypeChange, Comp ) );
		MenuBuilder.AddMenuEntry(GetOverrideTypeDropDownText_Explicit(Comp),FText(),FSlateIcon(),NoAction);
	}
	return MenuBuilder.MakeWidget();
}

FText SGASAttachEditorImpl::GetOverrideTypeDropDownText() const
{
	if (SelectAbilitySystemComponent.IsValid())
	{
		if (AActor* LocalGASActor = GetGASActor(SelectAbilitySystemComponent))
		{
			return FText::Format(FText::FromString(TEXT("{0}[{1}]")), FText::FromString(LocalGASActor->GetName()), GetLocalRoleText(LocalGASActor->GetLocalRole()));
		}
	}

	return FText();
}

void SGASAttachEditorImpl::HandleOverrideTypeChange(TWeakObjectPtr<UAbilitySystemComponent> InComp)
{
	if (!InComp.IsValid())
	{
		return;
	}

	SelectAbilitySystemComponent = InComp;
	
	UpdateGameplayCueListItems();
}

ECheckBoxState SGASAttachEditorImpl::HandleGetPickingButtonChecked() const
{
	return bPickingTick ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SGASAttachEditorImpl::HandlePickingModeStateChanged(ECheckBoxState NewValue)
{
	SetPickingMode(!bPickingTick);
}

void SGASAttachEditorImpl::SetPickingMode(bool bTick)
{
/*
#if WITH_SLATE_DEBUGGING
	static auto CVarSlateGlobalInvalidation = IConsoleManager::Get().FindConsoleVariable(TEXT("Slate.EnableGlobalInvalidation"));
#endif*/

	bPickingTick = bTick;
}

FText SGASAttachEditorImpl::HandleGetPickingModeText() const
{
	//return bPickingTick ? LOCTEXT("bPickingTickYes", "按 END 键停止刷新") : LOCTEXT("bPickingTickNo", "持续更新") ;
	return bPickingTick ? LOCTEXT("bPickingTickYes", "Press 'END' to interrupt") : LOCTEXT("bPickingTickNo", "Continuous Update") ;
}

FReply SGASAttachEditorImpl::OnExpandAllClicked()
{
	SetGASTreeItemExpansion(true);
	return FReply::Handled();
}

FReply SGASAttachEditorImpl::OnCollapseAllClicked()
{
	SetGASTreeItemExpansion(false);
	return FReply::Handled();
}

void SGASAttachEditorImpl::SetGASTreeItemExpansion(bool bExpand)
{
	bGASTreeExpand = bExpand;
	if (SelectAbilitieCategories == EDebugAbilitieCategories::Ability && AbilitieReflectorTree.IsValid())
	{
		for (TSharedRef<FGASAbilitieNodeBase> Item : AbilitieFilteredTreeRoot)
		{
			AbilitieReflectorTree->SetItemExpansion(Item, bExpand);
		}
	}
	else if (SelectAbilitieCategories == EDebugAbilitieCategories::GameplayEffects && GameplayEffectTree.IsValid())
	{
		for (TSharedRef<FGASGameplayEffectNodeBase> Item : GameplayEffectTreeRoot)
		{
			GameplayEffectTree->SetItemExpansion(Item, bExpand);
		}
	}
}

void SGASAttachEditorImpl::SaveSettings()
{
	GConfig->SetArray(TEXT("GASAttachEditor"), TEXT("HiddenReflectorTreeColumns"), HiddenReflectorTreeColumns, *GEditorPerProjectIni);
	GConfig->SetArray(TEXT("GASAttachEditor"), TEXT("HiddenGameplayEffectTreeColumns"), HiddenGameplayEffectTreeColumns, *GEditorPerProjectIni);
}

void SGASAttachEditorImpl::LoadSettings()
{
	GConfig->GetArray(TEXT("GASAttachEditor"), TEXT("HiddenReflectorTreeColumns"), HiddenReflectorTreeColumns, *GEditorPerProjectIni);
	GConfig->GetArray(TEXT("GASAttachEditor"), TEXT("HiddenGameplayEffectTreeColumns"), HiddenGameplayEffectTreeColumns, *GEditorPerProjectIni);
}

void SGASAttachEditorImpl::UpdateGameplayCueListItems()
{

	FASCDebugTargetInfo* TargetInfo = GetASCDebugTargetInfo(GetWorld());

	if (UAbilitySystemComponent* ASC = GetDebugTarget(TargetInfo, SelectAbilitySystemComponent.Get(), SelectAbilitySystemComponentForActorName))
	{
		SelectAbilitySystemComponent = ASC;

		// 标签组
		// Tag group
		if (SelectAbilitieCategories == EDebugAbilitieCategories::Tags && FilteredOwnedTagsView.IsValid())
		{
			FGameplayTagContainer OwnerTags;
			SelectAbilitySystemComponent->GetOwnedGameplayTags(OwnerTags);
			if (OldOwnerTags != OwnerTags)
			{
				OldOwnerTags = OwnerTags;
				FilteredOwnedTagsView->ClearChildren();
#if WITH_EDITOR
				OwnweTagContainer.Reset();
#endif
				
				for (FGameplayTag InTag : OwnerTags)
				{
					FilteredOwnedTagsView->AddSlot()
						[
							SNew(SCharacterTagsViewItem)
							.TagsItem(FGASCharacterTags::Create(ASC, InTag, "ActivationOwnedTags"))
						];
#if WITH_EDITOR
					OwnweTagContainer.AddTag(InTag);
#endif
				}

			}
			

			FGameplayTagContainer BlockTags;
			ASC->GetBlockedAbilityTags(BlockTags);

			if (BlockTags != OldBlockedTags)
			{
				FilteredBlockedTagsView->ClearChildren();
#if WITH_EDITOR
				BlockedTagContainer.Reset();
#endif
				for (FGameplayTag InTag : BlockTags)
				{
					FilteredBlockedTagsView->AddSlot()
						[
							SNew(SCharacterTagsViewItem)
							.TagsItem(FGASCharacterTags::Create(ASC, InTag, "ActivationBlockedTags"))
						];
#if WITH_EDITOR
					BlockedTagContainer.AddTag(InTag);
#endif
				}

			}
		}

		/*TArray<FName> LocalDisplayNames;
		LocalDisplayNames.Add(TargetInfo->DebugCategories[TargetInfo->DebugCategoryIndex]);*/

		// 技能组
		// Ability group
		if (SelectAbilitieCategories == EDebugAbilitieCategories::Ability && AbilitieReflectorTree.IsValid())
		{
			AbilitieFilteredTreeRoot.Reset();

			for (FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
			{
				if (!AbilitySpec.Ability) continue;
				TSharedRef<FGASAbilitieNode> NewItem = FGASAbilitieNode::Create(ASC, AbilitySpec);

				NewItem->SetItemVisility(NewItem->ScreenGAMode & ScreenModeState);

				AbilitieFilteredTreeRoot.Add(NewItem);

				AbilitieReflectorTree->SetItemExpansion(AbilitieFilteredTreeRoot.Top(), bGASTreeExpand);
			}

			RequestSort();


		}

		// 属性组
		// Attribute group
		if (SelectAbilitieCategories == EDebugAbilitieCategories::Attributes &&  AttributesReflectorTree.IsValid())
		{
			AttributesFilteredTreeRoot.Reset();
			for (UAttributeSet* Set : ASC->GetSpawnedAttributes())
			{
				if (!Set)
				{
					continue;
				}

				for (TFieldIterator<FStructProperty> It(Set->GetClass()); It; ++It)
				{
					if ((*It)->Struct == FGameplayAttributeData::StaticStruct())
					{
						FGameplayAttribute	Attribute(*It);
						TSharedRef<FGASAttributesNode> NewItem = FGASAttributesNode::Create(ASC, Attribute);

						AttributesFilteredTreeRoot.Add(NewItem);
					}
				}
			}
			AttributesReflectorTree->RequestTreeRefresh();

		}

		// 游戏效果组
		// GameplayEffect group
		if (SelectAbilitieCategories == EDebugAbilitieCategories::GameplayEffects && GameplayEffectTree.IsValid())
		{
			GameplayEffectTreeRoot.Reset();

			FProperty* GameplayEffectsPtr = FindFProperty<FProperty>(ASC->GetClass(), "ActiveGameplayEffects");

			if (!GameplayEffectsPtr) return;
			FActiveGameplayEffectsContainer* ActiveGameplayEffectsPtr = GameplayEffectsPtr->ContainerPtrToValuePtr<FActiveGameplayEffectsContainer>(ASC);
			if (!ActiveGameplayEffectsPtr) return;

			for (FActiveGameplayEffect& ActiveGE : ActiveGameplayEffectsPtr)
			{
				GameplayEffectTreeRoot.Add(FGASGameplayEffectNode::Create(GetWorld(), ActiveGE));

				GameplayEffectTree->SetItemExpansion(GameplayEffectTreeRoot.Top(), bGASTreeExpand);
			}

			GameplayEffectTree->RequestTreeRefresh();
		}
	}
}

FReply SGASAttachEditorImpl::UpdateGameplayCueListItemsButtom()
{
	UpDataPlayerComp(GetWorld());

	UpdateGameplayCueListItems();

	return FReply::Handled();
}

TSharedRef<SWidget> SGASAttachEditorImpl::OnGetShowDebugAbilitieCategories()
{
	FMenuBuilder MenuBuilder(true, NULL);

	TArray<EDebugAbilitieCategories> Categories({ EDebugAbilitieCategories::Ability,EDebugAbilitieCategories::Attributes,EDebugAbilitieCategories::GameplayEffects, EDebugAbilitieCategories::Tags });

	for (EDebugAbilitieCategories& Type : Categories)
	{

		FUIAction NoAction(FExecuteAction::CreateSP(this, &SGASAttachEditorImpl::HandleShowDebugAbilitieCategories, Type));
		MenuBuilder.AddMenuEntry(FText::FromName(GetAbilitieCategoriesName(Type)), GetAbilitieCategoriesText(Type), FSlateIcon(), NoAction);
	}


	return MenuBuilder.MakeWidget();
}

FText SGASAttachEditorImpl::GetShowDebugAbilitieCategoriesDropDownText() const
{
	return FText::FromName(GetAbilitieCategoriesName(SelectAbilitieCategories));
}

void SGASAttachEditorImpl::HandleShowDebugAbilitieCategories(EDebugAbilitieCategories InType)
{
	SelectAbilitieCategories = InType;

	CreateDebugAbilitieCategories(InType);
}

FORCEINLINE FName SGASAttachEditorImpl::GetAbilitieCategoriesName(EDebugAbilitieCategories InType) const
{
	FName TypeName;
	switch (InType)
	{
	case EDebugAbilitieCategories::Tags:
		TypeName = "Tags";
		break;
	case EDebugAbilitieCategories::Ability:
		TypeName = "Ability";
		break;
	case EDebugAbilitieCategories::Attributes:
		TypeName = "Attributes";
		break;
	case EDebugAbilitieCategories::GameplayEffects:
		TypeName = "GameplayEffects";
		break;
	}

	return TypeName;
}

FORCEINLINE FText SGASAttachEditorImpl::GetAbilitieCategoriesText(EDebugAbilitieCategories InType) const
{
	FText TypeText;
	switch (InType)
	{
	case EDebugAbilitieCategories::Tags:
		//TypeText = LOCTEXT("Categories_Tigs", "标签");
		TypeText = LOCTEXT("Categories_Tigs", "Tigs");
		break;
	case EDebugAbilitieCategories::Ability:
		//TypeText = LOCTEXT("Categories_Ability", "技能");
		TypeText = LOCTEXT("Categories_Ability", "Ability");
		break;
	case EDebugAbilitieCategories::Attributes:
		//TypeText = LOCTEXT("Categories_Attributes", "属性");
		TypeText = LOCTEXT("Categories_Attributes", "Attributes");
		break;
	case EDebugAbilitieCategories::GameplayEffects:
		//TypeText = LOCTEXT("Categories_GameplayEffects", "效果");
		TypeText = LOCTEXT("Categories_GameplayEffects", "GameplayEffects");
		break;
	}

	return TypeText;
}

void SGASAttachEditorImpl::CreateDebugAbilitieCategories(EDebugAbilitieCategories InType)
{
	if (!CategoriesToolSlot.IsValid())
	{
		return;
	}

	CategoriesToolSlot->ClearChildren();

	TSharedPtr<SWidget> CategoriesWidget;

	switch (InType)
	{
	case EDebugAbilitieCategories::Attributes:
		CategoriesWidget = CreateAttributesToolWidget();
		break;
	case EDebugAbilitieCategories::GameplayEffects:
		CategoriesWidget = CreateGameplayEffectToolWidget();
		break;
	case EDebugAbilitieCategories::Ability:
		CategoriesWidget = CreateAbilityToolWidget();
		break;
	case EDebugAbilitieCategories::Tags:
		CategoriesWidget = CreateAbilityTagWidget();
		break;
	}

	if (!CategoriesWidget.IsValid())
	{
		return;
	}
	
	CategoriesToolSlot->AddSlot()
		[
			CategoriesWidget.ToSharedRef()
		];
}

UWorld* SGASAttachEditorImpl::GetWorld()
{

	UWorld* World = nullptr;

	const TIndirectArray<FWorldContext>& WorldList = GEngine->GetWorldContexts();

	TArray<FWorldContext> NewWorldList;

	for (auto& Item : WorldList)
	{
		if (Item.WorldType == EWorldType::Type::PIE || Item.WorldType == EWorldType::Type::Game)
		{
			NewWorldList.Add(Item);
			if (Item.ContextHandle == SelectWorldSceneConetextHandle)
			{
				return Item.World();
			}
		}
	}

	if (NewWorldList.Num())
	{
		SelectWorldSceneConetextHandle = NewWorldList[0].ContextHandle;

		if (NewWorldList[0].RunAsDedicated)
		{
			//SelectWorldSceneText = LOCTEXT("Dedicated", "专用服务器");
			SelectWorldSceneText = LOCTEXT("Dedicated", "Dedicated");
		}
		else
		{
			if (NewWorldList[0].WorldType == EWorldType::Type::Game)
			{
				SelectWorldSceneText = LOCTEXT("Client","Client");
			}
			else
			{
				SelectWorldSceneText = FText::Format(FText::FromString("{0}  [{1}]"), LOCTEXT("Client", "Client"), FText::AsNumber(NewWorldList[0].PIEInstance));
			}
			
		}

		return NewWorldList[0].World();
	}

	return nullptr;

}

TSharedPtr<SWidget> SGASAttachEditorImpl::CreateAbilityTagWidget()
{
	return SNew(SSplitter)
		.Orientation(Orient_Vertical)
		+ SSplitter::Slot()
		.Value(0.7f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(2.f)
			[
				SNew(STextBlock)
				//.Text(LOCTEXT("CharacterHasOwnTags", "当前角色拥有的Tags"))
				.Text(LOCTEXT("CharacterHasOwnTags", "Player Owned Tags"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SBorder)
				.Padding(2.f)
#if WITH_EDITOR
				.OnMouseButtonUp(this, &SGASAttachEditorImpl::OnMouseButtonUpTags, FName("OwnTags"))
#endif
				[
					SAssignNew(FilteredOwnedTagsView, SWrapBox)
					.Orientation(EOrientation::Orient_Horizontal)
					.UseAllottedSize(true)
					.InnerSlotPadding(FVector2D(5.f))
				]
			]
		]

		+ SSplitter::Slot()
		.Value(0.3f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(2.f)
			[
				SNew(STextBlock)
				//.Text(LOCTEXT("CharacterHasBlaTags", "当前角色阻止的Tags"))
			.Text(LOCTEXT("CharacterHasBlaTags", "Player Back Tags"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SBorder)
#if WITH_EDITOR
				.OnMouseButtonUp(this, &SGASAttachEditorImpl::OnMouseButtonUpTags, FName("BlaTags"))
#endif
				.Padding(2.f)
				[
					SAssignNew(FilteredBlockedTagsView, SWrapBox)
					.Orientation(EOrientation::Orient_Horizontal)
					.UseAllottedSize(true)
					.InnerSlotPadding(FVector2D(5.f))
				]
			]
	];
}

void SGASAttachEditorImpl::OnApplicationPreInputKeyDownListener(const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::End)
	{
		SetPickingMode(false);
	}
}
#if WITH_EDITOR
FReply SGASAttachEditorImpl::OnMouseButtonUpTags(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FName TagsName)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		TSharedPtr<SGameplayTagWidget> TagWidget = 
			SNew(SGameplayTagWidget, TagsName == "OwnTags" ? EditableOwnerContainers : EditableBlockedContainers)
			.GameplayTagUIMode(EGameplayTagUIMode::SelectionMode)
			.ReadOnly(false)
			.OnTagChanged(this, &SGASAttachEditorImpl::RefreshTagList, TagsName);

		if (TagWidget.IsValid())
		{
			if (TagsName == "OwnTags")
			{
				OldOwnweTagContainer = OwnweTagContainer;
			}
			else
			{
				OldBlockedTagContainer = BlockedTagContainer;
			}

			FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
			FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, TagWidget.ToSharedRef(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
		}

	}
	return FReply::Handled();
}


void SGASAttachEditorImpl::RefreshTagList(FName TagsName)
{
	bool bIsOwnTag = TagsName == "OwnTags";

	FGameplayTagContainer* NewTagContainer =  bIsOwnTag ? &OwnweTagContainer : &BlockedTagContainer;
	FGameplayTagContainer* OldTagContainer = bIsOwnTag ? &OldOwnweTagContainer : &OldBlockedTagContainer;

	if (!SelectAbilitySystemComponent.IsValid() || !NewTagContainer || !OldTagContainer) return;

	if (NewTagContainer->Num() > OldTagContainer->Num())
	{
		TArray<FGameplayTag> NewGameplayTagArr;
		NewTagContainer->GetGameplayTagArray(NewGameplayTagArr);

		// 是否增加
		// Is Add
		for (const FGameplayTag& Item : NewGameplayTagArr)
		{
			if (OldTagContainer->HasTag(Item)) continue;

			if (bIsOwnTag)
			{
				SelectAbilitySystemComponent->AddLooseGameplayTag(Item);
			}
			else
			{

			}
		}
	}
	else
	{
		TArray<FGameplayTag> OldGameplayTagArr;
		OldTagContainer->GetGameplayTagArray(OldGameplayTagArr);
		
		// 是否是减少
		// Is Reduce
		for (const FGameplayTag& Item : OldGameplayTagArr)
		{
			if (NewTagContainer->HasTag(Item)) continue;

			if (bIsOwnTag)
			{
				SelectAbilitySystemComponent->RemoveLooseGameplayTag(Item);
			}
			else
			{

			}
		}
	}
	
	*OldTagContainer = *NewTagContainer;
}
#endif
void SGASAttachEditorImpl::HandleScreenModeStateChanged(ECheckBoxState NewValue, EScreenGAModeState InState)
{
	switch (NewValue)
	{
	case ECheckBoxState::Unchecked:
		ScreenModeState ^= InState;
		break;
	case ECheckBoxState::Checked:
		ScreenModeState |= InState;
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}

	for (TSharedRef<FGASAbilitieNodeBase>& Item : AbilitieFilteredTreeRoot)
	{
		Item->SetItemVisility(Item->ScreenGAMode & ScreenModeState);
	}
}

ECheckBoxState SGASAttachEditorImpl::HandleGetScreenButtonChecked(EScreenGAModeState InState) const
{
	return ScreenModeState & InState ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

TSharedPtr<SCheckBox> SGASAttachEditorImpl::CreateScreenCheckBox(EScreenGAModeState InModeState)
{
	return SNew(SCheckBox)
			.Padding(FMargin(4, 0))
			.IsChecked(this, &SGASAttachEditorImpl::HandleGetScreenButtonChecked, InModeState)
			.OnCheckStateChanged_Lambda([this,InModeState](ECheckBoxState NewValue){ HandleScreenModeStateChanged(NewValue,InModeState);})
			[
				SNew(SBox)
				.MinDesiredWidth(80.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SGASAttachEditorImpl::HandleGeScreenModeText, InModeState)
				]
			];
}

FText SGASAttachEditorImpl::HandleGeScreenModeText(EScreenGAModeState InState) const
{
	FText StateText;
	switch (InState)
	{
	case Active:
		//StateText = LOCTEXT("ActiveScreen","激活");
		StateText = LOCTEXT("ActiveScreen","Active");
		break;
	case Blocked:
		//StateText = LOCTEXT("BlockedScreen","阻止");
		StateText = LOCTEXT("BlockedScreen","Blocked");
		break;
	case NoActive:
		//StateText = LOCTEXT("NoActive","未激活");
		StateText = LOCTEXT("NoActive","Inactive");
		break;
	default:
		break;
	}

	return StateText;
}

TSharedPtr<SWidget> SGASAttachEditorImpl::CreateAbilityToolWidget()
{

	TArray<FName> HiddenColumnsList;
	HiddenColumnsList.Reserve(HiddenReflectorTreeColumns.Num());
	for (const FString& Item : HiddenReflectorTreeColumns)
	{
		HiddenColumnsList.Add(*Item);
	}

	TSharedPtr<SVerticalBox> Widget = 
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(2.f, 2.f)
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.Padding(FMargin(8.f, 0.f))
				.AutoWidth()
				[
					SNew(SButton)
					.OnClicked(this, &SGASAttachEditorImpl::OnExpandAllClicked)
					.Text(NSLOCTEXT("GameplayTagWidget", "GameplayTagWidget_ExpandAll", "Expand All"))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.OnClicked(this, &SGASAttachEditorImpl::OnCollapseAllClicked)
					.Text(NSLOCTEXT("GameplayTagWidget", "GameplayTagWidget_CollapseAll", "Collapse All"))
				]
			]

			+ SVerticalBox::Slot()
			.Padding(2.f, 2.f)
			.AutoHeight()
			.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				[
					CreateScreenCheckBox(Active).ToSharedRef()
				]

				+ SHorizontalBox::Slot()
				[
					CreateScreenCheckBox(Blocked).ToSharedRef()
				]

				+ SHorizontalBox::Slot()
				[
					CreateScreenCheckBox(NoActive).ToSharedRef()
				]
			]

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(0)
			[
				SAssignNew(AbilitieReflectorTree, SAbilitieTree)
				.TreeItemsSource(&AbilitieFilteredTreeRoot)
				.OnGenerateRow(this, &SGASAttachEditorImpl::OnGenerateWidgetForFilterListView)
				.OnGetChildren(this, &SGASAttachEditorImpl::HandleReflectorTreeGetChildren)
				.OnSelectionChanged(this, &SGASAttachEditorImpl::HandleReflectorTreeSelectionChanged)
				.OnContextMenuOpening(this, &SGASAttachEditorImpl::HandleReflectorTreeContextMenuPtr)
				.HighlightParentNodesForSelection(true)
				.HeaderRow
				(
					SNew(SHeaderRow)
					.CanSelectGeneratedColumn(true)
					.HiddenColumnsList(HiddenColumnsList)
					.OnHiddenColumnsListChanged(this, &SGASAttachEditorImpl::HandleReflectorTreeHiddenColumnsListChanged)

					+ SHeaderRow::Column(NAME_AbilitietName)
					.SortMode(this,&SGASAttachEditorImpl::GetColumnSortMode, NAME_AbilitietName)
					.OnSort(this, &SGASAttachEditorImpl::OnColumnSortModeChanged)
					//.DefaultLabel(LOCTEXT("AbilitietName", "名称"))
					.DefaultLabel(LOCTEXT("AbilitietName", "Ability Name"))
					//.DefaultTooltip(LOCTEXT("AbilitietNameToolTip", "技能名称/任务名称/调试名称"))
					.DefaultTooltip(LOCTEXT("AbilitietNameToolTip", "Ability/Task name"))
					.FillWidth(0.6f)
					.ShouldGenerateWidget(true)

					+ SHeaderRow::Column(NAME_GAStateType)
					/*.DefaultLabel(LOCTEXT("GAAbilitietStateType", "当前状态"))
					.DefaultTooltip(LOCTEXT("GAStateTypeToolTip", "当前状态是否激活，或者是可以被激活但是因为某些原因被拦截"))*/
					.DefaultLabel(LOCTEXT("GAAbilitietStateType", "State"))
					.DefaultTooltip(LOCTEXT("GAStateTypeToolTip", "Whether the ability is active, inactive or blocked"))
					.FillWidth(0.2f)

					+ SHeaderRow::Column(NAME_GAIsActive)
					//.DefaultLabel(LOCTEXT("GAIsActive", "是否激活"))
					.DefaultLabel(LOCTEXT("GAIsActive", "Active"))
					.FixedWidth(60.f)

					+ SHeaderRow::Column(NAME_GAAbilityTriggers)
					//.DefaultLabel(LOCTEXT("GAAbilityTriggers", "存在的激活Tag"))
					.DefaultLabel(LOCTEXT("GAAbilityTriggers", "Triggers"))
					.HAlignHeader(EHorizontalAlignment::HAlign_Center)
					.FillWidth(0.2)
				)
			]
		];


	return Widget;
}

TSharedPtr<SWidget> SGASAttachEditorImpl::CreateAttributesToolWidget()
{
	return SNew(SBorder)
			.Padding(0)
			[
				SAssignNew(AttributesReflectorTree,SAttributesTree)
				.TreeItemsSource(&AttributesFilteredTreeRoot) 
				.OnGenerateRow(this, &SGASAttachEditorImpl::HandleAttributesWidgetForFilterListView)
				.OnGetChildren(this, &SGASAttachEditorImpl::HandleAttributesTreeGetChildren)
				.HighlightParentNodesForSelection(true)
				.HeaderRow
				(
					SNew(SHeaderRow)
					.CanSelectGeneratedColumn(true)

					+ SHeaderRow::Column(NAME_AttributesName)
					//.DefaultLabel(LOCTEXT("AttributesName", "属性名称"))
					.DefaultLabel(LOCTEXT("AttributesName", "AttributesName"))
					.FillWidth(0.4f)
					.ShouldGenerateWidget(true)

					+ SHeaderRow::Column(NAME_GANumericAttribute)
					//.DefaultLabel(LOCTEXT("GAAttributeStateType", "当前属性值"))
					.DefaultLabel(LOCTEXT("GAAttributeStateType", "Attribute Value"))
					.FillWidth(0.6f)

				)
			];
}

TSharedPtr<SWidget> SGASAttachEditorImpl::CreateGameplayEffectToolWidget()
{
	TArray<FName> HiddenColumnsList;
	HiddenColumnsList.Reserve(HiddenGameplayEffectTreeColumns.Num());
	for (const FString& Item : HiddenGameplayEffectTreeColumns)
	{
		HiddenColumnsList.Add(*Item);
	}

	TSharedPtr<SVerticalBox> Widget =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(2.f, 2.f)
		.AutoHeight()
		.HAlign(HAlign_Left)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(FMargin(8.f, 0.f))
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked(this, &SGASAttachEditorImpl::OnExpandAllClicked)
				.Text(NSLOCTEXT("GameplayTagWidget", "GameplayTagWidget_ExpandAll", "Expand All"))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked(this, &SGASAttachEditorImpl::OnCollapseAllClicked)
				.Text(NSLOCTEXT("GameplayTagWidget", "GameplayTagWidget_CollapseAll", "Collapse All"))
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(0.f)
			[
				SAssignNew(GameplayEffectTree, SGameplayEffectTree)
				.TreeItemsSource(&GameplayEffectTreeRoot)
				.OnGenerateRow(this, &SGASAttachEditorImpl::OneGameplayEffecGenerateWidgetForFilterListView)
				.OnGetChildren(this, &SGASAttachEditorImpl::HandleGameplayEffectTreeGetChildren)
				.OnSelectionChanged(this, &SGASAttachEditorImpl::HandleGameplayEffectTreeSelectionChanged)
				.HighlightParentNodesForSelection(true)
				.HeaderRow
				(
					SNew(SHeaderRow)
					.CanSelectGeneratedColumn(true)
					.HiddenColumnsList(HiddenColumnsList)
					.OnHiddenColumnsListChanged(this, &SGASAttachEditorImpl::HandleGameplayEffectTreeHiddenColumnsListChanged)

				+ SHeaderRow::Column(NAME_GAGameplayEffectName)
				//.DefaultLabel(LOCTEXT("GAGameplayEffectName", "名称"))
				.DefaultLabel(LOCTEXT("GAGameplayEffectName", "GameplayEffectName"))
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				//.DefaultTooltip(LOCTEXT("GAGameplayEffectToolTip", "GE名称 / 加成属性"))
				.DefaultTooltip(LOCTEXT("GAGameplayEffectToolTip", "GameplayEffect Name / Bonus Attribute"))
				.FillWidth(0.2f)

				+ SHeaderRow::Column(NAME_GAGameplayEffectDuration)
				//.DefaultLabel(LOCTEXT("GAGameplayEffectDuration", "时间"))
				.DefaultLabel(LOCTEXT("GAGameplayEffectDuration", "Time"))
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				//.DefaultTooltip(LOCTEXT("GAGameplayEffectDurationToolTip", "GE时间详细叙述"))
				.DefaultTooltip(LOCTEXT("GAGameplayEffectDurationToolTip", "GameplayEffect Time"))
				.FillWidth(0.4f)

				+ SHeaderRow::Column(NAME_GAGameplayEffectStack)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				//.DefaultLabel(LOCTEXT("GAGameplayEffectStack", "堆栈信息"))
				.DefaultLabel(LOCTEXT("GAGameplayEffectStack", "GameplayEffectStack"))
				.FillWidth(0.1f)

				+ SHeaderRow::Column(NAME_GAGameplayEffectLevel)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				//.DefaultLabel(LOCTEXT("GAGameplayEffectLevel", "等级"))
				.DefaultLabel(LOCTEXT("GAGameplayEffectLevel", "Level"))
				.FillWidth(0.1f)

				+ SHeaderRow::Column(NAME_GAGameplayEffectGrantedTags)
				.HAlignHeader(EHorizontalAlignment::HAlign_Center)
				//.DefaultLabel(LOCTEXT("GAGameplayEffectGrantedTags", "标签"))
				//.DefaultTooltip(LOCTEXT("GAGameplayEffectToolTip", "含有的所有标签"))
				.DefaultLabel(LOCTEXT("GAGameplayEffectGrantedTags", "Tags"))
				.DefaultTooltip(LOCTEXT("GAGameplayEffectToolTip", "GameplayEffect Own Tags"))
				.FillWidth(0.2f)
			)
			]
		];

	return Widget;
}

TSharedRef<ITableRow> SGASAttachEditorImpl::OneGameplayEffecGenerateWidgetForFilterListView(TSharedRef< FGASGameplayEffectNodeBase > InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGASGameplayEffectTreeItem, OwnerTable)
		.WidgetInfoToVisualize(InItem);
}

void SGASAttachEditorImpl::HandleGameplayEffectTreeGetChildren(TSharedRef<FGASGameplayEffectNodeBase> InReflectorNode, TArray<TSharedRef<FGASGameplayEffectNodeBase>>& OutChildren)
{
	OutChildren = InReflectorNode->GetChildNodes();
}

void SGASAttachEditorImpl::HandleGameplayEffectTreeHiddenColumnsListChanged()
{
#if WITH_EDITOR
	if (GameplayEffectTree && GameplayEffectTree->GetHeaderRow())
	{
		const TArray<FName> HiddenColumnIds = GameplayEffectTree->GetHeaderRow()->GetHiddenColumnIds();
		HiddenGameplayEffectTreeColumns.Reset(HiddenColumnIds.Num());
		for (const FName Id : HiddenColumnIds)
		{
			HiddenGameplayEffectTreeColumns.Add(Id.ToString());
		}
		SaveSettings();
	}
#endif
}

void SGASAttachEditorImpl::HandleGameplayEffectTreeSelectionChanged(TSharedPtr<FGASGameplayEffectNodeBase>, ESelectInfo::Type /*SelectInfo*/)
{

}

FAttachInputProcessor::FAttachInputProcessor(SGASAttachEditor* InWidgetPtr)
	:GASAttachEditorWidgetPtr(InWidgetPtr)
{
}

bool FAttachInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::End && GASAttachEditorWidgetPtr)
	{
		GASAttachEditorWidgetPtr->SetPickingMode(false);
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
