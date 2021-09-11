// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAddNewRestrictedGameplayTagWidget.h"
#include "DetailLayoutBuilder.h"
#include "GameplayTagsSettings.h"
#include "GameplayTagsEditorModule.h"
#include "GameplayTagsModule.h"
#include "SGameplayTagWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "AddNewRestrictedGameplayTagWidget"

SAddNewRestrictedGameplayTagWidget::~SAddNewRestrictedGameplayTagWidget()
{
	if (!GExitPurge)
	{
		IGameplayTagsModule::OnTagSettingsChanged.RemoveAll(this);
	}
}

void SAddNewRestrictedGameplayTagWidget::Construct(const FArguments& InArgs)
{
	FText HintText = LOCTEXT("NewTagNameHint", "X.Y.Z");
	DefaultNewName = InArgs._NewRestrictedTagName;
	if (DefaultNewName.IsEmpty() == false)
	{
		HintText = FText::FromString(DefaultNewName);
	}


	bAddingNewRestrictedTag = false;
	bShouldGetKeyboardFocus = false;

	OnRestrictedGameplayTagAdded = InArgs._OnRestrictedGameplayTagAdded;
	PopulateTagSources();

	IGameplayTagsModule::OnTagSettingsChanged.AddRaw(this, &SAddNewRestrictedGameplayTagWidget::PopulateTagSources);

	ChildSlot
	[
		SNew(SVerticalBox)

		// Restricted Tag Name
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
				.OnTextCommitted(this, &SAddNewRestrictedGameplayTagWidget::OnCommitNewTagName)
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
				.OnTextCommitted(this, &SAddNewRestrictedGameplayTagWidget::OnCommitNewTagName)
			]
		]

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
				.Text(LOCTEXT("AllowNonRestrictedChildren", "Allow non-restricted children:"))
			]

			+ SHorizontalBox::Slot()
			.Padding(2.0f, 2.0f)
			.FillWidth(1.0f)
			.HAlign(HAlign_Right)
			[
				SAssignNew(AllowNonRestrictedChildrenCheckBox, SCheckBox)
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
				.OptionsSource(&RestrictedTagSources)
				.OnGenerateWidget(this, &SAddNewRestrictedGameplayTagWidget::OnGenerateTagSourcesComboBox)
				.ContentPadding(2.0f)
				.Content()
				[
					SNew(STextBlock)
					.Text(this, &SAddNewRestrictedGameplayTagWidget::CreateTagSourcesComboBoxContent)
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
				.OnClicked(this, &SAddNewRestrictedGameplayTagWidget::OnAddNewTagButtonPressed)
			]
		]
	];

	Reset();
}

void SAddNewRestrictedGameplayTagWidget::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	if (bShouldGetKeyboardFocus)
	{
		bShouldGetKeyboardFocus = false;
		FSlateApplication::Get().SetKeyboardFocus(TagNameTextBox.ToSharedRef(), EFocusCause::SetDirectly);
	}
}

void SAddNewRestrictedGameplayTagWidget::PopulateTagSources()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	RestrictedTagSources.Empty();

	TArray<const FGameplayTagSource*> Sources;
	Manager.GetRestrictedTagSources(Sources);

	// Used to make sure we have a non-empty list of restricted tag sources. Not an actual source.
	FName PlaceholderSource = NAME_None;

	// Add the placeholder source if no other sources exist
	if (Sources.Num() == 0)
	{
		RestrictedTagSources.Add(MakeShareable(new FName(PlaceholderSource)));
	}

	for (const FGameplayTagSource* Source : Sources)
	{
		if (Source != nullptr && Source->SourceName != PlaceholderSource)
		{
			RestrictedTagSources.Add(MakeShareable(new FName(Source->SourceName)));
		}
	}
}

void SAddNewRestrictedGameplayTagWidget::Reset(FName TagSource)
{
	SetTagName();
	SelectTagSource(TagSource);
	SetAllowNonRestrictedChildren();
	TagCommentTextBox->SetText(FText());
}

void SAddNewRestrictedGameplayTagWidget::SetTagName(const FText& InName)
{
	TagNameTextBox->SetText(InName.IsEmpty() ? FText::FromString(DefaultNewName) : InName);
}

void SAddNewRestrictedGameplayTagWidget::SelectTagSource(const FName& InSource)
{
	// Attempt to find the location in our sources, otherwise just use the first one
	int32 SourceIndex = 0;

	if (!InSource.IsNone())
	{
		for (int32 Index = 0; Index < RestrictedTagSources.Num(); ++Index)
		{
			TSharedPtr<FName> Source = RestrictedTagSources[Index];

			if (Source.IsValid() && *Source.Get() == InSource)
			{
				SourceIndex = Index;
				break;
			}
		}
	}

	TagSourcesComboBox->SetSelectedItem(RestrictedTagSources[SourceIndex]);
}

void SAddNewRestrictedGameplayTagWidget::SetAllowNonRestrictedChildren(bool bInAllowNonRestrictedChildren)
{
	AllowNonRestrictedChildrenCheckBox->SetIsChecked(bInAllowNonRestrictedChildren ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
}

void SAddNewRestrictedGameplayTagWidget::OnCommitNewTagName(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType == ETextCommit::OnEnter)
	{
		ValidateNewRestrictedTag();
	}
}

FReply SAddNewRestrictedGameplayTagWidget::OnAddNewTagButtonPressed()
{
	ValidateNewRestrictedTag();
	return FReply::Handled();
}

void SAddNewRestrictedGameplayTagWidget::AddSubtagFromParent(const FString& ParentTagName, const FName& ParentTagSource, bool bAllowNonRestrictedChildren)
{
	FText SubtagBaseName = !ParentTagName.IsEmpty() ? FText::Format(FText::FromString(TEXT("{0}.")), FText::FromString(ParentTagName)) : FText();

	SetTagName(SubtagBaseName);
	SelectTagSource(ParentTagSource);
	SetAllowNonRestrictedChildren(bAllowNonRestrictedChildren);

	bShouldGetKeyboardFocus = true;
}

void SAddNewRestrictedGameplayTagWidget::ValidateNewRestrictedTag()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	FString TagName = TagNameTextBox->GetText().ToString();
	FString TagComment = TagCommentTextBox->GetText().ToString();
	bool bAllowNonRestrictedChildren = AllowNonRestrictedChildrenCheckBox->IsChecked();
	FName TagSource = *TagSourcesComboBox->GetSelectedItem().Get();

	if (TagSource == NAME_None)
	{
		FNotificationInfo Info(LOCTEXT("NoRestrictedSource", "You must specify a source file for restricted gameplay tags."));
		Info.ExpireDuration = 10.f;
		Info.bUseSuccessFailIcons = true;
		Info.Image = FEditorStyle::GetBrush(TEXT("MessageLog.Error"));

		AddRestrictedGameplayTagDialog = FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	TArray<FString> TagSourceOwners;
	Manager.GetOwnersForTagSource(TagSource.ToString(), TagSourceOwners);

	bool bHasOwner = false;
	for (const FString& Owner : TagSourceOwners)
	{
		if (!Owner.IsEmpty())
		{
			bHasOwner = true;
			break;
		}
	}

	if (bHasOwner)
	{
		// check if we're one of the owners; if we are then we don't need to pop up the permission dialog
		bool bRequiresPermission = true;
		const FString& UserName = FPlatformProcess::UserName();
		for (const FString& Owner : TagSourceOwners)
		{
			if (Owner.Equals(UserName))
			{
				CreateNewRestrictedGameplayTag();
				bRequiresPermission = false;
			}
		}

		if (bRequiresPermission)
		{
			FString StringToDisplay = TEXT("Do you have permission from ");
			StringToDisplay.Append(TagSourceOwners[0]);
			for (int Idx = 1; Idx < TagSourceOwners.Num(); ++Idx)
			{
				StringToDisplay.Append(TEXT(" or "));
				StringToDisplay.Append(TagSourceOwners[Idx]);
			}
			StringToDisplay.Append(TEXT(" to modify "));
			StringToDisplay.Append(TagSource.ToString());
			StringToDisplay.Append(TEXT("?"));

			FNotificationInfo Info(FText::FromString(StringToDisplay));
			Info.ExpireDuration = 10.f;
			Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("RestrictedTagPopupButtonAccept", "Yes"), FText(), FSimpleDelegate::CreateSP(this, &SAddNewRestrictedGameplayTagWidget::CreateNewRestrictedGameplayTag), SNotificationItem::CS_None));
			Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("RestrictedTagPopupButtonReject", "No"), FText(), FSimpleDelegate::CreateSP(this, &SAddNewRestrictedGameplayTagWidget::CancelNewTag), SNotificationItem::CS_None));

			AddRestrictedGameplayTagDialog = FSlateNotificationManager::Get().AddNotification(Info);
		}
	}
	else
	{
		CreateNewRestrictedGameplayTag();
	}
}

void SAddNewRestrictedGameplayTagWidget::CreateNewRestrictedGameplayTag()
{
	if (AddRestrictedGameplayTagDialog.IsValid())
	{
		AddRestrictedGameplayTagDialog->SetVisibility(EVisibility::Collapsed);
	}

	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// Only support adding tags via ini file
	if (Manager.ShouldImportTagsFromINI() == false)
	{
		return;
	}

	FString TagName = TagNameTextBox->GetText().ToString();
	FString TagComment = TagCommentTextBox->GetText().ToString();
	bool bAllowNonRestrictedChildren = AllowNonRestrictedChildrenCheckBox->IsChecked();
	FName TagSource = *TagSourcesComboBox->GetSelectedItem().Get();

	if (TagName.IsEmpty())
	{
		return;
	}

	// set bIsAddingNewTag, this guards against the window closing when it loses focus due to source control checking out a file
	TGuardValue<bool>	Guard(bAddingNewRestrictedTag, true);

	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(TagName, TagComment, TagSource, true, bAllowNonRestrictedChildren);

	OnRestrictedGameplayTagAdded.ExecuteIfBound(TagName, TagComment, TagSource);

	Reset(TagSource);
}

void SAddNewRestrictedGameplayTagWidget::CancelNewTag()
{
	AddRestrictedGameplayTagDialog->SetVisibility(EVisibility::Collapsed);
}

TSharedRef<SWidget> SAddNewRestrictedGameplayTagWidget::OnGenerateTagSourcesComboBox(TSharedPtr<FName> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromName(*InItem.Get()));
}

FText SAddNewRestrictedGameplayTagWidget::CreateTagSourcesComboBoxContent() const
{
	const bool bHasSelectedItem = TagSourcesComboBox.IsValid() && TagSourcesComboBox->GetSelectedItem().IsValid();

	return bHasSelectedItem ? FText::FromName(*TagSourcesComboBox->GetSelectedItem().Get()) : LOCTEXT("NewTagLocationNotSelected", "Not selected");
}

#undef LOCTEXT_NAMESPACE
