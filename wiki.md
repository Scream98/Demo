# GameDemo 项目文档

最后更新：2026-07-13（内容/UI 补充分类）

---

## 项目概述

| 项目 | 值 |
|------|-----|
| 引擎 | UE 5.8 |
| 主模块 | `GameDemo` (Runtime, C++ + UnLua Lua 驱动) |
| 构建 | BuildSettings V7, PCHUseExplicitOrSharedPCHs |
| 渲染 | DX12 SM6, 虚拟阴影, 光线追踪, Substrate |
| 架构 | C++ 层(引擎基础) → Lua 层(MVC 业务) |

### 插件依赖

| 插件 | 范围 | 用途 |
|------|------|------|
| UnLua | 全部 | Lua 脚本集成、UObject 自动绑定 |
| GameplayAbilities | 全部 | 游戏能力系统 |
| KDDGitHelper | 全部 | Git 辅助工具插件 |
| ModelingToolsEditorMode | Editor | 编辑器建模工具 |
| PythonScriptPlugin | 全部 | Python 脚本支持 |
| UIPacker | 全部 | UI 打包工具 |

---

## 目录结构

```
GameDemo/
├── GameDemo.uproject           ← 项目文件 (UE 5.8)
├── GameDemo.sln / .slnx       ← VS 解决方案
│
├── Config/
│   ├── DefaultEngine.ini      ← 引擎设置 (UnLua/地图/渲染/UI)
│   ├── DefaultGame.ini        ← 游戏设置 (CommonUI/打包)
│   ├── DefaultInput.ini       ← 输入设置 (WASD/手柄/EnhancedInput)
│   └── DefaultEditor.ini      ← 编辑器设置
│
├── Source/GameDemo/
│   ├── GameDemo.h/.cpp        ← 模块入口
│   ├── CS.h/.cpp              ← ACS 角色基类
│   ├── Core/
│   │   └── KDDGameInstance.h/.cpp  ← 游戏实例 (C++→Lua 启动)
│   ├── UMG/
│   │   ├── KDDWidget.h/.cpp   ← 控件基类 (生命周期+动画)
│   │   ├── KDDView.h/.cpp     ← 视图基类 (全屏/面板)
│   │   └── Widgets/           ← 具体 Widget 子类蓝图
│   └── Unlua/
│       ├── KDDBindingManager.h/.cpp  ← 绑定配表管理器
│       └── KDDModuleLocator.h/.cpp   ← Lua 模块定位器
│
├── Content/
│   ├── Map/Main/MainMap       ← 主关卡
│   ├── Script/                ← Lua 脚本
│   │   └── KDD/
│   │       ├── Core/          ← MVC 基类 (Model/Controller/Command/View/Widget)
│   │       ├── Biz/           ← 业务模块 (Main 等)
│   │       └── Game/GameApp.lua ← Lua 业务入口
│   ├── UI/                    ← UI 资源（蓝图 + 贴图 + 图集）
│   │   ├── BP/                ← 蓝图文件（Widget 蓝图）
│   │   ├── DesignResource/    ← 设计源素材（原始 PNG + 导入 uasset）
│   │   ├── SpriteSheet/       ← 精灵图集（打包后的帧 + 纹理）
│   │   └── Texture/           ← 独立纹理（预留，当前为空）
│   ├── Char/                  ← 角色资源
│   ├── EldenRing/             ← 项目特定美术资源
│   ├── Input/                 ← 输入映射资源
│   └── Collections/           ← 资源集合
│
├── Plugins/                   ← 插件目录
├── Binaries/                  ← 编译输出 (gitignored)
├── Intermediate/              ← 中间文件 (gitignored)
├── Saved/                     ← 编辑器保存 (gitignored)
└── DerivedDataCache/          ← DDC 缓存 (gitignored)
```

---

### 1. 启动流程：`UKDDGameInstance` → Lua

**文件**：`Source/GameDemo/Core/KDDGameInstance.h/.cpp`

```
Init()
  ├── World 就绪？→ TimerManager.SetTimerForNextTick(OnLuaStart)
  └── World 未就绪？→ FTSTicker 0.1s 后重试

OnLuaStart() → TryStart()
  ├── 获取 UnLua Lua VM (IUnLuaModule::Get().GetEnv())
  ├── PlayerController 就绪？
  │   ├── 是 → PushUObject 到 Lua 全局变量 KDD_PC
  │   └── 否 → 延迟一帧重试
  ├── 判断当前地图
  │   ├── MainMap → require('KDD.Game.GameApp'); GameStart()
  │   └── 其他地图 → require('KDD.Game.GameApp')（只加载环境）
  └── PC 未就绪 → 延迟一帧再试直至成功
```

**关键设计**：
- 纯 C++ → Lua 驱动，不依赖任何蓝图节点逻辑
- Init() 时 World 可能未就绪（尤其是 PIE 环境），用 FTSTicker 兜底
- 地图名判断用 `EndsWith("MainMap")`，兼容 PIE 环境 `UEDPIE_0_` 前缀

---

### 2. UI 基类：`UKDDWidget` / `UKDDView`

**文件**：`Source/GameDemo/UMG/KDDWidget.h/.cpp`、`KDDView.h/.cpp`

#### 类层次

```
UUserWidget
  └── UKDDWidget : UUserWidget, IUnLuaInterface  ← 控件基类（可复用组件）
        └── UKDDView                              ← 视图基类（全屏/面板级）
```

#### 生命周期流程

```
NativeConstruct → OnCreate() → OnOpen()  → PlayShowAnimation()
NativeDestruct  → OnClose()  → PlayHideAnimation() → OnDestroy()
                    ↑              ↑
               (子类可覆盖)    (由子类调用时机)
```

#### 动画系统

| 动画类型 | 默认查找名 | 自定义属性 | 触发时机 |
|----------|-----------|-----------|---------|
| ShowAnim | `ShowAnim` | `CustomShowAnimName` | OnOpen 自动播放 |
| HideAnim | `HideAnim` | `CustomHideAnimName` | OnClose 自动播放 |
| LoopAnim | `LoopAnim` | `CustomLoopAnimName` | 手动 PlayLoopAnimation |

**动画查找流程** (`FindAnimationByName`)：
1. 反射查找同名 `FObjectProperty`（蓝图生成的动画属性）
2. 兜底：扫描 `WidgetBlueprintGeneratedClass::Animations` 按名字匹配
3. `CacheAutoAnimations()` 在 `NativeConstruct` 时自动缓存

#### UnLua 自动绑定

- 实现了 `IUnLuaInterface`，`GetModuleName()` 返回空串（由 `KDDModuleLocator` 从配表查）
- `UKDDView` 专为全屏/面板视图设计，蓝图直接创建即可，无需任何手动配置

---

### 3. 绑定系统：`KDDBindingManager` + `KDDModuleLocator`

**文件**：`Source/GameDemo/Unlua/*`

#### 数据流

```
KDDLuaBindingConfig.lua（启动时）
    ↓ RegisterBinding(ClassName, LuaPath)
UKDDBindingManager（C++ 静态 TMap）
    ↓ GetLuaPath(ClassName)
KDDModuleLocator::Locate(Object)  ← ModuleLocatorClass 配置在 DefaultEngine.ini
    ↓
UnLua 自动绑定：UObject 创建时找到对应 Lua 模块并加载
```

#### KDDBindingManager

- 静态 `TMap<FString, FString>` 存储 `ClassName → LuaPath` 映射
- `RegisterBinding()` 暴露给 Lua 侧调用（BlueprintCallable）
- `GetLuaPath()` 提供兜底逻辑：如果没找到，去掉 `_C` 后缀再查一次

#### KDDModuleLocator

- 继承 `ULuaModuleLocator`，在 `DefaultEngine.ini` 中配置为默认定位器
- `Locate()` 直接调用 `UKDDBindingManager::GetLuaPath()`

---

### 4. 角色基类：`ACS`

**文件**：`Source/GameDemo/CS.h/.cpp`

- 继承 `ACharacter`
- 支持 Tick、输入绑定（`SetupPlayerInputComponent`）
- 基础角色类，具体行为由 Lua 侧扩展

---

## 编辑器动画预览方案

### 背景与原理

UE5.8 UMG 蓝图编辑器中，`CallInEditor` 在**设计器预览 Widget 实例**上执行，该实例有有效的 `GetWorld()`（Editor World），可以正常创建 `FWidgetAnimationState`。但设计时没有世界 tick，动画不会被驱动。

### 方案：World TimerManager 手动 tick

```
CallInEditor 按钮点击
  → PlayAnimation(Anim) 创建 FWidgetAnimationState
  → 获取 GetAnimationState(Anim) 句柄
  → SetTimer(0.017s, loop) 启动定时器
  → 定时器回调：EditorAnimState->Tick(dt) + Invalidate 强制重绘
  → 动画完成回调：停止定时器
```

### 关键代码位置

`KDDWidget.cpp` 中 `#if WITH_EDITOR` 块：

| 函数 | 职责 |
|------|------|
| `Editor_PlayShowAnim()` | Show 动画测试入口 + 绑定完成回调 |
| `Editor_PlayLoopAnim()` | Loop 动画测试入口（不限循环） |
| `Editor_PlayHideAnim()` | Hide 动画测试入口 |
| `Editor_StartPreviewAnimTimer()` | 启动 0.017s 世界定时器 |
| `Editor_TickPreviewAnim()` | 每帧驱动 `EditorAnimState->Tick(dt)` + 强制重绘 |
| `Editor_StopPreviewAnimTimer()` | 停止定时器 |
| `Editor_OnAnimFinished()` | 动画完成回调 + 清理资源 |

### 模块依赖

```
PrivateDependencyModuleNames.Add("UnrealEd");    // GEditor
PrivateDependencyModuleNames.Add("UMGEditor");    // FWidgetBlueprintEditor
```

---

## Lua 层概览

Lua 目录：`Content/Script/KDD/`

```
KDD/
├── Core/           ← MVC 基类 (Model / Controller / Command / View / Widget)
├── Biz/            ← 业务模块
│   └── Main/       ← 主界面 (MainView / MainViewModel / MainViewController)
└── Game/
    └── GameApp.lua ← Lua 业务入口 (GameStart / GameShutdown)
```

### MVC 模式

| 层 | 职责 | 文件位置 |
|----|------|---------|
| View | 界面显示，绑定 C++ Widget | `KDD.Biz.{Module}.{Module}View` |
| ViewModel | 数据状态管理 | `KDD.Biz.{Module}.{Module}ViewModel` |
| Controller | 业务逻辑处理 | `KDD.Biz.{Module}.{Module}Controller` |

---

## 构建配置

### GameDemo.Build.cs

| 类型 | 依赖 |
|------|------|
| 公开 | Core, CoreUObject, Engine, InputCore, EnhancedInput, UnLua, UMG |
| 私有(编辑器) | Slate, SlateCore, Lua |
| Include | `Core/`, `UMG/`, `Unlua/` 子目录 |

### 打包配置 (DefaultGame.ini)

- Pak + IoStore + ZenStore
- Oodle Kraken 压缩
- 始终打包 `Script/` 和 `UnLua/Content/Script` 目录

---

## 调试说明

### 日志前缀

| 前缀 | 来源 |
|------|------|
| `[KDDGameInstance]` | C++ 启动流程 |
| `[UKDDWidget]` | UI 控件生命周期/动画 |
| `[KDDBindingManager]` | 绑定配表注册/查询 |
| `[KDDWidget]` | 编辑器动画预览 |

### 常见排查路径

1. 启动卡住 → 检查 `[KDDGameInstance]` 日志，看 Lua VM 和 PC 是否就绪
2. 动画不播放 → 检查 `[UKDDWidget]` 日志看动画是否成功缓存
3. 绑定不生效 → 检查 `[KDDBindingManager]` 看 `RegisterBinding` 是否被调用
4. 编辑器动画不动 → 检查 `[KDDWidget]` 日志看 `EditorAnimState` 是否 valid
