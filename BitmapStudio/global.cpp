#include "global.h"
#include <QSettings>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

int Global::pixelSize = 8;
int Global::scaleWidth = 16;
int Global::scaleOffset = 4;
int Global::pixelColor_1 = 0x9ce0ef;
int Global::pixelColor_0 = 0x495028;
int Global::gridColor = 0x303030;
int Global::itemBoundColor = 0x808080;
int Global::selectedItemBoundColor = 0x00FFFF;
int Global::selectionBoxColor = 0xFFFF00;
int Global::guidesColor = 0xFFFF00;
int Global::selectedGuidesColor = 0xFF8C00;
int Global::backgroundColorLeftTop = 0xC5CFDF;
int Global::backgroundColorRightBottom = 0xDCE5F2;
bool Global::editMode = false;

int Global::exportImgColor_0 = 0xFFFFFF;
int Global::exportImgColor_1 = 0x000000;

// 新建项目默认尺寸
int Global::defaultProjectWidth = 128;
int Global::defaultProjectHeight = 64;
// 新建项目默认取模方式 (-1 表示未选择，使用空)
int Global::defaultProjectMode = -1;


void Global::initSettings()
{
    QString iniFilePath = QCoreApplication::applicationDirPath() + "/config.ini";
    // 判断文件是否存在
    if (!QFile::exists(iniFilePath)) {
        // 文件不存在，创建默认配置
        createDefaultSettings(iniFilePath);
    } else {
        // 文件存在，读取配置
        readSettings(iniFilePath);
    }
}

void Global::createDefaultSettings(const QString &filePath)
{
    // 创建QSettings对象，指定文件路径和格式
    QSettings settings(filePath, QSettings::IniFormat);

    settings.beginGroup("General");
    settings.setValue("export_image_color_0", QString::number(exportImgColor_0, 16).toUpper());
    settings.setValue("export_image_color_1", QString::number(exportImgColor_1, 16).toUpper());
    settings.setValue("default_project_width", defaultProjectWidth);
    settings.setValue("default_project_height", defaultProjectHeight);
    settings.setValue("default_project_mode", defaultProjectMode);
    settings.endGroup();
}

void Global::readSettings(const QString &filePath)
{
    // 创建QSettings对象，指定文件路径和格式
    QSettings settings(filePath, QSettings::IniFormat);

    settings.beginGroup("General");
    bool ok;
    exportImgColor_0 = settings.value("export_image_color_0", QString::number(exportImgColor_0, 16)).toString().toInt(&ok, 16);
    exportImgColor_1 = settings.value("export_image_color_1", QString::number(exportImgColor_1, 16)).toString().toInt(&ok, 16);
    defaultProjectWidth = settings.value("default_project_width", defaultProjectWidth).toInt();
    defaultProjectHeight = settings.value("default_project_height", defaultProjectHeight).toInt();
    defaultProjectMode = settings.value("default_project_mode", defaultProjectMode).toInt();
    settings.endGroup();
}

void Global::saveDefaultProjectSize(int width, int height)
{
    QString iniFilePath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(iniFilePath, QSettings::IniFormat);

    settings.beginGroup("General");
    settings.setValue("default_project_width", width);
    settings.setValue("default_project_height", height);
    settings.endGroup();

    // 更新静态变量
    defaultProjectWidth = width;
    defaultProjectHeight = height;
}

void Global::saveDefaultProjectMode(int mode)
{
    QString iniFilePath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(iniFilePath, QSettings::IniFormat);

    settings.beginGroup("General");
    settings.setValue("default_project_mode", mode);
    settings.endGroup();

    // 更新静态变量
    defaultProjectMode = mode;
}
