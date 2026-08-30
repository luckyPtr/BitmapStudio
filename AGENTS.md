# AGENTS.md

Guidance for ZCode agents working in this repository.

## Repository Layout

The Git repo root holds the Qt project directly (`BitmapStudio.pro` + sources in `core/`, `gui/`, `custom/`, `cli/`) alongside docs (`README.md`, `CHANGELOG.md`, `Docs/`, `README/`, `LICENSE`). All build/edit work happens at the repo root.

Branches: `dev-0.3` is the active development branch; `main` is the release branch (PR target).

## Project Overview

Bitmap Studio is a Qt 5.15.2 (QtWidgets, C++17, MSVC 2019 64-bit) desktop tool for managing, editing, and auto-extracting bitmaps ("取模") for MCU OLED/LCD projects. Projects are single-file JSON `.bms` archives (identified by `"format": "bms"`; images stored as base64 PNG; composite members referenced by tree path); images export as C arrays (`.c`/`.h` with a common `bms.h` header) or binary files.

## Build

Run from the repo root (Qt and MSVC paths are hardcoded in these scripts):

```bash
.vscode/build.bat        # Release build (GUI + bms-cli.exe CLI)
.vscode/build_run.bat    # Build + windeployqt + run exe
```

Build directories (out-of-source, as siblings of the repo dir): `<repo-parent>/BitmapStudio-Desktop_Qt_5_15_2_MSVC2019_64bit-Release` (GUI) and `<repo-parent>/BitmapStudio-cli-Release` (CLI). Keep build dirs outside the source tree — qmake miscomputes relative dependency paths for in-source build dirs (paths climb past the drive root, which jom cannot resolve), and it does not normalize `\..\` segments in the `.pro` path. Qt kit: `C:\Dev\Kits\Qt\5.15.2\msvc2019_64`, jom from QtCreator tools. There are no automated tests or linters — verify changes by compiling.

## CLI companion (`bms-cli.exe`, console app)

`cli/cli.pro` builds a widget-free console target sharing `core/core.pri` (projectmng lives in `gui/gui.pri` because it is a QWidget — do not add QWidget-dependent code to core/). Output goes to the GUI's shared release dir. Commands: `init / check [--json] / info --json / render [-o][-s][--ascii][--invert] / compose [-o][-s][--ascii] (items: image + line/fillrect/invertrect/points draw primitives) / export [-o][--json sha256 manifest] / rename / move / delete / add / item-add / item-rm`. Exit codes: 0 ok, 1 failure (or check findings), 2 invalid project file. `init` defaults are fixed (ZH scan, MSB bit order, 128x64 screen) — the CLI is a standalone module and never reads the GUI's `config.ini`. Conventions for AI/automation: low-risk edits (reorder, notes, settings) go directly into the JSON followed by `bms-cli check`; path-changing operations (rename/move/delete/add) use the CLI so composite references cascade automatically. Note Git Bash rewrites leading-`/` arguments (set `MSYS_NO_PATHCONV=1`); paths also work without the leading slash. A **user-facing** skill ships at `tools/skill/bms/` — it is a distribution asset (users copy it into their own firmware projects to guide AI agents), not a skill for developing this repo; keep it in sync whenever the CLI surface or format rules change.

## Architecture (MVC + Qt signals/slots)

- `core/` — data & logic layer. `RawData`: JSON project store — loads/saves the whole `.bms` file (nested tree; node kind determined by shape: `children`=folder, `frames`=image group, `png`=image, `items`=composite); in-memory ids/pids are assigned at load (containers-first DFS) so dataMap order equals file order. `BmFile` is any tree node. `ProjectMng`: tree controller bridging data and UI across multiple projects. `ImgEncoderFactory`: strategy pattern with 8 bitmap encode modes (ZH/ZL/HL/LH traversal × LSB/MSB bit order; ZH=逐行, ZL=逐列, HL=行列, LH=列行; enum values 0-7 = ZH_LSB…LH_MSB). `ImgConvertor`: export to C/bin/H with configurable keywords.
- Internal node type enum (`RawData`): `TypeImgFile=0`, `TypeComImgFile=1`, `TypeImgFolder=2`, `TypeImgGrpFolder=3`, `TypeComImgFolder=4`; virtual tree ids: -1=project, -3=image class root, -4=composite class root.
- `gui/` — 13 Qt Designer dialogs plus `FormPixelEditor` (pixel-level editor, QGraphicsView) and `FormComImgEditor` (composite/sprite editor).
- `custom/` — reusable widgets: TreeModel/TreeItem, CustomTabWidget, QGraphicsCanvasItem, TreeItemDelegate, etc.
- Project root files: `main.cpp`, `mainwindow`, `singleapplication` (single-instance enforcement), `global`.

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
