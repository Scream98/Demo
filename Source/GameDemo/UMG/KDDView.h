#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnLuaInterface.h"
#include "Animation/WidgetAnimation.h"
#include "KDDView.generated.h"

/**
 * KDD 视图基类。
 * 设计为全屏/面板级 UI 的基类，蓝图设计师直接选中此类创建蓝图即可。
 * 不需在蓝图中配置任何 Lua 路径。
 *
 * 生命周期（由子类覆盖）：
 *   NativeConstruct → OnCreate → OnOpen
 *   NativeDestruct  → OnClose → OnDestroy
 *
 * 实现了 IUnLuaInterface，配合 KDDModuleLocator，
 * 在 Widget 被创建时 UnLua 自动从配表中查找并绑定对应的 Lua 模块。
 */
UCLASS(Blueprintable, BlueprintType, Abstract, meta=(Category="KDD"))
class UKDDView : public UUserWidget, public IUnLuaInterface
{
    GENERATED_BODY()

public:
    UKDDView(const FObjectInitializer& ObjectInitializer);

    // IUnLuaInterface: 标记此类需要 UnLua 自动绑定，ModuleLocator 会从配表查找路径
    virtual FString GetModuleName_Implementation() const override;

    /** 获取显示动画（由子类覆盖返回动画实例） */
    UFUNCTION(BlueprintNativeEvent, Category="KDD|Animation")
    UWidgetAnimation* GetShowAnimation() const;
    virtual UWidgetAnimation* GetShowAnimation_Implementation() const;

    /** 获取隐藏动画 */
    UFUNCTION(BlueprintNativeEvent, Category="KDD|Animation")
    UWidgetAnimation* GetHideAnimation() const;
    virtual UWidgetAnimation* GetHideAnimation_Implementation() const;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** 子类可覆盖的详细生命周期 */
    virtual void OnCreate();
    virtual void OnOpen();
    virtual void OnClose();
    virtual void OnRefresh();
    virtual void OnDestroy();

    /** 获取指定名称的动画 */
    UWidgetAnimation* FindAnimationByName(const FString& AnimName) const;
};
