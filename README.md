# JCPawns

[中文文档](README.zh-CN.md)

JCPawns is an Unreal Engine 5 runtime plugin that provides reusable pawn and input building blocks for scene navigation and digital twin style viewers.

The plugin includes C++ classes, Blueprint-ready APIs, Enhanced Input assets, sample pawn Blueprints, sample GameModes, and a sample map.

## Features

- Digital twin navigation pawn based on `AJCDigitalTwinPawn`.
- Ground movement, view-plane movement, camera look, and spring-arm zoom input handling.
- Enhanced Input binding through gameplay tags and `UJCInputConfig`.
- Smooth focus helpers for moving the viewport to an actor bounding box.
- Camera handoff helper for blending to a `CameraActor` and returning control to the pawn.
- Interruptible camera focus blends that continue from the current rendered POV when a new camera focus request starts.
- Focus completion fallback that restores pawn input if an external view-target change interrupts the blend.
- Blueprint-callable input activation and focus APIs.
- Example content under `Content/` for quick testing and integration reference.

## Requirements

- Unreal Engine 5.
- Enhanced Input plugin enabled.
- A C++ build toolchain configured for your Unreal Engine project when building the plugin from source.

The repository does not declare a specific Unreal Engine 5 minor version. Test the plugin against the UE5 version used by your project.

## Installation

1. Copy or clone this repository into your project plugins folder:

   ```text
   YourProject/Plugins/JCPawns
   ```

2. Open `YourProject.uproject` or regenerate project files if your project uses C++.
3. Enable the `JCPawns` plugin in the Unreal Editor plugin browser if it is not enabled automatically.
4. Build or reopen the project when Unreal Engine asks to compile the plugin.

## Quick Start

### Try the sample content

1. Enable plugin content in the Content Browser if it is hidden.
2. Open the sample map:

   ```text
   Content/Map/Map_JCPawn.umap
   ```

3. Review the sample assets:

   ```text
   Content/Pawn/BP_JCDigitalTwinPawn.uasset
   Content/Pawn/BP_JCDigitalTwinPawn_Cpp.uasset
   Content/Pawn/BP_JCDigitalTwinPawn_Blueprint.uasset
   Content/Pawn/BP_JCDigitalTwinGameMode.uasset
   Content/Pawn/BP_JCPawns_GameMode.uasset
   ```

### Use the digital twin pawn in a project

1. Use `AJCDigitalTwinPawn` directly or create a Blueprint based on it.
2. Configure the `UJCDigitalTwinPawnInputComponent` properties:
   - `InputMappingContext`
   - `JCInputConfig`
   - movement, look, zoom speeds
   - optional ground movement and zoom curves
3. Set the project input component class to `UJCEnhancedInputComponent`.
4. Possess the pawn with a `PlayerController`.
5. Call `ActivateInput()` when the pawn should start receiving its mapping context.
6. Call `DeactivateInput()` when the mapping context should be removed.

### Blend to a camera

Use `FocusViewportAsCamera` when Blueprint logic needs to move the viewport to a placed `CameraActor` and then return control to the pawn:

```cpp
FocusViewportAsCamera(CameraActor, BlendTime, BlendFunction, BlendExp, bInLockOutgoing);
```

Behavior notes:

- A second `FocusViewportAsCamera` call during an active blend interrupts the previous request.
- The new blend starts from the current rendered camera POV, not from the original blend start.
- Only the latest focus request is allowed to sync pawn location, spring-arm length, control rotation, and input state.
- If another system changes the view target during the blend, the pawn uses a fallback completion path so input is restored instead of staying deactivated.

When combining with a Level Sequence camera cut, pause the sequence before starting pawn focus, then stop it only if you no longer need it:

```text
Pause -> FocusViewportAsCamera -> Stop
```

## Input Setup Notes

`UJCDigitalTwinPawnInputComponent` expects the player input component to be `UJCEnhancedInputComponent`. If the project still uses the default `UEnhancedInputComponent`, the plugin will report:

```text
Please replace EnhancedInputComponent by JCEnhancedInputComponent in project setting!
```

In Project Settings, set the default input component class to `JCEnhancedInputComponent` / `UJCEnhancedInputComponent`.

Input actions are resolved through `UJCInputConfig`, which maps `UInputAction` assets to gameplay tags. The plugin defines these native gameplay tags:

| Gameplay tag | Purpose |
| --- | --- |
| `JC.Pawn.DigitalTwin.Move.Ground` | Ground-plane pawn movement |
| `JC.Pawn.DigitalTwin.Move.View` | View-plane pawn movement |
| `JC.Pawn.DigitalTwin.Look` | Camera look |
| `JC.Pawn.DigitalTwin.Zoom` | Camera zoom |

The included input assets are located in:

```text
Content/Input/IA/
Content/Input/IMC/
Content/Input/Config/
Content/Input/Curve/
```

## Main C++ and Blueprint APIs

### `AJCDigitalTwinPawn`

Runtime pawn for digital twin style scene navigation.

Key Blueprint-callable methods:

- `ActivateInput()`
- `DeactivateInput()`
- `FocusViewportOnActor(const AActor* InTargetActor)`
- `FocusViewportAsCamera(ACameraActor* InCameraActor, float InBlendTime = -1, EViewTargetBlendFunction InBlendFunction = VTBlend_EaseInOut, float InBlendExp = 1.0f, bool bInLockOutgoing = false)`

`FocusViewportAsCamera` parameters:

| Parameter | Purpose |
| --- | --- |
| `InCameraActor` | Target camera actor to blend to. |
| `InBlendTime` | Blend duration. `-1` uses the pawn `BlendTime` property. `0` completes immediately. |
| `InBlendFunction` | View-target blend curve. |
| `InBlendExp` | Exponent used by ease blend functions. |
| `bInLockOutgoing` | Passed to UE view-target blending to lock the outgoing POV while blending. |

Important editable properties:

- `FocusSpeed`
- `LineTraceDistance`
- `BlendTime`

### `UJCDigitalTwinPawnInputComponent`

Actor component owned by `AJCDigitalTwinPawn` that binds Enhanced Input actions and implements movement, look, and zoom behavior.

Main Blueprint-native input events:

- `OnInputEvent_GroundMove`
- `OnInputEvent_ViewMove`
- `OnInputEvent_Look`
- `OnInputEvent_Zoom`

### `UJCInputConfig`

Data asset that maps gameplay tags to input actions through `FJCTaggedInputAction`.

### `UJCEnhancedInputComponent`

Enhanced Input component with a helper method for binding input actions by gameplay tag:

```cpp
BindJCInputAction(InputConfig, InputTag, TriggerEvent, Object, Func);
```

### `AJCDefaultPawn`

Small `ADefaultPawn` extension with a Blueprint-callable `FocusViewportOnActor` helper.

## Content Overview

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

## Repository Layout

```text
JCPawns.uplugin
Source/JCPawns/
Content/
Resources/Icon128.png
LICENSE
README.md
README.zh-CN.md
```

## License

This project is licensed under the [MIT License](LICENSE).
