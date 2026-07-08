#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KDDGameInstance.generated.h"

/**
 * KDD 游戏实例入口。
 * 在 Init() 后通过全局 UnLua Lua VM 调用 Bootstrap.lua，
 * 由 Lua 侧接管后续的 View 创建和游戏流程。
 * 全程不依赖任何蓝图节点，纯 C++ → Lua 驱动。
 */
UCLASS()
class GAMEDEMO_API UKDDGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

private:
    /** 延迟一帧后首次尝试执行 Lua Bootstrap */
    void OnLuaBootstrap();

    /** 在 Lua VM 中执行 Bootstrap:Init()，若 PC 未就绪则重试 */
    void TryBootstrap();
};
