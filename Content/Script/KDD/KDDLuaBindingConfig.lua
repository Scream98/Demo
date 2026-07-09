-- ============================================================
-- KDD Lua 绑定配表
-- 在此处注册"蓝图路径 ↔ Lua 模块路径"的映射关系。
-- 蓝图设计师只需选择 UKDDView / UKDDWidget 作为基类，
-- 无需在蓝图中填写任何 Lua 信息。
-- ============================================================

local KDDLuaBindingConfig = {
    -- ======== 页面级 View（每个业务模块在 Biz/ 下有完整 MVC 三件套） ========
    --
    -- ResPath 只填基础资产路径（去掉 .uasset 后缀），
    -- _C 后缀由 KDDBindingManager.lua 自动补全。
    MainView = {
        ResPath = "/Game/UI/Main/MainView",
        LuaPath = "KDD.Biz.Main.MainView"
    },

    -- ======== MVC 示例（1 前缀确保排序靠前作示例） ========
    ["1Example"] = {
        ResPath = "/Game/UI/Example/WBP_1Example",
        LuaPath = "KDD.Biz.1Example.1ExampleView"
    },
}

return KDDLuaBindingConfig
