#include "imgconvertor.h"
#include <QPoint>
#include <QRgb>
#include <QDebug>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTime>
#include "checksumutility.h"
#include "qchar.h"
#include "qglobal.h"


ImgConvertor::ImgConvertor(QVector<BmFile> dataMap, RawData::Settings settings)
{
    this->dataList = dataMap;
    this->settings = settings;
    imgEncoder = ImgEncoderFactory::create(settings.mode);
    std::sort(dataList.begin(), dataList.end(), [=](BmFile a, BmFile b){
        return a.fullName.toLower() < b.fullName.toLower();
    });

    foreach (auto i, dataList)
    {
        this->dataMap.insert(i.id, i);
    }
}

ImgConvertor::~ImgConvertor()
{
    delete imgEncoder;
}



QString ImgConvertor::ImgFileToString(BmFile bf)
{
    QString fullName = bf.fullName;
    QString res;
    res.append("// " + fullName + "\n");

    auto imgByteArrayToString = [](QByteArray ba){
        QString str;
        for(int i = 0; i < ba.size(); i++)
        {
            if(i % 32 == 0)
            {
                str.append("\n\t");
            }
            str.append(QString::asprintf("0x%02X, ", (quint8)ba[i]));
        }
        return str;
    };

    if(bf.type == RawData::TypeImgFile)
    {
        res.append(QString::asprintf("// %dx%d\n", bf.image.width(), bf.image.height()));
        res.append(QString("BMS_CONST BmsImage_t %1[] = \n{%2\n};\n").arg(fullName).arg(imgByteArrayToString(imgEncoder->encode(bf.image))));
    }

    return res;
}

QPair<QString, int> ImgConvertor::ImgArrayToString(BmFile bf)
{
    QString fullName = bf.fullName;
    QString res;
    QString imgArray;
    int count;
    QSize imgSize;


    auto imgByteArrayToString = [](QByteArray ba){
        QString str;
        for(int i = 0; i < ba.size(); i++)
        {
            if(i % 32 == 0)
            {
                str.append("\n\t\t");
            }
            str.append(QString::asprintf("0x%02X, ", (quint8)ba[i]));
        }
        return str;
    };

    foreach(auto i, dataList)
    {
        if(i.pid == bf.id)
        {
            QByteArray ba = imgEncoder->encode(i.image);
            imgArray.append(QString("\t{// %1%2\n\t},\n").arg(i.name).arg(imgByteArrayToString(ba)));
            count = ba.size();
            imgSize = i.image.size();
        }
    }

    res.append("// " + fullName + "\n");
    res.append(QString::asprintf("// %dx%d\n", imgSize.width(), imgSize.height()));
    res.append(QString("BMS_CONST BmsImage_t %1[][%2] = \n{\n%3};\n").arg(fullName).arg(count).arg(imgArray));


    return QPair<QString, int>(res, count);
}

QString ImgConvertor::ComImgFileToString(BmFile bf)
{
    QString res;
    QString fullName = bf.fullName;
    QString array;

    auto getBmFile = [=](int id){
        foreach(auto i, dataList)
        {
            if(i.id == id)
            {
                return i;
            }
        }
        return BmFile();
    };

    // bin格式下，数组元素中的image字段不需要BMS_CONST修饰（因为image是地址值类型）
    // C格式下，需要BMS_CONST修饰（因为image是指针类型）
    QString imagePrefix = (settings.format == "bin") ? "" : "BMS_CONST ";

    foreach(auto i, bf.comImg.items)
    {
        array.append(QString("\t{%1%2, %3, %4, %5, %6},\n")
                     .arg(imagePrefix)
                     .arg(getBmFile(i.id).fullName)
                     .arg(i.x)
                     .arg(i.y)
                     .arg(getBmFile(i.id).image.width())
                     .arg(getBmFile(i.id).image.height()));
    }
    array.append("\t{BMS_END_OF_IMG}\n");

    res.append("// " + fullName + "\n");
    res.append(QString::asprintf("// %dx%d\n", bf.comImg.size.width(), bf.comImg.size.height()));
    res.append(QString("BMS_CONST BmsSprite_t %1[] = \n{\n%2};\n").arg(fullName).arg(array));

    return res;
}

int ImgConvertor::getParentType(BmFile bf)
{
    foreach(auto data, dataList)
    {
        if(bf.pid == data.id)
        {
            return data.type;
        }
    }
    return bf.type;
}

quint32 ImgConvertor::getOffset(BmFile bf)
{
    QSet<int> visited;

    while (1)
    {
        // 防止无限循环
        if (visited.contains(bf.id)) {
            // 检测到循环引用，返回错误值或抛出异常
            return 0; // 或者其他合适的默认值
        }
        visited.insert(bf.id);

        if (bf.pid == 0)
        {
            return bf.offset;
        }

        // 检查父节点是否存在
        if (!dataMap.contains(bf.pid)) {
            // 父节点不存在，返回错误值
            return 0; // 或者其他合适的默认值
        }

        bf = dataMap[bf.pid];
    }
}



bool ImgConvertor::generateImgC(const QString &outputPath)
{
    QFile fileC(outputPath + "/bms_image.c");
    if (!fileC.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outC(&fileC);

    QFile fileH(outputPath + "/bms_image.h");
    if (!fileH.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outH(&fileH);

    outC << "#include \"bms_image.h\"\n\n";

    outH << "#ifndef __INC_BMS_IMAGE_H__\n";
    outH << "#define __INC_BMS_IMAGE_H__\n";
    outH << "#include \"bms_typedef.h\"\n\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgFile)
        {
            if(getParentType(i) != RawData::TypeImgGrpFolder)
            {
                outC << ImgFileToString(i);
                outH << QString("extern BMS_CONST BmsImage_t %1[];\n").arg(i.fullName);
                QCoreApplication::processEvents();
            }
        }
    }

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgGrpFolder)
        {
            auto res = ImgArrayToString(i);
            outC << res.first;
            outH << QString("extern BMS_CONST BmsImage_t %2[][%3];\n").arg(i.fullName).arg(res.second);
            QCoreApplication::processEvents();
        }
    }

    outH << "\n#endif\n";

    outC.flush();
    fileC.close();
    outH.flush();
    fileH.close();

    return outC.status() == QTextStream::Ok && outH.status() == QTextStream::Ok;
}



bool ImgConvertor::generateImgBin(const QString &outputPath)
{
    QMap<quint32, quint32> offsetMap;

    foreach (auto i, dataList)
    {
        quint32 offset = getOffset(i);
        if (!offsetMap.contains(offset))
        {
            offsetMap[offset] = offset + 64;
        }
    }

    QFile file(outputPath + "/bms.bin");
    if (!file.open(QIODevice::ReadWrite)) {
        return false;
    }
    file.resize(0);     // 删除原本内容

    QFile headerFile(outputPath + "/bms_image.h");
    if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outHeaderFile(&headerFile);

    outHeaderFile << "#ifndef __INC_BMS_IMAGE_H__\n";
    outHeaderFile << "#define __INC_BMS_IMAGE_H__\n";
    outHeaderFile << "#include \"bms_typedef.h\"\n\n";


    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgFile)
        {
            if(getParentType(i) != RawData::TypeImgGrpFolder)
            {
                quint32 offset = getOffset(i);
                quint32 addr = offsetMap[offset];
                outHeaderFile << QString("#define %1    0x%2\n").arg(i.fullName).arg(addr, 8, 16, QChar('0'));
                file.seek(addr);
                file.write(imgEncoder->encode(i.image));
                offsetMap[offset] = file.pos();
                QCoreApplication::processEvents();
            }
        }
    }

    outHeaderFile << "\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgGrpFolder)
        {
            quint32 offset = getOffset(i);
            quint32 addr = offsetMap[offset];
            file.seek(addr);
            outHeaderFile << QString("#define %1    0x%2\n").arg(i.fullName).arg(file.pos(), 8, 16, QChar('0'));
            bool gotFrameSize = false;
            foreach (auto j, dataList)
            {
                if (j.pid == i.id)
                {
                    QByteArray data = imgEncoder->encode(j.image);
                    file.write(data);
                    if (!gotFrameSize) {
                        gotFrameSize = true;
                        outHeaderFile << QString("#define %1_OFFSET    0x%2\n").arg(i.fullName).arg(data.size(), 8, 16, QChar('0'));
                    }
                }
            }
            offsetMap[offset] = file.pos();
            QCoreApplication::processEvents();
        }
    }

    outHeaderFile << "\n#endif\n";

    file.flush();
    file.close();

    outHeaderFile.flush();
    headerFile.close();

    return file.error() == QFile::NoError && headerFile.error() == QFile::NoError;
}

bool ImgConvertor::generateSprite(const QString &outputPath)
{
    QFile fileC(outputPath + "/bms_sprite.c");
    if (!fileC.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outC(&fileC);

    QFile fileH(outputPath + "/bms_sprite.h");
    if (!fileH.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outH(&fileH);

    outC << "#include \"bms_image.h\"\n\n";
    outH << "#ifndef __INC_BMS_SPRITE_H__\n#define __INC_BMS_SPRITE_H__\n";
    outH << "#include \"bms_typedef.h\"\n\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            outC << ComImgFileToString(i);
            outH << QString("extern BMS_CONST BmsSprite_t %1[];\n").arg(i.fullName);
            QCoreApplication::processEvents();
        }
    }

    outH << "\n#endif\n";

    outC.flush();
    fileC.close();

    outH.flush();
    fileH.close();

    return outC.status() == QTextStream::Ok && outH.status() == QTextStream::Ok;
}



bool ImgConvertor::generateTypedef(const QString &outputPath)
{
    QFile file(outputPath + "/bms_typedef.h");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QString contentC =
        "#ifndef __INC_BMS_TYPEDEF_H__\n"
        "#define __INC_BMS_TYPEDEF_H__\n"
        "\n"
        "#define BMS_CONST      %1\n"
        "#define BMS_END_OF_IMG (0)\n"
        "\n"
        "typedef unsigned char BmsImage_t;\n"
        "\n"
        "typedef struct\n"
        "{\n"
        "    BmsImage_t *image;\n"
        "    %2 x;\n"
        "    %2 y;\n"
        "    %3 width;\n"
        "    %3 height;\n"
        "} BmsSprite_t;\n"
        "\n"
        "\n"
        "#endif\n";

    const QString contentBin =
        "#ifndef __INC_BMS_TYPEDEF_H__\n"
        "#define __INC_BMS_TYPEDEF_H__\n"
        "\n"
        "#define BMS_CONST      %1\n"
        "#define BMS_END_OF_IMG (0)\n"
        "\n"
        "typedef %2 BmsImage_t;\n"
        "\n"
        "typedef struct\n"
        "{\n"
        "    BmsImage_t image;\n"
        "    %3 x;\n"
        "    %3 y;\n"
        "    %4 width;\n"
        "    %4 height;\n"
        "} BmsSprite_t;\n"
        "\n"
        "\n"
        "#endif\n";

    const QString formattedContent = settings.format == "bin"
                                    ? contentBin
                                        .arg(settings.keywordConst)
                                        .arg(settings.keywordImgAddr)
                                        .arg(settings.keywordImgPos)
                                        .arg(settings.keywordImgSize)
                                    : contentC
                                        .arg(settings.keywordConst)
                                        .arg(settings.keywordImgPos)
                                        .arg(settings.keywordImgSize);

    QTextStream out(&file);
    out << formattedContent;
    out.flush();
    file.close();

    // 检查是否写入成功
    return out.status() == QTextStream::Ok;
}

bool ImgConvertor::generateMainHeader(const QString &outputPath)
{
    // 如果是 bin 格式，需要生成 bin 文件头和校验宏定义
    if (settings.format == "bin")
    {
        auto writeToByteArray = [](auto value) -> QByteArray {
            QByteArray byteArray;
            QDataStream stream(&byteArray, QIODevice::WriteOnly);
            stream.setByteOrder(QDataStream::BigEndian);
            stream << value;
            return byteArray;
        };

        auto versionToU32 = [](const QString &version) -> quint32 {
            QRegularExpression regex(R"(^(\d+)\.(\d+)\.(\d+)$)");
            QRegularExpressionMatch match = regex.match(version);
            if (!match.hasMatch()) {
                return 0;
            }
            quint8 major = match.captured(1).toUShort();
            quint8 minor = match.captured(2).toUShort();
            quint8 patch = match.captured(3).toUShort();
            return (major << 16) | (minor << 8) | patch;
        };

        // 打开 bin 文件
        QFile fileBin(outputPath + "/bms.bin");
        if (!fileBin.open(QIODevice::ReadWrite)) {
            return false;
        }

        // 计算校验值
        fileBin.seek(64);
        QByteArray data = fileBin.readAll();
        quint32 sum32 = ChecksumUtility::SUM32(data);
        quint32 crc32 = ChecksumUtility::CRC32(data);
        quint16 crc16 = ChecksumUtility::CRC16(data);
        quint8 crc8 = ChecksumUtility::CRC8(data);
        quint8 xor8 = ChecksumUtility::XOR8(data);

        // 数据长度(不包括64字节数据头)
        quint32 dataLength = fileBin.size() - 64;

        // 写入 bin 文件头

        // 1. 0-11 "BitmapStudio" 头
        fileBin.seek(0);
        QByteArray magic = "BitmapStudio";
        fileBin.write(magic);

        // 2. 12-15 版本号
        fileBin.seek(12);
        quint32 version = versionToU32(APP_VERSION);
        fileBin.write(writeToByteArray(version));

        // 3. 16-19 数据长度(不包括64字节数据头)
        fileBin.seek(16);
        fileBin.write(writeToByteArray(dataLength));

        // 4. 20-31 校验码
        fileBin.seek(20);
        fileBin.write(writeToByteArray(sum32));
        fileBin.write(writeToByteArray(crc32));
        fileBin.write(writeToByteArray(crc16));
        fileBin.write(writeToByteArray(crc8));
        fileBin.write(writeToByteArray(xor8));

        // 5. 32-63 brief
        fileBin.seek(32);
        QByteArray brief = settings.brief.toUtf8().left(32);
        fileBin.write(brief);

        fileBin.flush();
        fileBin.close();

        // 生成 bms.h 统一头文件，包含校验宏定义
        QFile fileH(outputPath + "/bms.h");
        if (!fileH.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream outH(&fileH);
        outH << "#ifndef __INC_BMS_H__\n";
        outH << "#define __INC_BMS_H__\n";
        outH << "\n";
        outH << "#include \"bms_typedef.h\"\n";
        outH << "#include \"bms_image.h\"\n";
        outH << "#include \"bms_sprite.h\"\n";
        outH << "\n";

        // 写入校验宏定义到 bms.h
        outH << QString("#define BMS_SIZE_ADDR    0x%1\n").arg(16, 8, 16, QChar('0'));
        outH << QString("#define BMS_SUM32_ADDR   0x%1\n").arg(20, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC32_ADDR   0x%1\n").arg(24, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC16_ADDR   0x%1\n").arg(28, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC8_ADDR    0x%1\n").arg(30, 8, 16, QChar('0'));
        outH << QString("#define BMS_XOR8_ADDR    0x%1\n").arg(31, 8, 16, QChar('0'));
        outH << "\n";

        outH << QString("#define BMS_SIZE         0x%1\n").arg(dataLength, 8, 16, QChar('0'));
        outH << QString("#define BMS_SUM32        0x%1\n").arg(sum32, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC32        0x%1\n").arg(crc32, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC16        0x%1\n").arg(crc16, 4, 16, QChar('0'));
        outH << QString("#define BMS_CRC8         0x%1\n").arg(crc8, 2, 16, QChar('0'));
        outH << QString("#define BMS_XOR8         0x%1\n").arg(xor8, 2, 16, QChar('0'));
        outH << QString("#define BMS_BRIEF        \"%1\"\n").arg(settings.brief);
        outH << "\n";

        outH << "#endif\n";

        outH.flush();
        fileH.close();

        return outH.status() == QTextStream::Ok;
    }
    else
    {
        // C 格式：生成简单的 bms.h
        QFile fileH(outputPath + "/bms.h");
        if (!fileH.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream outH(&fileH);
        outH << "#ifndef __INC_BMS_H__\n";
        outH << "#define __INC_BMS_H__\n";
        outH << "\n";
        outH << "#include \"bms_typedef.h\"\n";
        outH << "#include \"bms_image.h\"\n";
        outH << "#include \"bms_sprite.h\"\n";
        outH << "\n";

        // 写入 bin 文件头格式地址宏定义（供用户参考 bin 文件格式）
        outH << "// Bin file header format (for reference)\n";
        outH << QString("#define BMS_SIZE_ADDR    0x%1\n").arg(16, 8, 16, QChar('0'));
        outH << QString("#define BMS_SUM32_ADDR   0x%1\n").arg(20, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC32_ADDR   0x%1\n").arg(24, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC16_ADDR   0x%1\n").arg(28, 8, 16, QChar('0'));
        outH << QString("#define BMS_CRC8_ADDR    0x%1\n").arg(30, 8, 16, QChar('0'));
        outH << QString("#define BMS_XOR8_ADDR    0x%1\n").arg(31, 8, 16, QChar('0'));
        outH << "\n";

        outH << "#endif\n";

        outH.flush();
        fileH.close();

        return outH.status() == QTextStream::Ok;
    }
}
