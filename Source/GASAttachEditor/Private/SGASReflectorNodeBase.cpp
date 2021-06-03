#include "SGASReflectorNodeBase.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SHyperlink.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Editor/EditorEngine.h"
#include "AssetRegistry/AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "SGASAttachEditor"

EGAAbilitieNode FGASAbilitieNodeBase::GetNodeType() const
{
	return GAAbilitieNode;
}

FGASAbilitieNodeBase::FGASAbilitieNodeBase()
	:Tint(FLinearColor(1.f,1.f,1.f,0.5f))
	,bIsShow(true)
	,GAAbilitieNode(EGAAbilitieNode::Node_Abilitie)
{

}

const FLinearColor& FGASAbilitieNodeBase::GetTint() const
{
	return Tint;
}

void FGASAbilitieNodeBase::SetTint(const FLinearColor& InTint)
{
	Tint = InTint;
}

void FGASAbilitieNodeBase::AddChildNode(TSharedRef<FGASAbilitieNodeBase> InChildNode)
{
	ChildNodes.Add(MoveTemp(InChildNode));
}

const TArray<TSharedRef<FGASAbilitieNodeBase>>& FGASAbilitieNodeBase::GetChildNodes() const
{
	return ChildNodes;
}

void FGASAbilitieNodeBase::SetTreeItemVis(FOnTreeItemVis InHandle)
{
	OnShowHandle = InHandle;
}


void FGASAbilitieNodeBase::SetItemVisility(bool bShow)
{
	bIsShow = bShow;
	if (OnShowHandle.IsBound())
	{
		OnShowHandle.Execute(bShow);
	}
}

bool FGASAbilitieNodeBase::IsShow() const
{
	return bIsShow;
}

void SGASAbilitieTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	this->WidgetInfo = InArgs._WidgetInfoToVisualize;
	this->SetPadding(0);

	check(WidgetInfo.IsValid());

	GAName = WidgetInfo->GetGAName();
	GAStateType = WidgetInfo->GetGAStateType();
	bGAIsActive = WidgetInfo->GetGAIsActive();
	GAAbilitieNode = WidgetInfo->GetNodeType();

	CachedWidgetFile = WidgetInfo->GetWidgetFile();
	CachedWidgetLineNumber = WidgetInfo->GetWidgetLineNumber();
	CachedAssetDataStr = WidgetInfo->GetWidgetAssetData();

	WidgetInfo->SetTreeItemVis(FOnTreeItemVis::CreateRaw(this, &SGASAbilitieTreeItem::HanldeTreeItemVis));

	if (!WidgetInfo->IsShow())
	{
		SetVisibility(EVisibility::Collapsed);
	}

	SMultiColumnTableRow< TSharedRef<FGASAbilitieNodeBase> >::Construct(SMultiColumnTableRow< TSharedRef<FGASAbilitieNodeBase> >::FArguments().Padding(0), InOwnerTableView);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> SGASAbilitieTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (NAME_AbilitietName == ColumnName)
	{
		if (GAAbilitieNode == Node_Abilitie)
		{
			return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SExpanderArrow, SharedThis(this))
					.IndentAmount(16)
					.ShouldDrawWires(true)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					//.Padding(FMargin(2.0f, 0.0f))
					.Visibility(EVisibility::SelfHitTestInvisible)
					.BorderBackgroundColor(FSlateColor(FLinearColor(1.f,1.f,1.f,0.f)))
					.ColorAndOpacity(this, &SGASAbilitieTreeItem::GetTint)
					[
						SNew(SHyperlink)
						.Text(this, &SGASAbilitieTreeItem::GetReadableLocationAsText)
						.OnNavigate(this, &SGASAbilitieTreeItem::HandleHyperlinkNavigate)
					]
				] ;
		}
		else
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SExpanderArrow, SharedThis(this))
					.IndentAmount(16)
					.ShouldDrawWires(true)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SGASAbilitieTreeItem::GetReadableLocationAsText)
				];
		}
	}
	else if (NAME_GAStateType == ColumnName)
	{
		return SNew(SBorder)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			.Visibility(EVisibility::SelfHitTestInvisible)
			.BorderBackgroundColor(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)))
			.ColorAndOpacity(this, &SGASAbilitieTreeItem::GetTint)
			[
				SNew(STextBlock)
				.Text(this, &SGASAbilitieTreeItem::GetGAStateTypeAsString)
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAIsActive == ColumnName)
	{
		return SNew(SBorder)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			.Visibility(EVisibility::SelfHitTestInvisible)
			.BorderBackgroundColor(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)))
			.ColorAndOpacity(this, &SGASAbilitieTreeItem::GetTint)
			[
				SNew(STextBlock)
				.Text(this, &SGASAbilitieTreeItem::GetGAIsActiveAsString)
				.Justification(ETextJustify::Center)
			];
	}


	return SNullWidget::NullWidget;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FText SGASAbilitieTreeItem::GetReadableLocationAsText() const
{
	return FText::FromName(GAName);
}

void SGASAbilitieTreeItem::HandleHyperlinkNavigate()
{
	if (CachedAssetDataStr.IsEmpty())
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*CachedAssetDataStr);

	if (AssetData.IsValid())
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetData.GetAsset());
	}

	//FAccessSourceCode::CreateRaw( CurrentSourceCodeAccessor, &ISourceCodeAccessor::OpenFileAtLine )
}

void SGASAbilitieTreeItem::HanldeTreeItemVis(bool IsShow)
{
	SetVisibility(IsShow ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
}

TSharedRef<FGASAbilitieNode> FGASAbilitieNode::Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayAbilitySpec InAbilitySpecPtr)
{
	return MakeShareable(new FGASAbilitieNode(InASComponent, InAbilitySpecPtr));
}

TSharedRef<FGASAbilitieNode> FGASAbilitieNode::Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayAbilitySpec InAbilitySpecPtr, TWeakObjectPtr<UGameplayTask> InGameplayTask)
{
	return MakeShareable(new FGASAbilitieNode(InASComponent,InAbilitySpecPtr, InGameplayTask));
}

FName FGASAbilitieNode::GetGAName() const
{
	if (!ASComponent.IsValid())
	{
		return FName();
	}

	switch (GAAbilitieNode)
	{
	case Node_Abilitie:
		return *ASComponent->CleanupName(GetNameSafe(AbilitySpecPtr.Ability));
		break;
	case Node_Task:
		return GameplayTask.IsValid() ? *GameplayTask->GetDebugString() : FName();
		break;
	case Node_Message:
		break;
	}
	return FName();
}

FText FGASAbilitieNode::GetGAStateType()
{
	FText OutType;
	if (GAAbilitieNode != Node_Abilitie)
	{
		return OutType;
	}

	ScreenGAMode = NoActive;
	FGameplayTagContainer FailureTags;
	if (!ASComponent.IsValid())
	{
		return OutType;
	}
	if (AbilitySpecPtr.IsActive())
	{
		OutType = FText::Format(FText::FromString(TEXT("{0}:{1}")),LOCTEXT("ActiveIndex", "激活数"),AbilitySpecPtr.ActiveCount);
		Tint = FLinearColor::White;
		ScreenGAMode = Active;
	}
	else if (ASComponent->IsAbilityInputBlocked(AbilitySpecPtr.InputID))
	{
		OutType = LOCTEXT("InputBlocked", "输入阻止");
		Tint = FLinearColor::Red;
		ScreenGAMode = Blocked;
	}
	else if (ASComponent->AreAbilityTagsBlocked(AbilitySpecPtr.Ability->AbilityTags))
	{
		FGameplayTagContainer BlockedAbility;
		ASComponent->GetBlockedAbilityTags(BlockedAbility);
		OutType = LOCTEXT("TagBlocked", "有阻止的Tag");
		Tint = FLinearColor::Red;
		ScreenGAMode = Blocked;
	}
	else if (AbilitySpecPtr.Ability->CanActivateAbility(AbilitySpecPtr.Handle, ASComponent->AbilityActorInfo.Get(), nullptr, nullptr, &FailureTags) == false)
	{
		OutType = LOCTEXT("CantActivate","被阻止激活");
		float Cooldown =  AbilitySpecPtr.Ability->GetCooldownTimeRemaining(ASComponent->AbilityActorInfo.Get());
		if (Cooldown > 0.f)
		{
			OutType = FText::Format(FText::FromString(TEXT("{0},{1}:{2}s")),OutType ,LOCTEXT("Cooldown", "CD时间未完"),Cooldown);
		}
		Tint = FLinearColor::Red;
		ScreenGAMode = Blocked;
	}

	return OutType;
}

bool FGASAbilitieNode::GetGAIsActive() const
{
	if (!ASComponent.IsValid())
	{
		return false;
	}

	return AbilitySpecPtr.IsActive();
}

FText FGASAbilitieNode::GetAbilitieHasTag() const
{
	return FText();
}

FString FGASAbilitieNode::GetWidgetFile() const
{
	if (!ASComponent.IsValid() || GAAbilitieNode != Node_Abilitie)
	{
		return FString();
	}

	if (UGameplayAbility* Ability = AbilitySpecPtr.Ability)
	{
		return Ability->GetPathName();
	}

	return FString();
}

int32 FGASAbilitieNode::GetWidgetLineNumber() const
{
	return 0;
}

bool FGASAbilitieNode::HasValidWidgetAssetData() const
{
	return GAAbilitieNode == Node_Abilitie;
}

FString FGASAbilitieNode::GetWidgetAssetData() const
{
	if (GAAbilitieNode != Node_Abilitie)
	{
		return FString();
	}

	if (UGameplayAbility* Ability = AbilitySpecPtr.Ability)
	{

		if (Ability->IsAsset())
		{
			Ability->GetPathName();
		}
		else
		{
			if (UObject* OutComp = Ability->GetOuter())
			{
				return FString::Printf(TEXT("Blueprint'%s.%s'"), *OutComp->GetPathName(), *(Ability->GetClass()->GetFName().ToString()));
			}
		}
	}

	return FString();
}


FGASAbilitieNode::FGASAbilitieNode(TWeakObjectPtr<UAbilitySystemComponent> InASComponent,  FGameplayAbilitySpec InAbilitySpecPtr)
	:FGASAbilitieNodeBase()
{
	AbilitySpecPtr = InAbilitySpecPtr;
	ASComponent = InASComponent;

	GAAbilitieNode = Node_Abilitie;

	GetGAStateType();

	CreateChild();
}

FGASAbilitieNode::FGASAbilitieNode(TWeakObjectPtr<UAbilitySystemComponent> InASComponent,  FGameplayAbilitySpec InAbilitySpecPtr, TWeakObjectPtr<UGameplayTask> InGameplayTask)
{
	ASComponent = InASComponent;
	AbilitySpecPtr = InAbilitySpecPtr;
	GameplayTask = InGameplayTask;

	GAAbilitieNode = Node_Task;
}

void FGASAbilitieNode::CreateChild()
{
	if (!ASComponent.IsValid()) return;

	if (!AbilitySpecPtr.IsActive()) return;

	TArray<UGameplayAbility*> Instances = AbilitySpecPtr.GetAbilityInstances();

	for (UGameplayAbility* Instance : Instances)
	{
		if (!Instance) continue;

		// 因为Instance->ActiveTasks在protected里面，也没有其Get方法，好在他是UPROPERTY带UE4反射的结构体

		UArrayProperty* ActiveTasksPtr = FindField<UArrayProperty>(Instance->GetClass(),"ActiveTasks");

		if (!ActiveTasksPtr) continue;

		TArray<UGameplayTask*> ActiveTasks = *ActiveTasksPtr->ContainerPtrToValuePtr<TArray<UGameplayTask*>>(Instance);

		for (UGameplayTask*& Item : ActiveTasks)
		{
			AddChildNode(FGASAbilitieNode::Create(ASComponent, AbilitySpecPtr , Item));
		}

	}
}

#undef LOCTEXT_NAMESPACE