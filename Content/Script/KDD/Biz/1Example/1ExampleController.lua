-- ============================================================
-- 1ExampleController.lua - 示例功能的 Controller（MVC 示范）
--
-- 串联 Model 和 View，处理用户交互，监听数据变化。
-- ============================================================

local KDDController = require("KDD.Core.KDDController")

local M = KDDController.New("1Example")

function M:OnInit()
    print("[1ExampleController] Init")

    -- 监听 Model 数据变化
    self:ListenModelEvent("TitleChanged", function(NewTitle)
        -- 通知 View 刷新标题显示
        if self.View and self.View.SetTitle then
            self.View:SetTitle(NewTitle)
        end
    end)

    self:ListenModelEvent("ItemsChanged", function(Items)
        if self.View and self.View.RefreshItemList then
            self.View:RefreshItemList(Items)
        end
    end)
end

-- ======== 用户交互处理 ========

--- 用户点了添加按钮
function M:OnAddBtnClicked(ItemName)
    if self.Model then
        self.Model:AddItem(ItemName)
    end
end

--- 用户点了删除按钮
function M:OnRemoveBtnClicked(Index)
    if self.Model then
        self.Model:RemoveItem(Index)
    end
end

--- 用户修改了标题
function M:OnTitleChanged(NewTitle)
    if self.Model then
        self.Model:SetTitle(NewTitle)
    end
end

function M:OnDestroy()
    print("[1ExampleController] Destroy")
end

return M
