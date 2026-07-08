# GameDemo 项目文档

最后更新：2026-07-08

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
              ↓                              ↓
       UnLua 路由到 Lua Construct()    KDDModuleLocator::Locate()
              ↓                              ↓ 查 TMap
       MainView:Construct(self)         → "KDD.UI.MainView"
              ↓                              ↓
       Btn_Start.OnClicked:Add(...)    Manager->Bind(Widget, ModuleName)
```

**关键变化**：不再有手动 `KDD_BindLua` 调用。绑定由 UnLua 框架在 UObject 创建时自动触发。

## C++ 源码结构

```
Source/GameDemo/
├── GameDemo.Build.cs              - 模块依赖声明
├── GameDemo.h / .cpp             - 模块入口（IMPLEMENT_PRIMARY_GAME_MODULE）
├── CS.h / .cpp                   - ACS 角色类（ACharacter 子类）
├── Public/                        - 公开头文件
│   ├── KDDModuleLocator.h             - ★ 自定义 ModuleLocator（查 TMap）
│   ├── KDDBindingManager.h           - 绑定配置 TMap + RegisterBinding()
│   ├── KDDView.h                     - View 基类（UUserWidget + IUnLuaInterface）
│   ├── KDDWidget.h                   - Widget 基类（UUserWidget + IUnLuaInterface）
│   └── KDDGameInstance.h             - 游戏实例入口（UGameInstance 子类）
└── Private/                       - 私有实现
    ├── KDDModuleLocator.cpp
    ├── KDDBindingManager.cpp
    ├── KDDView.cpp
    ├── KDDWidget.cpp
    └── KDDGameInstance.cpp
```

## Lua 脚本结构

```
Content/Script/KDD/
├── KDDBindingManager.lua       - ★ 简化：模块加载时自动注册配表 + View 工厂
├── KDDLuaBindingConfig.lua     - 蓝图 ↔ Lua 映射配表
├── Core/
│   ├── KDDView.lua             - View 轻量包装（缓存管理用，非绑定必需）
│   └── KDDWidget.lua           - Widget 轻量包装
├── Game/
│   └── Bootstrap.lua           - 游戏启动入口（C++ Init 后自动调用）
└── UI/
    ├── MainView.lua            - 主视图（Construct/Destruct 由 UnLua 自动路由）
    └── MainMenu.lua            - 主菜单（预留）
```

## 关键模块说明

### UKDDGameInstance（C++ 入口）
- 文件：`Public/KDDGameInstance.h` / `Private/KDDGameInstance.cpp`
- `Init()` → 延迟一帧 → 从全局 Lua VM 先加载配表，再调用 Bootstrap.lua
- 全程不依赖任何蓝图节点

### UKDDModuleLocator（自定义 Lua 模块查找器）
- 文件：`Public/KDDModuleLocator.h` / `Private/KDDModuleLocator.cpp`
- 继承 `ULuaModuleLocator`，覆盖 `Locate()`
- 接收 UObject → 取其 ClassName → 查 KDDBindingManager TMap → 返回 LuaPath
- 通过 `DefaultEngine.ini` 启用：`ModuleLocatorClass=/Script/GameDemo.KDDModuleLocator`

### UKDDView / UKDDWidget（IUnLuaInterface 基类）
- View 用于全屏/面板级 UI；Widget 用于可复用组件
- 实现了 `IUnLuaInterface`，通过 `bImplUnluaInterface` 检查使 UnLua 尝试自动绑定
- **不在蓝图中配置 Lua 路径**，路径由 `KDDModuleLocator` 从配表 C++ TMap 中查找
- 子类 BP 直接选中这些基类即可，无需额外配置

### KDDBindingManager（C++ 绑定配置表）
- 文件：`Public/KDDBindingManager.h` / `Private/KDDBindingManager.cpp`
- 维护静态 `TMap<FString, FString>`：ClassName → LuaPath
- `RegisterBinding()` 由 Lua 侧启动时自动调用填充
- `GetLuaPath()` 供 `KDDModuleLocator.Locate()` 查询

### Bootstrap.lua（Lua 入口）
- 文件：`Content/Script/KDD/Game/Bootstrap.lua`
- 等待 PlayerController 就绪 → 创建 MainView → Show()
- 不再需要手动绑定调用

### KDDBindingManager.lua（配置注册 + View 工厂）
- **模块加载时自动执行**：遍历 `KDDLuaBindingConfig` → 调用 `UE.UKDDBindingManager.RegisterBinding()`
- `CreateView(ViewName, Context)` → 加载蓝图 → 创建 Widget（UnLua 自动绑定）→ 返回 KDDView 实例
- 不再包含 `KDD_BindLua` 和手动 `Construct` 调用

### MainView.lua（视图逻辑）
- 使用 UnLua 标准生命周期函数 `Construct(self)` / `Destruct(self)`
- `self` 即 widget userdata，可通过 `self.Btn_Start` 访问子控件
- 按钮事件用闭包捕获引用

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
