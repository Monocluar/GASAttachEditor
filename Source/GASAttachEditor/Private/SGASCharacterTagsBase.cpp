#include "SGASCharacterTagsBase.h"
#include "HAL/PlatformApplicationMisc.h"

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
	FPlatformApplicationMisc::ClipboardCopy(*(ShowTextTag->GetText().ToString()));
	return FReply::Handled();
}

FText FGASCharacterTags::GetTagName() const
{
	if (!ASComponent.IsValid())
	{
		return FText();
	}
	return FText::FromString(FString::Printf(TEXT("%s [%d]"),*GameplayTag.ToString(),ASComponent->GetTagCount(GameplayTag)));
}

FText FGASCharacterTags::GetTagTipName() const
{
	if (!ASComponent.IsValid())
	{
		return FText();
	}
	return FText::FromString(FString::Printf(TEXT("%s [%d]"), *GameplayTag.ToString(), ASComponent->GetTagCount(GameplayTag)));
}

TSharedRef<FGASCharacterTags> FGASCharacterTags::Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayTag InGameplayTag)
{
	return MakeShareable(new FGASCharacterTags(InASComponent, InGameplayTag));
}

FGASCharacterTags::FGASCharacterTags(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayTag InGameplayTag)
	:FGASCharacterTagsBase()
{
	ASComponent = InASComponent;
	GameplayTag = InGameplayTag;
}

#undef LOCTEXT_NAMESPACE