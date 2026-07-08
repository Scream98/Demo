-- ============================================================
-- Bootstrap.lua - 游戏启动入口
--
-- 由 UKDDGameInstance 在 Init 后通过全局 UnLua Lua VM 调用。
-- C++ 侧已确保 KDDBindingManager 的配表已注册到 C++ TMap，
-- 并将 PlayerController 注入到全局变量 KDD_PC。
--
-- 职责：创建并显示 MainView。
-- ★ 无需手动绑定：Widget 创建时 UnLua 自动完成绑定。
-- ============================================================

local KDDBindingManager = require("KDD.KDDBindingManager")

local Bootstrap = {}

function Bootstrap:Init()
    local PC = KDD_PC
    if not PC then
        print("[Bootstrap] KDD_PC not yet injected, will be retried by C++")
        return
    end

    print("[Bootstrap] PlayerController ready, creating MainView...")

    local view = KDDBindingManager.CreateView("MainView", PC)
    if view then
        view:Show()
        print("[Bootstrap] MainView shown successfully!")
    else
        warn("[Bootstrap] Failed to create MainView!")
    end
end

return Bootstrap
