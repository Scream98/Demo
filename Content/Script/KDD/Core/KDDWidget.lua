-- ============================================================
-- KDDWidget.lua - Widget 的 Lua 侧包装基类
--
-- 继承 KDDView 的所有能力，额外提供：
--   1. 动画功能：自动识别 ShowAnim/HideAnim 并绑定播放
--   2. 自定义动画：Open/CustomClose/CustomLoop
--   3. 动画开关：bEnableAnimation（默认 true）
-- ============================================================

local KDDView = require("KDD.Core.KDDView")

local KDDWidget = {}
KDDWidget.__index = KDDWidget

-- 继承 KDDView 的方法
for k, v in pairs(KDDView) do
    KDDWidget[k] = v
end

function KDDWidget.New(Widget, WidgetName)
    local self = KDDView.New(Widget, WidgetName)
    setmetatable(self, KDDWidget)

    self.WidgetName = WidgetName
    self.bEnableAnimation = true

    -- 自动识别的动画名（蓝图侧需命名一致）
    self.AutoShowAnimName = "ShowAnim"
    self.AutoHideAnimName = "HideAnim"

    -- 自定义动画名（可外部设置）
    self.CustomShowAnimName = ""
    self.CustomLoopAnimName = ""
    self.CustomHideAnimName = ""

    return self
end

-- ======== 动画播放 ========

--- 获取蓝图中的 UWidgetAnimation 对象
function KDDWidget:_FindAnimation(AnimName)
    if not self.Content or not AnimName or AnimName == "" then return nil end
    -- 尝试通过 UMG 内置函数查找
    local OK, Anim = pcall(function()
        return self.Content:GetAnimation(AnimName)
    end)
    if OK and Anim then
        return Anim
    end
    return nil
end

--- 播放动画
function KDDWidget:_PlayAnim(Anim, StartAtTime, NumLoops, PlayMode)
    if not self.Content or not Anim then return end
    if not self.bEnableAnimation then return end
    local OK, Err = pcall(function()
        self.Content:PlayAnimation(Anim, StartAtTime or 0, NumLoops or 1, PlayMode or 0)
    end)
    if not OK then
        print("[KDDWidget] Play animation failed: " .. tostring(Err))
    end
end

--- 播放 Show 动画（自动识别 AutoShowAnimName）
function KDDWidget:PlayShowAnimation()
    if not self.bEnableAnimation then return end
    local Anim = self:_FindAnimation(self.AutoShowAnimName)
    if Anim then
        self:_PlayAnim(Anim, 0, 1)
        return
    end
    -- 回退到自定义动画
    if self.CustomShowAnimName and self.CustomShowAnimName ~= "" then
        local CustomAnim = self:_FindAnimation(self.CustomShowAnimName)
        if CustomAnim then
            self:_PlayAnim(CustomAnim, 0, 1)
        end
    end
end

--- 播放 Hide 动画（自动识别 AutoHideAnimName）
function KDDWidget:PlayHideAnimation()
    if not self.bEnableAnimation then return end
    local Anim = self:_FindAnimation(self.AutoHideAnimName)
    if Anim then
        self:_PlayAnim(Anim, 0, 1)
        return
    end
    -- 回退到自定义动画
    if self.CustomHideAnimName and self.CustomHideAnimName ~= "" then
        local CustomAnim = self:_FindAnimation(self.CustomHideAnimName)
        if CustomAnim then
            self:_PlayAnim(CustomAnim, 0, 1)
        end
    end
end

--- 播放 Loop 动画
function KDDWidget:PlayLoopAnimation()
    if not self.bEnableAnimation then return end
    if not self.CustomLoopAnimName or self.CustomLoopAnimName == "" then return end
    local Anim = self:_FindAnimation(self.CustomLoopAnimName)
    if Anim then
        -- Loop 模式：NumLoops = 0 表示无限循环
        self:_PlayAnim(Anim, 0, 0)
    end
end

--- 停止循环动画
function KDDWidget:StopLoopAnimation()
    if not self.Content then return end
    if not self.CustomLoopAnimName or self.CustomLoopAnimName == "" then return end
    local Anim = self:_FindAnimation(self.CustomLoopAnimName)
    if Anim then
        pcall(function()
            self.Content:StopAnimation(Anim)
        end)
    end
end

-- ======== 重写生命周期（注入动画） ========

function KDDWidget:OnOpen()
    self:PlayShowAnimation()
end

function KDDWidget:OnClose()
    self:PlayHideAnimation()
end

-- ======== 重写 Show/Hide（集成动画） ========

function KDDWidget:Show()
    if not self.Content then return end
    if self._isOpen then return end

    if not self._isCreated then
        self:OnCreate()
    end

    self.Content:AddToViewport()
    self._isOpen = true
    self:OnOpen()
end

function KDDWidget:Hide()
    if not self.Content then return end
    if not self._isOpen then return end

    self:OnClose()
    self.Content:RemoveFromParent()
    self._isOpen = false
end

-- ======== 可重写生命周期回调 ========

function KDDWidget:OnInit() end
function KDDWidget:OnShow() end
function KDDWidget:OnHide() end

-- ======== 设置可见性 ========

function KDDWidget:SetVisibility(Visibility)
    if self.Content then
        self.Content:SetVisibility(Visibility)
    end
end

-- ======== 设置自身为父控件的子控件 ========

function KDDWidget:AddToParent(ParentWidget)
    if self.Content and ParentWidget then
        self.Content:SetParent(ParentWidget)
    end
end

return KDDWidget
