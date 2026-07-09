# GameDemo 项目文档

最后更新：2026-07-09

## 项目概述

基于 UE 5.8 的 UnLua 集成示范项目，采用 C++ + Lua 纯代码驱动架构（即"零蓝图节点"模式）。

UI 基类（UKDDView / UKDDWidget）实现 `IUnLuaInterface`，配合自定义 `UKDDModuleLocator` 从配表中自动查找 Lua 模块路径。**蓝图设计师只需选好 C++ 基类创建蓝图，不需在蓝图中配置任何 Lua 路径**。

## 架构概览

```
游戏启动
  ↓
UKDDGameInstance::Init()
  ↓  延迟一帧
TryBootstrap()
  ┌──────────────────────────────────┐
  │ 1. PushUObject(PC) → KDD_PC     │
  │ 2. require('KDDBindingManager') │ ← 模块级代码自动注册绑
  │    → RegisterBinding() x N      │   定配表到 C++ TMap
  │ 3. Bootstrap:Init()             │
  └──────────────────────────────────┘
  ↓  PC 就绪?
  ├── 否 → 延迟一帧重试
  └── 是 → Bootstrap.lua:Init()                     Lua 层
              ↓
       KDDBindingManager.CreateView("MainView", PC)
              ↓
       LoadObject → CreateWidget          ← UObject 创建
              ↓                              ↓
       view:Show() → AddToViewport()   UnLua NotifyUObjectCreated
              ↓                              ↓
       NativeConstruct()               TryBind → IUnLuaInterface 检查通过
            ↓ OnCreate → OnOpen             ↓
       UnLua 路由到 Lua Construct()    KDDModuleLocator::Locate()
              ↓                              ↓ 查 TMap
       MainView:Construct(self)         → "KDD.UI.MainView"
              ↓                              ↓
       Btn_Start.OnClicked:Add(...)    Manager->Bind(Widget, ModuleName)

  (新版 UKDDWidget 还多了动画流程)
  OnOpen → PlayShowAnimation()  ← 自动识别 ShowAnim / HideAnim
  OnClose → PlayHideAnimation()
```

## C++ 源码结构

```
Source/GameDemo/
├── GameDemo.Build.cs              - 模块依赖声明（含子目录包含路径）
├── GameDemo.h / .cpp             - 模块入口（IMPLEMENT_PRIMARY_GAME_MODULE）
├── CS.h / .cpp                   - ACS 角色类（ACharacter 子类）
│
├── Unlua/                        - UnLua 集成相关
│   ├── KDDBindingManager.h/.cpp       - 绑定配置 TMap + RegisterBinding()
│   └── KDDModuleLocator.h/.cpp        - ★ 自定义 ModuleLocator（查 TMap）
│
├── UMG/                          - UI 基类
│   ├── KDDView.h/.cpp                 - View 基类（UUserWidget + IUnLuaInterface）
│   │                                    生命周期：OnCreate → OnOpen → OnClose → OnRefresh → OnDestroy
│   └── KDDWidget.h/.cpp               - Widget 基类（继承 UKDDView）
│                                        ★ 动画系统：自动识别 ShowAnim/HideAnim
│                                           3 自定义动画名 + 3 编辑器测试按钮
│
└── Core/                         - 应用核心
    └── KDDGameInstance.h/.cpp         - 游戏实例入口（UGameInstance 子类）
```

**类层次**：
```
UUserWidget + IUnLuaInterface
        ↑
     UKDDView      (页面级 View 基类，完整生命周期)
        ↑
    UKDDWidget     (可复用组件基类，增动画系统)
```

## Lua 脚本结构

Script 根目录有辅助文件帮助编辑器（AI 和人类）理解项目约定：

```
Content/Script/
├── README.md                ← ★ 项目约定总览，新模块必读
├── .luarc.json              ← Lua LSP 配置（VSCode 跳转/提示）
├── _global.lua              ← 全局变量类型声明（纯编辑器用，`---@meta _`）
│                            声明了 KDD_PC、UE.LoadObject 等全局 API
│
└── KDD/
```

### 全局变量（由 C++ 注入，无需在 Lua 中定义）

| 变量 | 注入位置 | 说明 |
|------|----------|------|
| `KDD_PC` | `Core/KDDGameInstance.cpp` → `lua_setglobal(L, "KDD_PC")` | APlayerController。所有业务代码用到关卡切换、获取 World 时用此变量 |
| `UE.*` | UnLua 插件自动注册 | UE 引擎 API，如 `UE.LoadObject()`、`UE.UGameplayStatics.OpenLevel()`、`UE.UKDDBindingManager.RegisterBinding()` |

### 目录结构

```
KDD/
├── KDDBindingManager.lua       - 配置注册 + View 工厂（模块加载时自动注册配表）
├── KDDLuaBindingConfig.lua     - 蓝图 ↔ Lua 映射配表
│
├── Core/                       - MVC 框架核心基类
│   ├── KDDModel.lua            - Model 基类（数据层 + 事件系统）
│   ├── KDDController.lua       - Controller 基类（串联 Model↔View）
│   ├── KDDCommand.lua          - Command 调度器（解耦跨模块调用）
│   ├── KDDView.lua             - View 包装基类（增强生命周期 OnCreate/OnOpen/OnClose/OnRefresh/OnDestroy）
│   └── KDDWidget.lua           - Widget 包装基类（继承 KDDView + 动画播放）
│
├── Biz/                        - 业务模块（MVC 模式，每模块独立 Model/Controller）
│   ├── Main/                   - 主界面（MainView 的 MVC）
│   │   ├── MainViewModel.lua
│   │   └── MainViewController.lua
│   └── 1Example/               - ★ MVC 示例（按数字排序靠前）
│       ├── 1ExampleModel.lua
│       ├── 1ExampleController.lua
│       └── 1ExampleView.lua
│
├── Game/                       - 游戏应用入口
│   └── GameApp.lua             - C++ 调 GameStart() / GameShutdown()
│                               （Bootstrap 已重命名为 GameApp）
```

## MVC 模式说明

每个业务模块由三个文件组成（放在 `Biz/<ModuleName>/` 目录下）：

| 层 | 文件 | 职责 | 依赖 |
|---|---|---|---|
| **M**odel | `XXXModel.lua` | 纯数据 + 事件分发 | 无（纯 Lua table） |
| **V**iew | `XXXView.lua` | UI 显示 + 用户交互 | 无（UnLua 自动绑定） |
| **C**ontroller | `XXXController.lua` | 串联 M↔V，业务编排 | 持有 M 和 V 的引用 |

**启动流程**：
```
1ExampleView:Construct()
  → require Controller / Model
  → Controller:BindModel(Model)
  → Controller:BindView(self)
  → Controller:OnInit() / Model:OnInit()
  → 绑定按钮事件回调 Controller 方法
```

**通信方式**：
- Model → Controller：事件分发 `Model:Dispatch("ItemsChanged", data)` → 通过 `AddListener` 注册的 Controller 回调
- Controller → Controller：Command 命令 `Controller:SendCommand("OpenShop")` → `KDDCommand.Dispatch()`
- Controller → View：直接调用 `View:RefreshUI(data)`

## 关键模块说明

### UKDDGameInstance（C++ 入口）
- 文件：`Core/KDDGameInstance.h` / `Core/KDDGameInstance.cpp`
- `Init()` → 延迟一帧 → 从全局 Lua VM 先加载配表，再调用 Bootstrap.lua
- 全程不依赖任何蓝图节点

### UKDDModuleLocator（自定义 Lua 模块查找器）
- 文件：`Unlua/KDDModuleLocator.h` / `Unlua/KDDModuleLocator.cpp`
- 继承 `ULuaModuleLocator`，覆盖 `Locate()`
- 接收 UObject → 取其 ClassName → 查 KDDBindingManager TMap → 返回 LuaPath
- 通过 `DefaultEngine.ini` 启用：`ModuleLocatorClass=/Script/GameDemo.KDDModuleLocator`

### UKDDView（View 基类）
- 文件：`UMG/KDDView.h` / `UMG/KDDView.cpp`
- 用于全屏/面板级 UI
- 实现了 `IUnLuaInterface` + 完整生命周期：`OnCreate → OnOpen → OnClose → OnRefresh → OnDestroy`
- `GetShowAnimation()` / `GetHideAnimation()` 供子类覆盖
- `FindAnimationByName()` 按字符串名查找蓝图动画

### UKDDWidget（Widget 基类，继承 UKDDView）
- 文件：`UMG/KDDWidget.h` / `UMG/KDDWidget.cpp`
- 用于可复用组件（按钮、列表项、面板等）
- 继承 UKDDView 的所有能力 + 动画系统
- **动画功能**：
  - 自动识别蓝图中名为 `ShowAnim` / `HideAnim` 的动画
  - `OnOpen()` 自动播放 Show 动画，`OnClose()` 自动播放 Hide 动画
  - `bEnableAnimation`（bool，默认 true）动画总开关
  - `CustomShowAnimName / CustomHideAnimName / CustomLoopAnimName` 三个文本输入框配置自定义动画
  - 3 个编辑器测试按钮：▶ Play Show Animation / ▶ Play Loop Animation / ▶ Play Hide Animation

### KDDBindingManager（C++ 绑定配置表）
- 文件：`Unlua/KDDBindingManager.h` / `Unlua/KDDBindingManager.cpp`
- 维护静态 `TMap<FString, FString>`：ClassName → LuaPath
- `RegisterBinding()` 由 Lua 侧启动时自动调用填充
- `GetLuaPath()` 供 `KDDModuleLocator.Locate()` 查询

### GameApp.lua（Lua 入口）
- 文件：`Content/Script/KDD/Game/GameApp.lua`
- C++ 直接调用 `GameStart()` 函数（全局函数，非模块方法）
- 等待 PlayerController 就绪 → 创建 MainView → Show()
- C++ Shutdown 时调用 `GameShutdown()` 清理所有 View
- 原名 Bootstrap.lua，已重命名为含义更清晰的 GameApp.lua

### KDDBindingManager.lua（配置注册 + View 工厂）
- **模块加载时自动执行**：遍历 `KDDLuaBindingConfig` → 调用 `UE.UKDDBindingManager.RegisterBinding()`
- `CreateView(ViewName, Context)` → 加载蓝图 → 创建 Widget（UnLua 自动绑定）→ 返回 KDDView 实例

## 配置

- `Config/DefaultEngine.ini`：`GameInstanceClass=/Script/GameDemo.KDDGameInstance`
- `Config/DefaultEngine.ini`：`ModuleLocatorClass=/Script/GameDemo.KDDModuleLocator`（[/Script/UnLua.UnLuaSettings]）
- 默认地图：`/Game/Map/Main/MainMap`
- 战斗地图：`/Game/Map/Main/BattleMap`

## 地图

| 地图 | 路径 |
|------|------|
| MainMap | `/Game/Map/Main/MainMap` |
| BattleMap | `/Game/Map/Main/BattleMap` |

## 插件

- `UnLua`：Lua 集成框架（Tencent）
- `KDDGitHelper`：Git 辅助工具
