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
        res.append(QString("Img_t %1[] = \n{%2\n};\n").arg(fullName).arg(imgByteArrayToString(imgEncoder->encode(bf.image))));
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
    res.append(QString("Img_t %1[][%2] = \n{\n%3};\n").arg(fullName).arg(count).arg(imgArray));


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

    foreach(auto i, bf.comImg.items)
    {
        array.append(QString("\t{%1, %2, %3, %4, %5},\n").arg(getBmFile(i.id).fullName).arg(i.x).arg(i.y).arg(getBmFile(i.id).image.width()).arg(getBmFile(i.id).image.height()));
    }
    array.append("\t{END_OF_IMG}\n");

    res.append("// " + fullName + "\n");
    res.append(QString::asprintf("// %dx%d\n", bf.comImg.size.width(), bf.comImg.size.height()));
    res.append(QString("ComImg_t %1[] = \n{\n%2};\n").arg(fullName).arg(array));

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
    QFile fileC(outputPath + "/bm_img.c");
    if (!fileC.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outC(&fileC);

    QFile fileH(outputPath + "/bm_img.h");
    if (!fileH.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outH(&fileH);

    outC << "#include \"bm_img.h\"\n\n";

    outH << "#ifndef __INC_BITMAPSTUDIO_IMG_H__\n";
    outH << "#define __INC_BITMAPSTUDIO_IMG_H__\n";
    outH << "#include \"bm_typedef.h\"\n\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgFile)
        {
            if(getParentType(i) != RawData::TypeImgGrpFolder)
            {
                outC << ImgFileToString(i);
                outH << QString("extern Img_t %1[];\n").arg(i.fullName);
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
            outH << QString("extern Img_t %2[][%3];\n").arg(i.fullName).arg(res.second);
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

    QMap<quint32, quint32> offsetMap;

    foreach (auto i, dataList)
    {
        quint32 offset = getOffset(i);
        if (!offsetMap.contains(offset))
        {
            offsetMap[offset] = offset + 64;
        }
    }

    QFile file(outputPath + "bm_img.bin");
    if (!file.open(QIODevice::ReadWrite)) {
        return false;
    }
    file.resize(0);     // 删除原本内容

    QFile headerFile(outputPath + "bm_img.h");
    if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outHeaderFile(&headerFile);

    outHeaderFile << "#ifndef __INC_BITMAPSTUDIO_IMG_H__\n";
    outHeaderFile << "#define __INC_BITMAPSTUDIO_IMG_H__\n";
    outHeaderFile << "#include \"bm_typedef.h\"\n\n";


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

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgGrpFolder)
        {
            quint32 offset = getOffset(i);
            quint32 addr = offsetMap[offset];
            file.seek(addr);
            outHeaderFile << QString("#define %1    0x%2\n").arg(i.fullName).arg(file.pos(), 8, 16, QChar('0'));
            foreach (auto j, dataList)
            {
                if (j.pid == i.id)
                {
                    file.write(imgEncoder->encode(j.image));
                }
            }
            offsetMap[offset] = file.pos();
            QCoreApplication::processEvents();
        }
    }


    // 1. 0-11 "BitmapStudio"头
    file.seek(0);
    QByteArray magic = "BitmapStudio";
    file.write(magic);

    // 2. 12-15 版本号
    file.seek(12);
    quint32 version = versionToU32(APP_VERSION);
    file.write(writeToByteArray(version));

    // 3. 16-19 数据长度(不包括64字节数据头)
    quint32 dataLength = file.size() - 64;
    file.seek(16);
    file.write(writeToByteArray(dataLength));

    // 4. 20-31 校验码
    file.flush();
    file.seek(64);
    QByteArray data = file.readAll();
    quint32 sum32 = ChecksumUtility::SUM32(data);
    quint32 crc32 = ChecksumUtility::CRC32(data);
    quint16 crc16 = ChecksumUtility::CRC16(data);
    quint8 crc8 = ChecksumUtility::CRC8(data);
    quint8 xor8 = ChecksumUtility::XOR8(data);


    outHeaderFile << "\n";
    outHeaderFile << QString("#define BS_SIZE_ADDR    0x%1\n").arg(16, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_SUM32_ADDR   0x%1\n").arg(20, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_CRC32_ADDR   0x%1\n").arg(24, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_CRC16_ADDR   0x%1\n").arg(28, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_CRC8_ADDR    0x%1\n").arg(30, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_XOR8_ADDR    0x%1\n").arg(31, 8, 16, QChar('0'));

    outHeaderFile << QString("#define BS_SIZE         0x%1\n").arg(dataLength, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_SUM32        0x%1\n").arg(sum32, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_CRC32        0x%1\n").arg(crc32, 8, 16, QChar('0'));
    outHeaderFile << QString("#define BS_CRC16        0x%1\n").arg(crc16, 4, 16, QChar('0'));
    outHeaderFile << QString("#define BS_CRC8         0x%1\n").arg(crc8, 2, 16, QChar('0'));
    outHeaderFile << QString("#define BS_XOR8         0x%1\n").arg(xor8, 2, 16, QChar('0'));


    file.seek(20);
    file.write(writeToByteArray(sum32));
    file.write(writeToByteArray(crc32));
    file.write(writeToByteArray(crc16));
    file.write(writeToByteArray(crc8));
    file.write(writeToByteArray(xor8));


    // 4. 24-63 brief
    outHeaderFile << QString("#define BS_BRIEF	    \"%1\"\n").arg(settings.brief);

    file.seek(32);
    QByteArray brief = settings.brief.toUtf8().left(32);
    file.write(brief);

    outHeaderFile << "\n#endif\n";

    file.flush();
    file.close();

    outHeaderFile.flush();
    headerFile.close();

    return true;
}

bool ImgConvertor::generateComImg(const QString &outputPath)
{
    QFile fileC(outputPath + "/bm_com_img.c");
    if (!fileC.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outC(&fileC);

    QFile fileH(outputPath + "/bm_com_img.h");
    if (!fileH.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outH(&fileH);

    outC << "#include \"bm_img.h\"\n\n";
    outH << "#ifndef __INC_BITMAPSTUDIO_COM_IMG_H__\n#define __INC_BITMAPSTUDIO_COM_IMG_H__\n";
    outH << "#include \"bm_typedef.h\"\n\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            outC << ComImgFileToString(i);
            outH << QString("extern ComImg_t %1[];\n").arg(i.fullName);
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
    QFile file(outputPath + "bm_typedef.h");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QString contentC =
        "#ifndef __INC_BITMAPSTUDIO_TYPEDEF_H__\n"
        "#define __INC_BITMAPSTUDIO_TYPEDEF_H__\n"
        "\n"
        "typedef %1 unsigned char Img_t;\n"
        "\n"
        "typedef %1 struct\n"
        "{\n"
        "    Img_t *img;\n"
        "    %2 x;\n"
        "    %2 y;\n"
        "    %3 width;\n"
        "    %3 height;\n"
        "} ComImg_t;\n"
        "\n"
        "#define END_OF_IMG ((Img_t*)0)\n"
        "\n"
        "#endif\n";

    const QString contentBin =
        "#ifndef __INC_BITMAPSTUDIO_TYPEDEF_H__\n"
        "#define __INC_BITMAPSTUDIO_TYPEDEF_H__\n"
        "\n"
        "typedef %1 %2 img_t;\n"
        "\n"
        "typedef %1 struct\n"
        "{\n"
        "    Img_t img;\n"
        "    %3 x;\n"
        "    %3 y;\n"
        "    %4 width;\n"
        "    %4 height;\n"
        "} ComImg_t;\n"
        "\n"
        "#define END_OF_IMG ((Img_t)0)\n"
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
