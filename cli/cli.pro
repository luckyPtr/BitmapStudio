QT       += core gui
CONFIG   += c++17 console
TEMPLATE  = app
TARGET    = bms-cli

# 与GUI共享release目录，复用windeployqt已部署的Qt DLL；GUI影子构建目录位于源码树外
DESTDIR   = $$OUT_PWD/../BitmapStudio-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/release

# 仅生成Release配置：debug_and_release_target会把上面DESTDIR中的"Release"替换成"debug"，
# 导致构建时在源码树外创建空的BitmapStudio-...-debug\debug目录残留
CONFIG += release
CONFIG -= debug_and_release debug_and_release_target

# 解决MSVC编译中文乱码问题
msvc{
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

VERSION = 0.3.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

INCLUDEPATH += $$PWD/.. $$PWD/../core

# RawData引用了根目录的Global（导出预览颜色等静态配置）
SOURCES += $$PWD/../global.cpp

include(../core/core.pri)

SOURCES += bmscli_main.cpp commands.cpp
HEADERS += commands.h
