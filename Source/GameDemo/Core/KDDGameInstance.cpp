#include "KDDGameInstance.h"
#include "UnLuaModule.h"
#include "UnLuaBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UKDDGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Init, scheduling Lua startup..."));

    // 延迟一帧，确保 World、PlayerController 等基础设施已就绪
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UKDDGameInstance::OnLuaStart);
}

void UKDDGameInstance::Shutdown()
{
    // 通知 Lua 侧执行关闭（传入 nullptr，不指定特定 UObject）
    auto* Env = IUnLuaModule::Get().GetEnv(nullptr);
    if (Env)
    {
        Env->DoString(
            TEXT("require('KDD.Game.GameApp'); GameShutdown()"),
            TEXT("GameShutdown")
        );
    }

    UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Shutdown"));
    Super::Shutdown();
}

void UKDDGameInstance::OnLuaStart()
{
    UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Starting Lua..."));
    TryStart();
}

void UKDDGameInstance::TryStart()
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
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UKDDGameInstance::TryStart);
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

    // ★ 唯一的 Lua 入口：GameApp.lua 的 GameStart()
    //     GameApp.lua 内部会 require KDDBindingManager（含配表注册），
    //     然后创建并显示 MainView。Lua 侧依赖链完全由 Lua 自己管理。
    Env->DoString(
        TEXT("require('KDD.Game.GameApp'); GameStart()"),
        TEXT("GameStart")
    );

    // 如果 PlayerController 未就绪，延迟一帧重试
    if (!PC)
    {
        UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] PC not ready, scheduling retry..."));
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UKDDGameInstance::TryStart);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[KDDGameInstance] Start completed."));
    }
}
