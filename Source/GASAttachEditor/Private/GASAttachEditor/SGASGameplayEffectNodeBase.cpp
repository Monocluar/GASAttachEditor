#include "SGASGameplayEffectNodeBase.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "SGASAttachEditor"

FGASGameplayEffectNode::~FGASGameplayEffectNode()
{
}

TSharedRef<FGASGameplayEffectNode> FGASGameplayEffectNode::Create(const UWorld* InWorld, const FActiveGameplayEffect& InGameplayEffect)
{
	return MakeShareable(new FGASGameplayEffectNode(InWorld, InGameplayEffect));
}

FText FGASGameplayEffectNode::GetDurationText() const
{
	FText DurationText;
	if (!World)
	{
		if (ModSpec && ModInfo)
		{
			UEnum* e = StaticEnum<EGameplayModOp::Type>();
			FString ModifierOpStr = e->GetNameStringByValue(ModInfo->ModifierOp);
			DurationText = FText::Format(LOCTEXT("GameplayEffectMod", "Mod: {0}, Value: {1}"), FText::FromString(ModifierOpStr), ModSpec->GetEvaluatedMagnitude());
		}

		return DurationText;
	}

	DurationText = LOCTEXT("GameplayEffectInfiniteDurationText", "Infinite Duration");

	FNumberFormattingOptions NumberFormatOptions;
	NumberFormatOptions.MaximumFractionalDigits = 2;
	if (GameplayEffect.GetDuration() > 0.f)
	{
		DurationText = FText::Format(LOCTEXT("GameplayEffectDurationStr", "Duration: {0},Remaining: {1} (Start: {2} / {3} / {4})"),
			FText::AsNumber(GameplayEffect.GetDuration(), &NumberFormatOptions),
			FText::AsNumber(GameplayEffect.GetTimeRemaining(World->GetTimeSeconds()), &NumberFormatOptions),
			FText::AsNumber(GameplayEffect.StartServerWorldTime, &NumberFormatOptions),
			FText::AsNumber(GameplayEffect.CachedStartServerWorldTime, &NumberFormatOptions),
			FText::AsNumber(GameplayEffect.StartWorldTime, &NumberFormatOptions));
	}

	if (GameplayEffect.GetPeriod() > 0.f)
	{
		DurationText = FText::Format(LOCTEXT("GameplayEffectPeriod","{0} Period: {1}"), DurationText, FText::AsNumber(GameplayEffect.GetPeriod(), &NumberFormatOptions));
	}

	return DurationText;
}

FText FGASGameplayEffectNode::GetStackText() const
{
	FText StackText;

	if (World && GameplayEffect.Spec.GetStackCount() > 1)
	{
		if (GameplayEffect.Spec.Def->StackingType == EGameplayEffectStackingType::AggregateBySource)
		{
			StackText =  FText::Format(LOCTEXT("GameplayEffectStacksForm", "Stacks: {0},From: {1}"), GameplayEffect.Spec.GetStackCount(), FText::FromString(GetNameSafe(GameplayEffect.Spec.GetContext().GetInstigatorAbilitySystemComponent()->GetAvatarActor_Direct())));
		}
		else
		{
			StackText =  FText::Format(LOCTEXT("GameplayEffectStacks", "Stacks: {0}"), GameplayEffect.Spec.GetStackCount());
		}
	}

	return StackText;
}

FName FGASGameplayEffectNode::GetLevelStr() const
{
	if (!World) return FName();

	return *LexToSanitizedString(GameplayEffect.Spec.GetLevel());
}

FText FGASGameplayEffectNode::GetPredictedText() const
{
	FText PredictionText;

	if (World && GameplayEffect.PredictionKey.IsValidKey())
	{
		if (GameplayEffect.PredictionKey.WasLocallyGenerated())
		{
			PredictionText =  LOCTEXT("GameplayEffectPredictedWaiting", "Predicted and Waiting");
		}
		else
		{
			PredictionText = LOCTEXT("GameplayEffectPredictedCaught", "Predicted and Caught Up");
		}
	}

	return PredictionText;
}

FName FGASGameplayEffectNode::GetGrantedTagsName() const
{
	if (!World) return FName();

	FGameplayTagContainer GrantedTags;
	GameplayEffect.Spec.GetAllGrantedTags(GrantedTags);

	return *GrantedTags.ToStringSimple();
}

FName FGASGameplayEffectNode::GetGAName() const
{
	if (ModSpec && ModInfo)
	{
		return *ModInfo->Attribute.GetName();
	}

	return *GetNameSafe(GameplayEffect.Spec.Def);
}

FGASGameplayEffectNode::FGASGameplayEffectNode(const UWorld* InWorld, const FActiveGameplayEffect InGameplayEffect)
{
	GameplayEffect = InGameplayEffect;
	World = InWorld;
	ModInfo = nullptr;
	ModSpec = nullptr;

	CreateChild();
}

FGASGameplayEffectNode::FGASGameplayEffectNode(const FModifierSpec* InModSpec,const FGameplayModifierInfo* InModInfo)
{
	World = nullptr;
	ModInfo = InModInfo;
	ModSpec = InModSpec;
}

void FGASGameplayEffectNode::CreateChild()
{
	for (int32 ModIdx = 0; ModIdx < GameplayEffect.Spec.Modifiers.Num(); ++ModIdx)
	{
		if (GameplayEffect.Spec.Def == nullptr) break;
		AddChildNode(MakeShareable(new FGASGameplayEffectNode(&GameplayEffect.Spec.Modifiers[ModIdx], &GameplayEffect.Spec.Def->Modifiers[ModIdx])));
	}
}

void SGASGameplayEffectTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	this->WidgetInfo = InArgs._WidgetInfoToVisualize;
	this->SetPadding(0);

	check(WidgetInfo.IsValid());

	GAName = WidgetInfo->GetGAName();
	DurationText = WidgetInfo->GetDurationText();
	StackText = WidgetInfo->GetStackText();
	LevelStr = WidgetInfo->GetLevelStr();
	GrantedTagsName = WidgetInfo->GetGrantedTagsName();
	
	SMultiColumnTableRow< TSharedRef<FGASGameplayEffectNodeBase> >::Construct(SMultiColumnTableRow< TSharedRef<FGASGameplayEffectNodeBase> >::FArguments().Padding(0), InOwnerTableView);
}

TSharedRef<SWidget> SGASGameplayEffectTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (NAME_GAGameplayEffectName == ColumnName)
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
				SNew(SBox)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(FMargin(2.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromName(GAName))
					.Justification(ETextJustify::Center)
				]
			];
	}
	else if (NAME_GAGameplayEffectDuration == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(DurationText)
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAGameplayEffectStack == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(StackText)
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAGameplayEffectLevel == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(LevelStr))
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAGameplayEffectGrantedTags == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(GrantedTagsName))
				.Justification(ETextJustify::Center)
				.ToolTipText(FText::FromName(GrantedTagsName))
			];
	}

	return SNullWidget::NullWidget;
}


void FGASGameplayEffectNodeBase::AddChildNode(TSharedRef<FGASGameplayEffectNodeBase> InChildNode)
{
	ChildNodes.Add(MoveTemp(InChildNode));
}

const TArray<TSharedRef<FGASGameplayEffectNodeBase>>& FGASGameplayEffectNodeBase::GetChildNodes() const
{
	return ChildNodes;
}

#undef LOCTEXT_NAMESPACE

