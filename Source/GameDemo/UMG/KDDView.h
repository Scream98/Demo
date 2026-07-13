#pragma once

#include "CoreMinimal.h"
#include "UMG/KDDWidget.h"
#include "KDDView.generated.h"

/**
 * KDD 视图类（继承自 UKDDWidget）。
 * 设计为全屏/面板级 UI 的基类，蓝图设计师直接选中此类创建蓝图即可。
 * 不需在蓝图中配置任何 Lua 路径。
 *
 * 继承 UKDDWidget 的所有能力（生命周期 + 自动动画系统 + IUnLuaInterface）。
 */
UCLASS(Blueprintable, BlueprintType, Abstract, meta=(Category="KDD"))
class UKDDView : public UKDDWidget
{
    GENERATED_BODY()

public:
    UKDDView(const FObjectInitializer& ObjectInitializer);
};
