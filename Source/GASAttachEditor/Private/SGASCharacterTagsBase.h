#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STileView.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"

class UAbilitySystemComponent;

class FGASCharacterTagsBase
{
public:

	virtual ~FGASCharacterTagsBase(){}

	// 获取自身Tag名字
	virtual FText GetTagName() const = 0;

	// 获取Tag注释名称
	virtual FText GetTagTipName() const = 0;

protected:

	FGASCharacterTagsBase(){};
};

class SCharacterTagsView : public STileView<TSharedPtr<FGASCharacterTagsBase>>
{
public:
	virtual ~SCharacterTagsView(){};

public:
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

};

class SCharacterTagsViewItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SCharacterTagsViewItem )
		{}

		SLATE_ARGUMENT(TSharedPtr<FGASCharacterTagsBase>, TagsItem)

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs );
public:

	virtual ~SCharacterTagsViewItem() {};

protected:

	FReply HandleOnClicked();

protected:
	/** The data for this item */
	TSharedPtr<FGASCharacterTagsBase> TagsItem;

	TSharedPtr<STextBlock> ShowTextTag;
};

class FGASCharacterTags : public FGASCharacterTagsBase
{
public:

	virtual ~FGASCharacterTags(){};


	virtual FText GetTagName() const override;


	virtual FText GetTagTipName() const override;

	static TSharedRef<FGASCharacterTags> Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayTag InGameplayTag, FName InWidegtName);

private:
	explicit FGASCharacterTags(TWeakObjectPtr<UAbilitySystemComponent> InASComponent,  FGameplayTag InGameplayTag, FName InWidegtName);

protected:

	TWeakObjectPtr<UAbilitySystemComponent> ASComponent;

	FGameplayTag GameplayTag;

	FName WidegtName;

};