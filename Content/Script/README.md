# KDD Lua 脚本项目

## 全局变量（由 C++ 注入，Lua 侧直接用）

| 变量 | 来源 | 说明 |
|------|------|------|
| `KDD_PC` | `KDDGameInstance.cpp` 启动时注入 | APlayerController 对象。所有 Lua 业务代码通过它获取 World、OpenLevel 等 |
| `UE.*` | UnLua 插件 | UE 引擎 API。`UE.LoadObject()`、`UE.UGameplayStatics.*`、`UE.UKDDBindingManager.*` 等 |

类型声明见 `_global.lua`，VSCode + Lua 扩展可跳转。

## 目录结构

```
Content/Script/
├── README.md                ← 本文件
├── .luarc.json              ← Lua LSP 配置
├── _global.lua              ← 全局类型声明（纯编辑器用，不产生运行时效果）
├── KDD/
│   ├── KDDBindingManager.lua      - 配置注册 + View 工厂
│   ├── KDDLuaBindingConfig.lua    - 蓝图 ↔ Lua 映射配表
│   ├── Core/                      - MVC 框架基类（共用的）
│   │   ├── KDDModel.lua           - Model 基类
│   │   ├── KDDController.lua      - Controller 基类
│   │   ├── KDDCommand.lua         - 跨模块命令调度
│   │   ├── KDDView.lua            - View 包装基类
│   │   └── KDDWidget.lua          - Widget 包装基类（含动画）
│   ├── Biz/                       - 业务模块（独立 MVC 三件套）
│   │   ├── Main/                  - 主界面
│   │   │   ├── MainView.lua       (V)  ← UnLua 绑定入口
│   │   │   ├── MainViewModel.lua  (M)
│   │   │   └── MainViewController.lua (C)
│   │   └── 1Example/              - MVC 示例
│   │       ├── 1ExampleView.lua
│   │       ├── 1ExampleModel.lua
│   │       └── 1ExampleController.lua
    │   └── Game/
    │       └── GameApp.lua            - 应用入口（C++ 调 GameStart() / GameShutdown()）
```

## MVC 规范

每个业务模块在 `Biz/<模块名>/` 下建三个文件：

```
Biz/Shop/                    ← 新模块示例
├── ShopView.lua             (V)  ← UnLua 自动绑定此文件
├── ShopModel.lua            (M)  ← 数据 + 事件分发
└── ShopController.lua       (C)  ← 串联 M↔V，处理业务
```

**View 入口规范**：
- `Construct(self)` 中 `require` 对应的 Controller 和 Model
- 调用 `Controller:BindModel(Model)`、`Controller:BindView(self)`
- 调用 `Controller:OnInit()`、`Model:OnInit()`
- 绑定 UI 事件回调 Controller 方法
- `Destruct()` 中调用 `Controller:OnDestroy()`、`Model:OnDestroy()`

**新模块需同步更新**：`KDD/KDDLuaBindingConfig.lua` 添加蓝图路径↔Lua路径映射。

## 生命周期

UKDDView / UKDDWidget（C++ 侧自动路由）：
```
NativeConstruct → OnCreate → OnOpen → (动画播放)
NativeDestruct  → OnClose → OnDestroy
```

UnLua 路由到 Lua `Construct(self)` / `Destruct(self)` 在 `OnCreate/OnDestroy` 之间。

## 配表注册

`KDDLuaBindingConfig.lua` 是蓝图↔Lua 的唯一映射入口。新增页面或组件需在此添加：
```lua
MyNewView = {
    ResPath = "/Game/UI/MyNew/MyNewView",   -- 蓝图的 Content 路径
    LuaPath = "KDD.Biz.MyNew.MyNewView"      -- Lua 模块路径
}
```

## 构建

```bash
# 编译 C++ 项目（AutoBuild.bat 未整理时手动执行）
"D:\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" GameDemoEditor Win64 Development -Project="E:\UE5Project\Demo\GameDemo.uproject"
```
