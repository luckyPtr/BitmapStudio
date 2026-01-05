# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Bitmap Studio is a Qt-based desktop application for bitmap image editing and conversion, designed for embedded systems development. It converts images to C arrays or binary files for microcontroller projects. Built with C++17 and Qt 5.15.2 (QtWidgets).

## Build and Run

**Build (Release):**
```bash
.vscode/build.bat
```

**Build & Run:**
```bash
.vscode/build_run.bat
```

**Manual build (requires MSVC environment):**
```bash
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release
C:\Dev\Kits\Qt\5.15.2\msvc2019_64\bin\qmake.exe ..\..\BitmapStudio.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
C:\Dev\Kits\Qt\Tools\QtCreator\bin\jom\jom.exe -j 8
```

No automated tests or linting are configured.

## Architecture

The application follows an **MVC pattern** with Qt's signal-slot mechanism:

### Core Layer (`core/`)
- **RawData**: SQLite database manager - each project is a separate `.db` file with tables `tbl_settings` and `tbl_img`
- **ProjectMng**: Tree controller that bridges the data layer with UI, manages multiple projects
- **ImgEncoderFactory**: Strategy pattern with 8 bitmap encoding modes (LSB/MSB × ZH/ZL/HL/LH - combinations of column/row traversal and bit order)
- **ImgConvertor**: Exports images to C arrays, binary files, or H headers with configurable keywords

### GUI Layer (`gui/`)
- **FormPixelEditor**: Pixel-level bitmap editor using QGraphicsView/QGraphicsScene
- **FormComImgEditor**: Composite image (sprite/tile) editor
- 13 Qt Designer dialogs (DialogNewProject, DialogProjectSettings, DialogImportImg, etc.)

### Custom Widgets (`custom/`)
- **TreeModel/TreeItem**: Custom tree model for project structure display
- **CustomTabWidget/CustomTab**: Tab management for multiple editors
- **QGraphicsCanvasItem**: Pixel editing canvas with coordinate transforms
- **QGraphicsComImgCanvasItem**: Composite image canvas
- **TreeItemDelegate**: Custom rendering for tree items

### Key Data Types

**BmFile** (core/rawdata.h): Represents any node in the project tree with fields for id, pid, type, name, image, comImg, offset.

**ComImg** (core/rawdata.h): Composite image containing size and vector of ComImgItem elements (x, y, z, id).

**Type enums** (RawData class):
- `TypeImgFile`, `TypeComImgFile`: Leaf image files
- `TypeImgFolder`, `TypeComImgFolder`: Regular folders
- `TypeImgGrpFolder`: Image group folder (generates offset addresses)
- `TypeProject`: Root project node

## Encoding Modes

The 8 encoding modes (defined in ImgEncoderFactory) control how pixels are mapped to bits:

| Mode | Description |
|------|-------------|
| ZH_* | Row-wise (逐行) |
| ZL_* | Column-wise (逐列) |
| HL_* | Row-column (行列) |
| LH_* | Column-row (列行) |
| *_LSB | Reverse bit order |
| *_MSB | Forward bit order |

## QMake Structure

The project uses `.pri` include files for modularization:
- `BitmapStudio.pro` includes `custom/custom.pri`, `gui/gui.pri`, `core/core.pri`
- Each `.pri` lists SOURCES, HEADERS, and FORMS for its directory

## Important Conventions

- Single-instance application enforced via `SingleApplication` class
- UTF-8 encoding forced for MSVC builds (`/utf-8` flag)
- Chinese translation support via `BitmapStudio_zh_CN.ts`
- Qt Resource System (`Img.qrc`) bundles all UI icons/images
- Signals follow `on_Sender_Signal` naming convention for auto-connect
- Version defined as `APP_VERSION` macro, current: 0.3.0
