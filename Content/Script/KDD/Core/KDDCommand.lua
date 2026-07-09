-- ============================================================
-- KDDCommand.lua - 跨模块命令调度器
--
-- 解耦不同 Controller 之间的调用。
-- ControllerA 不直接引用 ControllerB，改为发送 Command。
-- 任何 Controller 都可以通过 RegisterHandler 订阅命令。
-- ============================================================

local KDDCommand = {
    _handlers = {}   -- { CmdName = { handler1, handler2, ... } }
}

--- 注册命令处理器
-- @param CmdName  命令名称（字符串）
-- @param Handler  处理函数 function(...)
function KDDCommand.Register(CmdName, Handler)
    if not KDDCommand._handlers[CmdName] then
        KDDCommand._handlers[CmdName] = {}
    end
    table.insert(KDDCommand._handlers[CmdName], Handler)
end

--- 取消注册命令处理器
function KDDCommand.Unregister(CmdName, Handler)
    local list = KDDCommand._handlers[CmdName]
    if not list then return end
    for i, h in ipairs(list) do
        if h == Handler then
            table.remove(list, i)
            return
        end
    end
end

--- 派发命令
-- @param CmdName  命令名称
-- @param ...       附加参数
function KDDCommand.Dispatch(CmdName, ...)
    local list = KDDCommand._handlers[CmdName]
    if not list then
        print("[KDDCommand] No handler for: " .. tostring(CmdName))
        return
    end
    for _, handler in ipairs(list) do
        local OK, Err = pcall(handler, ...)
        if not OK then
            warn("[KDDCommand] Handler error for '" .. tostring(CmdName) .. "': " .. tostring(Err))
        end
    end
end

return KDDCommand
