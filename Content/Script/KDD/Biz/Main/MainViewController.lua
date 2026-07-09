-- ============================================================
-- MainViewController.lua - 主视图的 Controller
--
-- 串联 MainViewModel 和 MainView（BP Widget 绑定的 MainView.lua）。
-- ============================================================

local KDDController = require("KDD.Core.KDDController")

local M = KDDController.New("Main")

function M:OnInit()
    print("[MainViewController] Init")
end

-- ======== 用户交互处理 ========

--- 玩家点击了"开始战斗"按钮
function M:OnStartBtnClicked()
    print("[MainViewController] Start button clicked, opening BattleMap...")
    if KDD_PC then
        UE.UGameplayStatics.OpenLevel(KDD_PC, "BattleMap")
    end
end

function M:OnDestroy()
    print("[MainViewController] Destroy")
end

return M
