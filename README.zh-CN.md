# JCPawns

[English README](README.md)

JCPawns 是一个 Unreal Engine 5 Runtime 插件，提供可复用的 Pawn 与输入模块，用于场景浏览和数字孪生类视角控制。

插件包含 C++ 类、可用于蓝图的 API、Enhanced Input 资产、示例 Pawn 蓝图、示例 GameMode 和示例地图。

## 功能特性

- 基于 `AJCDigitalTwinPawn` 的数字孪生导航 Pawn。
- 支持地面移动、视图平面移动、相机视角控制和 Spring Arm 缩放。
- 通过 Gameplay Tags 和 `UJCInputConfig` 绑定 Enhanced Input。
- 提供平滑聚焦到 Actor 包围盒的视口辅助方法。
- 提供与 `CameraActor` 混合切换并回到 Pawn 控制的辅助方法。
- 支持可中断的相机聚焦 Blend，新的相机聚焦请求会从当前渲染 POV 继续 Blend。
- 提供 Focus 完成兜底逻辑；如果外部 ViewTarget 切换打断 Blend，会恢复 Pawn 输入，避免视角拖拽卡住。
- 提供可被蓝图调用的输入激活和聚焦 API。
- `Content/` 下包含用于快速测试和集成参考的示例内容。

## 需求

- Unreal Engine 5。
- 启用 Enhanced Input 插件。
- 如果从源码构建插件，需要项目已配置 Unreal Engine C++ 构建工具链。

仓库没有声明具体的 Unreal Engine 5 小版本。请在你的项目所使用的 UE5 版本中测试该插件。

## 安装

1. 将本仓库复制或克隆到项目插件目录：

   ```text
   YourProject/Plugins/JCPawns
   ```

2. 打开 `YourProject.uproject`；如果项目使用 C++，可重新生成项目文件。
3. 如果 Unreal Editor 没有自动启用插件，请在插件浏览器中启用 `JCPawns`。
4. 当 Unreal Engine 提示编译插件时，构建或重新打开项目。

## 快速开始

### 试用示例内容

1. 如果 Content Browser 隐藏了插件内容，请先显示插件内容。
2. 打开示例地图：

   ```text
   Content/Map/Map_JCPawn.umap
   ```

3. 查看示例资产：

   ```text
   Content/Pawn/BP_JCDigitalTwinPawn.uasset
   Content/Pawn/BP_JCDigitalTwinPawn_Cpp.uasset
   Content/Pawn/BP_JCDigitalTwinPawn_Blueprint.uasset
   Content/Pawn/BP_JCDigitalTwinGameMode.uasset
   Content/Pawn/BP_JCPawns_GameMode.uasset
   ```

### 在项目中使用数字孪生 Pawn

1. 直接使用 `AJCDigitalTwinPawn`，或基于它创建蓝图。
2. 配置 `UJCDigitalTwinPawnInputComponent` 属性：
   - `InputMappingContext`
   - `JCInputConfig`
   - 移动、视角、缩放速度
   - 可选的地面移动曲线和缩放曲线
3. 将项目输入组件类设置为 `UJCEnhancedInputComponent`。
4. 使用 `PlayerController` Possess 该 Pawn。
5. 当 Pawn 需要开始接收输入映射上下文时，调用 `ActivateInput()`。
6. 当需要移除输入映射上下文时，调用 `DeactivateInput()`。

### Blend 到相机

当蓝图逻辑需要将视口移动到场景中的 `CameraActor`，并在完成后回到 Pawn 控制时，调用 `FocusViewportAsCamera`：

```cpp
FocusViewportAsCamera(CameraActor, BlendTime, BlendFunction, BlendExp, bInLockOutgoing);
```

行为说明：

- 在 Blend 期间再次调用 `FocusViewportAsCamera` 会中断上一次请求。
- 新 Blend 会从当前正在渲染的相机 POV 开始，而不是从上一次 Blend 的起点开始。
- 只有最新的 Focus 请求可以同步 Pawn 位置、Spring Arm 长度、控制旋转和输入状态。
- 如果 Blend 期间被其他系统切换 ViewTarget，Pawn 会通过兜底完成路径恢复输入，避免输入一直处于停用状态。

与 Level Sequence 的 Camera Cut 组合使用时，建议先暂停 Sequence，再启动 Pawn 聚焦；如果后续不再需要该 Sequence，再停止它：

```text
Pause -> FocusViewportAsCamera -> Stop
```

## 输入配置说明

`UJCDigitalTwinPawnInputComponent` 要求玩家输入组件为 `UJCEnhancedInputComponent`。如果项目仍使用默认的 `UEnhancedInputComponent`，插件会输出：

```text
Please replace EnhancedInputComponent by JCEnhancedInputComponent in project setting!
```

请在 Project Settings 中将默认输入组件类设置为 `JCEnhancedInputComponent` / `UJCEnhancedInputComponent`。

输入动作通过 `UJCInputConfig` 解析，该 Data Asset 将 `UInputAction` 资产映射到 Gameplay Tags。插件定义了以下 Native Gameplay Tags：

| Gameplay tag | 用途 |
| --- | --- |
| `JC.Pawn.DigitalTwin.Move.Ground` | 地面平面 Pawn 移动 |
| `JC.Pawn.DigitalTwin.Move.View` | 视图平面 Pawn 移动 |
| `JC.Pawn.DigitalTwin.Look` | 相机视角控制 |
| `JC.Pawn.DigitalTwin.Zoom` | 相机缩放 |

插件内置输入资产位于：

```text
Content/Input/IA/
Content/Input/IMC/
Content/Input/Config/
Content/Input/Curve/
```

## 主要 C++ 与蓝图 API

### `AJCDigitalTwinPawn`

用于数字孪生类场景导航的 Runtime Pawn。

主要蓝图可调用方法：

- `ActivateInput()`
- `DeactivateInput()`
- `FocusViewportOnActor(const AActor* InTargetActor)`
- `FocusViewportAsCamera(ACameraActor* InCameraActor, float InBlendTime = -1, EViewTargetBlendFunction InBlendFunction = VTBlend_EaseInOut, float InBlendExp = 1.0f, bool bInLockOutgoing = false)`

`FocusViewportAsCamera` 参数：

| 参数 | 用途 |
| --- | --- |
| `InCameraActor` | 要 Blend 到的目标相机 Actor。 |
| `InBlendTime` | Blend 时长。`-1` 使用 Pawn 的 `BlendTime` 属性，`0` 表示立即完成。 |
| `InBlendFunction` | UE ViewTarget Blend 曲线。 |
| `InBlendExp` | Ease 类型 Blend 使用的指数。 |
| `bInLockOutgoing` | 传给 UE ViewTarget Blend，用于在 Blend 期间锁定起始 POV。 |

重要可编辑属性：

- `FocusSpeed`
- `LineTraceDistance`
- `BlendTime`

### `UJCDigitalTwinPawnInputComponent`

由 `AJCDigitalTwinPawn` 持有的 Actor Component，负责绑定 Enhanced Input 动作并实现移动、视角和缩放行为。

主要 Blueprint Native 输入事件：

- `OnInputEvent_GroundMove`
- `OnInputEvent_ViewMove`
- `OnInputEvent_Look`
- `OnInputEvent_Zoom`

### `UJCInputConfig`

通过 `FJCTaggedInputAction` 将 Gameplay Tags 映射到输入动作的 Data Asset。

### `UJCEnhancedInputComponent`

Enhanced Input Component 扩展，提供按 Gameplay Tag 绑定输入动作的辅助方法：

```cpp
BindJCInputAction(InputConfig, InputTag, TriggerEvent, Object, Func);
```

### `AJCDefaultPawn`

一个小型 `ADefaultPawn` 扩展，提供可被蓝图调用的 `FocusViewportOnActor` 辅助方法。

## 内容概览

```text
Content/
├── Input/
│   ├── Config/DA_JCDigitalTwin_InputConfig.uasset
│   ├── Curve/C_JCDigitalTwin_GroundMove.uasset
│   ├── Curve/C_JCDigitalTwin_Zoom.uasset
│   ├── IA/
│   └── IMC/IMC_JCDigitalTwin_Default.uasset
├── Map/Map_JCPawn.umap
└── Pawn/
```

## 仓库结构

```text
JCPawns.uplugin
Source/JCPawns/
Content/
Resources/Icon128.png
LICENSE
README.md
README.zh-CN.md
```

## 许可证

本项目使用 [MIT License](LICENSE) 许可证。
