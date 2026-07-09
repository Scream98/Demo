-- ============================================================
-- KDDView.lua - View 的 Lua 侧包装基类
--
-- 生命周期：
--   New → Show → Hide → Destroy
--   内部自动调用：
--     OnCreate() → OnOpen() → OnRefresh() → OnClose() → OnDestroy()
--
-- 注：UnLua 自动路由 NativeConstruct → Lua Construct(self)
--     NativeDestruct → Lua Destruct(self)
--     包装层的生命周期与之协作。
-- ============================================================

local KDDView = {}
KDDView.__index = KDDView

function KDDView.New(Widget, ViewName, PC)
    local self = setmetatable({}, KDDView)
    self.Content = Widget
    self.ViewName = ViewName
    self.PC = PC
    self._isOpen = false
    self._isCreated = false
    return self
end

-- ======== 生命周期 ========

--- 创建（Widget 已构造但未显示）
function KDDView:OnCreate()
    self._isCreated = true
end

--- 打开/显示（触发 OnOpen，播放 Show 动画）
function KDDView:Show()
    if not self.Content then return end

    if not self._isCreated then
        self:OnCreate()
    end

    self.Content:AddToViewport()
    self._isOpen = true

    -- 调用生命周期回调
    self:OnOpen()

    -- 播放 Show 动画（由子类或 KDDWidget 层处理）
    if self.PlayShowAnimation then
        self:PlayShowAnimation()
    end
end

--- 关闭/隐藏（播放 Hide 动画后移除）
function KDDView:Hide()
    if not self.Content then return end
    if not self._isOpen then return end

    -- 播放 Hide 动画（由子类或 KDDWidget 层处理）
    if self.PlayHideAnimation then
        self:PlayHideAnimation()
    end

    self:OnClose()
    self.Content:RemoveFromParent()
    self._isOpen = false
end

--- 刷新（数据变化后更新 UI）
function KDDView:Refresh()
    self:OnRefresh()
end

--- 销毁（完全清理）
function KDDView:Destroy()
    if self._isOpen then
        self:Hide()
    end
    self:OnDestroy()
    self._isCreated = false
    self.Content = nil
end

-- ======== 生命周期回调（可重写） ========

function KDDView:OnOpen() end
function KDDView:OnClose() end
function KDDView:OnRefresh() end
function KDDView:OnDestroy() end

-- ======== 工具方法 ========

function KDDView:SetVisibility(Visibility)
    if self.Content then
        self.Content:SetVisibility(Visibility)
    end
end

function KDDView:GetPC()
    return self.PC
end

function KDDView:IsOpen()
    return self._isOpen
end

return KDDView
