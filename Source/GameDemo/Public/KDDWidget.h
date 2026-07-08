#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnLuaInterface.h"
#include "KDDWidget.generated.h"

/**
 * KDD 控件基类。
 * 设计为可复用组件的基类（按钮、列表项、面板等），蓝图设计师直接选中此类创建蓝图即可。
 * 不需在蓝图中配置任何 Lua 路径。
 *
 * 实现了 IUnLuaInterface，配合 KDDModuleLocator，
 * 在 Widget 被创建时 UnLua 自动从配表中查找并绑定对应的 Lua 模块。
 */
UCLASS(Blueprintable, BlueprintType, Abstract, meta=(Category="KDD"))
class UKDDWidget : public UUserWidget, public IUnLuaInterface
{
    GENERATED_BODY()

public:
    UKDDWidget(const FObjectInitializer& ObjectInitializer);

    // IUnLuaInterface
    virtual FString GetModuleName_Implementation() const override;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
