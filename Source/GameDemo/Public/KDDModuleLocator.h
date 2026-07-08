#pragma once

#include "CoreMinimal.h"
#include "LuaModuleLocator.h"
#include "KDDModuleLocator.generated.h"

/**
 * KDD 自定义 Lua Module Locator。
 *
 * 从 KDDBindingManager 的静态 TMap 中查找 UObject 对应的 Lua 模块路径。
 * 映射数据由 KDDLuaBindingConfig.lua 在启动时通过 RegisterBinding() 推入 C++ 侧。
 *
 * 通过 DefaultEngine.ini 配置启用：
 *   [/Script/UnLua.UnLuaSettings]
 *   ModuleLocatorClass=/Script/GameDemo.KDDModuleLocator
 *
 * 配合 UKDDView / UKDDWidget 实现的 IUnLuaInterface，使 UnLua 在 UObject 创建时
 * 自动完成绑定，无需在 Lua 或蓝图中手动调用绑定函数。
 */
UCLASS()
class GAMEDEMO_API UKDDModuleLocator : public ULuaModuleLocator
{
    GENERATED_BODY()

public:
    virtual FString Locate(const UObject* Object) override;
};
