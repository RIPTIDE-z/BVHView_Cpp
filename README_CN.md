# BVHView_Cpp

<p align="center"><a href="README.md">English</a> | <a href="README_CN.md">中文</a></p>

---

> `BVHView_Cpp` 是对 [orangeduck/BVHView](https://github.com/orangeduck/BVHView)
> 的 C++17 重写，旨在为更通用的动画查看器提供基础

*目前仅支持 Windows*

- 本次重写保留了原版 BVH 查看器的行为以及现有的 `raylib` / `raygui` 依赖设置，
  同时将原本的单文件 C 实现拆分为职责明确的模块。
- 它为未来的独立项目 `ASCII_Anim_Viewer` 提供了一个参考。
  - 目标是支持更多基于 ASCII 的动画文件格式，例如 `SMD`。

此外还增加了窗口缩放支持、`F11` 全屏切换、`1280 x 720` 最小窗口尺寸，
以及按比例的 UI 缩放、原生 High DPI 绘制和内嵌的 Inter Regular 英文字体。

## 环境要求

- Windows 系统，装有 `MinGW` 或 `MSVC`
- `CMake` ≥ 3.22
- 支持 `C++17` 的编译器
- `Git`

## 构建

raylib 和 raygui 通过 Git 子模块固定在 `external/` 目录下。
使用任意合适的 CMake 生成器克隆并构建项目：

```powershell
git clone --recursive <仓库地址>
cd BVHView_Cpp
cmake -S . -B build
cmake --build build --config Release
```

如需指定构建系统，可使用如下命令：

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_CXX_COMPILER=g++
cmake --build build
```

Ninja 等单配置生成器默认使用 `Release`。如需构建 Debug 配置，请显式指定：

```powershell
cmake -S . -B build-debug -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_CXX_COMPILER=g++
cmake --build build-debug
```

对于已克隆但未初始化子模块的仓库：

```powershell
git submodule update --init --recursive
```

CMake 会将 raylib 与应用程序一起构建，无需单独的 raylib 构建步骤。

构建完成后，在 `app` 目录下运行可执行文件即可。

## 使用方式

通过应用内文件对话框打开 `.bvh` 文件、将文件拖入窗口，或在命令行中传入一个
或多个文件路径：

```powershell
.\app\bvhview.exe animation.bvh
```

常用操作：

| 按键                    | 功能         |
| ----------------------- | ------------ |
| `Ctrl + 鼠标左键拖拽` | 旋转摄像机   |
| `Ctrl + 鼠标右键拖拽` | 平移摄像机   |
| 鼠标滚轮                | 缩放         |
| `H`                   | 显示/隐藏 UI |
| `F11`                 | 切换全屏     |

## 第三方资源

界面内嵌了 [Inter](https://github.com/rsms/inter) 的 `Inter Regular` 字体。
字体使用 SIL Open Font License，许可证位于 `external/inter/OFL.txt`。

## 许可证

MIT。本次重写基于 [orangeduck/BVHView](https://github.com/orangeduck/BVHView)，
该项目使用 MIT 许可证发布。
