-- ============================================================
-- KDDWidget.lua - Widget 的 Lua 侧包装基类
--
-- 用于可复用组件（按钮、列表项、面板等）的 Lua 逻辑封装。
-- 用法与 KDDView 一致，通过 self.Content 操作蓝图 Widget。
-- ============================================================

local KDDWidget = {}
KDDWidget.__index = KDDWidget

-- 构造函数
function KDDWidget.New(Widget, WidgetName)
    local self = setmetatable({}, KDDWidget)
    self.Content = Widget       -- 蓝图 Widget 实例
    self.WidgetName = WidgetName -- 控件名称
    return self
end

-- 设置可见性
function KDDWidget:SetVisibility(Visibility)
    if self.Content then
        self.Content:SetVisibility(Visibility)
    end
end

-- 设置自身为父控件的子控件
function KDDWidget:AddToParent(ParentWidget)
    if self.Content and ParentWidget then
        self.Content:SetParent(ParentWidget)
    end
end

-- ======== 可重写的生命周期回调 ========

function KDDWidget:OnInit() end
function KDDWidget:OnShow() end
function KDDWidget:OnHide() end
function KDDWidget:OnDestroy() end

return KDDWidget
