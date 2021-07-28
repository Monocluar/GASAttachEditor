#pragma once

#include "CoreMinimal.h"
#include "Widgets/SUserWidget.h"

class FTabManager;

class SGASTagLookAsset : public SUserWidget
{
public:

	SLATE_USER_ARGS(SGASTagLookAsset) {}
	SLATE_END_ARGS()



public:
	virtual void Construct(const FArguments& InArgs) = 0;

	// 该Tab控件名字
	static FName GetTabName();
#if	WITH_EDITOR
	static void RegisterTabSpawner(FTabManager& TabManager);
#endif
};