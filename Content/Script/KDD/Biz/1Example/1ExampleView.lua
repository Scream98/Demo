-- ============================================================
-- 1ExampleView.lua - 示例功能的 View（MVC 示范）
--
-- 只管显示数据和用户交互，不碰业务逻辑。
-- 创建后由 Controller 绑定，用户操作回调 Controller。
-- ============================================================

local ExampleView = {}

-- UnLua 自动路由的生命周期
function ExampleView:Construct()
    print("[1ExampleView] Construct")

    -- 1. 获取 Controller 实例
    local Controller = require("KDD.Biz.1Example.1ExampleController")
    local Model = require("KDD.Biz.1Example.1ExampleModel")

    -- 2. 三角绑定
    Controller:BindModel(Model)
    Controller:BindView(self)

    -- 3. 初始化
    Controller:OnInit()
    Model:OnInit()

    -- 4. 绑定按钮事件
    local BtnAdd = self.Btn_Add
    if BtnAdd then
        BtnAdd.OnClicked:Add(BtnAdd, function()
            Controller:OnAddBtnClicked(self.EditableTextBox:GetText())
        end)
    end

    local BtnRemove = self.Btn_Remove
    if BtnRemove then
        BtnRemove.OnClicked:Add(BtnRemove, function()
            Controller:OnRemoveBtnClicked(1)
        end)
    end
end

function ExampleView:SetTitle(NewTitle)
    local TxtTitle = self.Txt_Title
    if TxtTitle then
        TxtTitle:SetText(NewTitle)
    end
end

function ExampleView:RefreshItemList(Items)
    local List = self.Wrap_ItemList
    if not List then return end
    List:ClearChildren()

    for _, item in ipairs(Items) do
        print("[1ExampleView] Item: " .. tostring(item))
    end
end

function ExampleView:Destruct()
    print("[1ExampleView] Destruct")
    local Controller = require("KDD.Biz.1Example.1ExampleController")
    local Model = require("KDD.Biz.1Example.1ExampleModel")
    Controller:OnDestroy()
    Model:OnDestroy()
end

return ExampleView
