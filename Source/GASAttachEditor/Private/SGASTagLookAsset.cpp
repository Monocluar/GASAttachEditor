#include "SGASTagLookAsset.h"
#include "SGameplayTagWidget.h"
#include "Widgets/Layout/SWrapBox.h"
#include "TagLookAsset/SGASLookAssetBase.h"
#include "Layout/Children.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Abilities/GameplayAbility.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "FGASAttachEditorModule"

class SGASTagLookAssetImpl : public SGASTagLookAsset
{
	typedef STreeView<TSharedRef<FGASLookAssetBase>> SLookAssetTree;
public:
	virtual void Construct(const FArguments& InArgs) override;

protected:
	// 右键添加或者删除Tag
#if WITH_EDITOR
	FReply OnMouseButtonUpTags(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
#endif
	// <Tag控件选择
	void RefreshTagList();

	void OnDelTag(FGameplayTag Item);

	/** 树视图生成树的回调 */
	TSharedRef<ITableRow> HandleAttributesWidgetForFilterListView(TSharedRef< FGASLookAssetBase > InItem, const TSharedRef<STableViewBase>& OwnerTable);

	void HandleAttributesTreeGetChildren( TSharedRef<FGASLookAssetBase> InReflectorNode, TArray<TSharedRef<FGASLookAssetBase>>& OutChildren );

	void FillLookTagAsset();

	void SetGraphRootIdentifiers(const TArray<FAssetIdentifier>& NewGraphRootIdentifiers, const FReferenceViewerParams& ReferenceViewerParams);


private:
#if WITH_EDITOR
	// 当前拥有的Tag
	TArray<SGameplayTagWidget::FEditableGameplayTagContainerDatum> EditableContainers;
	// 筛选的tag
	FGameplayTagContainer TagContainer;
	FGameplayTagContainer OldTagContainer;
#endif

	// 筛选标签组控件
	TSharedPtr<SWrapBox> FilteredOwnedTagsView;

	// 树状控件
	TSharedPtr<SLookAssetTree> LookGAAssetTree;
	// 树状控件根部
	TArray<TSharedRef<FGASLookAssetBase>> LookGAAssetTreeRoot;

};

void SGASTagLookAssetImpl::Construct(const FArguments& InArgs)
{

	ChildSlot
		[
			SNew(SSplitter)
			.Orientation(Orient_Vertical)

			+ SSplitter::Slot()
			.Value(0.2f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				.Padding(2.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AbilityTriggersEvent", "调用GA事件的Tags"))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SNew(SBorder)
					.Padding(2.f)
#if WITH_EDITOR
					.OnMouseButtonUp(this, &SGASTagLookAssetImpl::OnMouseButtonUpTags)
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
			.Value(0.8f)
			[
				SNew(SBorder)
				.Padding(0.f)
				[
					SAssignNew(LookGAAssetTree,SLookAssetTree)
					.ItemHeight(32.f)
					.TreeItemsSource(&LookGAAssetTreeRoot)
					.OnGenerateRow(this, &SGASTagLookAssetImpl::HandleAttributesWidgetForFilterListView)
					.OnGetChildren(this, &SGASTagLookAssetImpl::HandleAttributesTreeGetChildren)
					.HighlightParentNodesForSelection(true)
					.HeaderRow
					(
						SNew(SHeaderRow)
						.CanSelectGeneratedColumn(true)

						+ SHeaderRow::Column(NAME_TagName)
						.DefaultLabel(LOCTEXT("TagName", "标签名称"))
						.FillWidth(0.3f)
						.ShouldGenerateWidget(true)

						+ SHeaderRow::Column(NAME_AbilitieAsset)
						.DefaultLabel(LOCTEXT("AbilitieAsset", "资源"))
						.FillWidth(0.5f)

						+ SHeaderRow::Column(NAME_TriggerSource)
						.DefaultLabel(LOCTEXT("TriggerSource", "响应类型"))
						.FillWidth(0.2f)
					)
				]
			]
		];

	EditableContainers.Empty();
	TagContainer.Reset();
	EditableContainers.Add(SGameplayTagWidget::FEditableGameplayTagContainerDatum(nullptr,&TagContainer));
}

#if WITH_EDITOR
FReply SGASTagLookAssetImpl::OnMouseButtonUpTags(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		TSharedPtr<SGameplayTagWidget> TagWidget =
			SNew(SGameplayTagWidget,  EditableContainers)
			.GameplayTagUIMode(EGameplayTagUIMode::SelectionMode)
			.ReadOnly(false)
			.OnTagChanged(this, &SGASTagLookAssetImpl::RefreshTagList);

		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, TagWidget.ToSharedRef(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	}
	return FReply::Handled();
}

#endif

void SGASTagLookAssetImpl::RefreshTagList()
{
	if (TagContainer.Num() > OldTagContainer.Num())
	{
		TArray<FGameplayTag> NewGameplayTagArr;
		TagContainer.GetGameplayTagArray(NewGameplayTagArr);
		// 是否增加
		for (const FGameplayTag& Item : NewGameplayTagArr)
		{
			if (OldTagContainer.HasTag(Item)) continue;
			FilteredOwnedTagsView->AddSlot()
				[
					SNew(SGASTagViewItem)
					.TagName(Item)
					.OnLookAssetDel(this,&SGASTagLookAssetImpl::OnDelTag)
				];
			//SelectAbilitySystemComponent->AddLooseGameplayTag(Item);
		}
	}
	else
	{
		TArray<FGameplayTag> OldGameplayTagArr;
		OldTagContainer.GetGameplayTagArray(OldGameplayTagArr);
		// 是否是减少
		for (const FGameplayTag& Item : OldGameplayTagArr)
		{
			if(TagContainer.HasTag(Item)) continue;
			TPanelChildren<SWrapBox::FSlot>* ViewSlots = static_cast<TPanelChildren<SWrapBox::FSlot>*>(FilteredOwnedTagsView->GetChildren());

			for (int32 SlotIdx = 0; SlotIdx < ViewSlots->Num(); ++SlotIdx)
			{
				SWidget* SlotItem = &ViewSlots->GetChildAt(SlotIdx).Get();
				if (static_cast<SGASTagViewItem*>(SlotItem)->TagName.Get() == Item)
				{
					ViewSlots->RemoveAt(SlotIdx);
				}
			}

		}
	}
	
	OldTagContainer = TagContainer;

	FillLookTagAsset();
}

void SGASTagLookAssetImpl::OnDelTag(FGameplayTag Item)
{
	if (!TagContainer.HasTag(Item)) return;

	TPanelChildren<SWrapBox::FSlot>* ViewSlots = static_cast<TPanelChildren<SWrapBox::FSlot>*>(FilteredOwnedTagsView->GetChildren());

	for (int32 SlotIdx = 0; SlotIdx < ViewSlots->Num(); ++SlotIdx)
	{
		SWidget* SlotItem = &ViewSlots->GetChildAt(SlotIdx).Get();
		if (static_cast<SGASTagViewItem*>(SlotItem)->TagName.Get() == Item)
		{
			ViewSlots->RemoveAt(SlotIdx);
		}
	}

	TagContainer.RemoveTag(Item);

	OldTagContainer = TagContainer;

	FillLookTagAsset();
}

TSharedRef<ITableRow> SGASTagLookAssetImpl::HandleAttributesWidgetForFilterListView(TSharedRef< FGASLookAssetBase > InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGASLookAssetTreeItem,OwnerTable)
		.WidgetInfoToVisualize(InItem);
}

void SGASTagLookAssetImpl::HandleAttributesTreeGetChildren(TSharedRef<FGASLookAssetBase> InReflectorNode, TArray<TSharedRef<FGASLookAssetBase>>& OutChildren)
{

}

void SGASTagLookAssetImpl::FillLookTagAsset()
{
	TArray<FGameplayTag> NewGameplayTagArr;
	TagContainer.GetGameplayTagArray(NewGameplayTagArr);

	TArray<FAssetIdentifier> AssetIdentifiers;

	for (const FGameplayTag& Item : NewGameplayTagArr)
	{
		AssetIdentifiers.Add(FAssetIdentifier(FGameplayTag::StaticStruct(), Item.GetTagName()));
	}

	SetGraphRootIdentifiers(AssetIdentifiers,FReferenceViewerParams());
}


void SGASTagLookAssetImpl::SetGraphRootIdentifiers(const TArray<FAssetIdentifier>& NewGraphRootIdentifiers, const FReferenceViewerParams& ReferenceViewerParams)
{
	LookGAAssetTreeRoot.Reset();
#if WITH_EDITOR
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetDependency> LinksToAsset;
	for (const FAssetIdentifier& AssetId : NewGraphRootIdentifiers)
	{
		AssetRegistry.GetReferencers(AssetId, LinksToAsset, UE::AssetRegistry::EDependencyCategory::SearchableName, UE::AssetRegistry::EDependencyQuery::NoRequirements);
	}
	for (FAssetDependency& Asset : LinksToAsset)
	{
		FString PackStr = Asset.AssetId.PackageName.ToString();
		int32 Index = PackStr.Find("/",ESearchCase::CaseSensitive,ESearchDir::FromEnd);

		FString AssPath = FString::Printf(TEXT("Blueprint'%s.%s'"), *PackStr,*PackStr.Right(PackStr.Len() - Index - 1));

		FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(*AssPath);

		if (!AssetData.IsValid()) continue;

		UObject* AssObj = AssetData.GetAsset();

		UGameplayAbility* GAAssObj = Cast<UGameplayAbility>(AssObj);
		if (!GAAssObj)
		{
			UBlueprint* GAAssBlue = Cast<UBlueprint>(AssObj);
			if (!GAAssBlue) continue;

			//TSubclassOf<UGameplayAbility> GAAssClass = TSubclassOf<UGameplayAbility>(GAAssBlue->GeneratedClass);

			GAAssObj = Cast<UGameplayAbility>(GAAssBlue->GeneratedClass->GetDefaultObject());
			/*FSoftClassPath ClassPath(GAAssBlue->GeneratedClass.Get()->GetPathName());
			if (UClass* Class = ClassPath.TryLoadClass<UGameplayAbility>())
			{
				GAAssObj = NewObject<UGameplayAbility>(Class, NAME_None, RF_Transactional);
			}*/

			if (!GAAssObj)	continue;
		}


		FArrayProperty* ActiveTagsPtr = FindFProperty<FArrayProperty>(GAAssObj->GetClass(), "AbilityTriggers");
		if (!ActiveTagsPtr) continue;
		TArray<FAbilityTriggerData> ActivationTags = *ActiveTagsPtr->ContainerPtrToValuePtr<TArray<FAbilityTriggerData>>(GAAssObj);

		bool bHasTag = false;
		for (FAbilityTriggerData& Item : ActivationTags)
		{
			if (TagContainer.HasTag(Item.TriggerTag))
			{
				bHasTag = true;
				LookGAAssetTreeRoot.Add(FGASLookAsset::Create(AssObj, Item));
			}
		}

		if (bHasTag)
		{
		}
	}

	LookGAAssetTreeRoot.Sort([](TSharedRef<FGASLookAssetBase> A,TSharedRef<FGASLookAssetBase> B)
	{
		if (A->GetTagName() != B->GetTagName())
		{
			return A->GetTagName().GetStringLength() == B->GetTagName().GetStringLength();
		}
		return true;
	});

#endif
	LookGAAssetTree->RequestTreeRefresh();
}

TSharedRef<SGASTagLookAsset> SGASTagLookAsset::New()
{
	return MakeShareable(new SGASTagLookAssetImpl());
}

FName SGASTagLookAsset::GetTabName()
{
	return "GASTagLookAssetApp";
}

void SGASTagLookAsset::RegisterTabSpawner(FTabManager& TabManager)
{
	const auto SpawnCallStackViewTab = [](const FSpawnTabArgs& Args)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::PanelTab)
			.Label(LOCTEXT("TabTitle", "Tag调用GA查询器"))
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
				.BorderBackgroundColor(FSlateColor(FLinearColor(0.2f,0.2f,0.2f,1.f)))
			[
				SNew(SGASTagLookAsset)
			]
			];
	};

	TabManager.RegisterTabSpawner(SGASTagLookAsset::GetTabName(), FOnSpawnTab::CreateStatic(SpawnCallStackViewTab))
		.SetDisplayName(LOCTEXT("TabTitle", "Tag调用GA查询器"));
}

#undef LOCTEXT_NAMESPACE