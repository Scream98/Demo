-- ============================================================
-- 1ExampleModel.lua - 示例功能的 Model（MVC 示范）
--
-- 纯数据层，通过 Dispatch 通知 Controller 数据变化。
-- ============================================================

local KDDModel = require("KDD.Core.KDDModel")

local M = KDDModel.New("1Example")

-- 示例数据
M.Data = {
    Title = "KDD MVC 示例",
    Items = {}
}

function M:OnInit()
    print("[1ExampleModel] Init")
end

function M:SetTitle(NewTitle)
    self.Data.Title = NewTitle
    self:Dispatch("TitleChanged", NewTitle)
end

function M:AddItem(ItemName)
    table.insert(self.Data.Items, #self.Data.Items + 1, ItemName)
    self:Dispatch("ItemsChanged", self.Data.Items)
end

function M:RemoveItem(Index)
    if self.Data.Items[Index] then
        table.remove(self.Data.Items, Index)
        self:Dispatch("ItemsChanged", self.Data.Items)
    end
end

function M:OnDestroy()
    print("[1ExampleModel] Destroy")
end

return M
