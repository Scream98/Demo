-- ============================================================
-- KDDController.lua - Controller 基类
--
-- 控制层：串联 Model 和 View。
-- 职责：
--   1. 监听 View 的用户交互事件
--   2. 调用 Model 修改数据
--   3. 监听 Model 数据变化并通知 View 刷新
--   4. 发送 Command 与其他 Controller 通信
-- ============================================================

local KDDController = {}
KDDController.__index = KDDController

function KDDController.New(ControllerName)
    local self = setmetatable({}, KDDController)
    self.ControllerName = ControllerName
    self.Model = nil
    self.View = nil
    self._eventConnections = {}  -- { {EventName, Callback}, ... } 用于自动清理
    return self
end

--- 绑定 Model
function KDDController:BindModel(Model)
    self.Model = Model
end

--- 绑定 View
function KDDController:BindView(View)
    self.View = View
end

--- 监听 Model 事件（自动记录以便清理）
function KDDController:ListenModelEvent(EventName, Callback)
    if not self.Model then
        warn("[KDDController] No model bound, cannot listen: " .. tostring(EventName))
        return
    end
    self.Model:AddListener(EventName, Callback)
    table.insert(self._eventConnections, { EventName, Callback })
end

--- 发送跨模块 Command
function KDDController:SendCommand(CmdName, ...)
    -- 交由 KDDCommand 模块统一调度
    local KDDCommand = require("KDD.Core.KDDCommand")
    if KDDCommand and KDDCommand.Dispatch then
        KDDCommand:Dispatch(CmdName, ...)
    end
end

-- ======== 可重写生命周期 ========

function KDDController:OnInit() end
function KDDController:OnDestroy()
    -- 自动清理所有事件监听
    if self.Model and self._eventConnections then
        for _, conn in ipairs(self._eventConnections) do
            self.Model:RemoveListener(conn[1], conn[2])
        end
    end
    self._eventConnections = {}
    self.Model = nil
    self.View = nil
end

return KDDController
