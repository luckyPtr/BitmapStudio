#include "imgconvertor.h"
#include <QPoint>
#include <QRgb>
#include <QDebug>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTime>


ImgConvertor::ImgConvertor(QVector<BmFile> dataMap, RawData::Settings settings)
{
    this->dataList = dataMap;
    this->settings = settings;
    imgEncoder = ImgEncoderFactory::create(settings.mode);
    std::sort(dataList.begin(), dataList.end(), [=](BmFile a, BmFile b){
        return a.fullName.toLower() < b.fullName.toLower();
    });
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

QString ImgConvertor::ImgArrayToString(BmFile bf)
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


    return res;
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



bool ImgConvertor::generateImgC(const QString &outputPath)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);

    out << "#include \"bm_img.h\"\n\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgFile)
        {
            if(getParentType(i) != RawData::TypeImgGrpFolder)
            {
                out << ImgFileToString(i);
                QCoreApplication::processEvents();
            }
        }
    }

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgGrpFolder)
        {
            out << ImgArrayToString(i);
            QCoreApplication::processEvents();
        }
    }

    out.flush();
    file.close();

    return out.status() == QTextStream::Ok;
}

bool ImgConvertor::generateImgH(const QString &outputPath)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);

    out << "#ifndef __INC_BITMAPSTUDIO_IMG_H__\n";
    out << "#define __INC_BITMAPSTUDIO_IMG_H__\n";
    out << "#include \"bm_typedef.h\"\n\n";

    foreach(auto bf, dataList)
    {
        if(bf.type == RawData::TypeImgGrpFolder)
        {
            int n = 1;
            foreach(auto i, dataList)
            {
                if(i.pid == bf.id)
                {
                    n = i.image.width() * ((i.image.height() + 7) / 8);
                }
            }

            out << QString("extern Img_t %2[][%3];\n").arg(bf.fullName).arg(n);
            QCoreApplication::processEvents();
        }
        else if(bf.type == RawData::TypeImgFile)
        {
            if(getParentType(bf) != RawData::TypeImgGrpFolder)
            {
                out << QString("extern Img_t %1[];\n").arg(bf.fullName);
                QCoreApplication::processEvents();
            }
        }
    }

    out << "#endif\n";

    out.flush();
    file.close();

    return out.status() == QTextStream::Ok;
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

    QFile file(outputPath + "bm_img.bin");
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QFile headerFile(outputPath + "bm_img_bin.h");
    if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream outHeaderFile(&headerFile);

    // 1. 0-11 "BitmapStudio"头
    QByteArray magic = "BitmapStudio";
    file.write(magic);

    // 2. 12-15 版本号
    file.seek(12);
    quint32 version = versionToU32(APP_VERSION);
    file.write(writeToByteArray(version));

    // 3. 16-23 校验码
    quint16 sum16 = 0x55AA;
    quint16 crc16 = 0x1122;
    quint32 crc32 = 0x33445566;
    file.write(writeToByteArray(sum16));
    file.write(writeToByteArray(crc16));
    file.write(writeToByteArray(crc32));

    // 4. 24-63 brief
    QByteArray brief = "Demo 1234";
    file.write(brief);

    file.seek(64);
    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgFile)
        {
            if(getParentType(i) != RawData::TypeImgGrpFolder)
            {
                outHeaderFile << QString("#define %1    0x%2\n").arg(i.fullName).arg(file.pos(), 8, 16, QChar('0'));
                file.write(imgEncoder->encode(i.image));
                QCoreApplication::processEvents();
            }
        }
    }

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgGrpFolder)
        {
            outHeaderFile << QString("#define %1    0x%2\n").arg(i.fullName).arg(file.pos(), 8, 16, QChar('0'));
            quint32 startPos = file.pos();
            bool getOffsetFlag = false;
            foreach (auto j, dataList)
            {
                if (j.pid == i.id)
                {
                    file.write(imgEncoder->encode(j.image));
                    if (!getOffsetFlag)   // 取第一个图片尺寸作为大小
                    {
                        getOffsetFlag = true;
                        outHeaderFile << QString("#define %1_OFFSET    0x%2\n").arg(i.fullName).arg(file.pos() - startPos, 8, 16, QChar('0'));
                    }
                }
            }
            QCoreApplication::processEvents();
        }
    }

    file.close();

    outHeaderFile.flush();
    headerFile.close();

    return true;
}

bool ImgConvertor::generateComImgC(const QString &outputPath)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);

    out << "#include \"bm_img.h\"\n\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            out << ComImgFileToString(i);
            QCoreApplication::processEvents();
        }
    }

    out.flush();
    file.close();

    return out.status() == QTextStream::Ok;
}

bool ImgConvertor::generateComImgH(const QString &outputPath)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "#ifndef __INC_BITMAPSTUDIO_COM_IMG_H__\n#define __INC_BITMAPSTUDIO_COM_IMG_H__\n";
    out << "#include \"bm_typedef.h\"\n\n";

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            out << QString("extern ComImg_t %1[];\n").arg(i.fullName);
        }
    }
    out << "#endif\n";

    out.flush();
    file.close();

    return out.status() == QTextStream::Ok;
}


bool ImgConvertor::generateTypedefH(const QString &outputPath)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    const QString content =
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
        "#define END_OF_IMG ((Img_t*)-1)\n"
        "\n"
        "#endif\n";

    const QString formattedContent = content
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




