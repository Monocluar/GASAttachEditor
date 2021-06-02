
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SUserWidget.h"


enum EDebugAbilitieCategories
{
	// 数据
	Attributes,

	// 效果
	GameplayEffects,

	// 技能
	Ability,
};


class SGASAttachEditor : public SUserWidget
{
public:

	SLATE_USER_ARGS(SGASAttachEditor) {}
	SLATE_END_ARGS()

public:
	virtual void Construct(const FArguments& InArgs) = 0;

};
