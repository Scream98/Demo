#pragma once

#include "CoreMinimal.h"
#include "KDDBindingManager.generated.h"

/**
 * KDD 绑定配置管理器。
 *
 * 职责：
 *   1. 维护 ClassName → LuaPath 的静态 TMap（从 KDDLuaBindingConfig.lua 填充）
 *   2. 供 KDDModuleLocator 查询对象的 Lua 模块路径
 *   3. 暴露 RegisterBinding() 给 Lua 侧在启动时注册映射
 *
 * 不再负责手动 KDD_BindLua —— 绑定由 UnLua 的 IUnLuaInterface 自动完成。
 */
UCLASS()
class UKDDBindingManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * 注册一条绑定映射：ClassName → LuaPath。
     * 由 KDDLuaBindingConfig.lua 在启动时自动调用。
     * @param ClassName 蓝图类的名称（如 "MainView_C"）
     * @param LuaPath   Lua 模块路径（如 "KDD.UI.MainView"）
     */
    UFUNCTION(BlueprintCallable, Category="KDD|Binding")
    static void RegisterBinding(const FString& ClassName, const FString& LuaPath);

    /**
     * 查询 ClassName 对应的 Lua 模块路径（供 KDDModuleLocator 调用）。
     */
    static FString GetLuaPath(const FString& ClassName);

    /** 获取全部绑定配置（只读） */
    static const TMap<FString, FString>& GetBindingConfig();

private:
    static TMap<FString, FString> BindingConfig;
};
