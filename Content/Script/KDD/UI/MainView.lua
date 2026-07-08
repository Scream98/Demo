-- ============================================================
-- MainView.lua - 主视图业务逻辑
--
-- 注意：
--   Construct 收到的是 widget userdata，self 即 widget 本身，
--   可通过 self.Btn_Start 访问子控件。
--   按钮回调用闭包捕获 widget 和 PC，避免 delegate 的 self 丢失问题。
-- ============================================================

local MainView = {}

function MainView:Construct()
    -- self = widget userdata，可通过 UE 属性系统访问子控件
    print("[MainView] Construct")

    local Btn = self.Btn_Start
    if not Btn then
        print("[MainView] Btn_Start not found!")
        return
    end
    print("[MainView] Btn_Start found: " .. tostring(Btn:GetName()))

    -- 用闭包绑定按钮事件。
    -- 使用 UGameplayStatics::OpenLevel 切换地图（避免 ConsoleCommand 的 UFUNCTION 兼容问题）
    Btn.OnClicked:Add(Btn, function()
        print("[MainView] Button clicked!")
        if KDD_PC then
            UE.UGameplayStatics.OpenLevel(KDD_PC, "BattleMap")
            print("[MainView] BattleMap OpenLevel sent")
        else
            print("[MainView] KDD_PC not available!")
        end
    end)

    print("[MainView] Button click bound via closure")
end

function MainView:Destruct()
    print("[MainView] Destruct called")
end

return MainView
