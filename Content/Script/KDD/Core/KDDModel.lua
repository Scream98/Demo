-- ============================================================
-- KDDModel.lua - Model 基类
--
-- 纯数据层，不关心 UI。
-- 数据变化时通过 EventDispatcher 通知 Controller。
-- ============================================================

local KDDModel = {}
KDDModel.__index = KDDModel

function KDDModel.New(ModelName)
    local self = setmetatable({}, KDDModel)
    self.ModelName = ModelName
    self._listeners = {}   -- { eventName = { callback1, callback2, ... } }
    return self
end

-- ======== 事件系统 ========

function KDDModel:AddListener(EventName, Callback)
    if not self._listeners[EventName] then
        self._listeners[EventName] = {}
    end
    table.insert(self._listeners[EventName], Callback)
end

function KDDModel:RemoveListener(EventName, Callback)
    local list = self._listeners[EventName]
    if not list then return end
    for i, cb in ipairs(list) do
        if cb == Callback then
            table.remove(list, i)
            return
        end
    end
end

function KDDModel:Dispatch(EventName, ...)
    local list = self._listeners[EventName]
    if not list then return end
    for _, cb in ipairs(list) do
        local OK, Err = pcall(cb, ...)
        if not OK then
            warn("[KDDModel] Listener error on '" .. tostring(EventName) .. "': " .. tostring(Err))
        end
    end
end

-- ======== 可重写生命周期 ========

function KDDModel:OnInit() end
function KDDModel:OnDestroy() end

return KDDModel
