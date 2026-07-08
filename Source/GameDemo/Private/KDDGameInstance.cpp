#include "KDDGameInstance.h"
#include "UnLuaModule.h"
#include "UnLuaBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UKDDGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Init, scheduling Lua bootstrap..."));

    // 延迟一帧，确保 World、PlayerController 等基础设施已就绪
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UKDDGameInstance::OnLuaBootstrap);
}

void UKDDGameInstance::Shutdown()
{
    UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Shutdown"));
    Super::Shutdown();
}

void UKDDGameInstance::OnLuaBootstrap()
{
    UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Starting Lua bootstrap..."));
    TryBootstrap();
}

void UKDDGameInstance::TryBootstrap()
{
    if (!GetWorld())
    {
        return;
    }

    // 获取全局 UnLua Lua VM
    auto* Env = IUnLuaModule::Get().GetEnv();
    if (!Env)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KDDGameInstance] Lua env not ready, will retry..."));
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UKDDGameInstance::TryBootstrap);
        return;
    }

    lua_State* L = Env->GetMainState();

    // 从 C++ 侧获取 PlayerController 并 Push 到 Lua 全局变量
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        UnLua::PushUObject(L, PC);
        lua_setglobal(L, "KDD_PC");
        UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Pushed KDD_PC to Lua"));
    }

    // ★ 第一步：加载绑定配置
    //     模块级代码会自动遍历 KDDLuaBindingConfig 并调用
    //     UKDDBindingManager::RegisterBinding() 注册到 C++ TMap。
    Env->DoString(
        TEXT("require('KDD.KDDBindingManager')"),
        TEXT("BindingInit")
    );

    // ★ 第二步：执行 Bootstrap（创建并显示 MainView）
    //     Widget 创建时会触发 UnLua 自动绑定（无需手动 Bind）。
    Env->DoString(
        TEXT("require('KDD.Game.Bootstrap'):Init()"),
        TEXT("BootstrapInit")
    );

    // 如果 PlayerController 未就绪，延迟一帧重试
    if (!PC)
    {
        UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] PC not ready, scheduling retry..."));
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UKDDGameInstance::TryBootstrap);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Bootstrap completed."));
    }
}
