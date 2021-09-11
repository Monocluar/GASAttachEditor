// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAddNewGameplayTagWidget.h"
#include "DetailLayoutBuilder.h"
#include "GameplayTagsSettings.h"
#include "GameplayTagsEditorModule.h"
#include "GameplayTagsModule.h"
#include "Widgets/Input/SButton.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "AddNewGameplayTagWidget"

SAddNewGameplayTagWidget::~SAddNewGameplayTagWidget()
{
	if (!GExitPurge)
	{
		IGameplayTagsModule::OnTagSettingsChanged.RemoveAll(this);
	}
}

void SAddNewGameplayTagWidget::Construct(const FArguments& InArgs)
{
	
	FText HintText = LOCTEXT("NewTagNameHint", "X.Y.Z");
	DefaultNewName = InArgs._NewTagName;
	if (DefaultNewName.IsEmpty() == false)
	{
		HintText = FText::FromString(DefaultNewName);
	}


	bAddingNewTag = false;
	bShouldGetKeyboardFocus = false;

	OnGameplayTagAdded = InArgs._OnGameplayTagAdded;
	IsValidTag = InArgs._IsValidTag;
	PopulateTagSources();

	IGameplayTagsModule::OnTagSettingsChanged.AddRaw(this, &SAddNewGameplayTagWidget::PopulateTagSources);

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
				.Text(LOCTEXT("NewTagName", "Name:"))
			]

			+ SHorizontalBox::Slot()
			.Padding(2.0f, 2.0f)
			.FillWidth(1.0f)
			.HAlign(HAlign_Right)
			[
				SAssignNew(TagNameTextBox, SEditableTextBox)
				.MinDesiredWidth(240.0f)
				.HintText(HintText)
				.OnTextCommitted(this, &SAddNewGameplayTagWidget::OnCommitNewTagName)
			]
		]

		// Tag Comment
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
				.Text(LOCTEXT("TagComment", "Comment:"))
			]

			+ SHorizontalBox::Slot()
			.Padding(2.0f, 2.0f)
			.FillWidth(1.0f)
			.HAlign(HAlign_Right)
			[
				SAssignNew(TagCommentTextBox, SEditableTextBox)
				.MinDesiredWidth(240.0f)
				.HintText(LOCTEXT("TagCommentHint", "Comment"))
				.OnTextCommitted(this, &SAddNewGameplayTagWidget::OnCommitNewTagName)
			]
		]

		// Tag Location
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2.0f, 6.0f)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CreateTagSource", "Source:"))
			]

			+ SHorizontalBox::Slot()
			.Padding(2.0f, 2.0f)
			.FillWidth(1.0f)
			.HAlign(HAlign_Right)
			[
				SAssignNew(TagSourcesComboBox, SComboBox<TSharedPtr<FName> >)
				.OptionsSource(&TagSources)
				.OnGenerateWidget(this, &SAddNewGameplayTagWidget::OnGenerateTagSourcesComboBox)
				.ContentPadding(2.0f)
				.Content()
				[
					SNew(STextBlock)
					.Text(this, &SAddNewGameplayTagWidget::CreateTagSourcesComboBoxContent)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		]

		// Add Tag Button
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
				.Text(LOCTEXT("AddNew", "Add New Tag"))
				.OnClicked(this, &SAddNewGameplayTagWidget::OnAddNewTagButtonPressed)
			]
		]
	];

	Reset(EResetType::ResetAll);
}

void SAddNewGameplayTagWidget::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	if (bShouldGetKeyboardFocus)
	{
		bShouldGetKeyboardFocus = false;
		FSlateApplication::Get().SetKeyboardFocus(TagNameTextBox.ToSharedRef(), EFocusCause::SetDirectly);
	}
}

void SAddNewGameplayTagWidget::PopulateTagSources()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	TagSources.Empty();

	FName DefaultSource = FGameplayTagSource::GetDefaultName();

	// Always ensure that the default source is first
	TagSources.Add( MakeShareable( new FName( DefaultSource ) ) );

	TArray<const FGameplayTagSource*> Sources;
	Manager.FindTagSourcesWithType(EGameplayTagSourceType::TagList, Sources);

	for (const FGameplayTagSource* Source : Sources)
	{
		if (Source != nullptr && Source->SourceName != DefaultSource)
		{
			TagSources.Add(MakeShareable(new FName(Source->SourceName)));
		}
	}

	//Set selection to the latest added source
	if (TagSourcesComboBox != nullptr)
	{
		TagSourcesComboBox->SetSelectedItem(TagSources.Last());
	}	
}

void SAddNewGameplayTagWidget::Reset(EResetType ResetType)
{
	SetTagName();
	if (ResetType != EResetType::DoNotResetSource)
	{
		SelectTagSource();
	}
	TagCommentTextBox->SetText(FText());
}

void SAddNewGameplayTagWidget::SetTagName(const FText& InName)
{
	TagNameTextBox->SetText(InName.IsEmpty() ? FText::FromString(DefaultNewName) : InName);
}

void SAddNewGameplayTagWidget::SelectTagSource(const FName& InSource)
{
	// Attempt to find the location in our sources, otherwise just use the first one
	int32 SourceIndex = 0;

	if (!InSource.IsNone())
	{
		for (int32 Index = 0; Index < TagSources.Num(); ++Index)
		{
			TSharedPtr<FName> Source = TagSources[Index];

			if (Source.IsValid() && *Source.Get() == InSource)
			{
				SourceIndex = Index;
				break;
			}
		}
	}

	TagSourcesComboBox->SetSelectedItem(TagSources[SourceIndex]);
}

void SAddNewGameplayTagWidget::OnCommitNewTagName(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType == ETextCommit::OnEnter)
	{
		CreateNewGameplayTag();
	}
}

FReply SAddNewGameplayTagWidget::OnAddNewTagButtonPressed()
{
	CreateNewGameplayTag();
	return FReply::Handled();
}

void SAddNewGameplayTagWidget::AddSubtagFromParent(const FString& ParentTagName, const FName& ParentTagSource)
{
	FText SubtagBaseName = !ParentTagName.IsEmpty() ? FText::Format(FText::FromString(TEXT("{0}.")), FText::FromString(ParentTagName)) : FText();

	SetTagName(SubtagBaseName);
	SelectTagSource(ParentTagSource);

	bShouldGetKeyboardFocus = true;
}

void SAddNewGameplayTagWidget::CreateNewGameplayTag()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// Only support adding tags via ini file
	if (Manager.ShouldImportTagsFromINI() == false)
	{
		return;
	}

	if (TagSourcesComboBox->GetSelectedItem().Get() == nullptr)
	{
		return;
	}

	FText TagNameAsText = TagNameTextBox->GetText();
	FString TagName = TagNameAsText.ToString();
	FString TagComment = TagCommentTextBox->GetText().ToString();
	FName TagSource = *TagSourcesComboBox->GetSelectedItem().Get();

	if (TagName.IsEmpty())
	{
		return;
	}

	// check to see if this is a valid tag
	// first check the base rules for all tags then look for any additional rules in the delegate
	FText ErrorMsg;
	if (!UGameplayTagsManager::Get().IsValidGameplayTagString(TagName, &ErrorMsg) || 
		(IsValidTag.IsBound() && !IsValidTag.Execute(TagName, &ErrorMsg))
		)
	{
		FText MessageTitle(LOCTEXT("InvalidTag", "Invalid Tag"));
		FMessageDialog::Open(EAppMsgType::Ok, ErrorMsg, &MessageTitle);
		return;
	}

	// set bIsAddingNewTag, this guards against the window closing when it loses focus due to source control checking out a file
	TGuardValue<bool>	Guard(bAddingNewTag, true);

	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(TagName, TagComment, TagSource);

	OnGameplayTagAdded.ExecuteIfBound(TagName, TagComment, TagSource);

	Reset(EResetType::DoNotResetSource);
}

TSharedRef<SWidget> SAddNewGameplayTagWidget::OnGenerateTagSourcesComboBox(TSharedPtr<FName> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromName(*InItem.Get()));
}

FText SAddNewGameplayTagWidget::CreateTagSourcesComboBoxContent() const
{
	const bool bHasSelectedItem = TagSourcesComboBox.IsValid() && TagSourcesComboBox->GetSelectedItem().IsValid();

	return bHasSelectedItem ? FText::FromName(*TagSourcesComboBox->GetSelectedItem().Get()) : LOCTEXT("NewTagLocationNotSelected", "Not selected");
}

#undef LOCTEXT_NAMESPACE
