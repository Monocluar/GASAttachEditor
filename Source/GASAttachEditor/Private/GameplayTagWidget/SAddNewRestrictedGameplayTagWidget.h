// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagsManager.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Notifications/SNotificationList.h"

/** Widget allowing the user to create new restricted gameplay tags */
class SAddNewRestrictedGameplayTagWidget : public SCompoundWidget
{
public:

	DECLARE_DELEGATE_ThreeParams(FOnRestrictedGameplayTagAdded, const FString& /*TagName*/, const FString& /*TagComment*/, const FName& /*TagSource*/);

	SLATE_BEGIN_ARGS(SAddNewRestrictedGameplayTagWidget)
		: _NewRestrictedTagName(TEXT(""))
		{}
		SLATE_EVENT( FOnRestrictedGameplayTagAdded, OnRestrictedGameplayTagAdded )	// Callback for when a new tag is added	
		SLATE_ARGUMENT( FString, NewRestrictedTagName ) // String that will initially populate the New Tag Name field
	SLATE_END_ARGS();

	virtual ~SAddNewRestrictedGameplayTagWidget();

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	void Construct( const FArguments& InArgs);

	/** Returns true if we're currently attempting to add a new restricted gameplay tag to an INI file */
	bool IsAddingNewRestrictedTag() const
	{
		return bAddingNewRestrictedTag;
	}

	/** Begins the process of adding a subtag to a parent tag */
	void AddSubtagFromParent(const FString& ParentTagName, const FName& ParentTagSource, bool bAllowNonRestrictedChildren);

	/** Resets all input fields */
	void Reset(FName TagSource = NAME_None);

private:

	/** Sets the name of the tag. Uses the default if the name is not specified */
	void SetTagName(const FText& InName = FText());

	/** Selects tag file location. Uses the default if the location is not specified */
	void SelectTagSource(const FName& InSource = FName());

	void SetAllowNonRestrictedChildren(bool bInAllowNonRestrictedChildren = false);

	/** Creates a list of all INIs that gameplay tags can be added to */
	void PopulateTagSources();

	/** Callback for when Enter is pressed when modifying a tag's name or comment */
	void OnCommitNewTagName(const FText& InText, ETextCommit::Type InCommitType);

	/** Callback for when the Add New Tag button is pressed */
	FReply OnAddNewTagButtonPressed();

	void ValidateNewRestrictedTag();

	/** Creates a new restricted gameplay tag and adds it to the INI files based on the widget's stored parameters */
	void CreateNewRestrictedGameplayTag();

	void CancelNewTag();

	/** Populates the widget's combo box with all potential places where a gameplay tag can be stored */
	TSharedRef<SWidget> OnGenerateTagSourcesComboBox(TSharedPtr<FName> InItem);

	/** Creates the text displayed by the combo box when an option is selected */
	FText CreateTagSourcesComboBoxContent() const;

private:

	/** All potential INI files where a gameplay tag can be stored */
	TArray<TSharedPtr<FName> > RestrictedTagSources;

	/** The name of the next gameplay tag to create */
	TSharedPtr<SEditableTextBox> TagNameTextBox;

	/** The comment to asign to the next gameplay tag to create*/
	TSharedPtr<SEditableTextBox> TagCommentTextBox;

	TSharedPtr<SCheckBox> AllowNonRestrictedChildrenCheckBox;

	/** The INI file where the next restricted gameplay tag will be created */
	TSharedPtr<SComboBox<TSharedPtr<FName> > > TagSourcesComboBox;

	/** Callback for when a new restricted gameplay tag has been added to the INI files */
	FOnRestrictedGameplayTagAdded OnRestrictedGameplayTagAdded;

	bool bAddingNewRestrictedTag;

	/** Tracks if this widget should get keyboard focus */
	bool bShouldGetKeyboardFocus;

	FString DefaultNewName;

	TSharedPtr<SNotificationItem> AddRestrictedGameplayTagDialog;
};
