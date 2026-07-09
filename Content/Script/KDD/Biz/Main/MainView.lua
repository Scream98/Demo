-- ============================================================
-- MainView.lua - 主视图（V 层）
--
-- 只管显示和用户交互，不碰业务逻辑。
-- Construct 时自动组建 MVC 三角绑定，Destruct 时自动清理。
-- ============================================================

local MainView = {}

function MainView:Construct()
    print("[MainView] Construct")

    -- 1. 获取 MVC 组件（与 Model/Controller 同目录）
    local Controller = require("KDD.Biz.Main.MainViewController")
    local Model = require("KDD.Biz.Main.MainViewModel")

    -- 2. 三角绑定
    Controller:BindModel(Model)
    Controller:BindView(self)

    -- 3. 初始化
    Controller:OnInit()
    Model:OnInit()

    -- 4. 绑定按钮事件
    local Btn = self.Btn_Start
    if not Btn then
        print("[MainView] Btn_Start not found!")
        return
    end
    print("[MainView] Btn_Start found: " .. tostring(Btn:GetName()))

    Btn.OnClicked:Add(Btn, function()
        Controller:OnStartBtnClicked()
    end)

    print("[MainView] MVC initialized")
end

function MainView:Destruct()
    print("[MainView] Destruct called")

    local Controller = require("KDD.Biz.Main.MainViewController")
    local Model = require("KDD.Biz.Main.MainViewModel")
    Controller:OnDestroy()
    Model:OnDestroy()
end

return MainView
