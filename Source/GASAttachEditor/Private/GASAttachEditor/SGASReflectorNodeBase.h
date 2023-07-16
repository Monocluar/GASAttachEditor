#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTask.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"


class STableViewBase;

enum EGAAbilitieNode
{
	// 技能
	Node_Abilitie,
	
	// 任务
	Node_Task,

	// 调试信息
	Node_Message,
};


enum EScreenGAModeState
{
	Active = 1,

	Blocked = 1 << 1,

	NoActive = 1 << 2
};


static FName NAME_AbilitietName(TEXT("AbilitiesName"));
static FName NAME_GAStateType(TEXT("GAStateType"));
static FName NAME_GAIsActive(TEXT("GAIsActive"));
static FName NAME_GAAbilityTriggers(TEXT("AbilityTriggers"));

DECLARE_DELEGATE_OneParam(FOnTreeItemVis, bool)

class FGASAbilitieNodeBase 
{
public:

	virtual ~FGASAbilitieNodeBase(){};

	// 当前GA名字
	virtual FName GetGAName() const = 0;

	// GA状态
	virtual FText GetGAStateType() = 0;

	// 是否激活
	virtual bool GetGAIsActive() const = 0;

	// 该类型是什么类型东东
	// Determine what the type is
	virtual EGAAbilitieNode GetNodeType() const;

	// 当前技能含有的Tag
	// Tag contained in current skill
	virtual FText GetAbilitieHasTag() const { return FText(); }

	// 当前GA含有的Triggers
	// Triggers contained in current GA
	virtual FString GetAbilityTriggersName() const = 0;

protected:

	FGASAbilitieNodeBase();

public:
	// GA 控件资源位置
	// GA location Path
	virtual FString GetWidgetFile() const = 0;

	// GA C++ Line 位置
	virtual int32 GetWidgetLineNumber() const = 0;

	// 是否是蓝图资源
	// Is it a blueprint resource
	virtual bool HasValidWidgetAssetData() const = 0;

	// 蓝图资源位置
	// Blueprint resource location
	virtual FString GetWidgetAssetData() const = 0;

public:

	// 获取状态颜色
	// Get status color
	const FLinearColor& GetTint() const;

	// 设置状态颜色
	// Set status color
	void SetTint(const FLinearColor& InTint);
	
	// 将给定节点添加到此小部件的子级列表中（此节点将保留对实例的强引用）
	// Adds the given node to the list of children of this widget (this node will retain a strong reference to the instance)
	void AddChildNode(TSharedRef<FGASAbilitieNodeBase> InChildNode);

	// 返回子条目的数组
	// Returns an array of subentries
	const TArray<TSharedRef<FGASAbilitieNodeBase>>& GetChildNodes() const;

	// 设置回调句柄
	// Set callback handle
	void SetTreeItemVis(FOnTreeItemVis Handle);

	// 设置自身显隠性
	// Set self visibility
	void SetItemVisility(bool bShow);

	// 获取显隠性
	// Get visility
	FORCEINLINE bool IsShow() const;


protected:

	TArray<TSharedRef<FGASAbilitieNodeBase>> ChildNodes;

	FLinearColor Tint;

	FOnTreeItemVis OnShowHandle;

	bool bIsShow;

	EGAAbilitieNode GAAbilitieNode;

public:
	EScreenGAModeState ScreenGAMode;

};


class SGASAbilitieTreeItem : public SMultiColumnTableRow<TSharedRef<FGASAbilitieNodeBase>>
{
public:

	SLATE_BEGIN_ARGS(SGASAbilitieTreeItem)
		: _WidgetInfoToVisualize()
	{}
		SLATE_ARGUMENT(TSharedPtr<FGASAbilitieNodeBase>, WidgetInfoToVisualize)
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

public:

	virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& ColumnName ) override;

protected:

	FText GetReadableLocationAsText() const;

	void HandleHyperlinkNavigate();

protected:

	void HanldeTreeItemVis(bool IsShow);


protected:

	FLinearColor GetTint() const
	{
		if (!WidgetInfo.IsValid())
		{
			return FLinearColor(1.f,1.f,1.f,.5f);
		}
		return WidgetInfo->GetTint();
	}

	FSlateColor GetFontColor() const
	{
		if (!WidgetInfo.IsValid())
		{
			return FSlateColor(FLinearColor(1, 1, 1, 1));
		}
		return FSlateColor(WidgetInfo->GetTint());
	}

	FText GetGAStateTypeAsString() const
	{
		return GAStateType;
	}

	FText GetGAIsActiveAsString() const
	{
		return bGAIsActive ? NSLOCTEXT("WidgetReflectorNode ","WidgetClippingYes", "Yes") : NSLOCTEXT("WidgetReflectorNode ", "WidgetClippingNo", "No");
	}

private:
	TSharedPtr<FGASAbilitieNodeBase> WidgetInfo;

	FName GAName;

	FText GAStateType;

	bool bGAIsActive;

	EGAAbilitieNode GAAbilitieNode;

	FString AbilityTriggersName;

	FString CachedWidgetFile;
	int32 CachedWidgetLineNumber;
	FString CachedAssetDataStr;

};


class FGASAbilitieNode : public FGASAbilitieNodeBase
{
public:
	virtual ~FGASAbilitieNode(){};

	static TSharedRef<FGASAbilitieNode> Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayAbilitySpec InAbilitySpecPtr);

	static TSharedRef<FGASAbilitieNode> Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent,  FGameplayAbilitySpec InAbilitySpecPtr, TWeakObjectPtr<UGameplayTask> InGameplayTask);



public:
	virtual FName GetGAName() const override;


	virtual FText GetGAStateType() override;


	virtual bool GetGAIsActive() const override;


	virtual FText GetAbilitieHasTag() const override;


	virtual FString GetWidgetFile() const override;


	virtual int32 GetWidgetLineNumber() const override;


	virtual bool HasValidWidgetAssetData() const override;


	virtual FString GetWidgetAssetData() const override;

	virtual FString GetAbilityTriggersName() const override;

private:
	/**
	 * Construct this node from the given widget geometry, caching out any data that may be required for future visualization in the widget reflector
	 */
	explicit FGASAbilitieNode(TWeakObjectPtr<UAbilitySystemComponent> InASComponent,  FGameplayAbilitySpec InAbilitySpecPtr);


	explicit FGASAbilitieNode(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, FGameplayAbilitySpec InAbilitySpecPtr, TWeakObjectPtr<UGameplayTask> InGameplayTask);

protected:

	void CreateChild();

private:

	FGameplayAbilitySpec AbilitySpecPtr;

	TWeakObjectPtr<UAbilitySystemComponent> ASComponent;

	TWeakObjectPtr<UGameplayTask> GameplayTask;
};