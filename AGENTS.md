# AGENTS.md

Guidance for ZCode agents working in this repository.

## Repository Layout

The Git repo root holds docs only (`README.md`, `CHANGELOG.md`, `Docs/`, `README/`, `LICENSE`). The actual Qt project lives one level down in `BitmapStudio/` (`BitmapStudio.pro` + sources). All build/edit work happens in that subdirectory.

Branches: `dev-0.3` is the active development branch; `main` is the release branch (PR target).

## Project Overview

Bitmap Studio is a Qt 5.15.2 (QtWidgets, C++17, MSVC 2019 64-bit) desktop tool for managing, editing, and auto-extracting bitmaps ("取模") for MCU OLED/LCD projects. Projects are single-file JSON `.bms` archives (identified by `"format": "bms"`; images stored as base64 PNG; composite members referenced by tree path); images export as C arrays (`.c`/`.h` with a common `bms.h` header) or binary files.

## Build

Run from `BitmapStudio/` (paths to Qt and MSVC are hardcoded in these scripts):

```bash
.vscode/build.bat        # Release build
.vscode/build_run.bat    # Build + windeployqt + run exe
```

Build directory: `BitmapStudio/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release`. Qt kit: `C:\Dev\Kits\Qt\5.15.2\msvc2019_64`, jom from QtCreator tools. There are no automated tests or linters — verify changes by compiling.

## Architecture (MVC + Qt signals/slots)

- `core/` — data & logic layer. `RawData`: JSON project store — loads/saves the whole `.bms` file (nested tree; node kind determined by shape: `children`=folder, `frames`=image group, `png`=image, `items`=composite); in-memory ids/pids are assigned at load (containers-first DFS) so dataMap order equals file order. `BmFile` is any tree node. `ProjectMng`: tree controller bridging data and UI across multiple projects. `ImgEncoderFactory`: strategy pattern with 8 bitmap encode modes (ZH/ZL/HL/LH traversal × LSB/MSB bit order; ZH=逐行, ZL=逐列, HL=行列, LH=列行; enum values 0-7 = ZH_LSB…LH_MSB). `ImgConvertor`: export to C/bin/H with configurable keywords.
- Internal node type enum (`RawData`): `TypeImgFile=0`, `TypeComImgFile=1`, `TypeImgFolder=2`, `TypeImgGrpFolder=3`, `TypeComImgFolder=4`; virtual tree ids: -1=project, -3=image class root, -4=composite class root.
- `gui/` — 13 Qt Designer dialogs plus `FormPixelEditor` (pixel-level editor, QGraphicsView) and `FormComImgEditor` (composite/sprite editor).
- `custom/` — reusable widgets: TreeModel/TreeItem, CustomTabWidget, QGraphicsCanvasItem, TreeItemDelegate, etc.
- Repo files at `BitmapStudio/` root: `main.cpp`, `mainwindow`, `singleapplication` (single-instance enforcement), `global`.

Tree node types (`RawData` enum): `TypeImgFile`/`TypeComImgFile` (leaves), `TypeImgFolder`/`TypeComImgFolder`, `TypeImgGrpFolder` (generates offset addresses), `TypeProject` (root).

## Conventions & Gotchas

- QMake with per-module `.pri` files: new sources/headers/forms must be added to `custom/custom.pri`, `gui/gui.pri`, or `core/core.pri` — files not listed there won't compile.
- MSVC builds force `/utf-8` because source comments and UI strings are in Chinese. Keep source files UTF-8.
- Every project mutation rewrites the whole `.bms` file atomically (`QSaveFile`); display/export order follows file order (containers first). Legacy SQLite projects are not openable in-app — convert with `tools/convert_legacy.py`.
- UI is Chinese-first; user-facing strings go through `BitmapStudio_zh_CN.ts` (lrelease + embedded translations are configured in the `.pro`).
- Auto-connect slot naming convention: `on_<Sender>_<Signal>`.
- Version lives in `BitmapStudio.pro` (`VERSION`, exposed as `APP_VERSION`); update it and `CHANGELOG.md` (Chinese, Keep-a-Changelog style) together for releases.
- Commit messages use conventional prefixes with Chinese descriptions (e.g., `feat: 新建项目记忆上次选择的尺寸`).
- UI icons/images are bundled via Qt resource file `Img.qrc`.
