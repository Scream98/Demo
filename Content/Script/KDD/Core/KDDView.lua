-- ============================================================
-- KDDView.lua - View 的 Lua 侧包装基类
--
-- 注意：
--   - 此包装仅用于缓存管理（防止 GC、统一 Show/Hide 入口）
--   - 生命周期事件（Construct / Destruct / Tick）由 UnLua 自动路由
--     到 Lua 模块的同名函数，与 KDDView 包装无关。
-- ============================================================

local KDDView = {}
KDDView.__index = KDDView

function KDDView.New(Widget, ViewName, PC)
    local self = setmetatable({}, KDDView)
    self.Content = Widget
    self.ViewName = ViewName
    self.PC = PC
    return self
end

function KDDView:Show()
    if not self.Content then
        return
    end
    self.Content:AddToViewport()
    -- UnLua 自动路由 NativeConstruct → Lua Construct(self)
end

function KDDView:Hide()
    if not self.Content then
        return
    end
    self.Content:RemoveFromParent()
    -- UnLua 自动路由 NativeDestruct → Lua Destruct(self)
end

function KDDView:SetVisibility(Visibility)
    if self.Content then
        self.Content:SetVisibility(Visibility)
    end
end

function KDDView:GetPC()
    return self.PC
end

-- ======== 生命周期回调 ========

function KDDView:OnDestroy() end

return KDDView
