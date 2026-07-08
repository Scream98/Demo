-- ============================================================
-- KDDBindingManager.lua - 配置注册 + View 工厂
--
-- ★ 模块加载时自动将 KDDLuaBindingConfig 注册到 C++ 侧，
--    后续 Widget 创建时由 UnLua 的 IUnLuaInterface + KDDModuleLocator
--    自动完成绑定，无需手动调用 KDD_BindLua。
--
-- 职责：
--   1. 模块级自动注册：读取配表 → push 到 C++ TMap
--   2. CreateView：创建 Widget（UnLua 自动绑定）+ 缓存管理
--   3. 提供 ViewName 到实例的缓存
-- ============================================================

local KDDLuaBindingConfig = require("KDD.KDDLuaBindingConfig")

-- ★ 模块级代码：启动时自动将配表注册到 C++ 侧
for ViewName, info in pairs(KDDLuaBindingConfig) do
    local ClassName = ViewName .. "_C"
    UE.UKDDBindingManager.RegisterBinding(ClassName, info.LuaPath)
end

local KDDView = require("KDD.Core.KDDView")

local KDDBindingManager = {
    ViewCache = {}
}

--- 自动补全蓝图类路径中的 .xxx_C 后缀
local function BuildBlueprintClassPath(ResPath)
    if ResPath:match("%.[^/]+_C$") then
        return ResPath
    end
    if ResPath:match("%.") then
        return ResPath .. "_C"
    end
    local AssetName = ResPath:match("([^/]+)$")
    if AssetName then
        return ResPath .. "." .. AssetName .. "_C"
    end
    return ResPath
end

function KDDBindingManager.CreateView(ViewName, WorldContext)
    local config = KDDLuaBindingConfig[ViewName]
    if not config then
        print("[KDD] View '" .. tostring(ViewName) .. "' not found in binding config")
        return nil
    end
    if KDDBindingManager.ViewCache[ViewName] then
        return KDDBindingManager.ViewCache[ViewName]
    end

    -- 1. 加载蓝图类
    local ClassPath = BuildBlueprintClassPath(config.ResPath)
    local WidgetClass = UE.LoadObject(ClassPath)
    if not WidgetClass then
        print("[KDD] Failed to load widget class: " .. ClassPath)
        return nil
    end

    -- 2. 创建 Widget 实例
    --    ★ UnLua 的 FUObjectCreateListener 会自动触发 TryBind，
    --       KDDModuleLocator 从配表中查找 Lua 路径并完成绑定。
    local Widget = UE.UWidgetBlueprintLibrary.Create(WorldContext, WidgetClass, WorldContext)
    if not Widget then
        print("[KDD] Failed to create widget")
        return nil
    end

    -- ★ 手动调用 Construct（兜底）
    --    UnLua 的自动路由在 BP 类的 Construct UFunction 跳过覆盖（见 LuaOverrides.cpp
    --    Override() 的 ExcludeSuper 检查），导致 Lua Construct 不会被自动调用。
    --    此处显式调用以确保子控件访问和事件绑定生效。
    local LuaModule = require(config.LuaPath)
    if LuaModule and LuaModule.Construct then
        local OK, Err = pcall(LuaModule.Construct, Widget)
        if not OK then
            print("[KDD] Construct call failed for '" .. config.LuaPath .. "': " .. tostring(Err))
        end
    end

    -- 3. 创建 KDDView 包装（缓存管理用的轻量层，非绑定必需）
    local ViewInstance = KDDView.New(Widget, ViewName, WorldContext)
    KDDBindingManager.ViewCache[ViewName] = ViewInstance

    print("[KDD] View '" .. ViewName .. "' created and ready (auto-bound by UnLua)")
    return ViewInstance
end

function KDDBindingManager.GetView(ViewName)
    return KDDBindingManager.ViewCache[ViewName]
end

function KDDBindingManager.DestroyView(ViewName)
    local instance = KDDBindingManager.ViewCache[ViewName]
    if instance then
        instance:OnDestroy()
        instance:Hide()
        KDDBindingManager.ViewCache[ViewName] = nil
        print("[KDD] View '" .. ViewName .. "' destroyed")
    end
end

function KDDBindingManager.DestroyAllViews()
    for name, instance in pairs(KDDBindingManager.ViewCache) do
        instance:OnDestroy()
        instance:Hide()
    end
    KDDBindingManager.ViewCache = {}
    print("[KDD] All views destroyed")
end

function KDDBindingManager.GetConfig()
    return KDDLuaBindingConfig
end

return KDDBindingManager
