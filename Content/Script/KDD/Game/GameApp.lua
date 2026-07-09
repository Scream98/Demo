-- ============================================================
-- GameApp.lua - 游戏应用入口
--
-- 由 UKDDGameInstance 在 Init 后直接调用。
-- C++ 侧已确保配表注册完成、KDD_PC 已注入。
--
-- 职责：创建并显示第一个界面（MainView）。
-- ============================================================

local KDDBindingManager = require("KDD.KDDBindingManager")

--- 应用初始化（C++ 在 Lua VM 就绪后调用此函数）
function GameStart()
    local PC = KDD_PC
    if not PC then
        print("[GameApp] KDD_PC not yet injected, will be retried by C++")
        return
    end

    print("[GameApp] PlayerController ready, creating MainView...")

    local view = KDDBindingManager.CreateView("MainView", PC)
    if view then
        view:Show()
        print("[GameApp] MainView shown successfully!")
    else
        warn("[GameApp] Failed to create MainView!")
    end
end

--- 应用关闭（C++ Shutdown 时调用）
function GameShutdown()
    KDDBindingManager.DestroyAllViews()
    print("[GameApp] All views destroyed, shutting down")
end
