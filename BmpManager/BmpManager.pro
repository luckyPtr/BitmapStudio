QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    projectdata.cpp \
    projectmng.cpp \
    rawdata.cpp \
    treeitem.cpp \
    treemodel.cpp

HEADERS += \
    mainwindow.h \
    projectdata.h \
    projectmng.h \
    rawdata.h \
    treeitem.h \
    treemodel.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    BmpManager_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = yaya256.ico
# 解决MSVC编译中文乱码问题
msvc{
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

RESOURCES += \
    Img.qrc
