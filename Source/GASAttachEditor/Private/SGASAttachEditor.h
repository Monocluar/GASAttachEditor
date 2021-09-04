// <Copyright (C) Monocluar. 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SUserWidget.h"
#include "Framework/Application/IInputProcessor.h"

class SWidget;
class FTabManager;

enum EDebugAbilitieCategories
{
	// 标签
	Tags,

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

	// 设置状态改变的东东
	// Set the status change
	virtual void SetPickingMode(bool bTick) = 0;

	// 该Tab控件名字
	// The tab control name
	static FName GetTabName();

	static void RegisterTabSpawner(FTabManager& TabManager);

};

class FAttachInputProcessor : public IInputProcessor
{
public:
	FAttachInputProcessor(SGASAttachEditor* InWidgetPtr);
	~FAttachInputProcessor() { GASAttachEditorWidgetPtr = nullptr; };

private:

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	//virtual const TCHAR* GetDebugName() const override { return TEXT("AttachInputProcessor"); }
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

private:
	SGASAttachEditor* GASAttachEditorWidgetPtr;
};