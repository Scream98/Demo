-- ============================================================
-- MainMenu.lua - 主菜单 Lua 模块
--
-- 配表映射:
--   MainMenu = {
--       ResPath = "/Game/UI/MainMenu/WBP_MainMenu",   -- 蓝图路径
--       LuaPath = "KDD.UI.MainMenu"                     -- Lua 模块路径
--   }
--
-- 绑定后，UE 生命周期事件自动路由到此模块的同名函数：
--   Construct  → Construct()
--   Destruct   → Destruct()
--   Tick       → Tick(DeltaTime)
--   self = 蓝图 Widget 实例
-- ============================================================

local MainMenu = {}

function MainMenu:Construct()
    print("[MainMenu] Construct called")
end

function MainMenu:Destruct()
    print("[MainMenu] Destruct called")
end

function MainMenu:Tick(DeltaTime)
end

-- 蓝图中按钮绑定到这些函数
function MainMenu:OnPlayBtnClicked()
    print("[MainMenu] Play button clicked")
end

function MainMenu:OnSettingsBtnClicked()
    print("[MainMenu] Settings button clicked")
end

function MainMenu:OnQuitBtnClicked()
    print("[MainMenu] Quit button clicked")
end

return MainMenu
