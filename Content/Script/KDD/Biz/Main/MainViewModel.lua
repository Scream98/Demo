-- ============================================================
-- MainViewModel.lua - 主视图的 Model
--
-- 管理主界面相关的数据状态。
-- ============================================================

local KDDModel = require("KDD.Core.KDDModel")

local M = KDDModel.New("Main")

-- 主界面状态
M.Data = {
    Title = "KDD Demo",
    BtnLabel = "开始战斗",
}

function M:OnInit()
    print("[MainViewModel] Init")
end

function M:SetTitle(NewTitle)
    self.Data.Title = NewTitle
    self:Dispatch("TitleChanged", NewTitle)
end

function M:SetBtnLabel(Label)
    self.Data.BtnLabel = Label
    self:Dispatch("BtnLabelChanged", Label)
end

function M:OnDestroy()
    print("[MainViewModel] Destroy")
end

return M
