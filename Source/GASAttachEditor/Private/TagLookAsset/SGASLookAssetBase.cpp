#include "TagLookAsset/SGASLookAssetBase.h"
#include "AbilitySystemComponent.h"
#include "Widgets/Input/SButton.h"
#include "EditorFontGlyphs.h"
#include "GameplayTagsManager.h"
#include "Widgets/Input/SHyperlink.h"

#define LOCTEXT_NAMESPACE "SGASAttachEditor"

FGASLookAsset::~FGASLookAsset()
{
}


TSharedRef<FGASLookAsset> FGASLookAsset::Create(UObject* AssObj, FAbilityTriggerData InActivationTag)
{
	return MakeShareable(new FGASLookAsset(AssObj,InActivationTag));
}

FName FGASLookAsset::GetTriggerSourceName() const
{
	return UEnum::GetValueAsName(ActivationTag.TriggerSource);
}

FName FGASLookAsset::GetTagName() const
{
	return *ActivationTag.TriggerTag.ToString();
}


FName FGASLookAsset::GetAbilitieAsset() const
{
	return LookAssObj->GetFName();
}

UObject* FGASLookAsset::GetAbilitieAssetObj() const
{
	return LookAssObj;
}

FGASLookAsset::FGASLookAsset(UObject* AssObj, FAbilityTriggerData InActivationTag)
{
	LookAssObj = AssObj;
	ActivationTag = InActivationTag;
}

void SGASLookAssetTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	this->WidgetInfo = InArgs._WidgetInfoToVisualize;
	this->SetPadding(0);

	check(WidgetInfo.IsValid());

	TagName = WidgetInfo->GetTagName();

	AbilitieAsset = WidgetInfo->GetAbilitieAsset();
	LookAssObj = WidgetInfo->GetAbilitieAssetObj();
	TriggerSourceName = WidgetInfo->GetTriggerSourceName();
	SMultiColumnTableRow< TSharedRef<FGASLookAssetBase> >::Construct(SMultiColumnTableRow< TSharedRef<FGASLookAssetBase> >::FArguments().Padding(0), InOwnerTableView);
}

TSharedRef<SWidget> SGASLookAssetTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (NAME_TagName == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(TagName))
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_AbilitieAsset == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				
				SNew(SBorder)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				//.Padding(FMargin(2.0f, 0.0f))
				.Visibility(EVisibility::SelfHitTestInvisible)
				.BorderBackgroundColor(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)))
				//.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,0.5f))
				[
					SNew(SHyperlink)
					.Text_Lambda([this]() -> FText { return  FText::FromName(AbilitieAsset); })
					.OnNavigate(this, &SGASLookAssetTreeItem::HandleHyperlinkNavigate)
				]
			];
	}
	else if (NAME_TriggerSource == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(TriggerSourceName))
				.Justification(ETextJustify::Center)
			];
	}

	return SNullWidget::NullWidget;
}


void SGASLookAssetTreeItem::HandleHyperlinkNavigate()
{
	if (!LookAssObj) return;

#if WITH_EDITOR
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(LookAssObj);
#endif
}

void SGASTagViewItem::Construct(const FArguments& InArgs)
{
	TagName = InArgs._TagName;
	OnLookAssetDel = InArgs._OnLookAssetDel;
	FTextBlockStyle InTextStyle = FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText");
	InTextStyle.ColorAndOpacity = FSlateColor(FLinearColor::White);

	ChildSlot
		[
			SNew(SBorder)
			.Padding(1.f)
			.ToolTipText_Raw(this, &SGASTagViewItem::OnTipTag)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
				
					SNew(SButton)
					.ContentPadding(FMargin(0))
					.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
					.ForegroundColor(FSlateColor::UseForeground())
					.OnClicked(this, &SGASTagViewItem::OnRemoveTagClicked)
					[
						SNew(STextBlock)
						.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
						.Text(FEditorFontGlyphs::Times)
					]
			
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TagName.Get().ToString()))
					.TextStyle(&InTextStyle)
				]
			]
		];
}

FReply SGASTagViewItem::OnRemoveTagClicked()
{
	if (OnLookAssetDel.IsBound())
	{
		OnLookAssetDel.Execute(TagName.Get());
	}
	return FReply::Handled();
}

FText SGASTagViewItem::OnTipTag() const
{
#if WITH_EDITOR
	FString OutComment; FName OutTagSource; bool bOutIsTagExplicit, bOutIsRestrictedTag, bOutAllowNonRestrictedChildren;
	if (UGameplayTagsManager::Get().GetTagEditorData(*TagName.Get().ToString(), OutComment, OutTagSource, bOutIsTagExplicit, bOutIsRestrictedTag, bOutAllowNonRestrictedChildren))
	{
		// 是否有注释
		if (!OutComment.IsEmpty())
		{
			return FText::FromString(OutComment);
		}
	}
#endif

	return FText();
}

#undef LOCTEXT_NAMESPACE

