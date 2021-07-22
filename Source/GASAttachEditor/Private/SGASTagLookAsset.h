#pragma once

#include "CoreMinimal.h"
#include "Widgets/SUserWidget.h"

class SGASTagLookAsset : public SUserWidget
{
public:

	SLATE_USER_ARGS(SGASTagLookAsset) {}
	SLATE_END_ARGS()



public:
	virtual void Construct(const FArguments& InArgs) = 0;

	// 该Tab控件名字
	static FName GetTabName();

	static void RegisterTabSpawner(FTabManager& TabManager);
};