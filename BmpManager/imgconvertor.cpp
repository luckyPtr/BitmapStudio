
#include "imgconvertor.h"
#include <QPoint>
#include <QRgb>
#include <QDebug>



QString ImgConvertor::getFullName(BmFile bf)
{
    QString fullName;
    if(RawData::isClassImgType(bf.type))
    {
        fullName = "IMG";
    }
    else if(RawData::isClassComImgType(bf.type))
    {
        fullName = "CI";
    }

    auto getParent = [this](BmFile data)
    {
        foreach(auto i, dataList)
        {
            if(i.id == data.pid)
            {
                return i;
            }
        }
        return BmFile();
    };

    auto appendParentName = [&](auto&& self, BmFile data)->void {
        if(data.pid != 0)
        {
            self(self, getParent(data));
        }
        fullName.append("_" + data.name);
    };

    appendParentName(appendParentName, bf);

    return fullName;
}

ImgConvertor::ImgConvertor(QVector<BmFile> dataMap)
{
    this->dataList = dataMap;
}

QByteArray ImgConvertor::imgToByteArray(QImage &img)
{
    QByteArray ba;
    ba.resize(img.width() * ((img.height() + 7) / 8));

    // 生成结果中的第i个数据对应的图片位置
    auto getPos = [img](int i){
        QPoint point;
        point.setX(i % img.width());
        point.setY((i / img.height()) * 8);
        return point;
    };

    // 某一点是否在图片内
    auto contain = [img](QPoint point) {
        if(point.x() < img.width() && point.y() < img.height())
        {
            return true;
        }
        return false;
    };

    for(int i = 0; i < ba.size(); i++)
    {
        QPoint point = getPos(i);

        quint8 temp = 0;
        for(int j = 0; j < 8; j++)
        {
            if(contain(point))
            {
                temp |= ((qGray(img.pixel(point)) < 128) << j);
                point.setY(point.y() + 1);
            }
            else
            {
                break;
            }
        }
        ba[i] = temp;
    }

    return ba;
}

QString ImgConvertor::encodeImgFile(BmFile bf)
{
    QString fullName = getFullName(bf);
    QString res;
    res.append("// " + fullName + "\n");

    auto imgByteArrayToString = [](QByteArray ba){
        QString str;
        for(int i = 0; i < ba.size(); i++)
        {
            if(i % 8 == 0)
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
        res.append(QString("const unsigned char %1[] = \n{%2\n};\n").arg(fullName).arg(imgByteArrayToString(imgToByteArray(bf.image))));
    }

    return res;
}

QString ImgConvertor::encodeImgArray(BmFile bf)
{
    QString fullName = getFullName(bf);
    QString res;
    QString imgArray;
    int size;


    auto imgByteArrayToString = [](QByteArray ba){
        QString str;
        for(int i = 0; i < ba.size(); i++)
        {
            if(i % 8 == 0)
            {
                str.append("\n\t");
            }
            str.append(QString::asprintf("0x%02X, ", (quint8)ba[i]));
        }
        return str;
    };

    foreach(auto i, dataList)
    {
        if(i.pid == bf.id)
        {
            QByteArray ba = imgToByteArray(i.image);
            imgArray.append(QString("{//%1\n%2\n},\n").arg(i.name).arg(imgByteArrayToString(ba)));
            size = ba.size();
        }
    }

    res.append("// " + fullName + "\n");
    res.append(QString::asprintf("// %dx%d\n", bf.image.width(), bf.image.height()));
    res.append(QString("const unsigned char %1[][%2] = \n{\n%3};\n").arg(fullName).arg(size).arg(imgArray));


    return res;
}

QString ImgConvertor::encodeComImgFile(BmFile bf)
{
    QString res;
    QString fullName = getFullName(bf);
    QString array;

    auto getName = [=](int id){
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
        array.append(QString("\t{%1, %2, %3, %4, %5},\n").arg(getFullName(getName(i.id))).arg(i.x).arg(i.y).arg(getName(i.id).image.width()).arg(getName(i.id).image.height()));
    }
    array.append("\t{IMG_END}\n");

    res.append("// " + fullName + "\n");
    res.append(QString::asprintf("// %dx%d\n", bf.comImg.width, bf.comImg.height));
    res.append(QString("const ComImg_t %1[] = \n{\n%2};\n").arg(fullName).arg(array));

    return res;
}

QString ImgConvertor::encode(BmFile bf)
{
    auto getParentType = [=](){
        foreach(auto data, dataList)
        {
            if(bf.pid == data.id)
            {
                return data.type;
            }
        }
        return bf.type;
    };

    if(bf.type == RawData::TypeImgGrpFolder)
    {
        return encodeImgArray(bf);
    }
    else if(bf.type == RawData::TypeImgFile)
    {
        if(getParentType() != RawData::TypeImgGrpFolder)
        {
            return encodeImgFile(bf);
        }
    }
    else if(bf.type == RawData::TypeComImgFile)
    {
        return encodeComImgFile(bf);
    }

    return NULL;
}

QString ImgConvertor::generateImgC()
{
    auto getParentType = [=](BmFile bf){
        foreach(auto data, dataList)
        {
            if(bf.pid == data.id)
            {
                return data.type;
            }
        }
        return bf.type;
    };

    QString res;

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgFile)
        {
            if(getParentType(i) != RawData::TypeImgGrpFolder)
            {
                res.append(encode(i));
            }
        }
    }
    return res;
}

QString ImgConvertor::generateImgH()
{
    QString res = "#ifndef __INC_BM_IMG_H__\n#define __INC_BM_IMG_H__\n\n";
    res.append("#include \"bm_typedef.h\"\n\n");

    auto getParentType = [=](BmFile bf){
        foreach(auto data, dataList)
        {
            if(bf.pid == data.id)
            {
                return data.type;
            }
        }
        return bf.type;
    };


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

            res.append(QString("extern const unsigned char %1[][%2];\n").arg(getFullName(bf)).arg(n));
        }
        else if(bf.type == RawData::TypeImgFile)
        {
            if(getParentType(bf) != RawData::TypeImgGrpFolder)
            {

                res.append(QString("extern const unsigned char %1[];\n").arg(getFullName(bf)));
            }
        }
    }

    res.append("#endif\n");

    return res;
}

QString ImgConvertor::generateComImgC()
{
    QString res;

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            res.append(encode(i));
        }
    }
    return res;
}

QString ImgConvertor::generateComImgH()
{
    QString res = "#ifndef __INC_BM_COM_IMG_H__\n#define __INC_BM_COM_IMG_H__\n\n";
    res.append("#include \"bm_typedef.h\"\n\n");

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            res.append(QString("extern const ComImg_t %1[];\n").arg(getFullName(i)));
        }
    }
    res.append("#endif\n");

    return res;
}

QString ImgConvertor::generateTypedefH()
{
    QString ret = "#ifndef __INC_BM_TYPEDEF_H__\n#define __INC_BM_TYPEDEF_H__\n";
    return ret;
}

void ImgConvertor::test()
{
    QFile file("1.txt");
    if(file.open(QIODevice::WriteOnly))
    {
        foreach(auto i, dataList)
        {
            qDebug().noquote() << encode(i);
            file.write(encode(i).toUtf8());
        }
        file.close();
    }
}


