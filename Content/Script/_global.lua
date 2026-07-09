---@meta _

-- ============================================================
-- UnLua 全局 API 类型声明
-- 为 Lua LSP 提供跳转定义和自动补全支持
-- ============================================================

---UnLua 暴露的 UE 全局 API 表
---@class UE_API
UE = {}

---加载一个 UObject（蓝图类/资源）
---@param Path string 蓝图资源路径，如 "/Game/UI/Main/MainView"
---@return userdata
function UE.LoadObject(Path) end

---@class UKDDBindingManager_C
UE.UKDDBindingManager = {}
---注册蓝图类到 Lua 模块的绑定映射
---@param ClassName string 蓝图类名（如 "MainView_C"）
---@param LuaPath string Lua 模块路径（如 "KDD.Biz.Main.MainView"）
function UE.UKDDBindingManager.RegisterBinding(ClassName, LuaPath) end

---@class UWidgetBlueprintLibrary_C
UE.UWidgetBlueprintLibrary = {}
---创建 Widget 实例
---@param WorldContext userdata
---@param WidgetClass userdata
---@param OwningPlayer userdata
---@return userdata
function UE.UWidgetBlueprintLibrary.Create(WorldContext, WidgetClass, OwningPlayer) end

---@class UGameplayStatics_C
UE.UGameplayStatics = {}
---切换关卡
---@param WorldContext userdata
---@param LevelName string 关卡路径
function UE.UGameplayStatics.OpenLevel(WorldContext, LevelName) end

---UnLua 路由到 Widget 的 Lua 生命周期函数
---@param self userdata Widget 实例，通过 self.Btn_Name 访问子控件
function Construct(self) end
function Destruct(self) end
function Tick(self, DeltaTime) end

---@type APlayerController
KDD_PC = nil
