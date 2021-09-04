// <Copyright (C) Monocluar. 2021. All Rights Reserved.

#include "SGASAttributesNodeBase.h"
#include "AbilitySystemComponent.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "SGASAttachEditor"

FGASAttributesNode::~FGASAttributesNode()
{
}

TSharedRef<FGASAttributesNode> FGASAttributesNode::Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, const FGameplayAttribute& InAttribute)
{
	return MakeShareable(new FGASAttributesNode(InASComponent, InAttribute));
}

FName FGASAttributesNode::GetGAName() const
{
	if (!ASComponent.IsValid()) return FName();

	return *Attribute.GetName();
}

float FGASAttributesNode::GetNumericAttribute() const
{
	if (!ASComponent.IsValid()) return -1.f;

	return ASComponent->GetNumericAttribute(Attribute);
}

FGASAttributesNode::FGASAttributesNode(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, const FGameplayAttribute InAttribute)
{
	ASComponent = InASComponent;
	Attribute = InAttribute;
}

void SGASAttributesTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	this->WidgetInfo = InArgs._WidgetInfoToVisualize;
	this->SetPadding(0);

	check(WidgetInfo.IsValid());

	GAName = WidgetInfo->GetGAName();

	NumericAttribute = WidgetInfo->GetNumericAttribute();

	SMultiColumnTableRow< TSharedRef<FGASAttributesNodeBase> >::Construct(SMultiColumnTableRow< TSharedRef<FGASAttributesNodeBase> >::FArguments().Padding(0), InOwnerTableView);
}

TSharedRef<SWidget> SGASAttributesTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (NAME_AttributesName == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(GAName))
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GANumericAttribute == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(NumericAttribute))
				.Justification(ETextJustify::Center)
			];
	}

	return SNullWidget::NullWidget;
}


#undef LOCTEXT_NAMESPACE