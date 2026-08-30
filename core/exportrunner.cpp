#include "exportrunner.h"
#include "imgconvertor.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

namespace ExportRunner {

bool run(const RawData &rd, const QString &outdirOverride, QStringList *generated)
{
    RawData::Settings settings = rd.getSettings();

    // 输出目录：工程文件所在目录 + settings.path（或显式覆盖）
    QFileInfo fileInfo(rd.getProject());
    QString path = outdirOverride.isEmpty()
            ? fileInfo.path() + "/" + settings.path
            : outdirOverride;
    if (!QDir(path).exists())
    {
        QDir().mkpath(path);
    }

    ImgConvertor ic(rd.getDataMap().values().toVector(), settings);

    bool ok = true;
    if (settings.format == "bin")
    {
        ok &= ic.generateImgBin(path);
        if (generated) *generated << QFileInfo(QDir(path), "bms.bin").absoluteFilePath()
                                  << QFileInfo(QDir(path), "bms_image.h").absoluteFilePath();
    }
    else
    {
        ok &= ic.generateImgC(path);
        if (generated) *generated << QFileInfo(QDir(path), "bms_image.c").absoluteFilePath()
                                  << QFileInfo(QDir(path), "bms_image.h").absoluteFilePath();
    }
    ok &= ic.generateSprite(path);
    if (generated) *generated << QFileInfo(QDir(path), "bms_sprite.c").absoluteFilePath()
                              << QFileInfo(QDir(path), "bms_sprite.h").absoluteFilePath();
    if (!settings.customTypedef)
    {
        ok &= ic.generateTypedef(path);
        if (generated) *generated << QFileInfo(QDir(path), "bms_typedef.h").absoluteFilePath();
    }
    ok &= ic.generateMainHeader(path);
    if (generated) *generated << QFileInfo(QDir(path), "bms.h").absoluteFilePath();

    if (!ok)
    {
        qWarning() << "导出未全部成功:" << path;
    }
    return ok;
}

} // namespace ExportRunner
