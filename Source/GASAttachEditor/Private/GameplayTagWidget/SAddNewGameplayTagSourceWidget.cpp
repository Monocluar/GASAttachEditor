// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAddNewGameplayTagSourceWidget.h"
#include "DetailLayoutBuilder.h"
#include "GameplayTagsSettings.h"
#include "GameplayTagsEditorModule.h"
#include "GameplayTagsModule.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "AddNewGameplayTagSourceWidget"

void SAddNewGameplayTagSourceWidget::Construct(const FArguments& InArgs)
{
	FText HintText = LOCTEXT("NewSourceNameHint", "SourceName.ini");
	DefaultNewName = InArgs._NewSourceName;
	if (DefaultNewName.IsEmpty() == false)
	{
		HintText = FText::FromString(DefaultNewName);
	}

	bShouldGetKeyboardFocus = false;

	OnGameplayTagSourceAdded = InArgs._OnGameplayTagSourceAdded;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Tag Name
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2.0f, 4.0f)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NewSourceName", "Name:"))
			]

			+ SHorizontalBox::Slot()
			.Padding(2.0f, 2.0f)
			.FillWidth(1.0f)
			.HAlign(HAlign_Right)
			[
				SAssignNew(SourceNameTextBox, SEditableTextBox)
				.MinDesiredWidth(240.0f)
				.HintText(HintText)
			]
		]

		// Add Source Button
		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		.Padding(8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("AddNew", "Add New Source"))
				.OnClicked(this, &SAddNewGameplayTagSourceWidget::OnAddNewSourceButtonPressed)
			]
		]
	];

	Reset();
}

void SAddNewGameplayTagSourceWidget::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	if (bShouldGetKeyboardFocus)
	{
		bShouldGetKeyboardFocus = false;
		FSlateApplication::Get().SetKeyboardFocus(SourceNameTextBox.ToSharedRef(), EFocusCause::SetDirectly);
	}
}

void SAddNewGameplayTagSourceWidget::Reset()
{
	SetSourceName();
}

void SAddNewGameplayTagSourceWidget::SetSourceName(const FText& InName)
{
	SourceNameTextBox->SetText(InName.IsEmpty() ? FText::FromString(DefaultNewName) : InName);
}

FReply SAddNewGameplayTagSourceWidget::OnAddNewSourceButtonPressed()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		
	if (!SourceNameTextBox->GetText().EqualTo(FText::FromString(DefaultNewName)))
	{
		Manager.FindOrAddTagSource(*SourceNameTextBox->GetText().ToString(), EGameplayTagSourceType::TagList);
	}

	IGameplayTagsModule::OnTagSettingsChanged.Broadcast();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
