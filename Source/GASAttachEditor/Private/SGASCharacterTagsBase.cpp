#include "SGASCharacterTagsBase.h"
#include "HAL/PlatformApplicationMisc.h"
#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "SGASAttachEditor"

void SCharacterTagsView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Regreshing an asset view is an intensive task. Do not do this while a user
	// is dragging arround content for maximum responsiveness.
	// Also prevents a re-entrancy crash caused by potentially complex thumbnail generators.
	if (!FSlateApplication::Get().IsDragDropping())
	{
		STileView<TSharedPtr<FGASCharacterTagsBase>>::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	}
}

void SCharacterTagsViewItem::Construct(const FArguments& InArgs)
{
	TagsItem = InArgs._TagsItem;

	FTextBlockStyle InTextStyle = FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText");
	InTextStyle.ColorAndOpacity = FSlateColor(FLinearColor::White);

	ChildSlot
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "NoBorder")
			.ToolTipText(TagsItem.IsValid() ? TagsItem->GetTagTipName() : FText())
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(this, &SCharacterTagsViewItem::HandleOnClicked)
			[
				SAssignNew(ShowTextTag,STextBlock)
				.Text(TagsItem.IsValid() ? TagsItem->GetTagName() : FText())
				.TextStyle(&InTextStyle)
				
			]
		]
	;
}

FReply SCharacterTagsViewItem::HandleOnClicked()
{
	FString Str = ShowTextTag->GetText().ToString();
	int32 StrIndex = ShowTextTag->GetText().ToString().Find(TEXT("["));
	FPlatformApplicationMisc::ClipboardCopy(*(Str.Left(StrIndex -1)));
	return FReply::Handled();
}

FText FGASCharacterTags::GetTagTipName() const
{
	if (!ASComponent.IsValid())
	{
		return FText();
	}

	FString Str = FString::Printf(TEXT("%s [%d]"),*GameplayTag.ToString(),ASComponent->GetTagCount(GameplayTag));
	FString OutComment; FName OutTagSource; bool bOutIsTagExplicit, bOutIsRestrictedTag, bOutAllowNonRestrictedChildren;
	if (UGameplayTagsManager::Get().GetTagEditorData(*GameplayTag.ToString(),OutComment,OutTagSource,bOutIsTagExplicit,bOutIsRestrictedTag,bOutAllowNonRestrictedChildren))
	{
		// <Tag命名为止
		if (bOutIsTagExplicit)
		{
			Str += FString::Printf(TEXT("(%s)"), *OutTagSource.ToString());;
		}
		else
		{
			Str += TEXT("Implicit");
		}
		// 是否有注释
		if (!OutComment.IsEmpty())
		{
			Str += FString::Printf(TEXT("\n\n%s"),*OutComment);
		}
	}

	for (FGameplayAbilitySpec& AbilitySpec : ASComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive() || !AbilitySpec.Ability) continue;

		FProperty* ActiveTagsPtr = FindFProperty<FProperty>(AbilitySpec.Ability->GetClass(), WidegtName);
		if (!ActiveTagsPtr) continue;
		FGameplayTagContainer* ActivationTags = ActiveTagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (!ActivationTags) continue;

		if (ActivationTags->HasTag(GameplayTag))
		{
			Str += FString::Printf(TEXT("\n\n  %s"),*ASComponent->CleanupName(GetNameSafe(AbilitySpec.Ability)));
		}

		//AbilitySpec.DynamicAbilityTags.
	}

	return FText::FromString(Str);
}

FText FGASCharacterTags::GetTagName() const
{
	if (!ASComponent.IsValid())
	{
		return FText();
	}
	return FText::FromString(FString::Printf(TEXT("%s [%d]"), *GameplayTag.ToString(), ASComponent->GetTagCount(GameplayTag)));
}

TSharedRef<FGASCharacterTags> FGASCharacterTags::Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayTag InGameplayTag, FName InWidegtName)
{
	return MakeShareable(new FGASCharacterTags(InASComponent, InGameplayTag, InWidegtName));
}

FGASCharacterTags::FGASCharacterTags(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayTag InGameplayTag, FName InWidegtName)
	:FGASCharacterTagsBase()
{
	ASComponent = InASComponent;
	GameplayTag = InGameplayTag;
	WidegtName = InWidegtName;
}

#undef LOCTEXT_NAMESPACE