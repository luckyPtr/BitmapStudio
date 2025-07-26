
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
        if(getParent(data).type == RawData::TypeImgGrpFolder)
        {
            fullName.append("[" + data.name + "]");
        }
        else
        {
            fullName.append("_" + data.name);
        }

    };

    appendParentName(appendParentName, bf);

    return fullName;
}



ImgConvertor::ImgConvertor(QVector<BmFile> dataMap, RawData::Settings settings)
{
    this->dataList = dataMap;
    this->settings = settings;
    imgEncoder = ImgEncoderFactory::create(settings.mode);
}

ImgConvertor::~ImgConvertor()
{
    delete imgEncoder;
}



QString ImgConvertor::encodeImgFile(BmFile bf)
{
    QString fullName = getFullName(bf);
    QString res;
    res.append("// " + fullName + "\r\n");

    auto imgByteArrayToString = [](QByteArray ba){
        QString str;
        for(int i = 0; i < ba.size(); i++)
        {
            if(i % 32 == 0)
            {
                str.append("\r\n\t");
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

QString ImgConvertor::encodeImgArray(BmFile bf)
{
    QString fullName = getFullName(bf);
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
                str.append("\r\n\t\t");
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
    array.append("\t{END_OF_IMG}\n");

    res.append("// " + fullName + "\n");
    res.append(QString::asprintf("// %dx%d\n", bf.comImg.size.width(), bf.comImg.size.height()));
    res.append(QString("ComImg_t %1[] = \n{\n%2};\n").arg(fullName).arg(array));

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
    // 按名称进行排序
    std::sort(dataList.begin(), dataList.end(), [=](BmFile a, BmFile b){
        return a.name.toLower() < b.name.toLower();
    });

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
    res.append("#include \"bm_img.h\"\r\n\r\n");

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgFile)
        {
            if(getParentType(i) != RawData::TypeImgGrpFolder)
            {
                res.append(encode(i));
                QCoreApplication::processEvents();
            }
        }
    }

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeImgGrpFolder)
        {
            res.append(encode(i));
            QCoreApplication::processEvents();
        }
    }

    return res;
}

QString ImgConvertor::generateImgH()
{
    QString res = "#ifndef __INC_BM_IMG_H__\r\n#define __INC_BM_IMG_H__\r\n";
    res.append("#include \"bm_typedef.h\"\r\n\r\n");

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

            res.append(QString("extern Img_t %2[][%3];\r\n").arg(getFullName(bf)).arg(n));
            QCoreApplication::processEvents();
        }
        else if(bf.type == RawData::TypeImgFile)
        {
            if(getParentType(bf) != RawData::TypeImgGrpFolder)
            {
                res.append(QString("extern Img_t %1[];\r\n").arg(getFullName(bf)));
                QCoreApplication::processEvents();
            }
        }
    }

    res.append("#endif\r\n");

    return res;
}

QString ImgConvertor::generateComImgC()
{
    QString res;
    res.append("#include \"bm_img.h\"\r\n\r\n");

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            res.append(encode(i));
            QCoreApplication::processEvents();
        }
    }
    return res;
}

QString ImgConvertor::generateComImgH()
{
    QString res = "#ifndef __INC_BM_COM_IMG_H__\n#define __INC_BM_COM_IMG_H__\r\n";
    res.append("#include \"bm_typedef.h\"\r\n\r\n");

    foreach(auto i, dataList)
    {
        if(i.type == RawData::TypeComImgFile)
        {
            res.append(QString("extern ComImg_t %1[];\r\n").arg(getFullName(i)));
        }
    }
    res.append("#endif\r\n");

    return res;
}

QString ImgConvertor::generateTypedefH()
{
    QString ret = QString("#ifndef __INC_BM_TYPEDEF_H__\r\n#define __INC_BM_TYPEDEF_H__\r\n\
typedef %1 unsigned char Img_t;\r\n\
typedef %1 struct\r\n\
{\r\n\
    Img_t *img;\r\n\
    %2 x;\r\n\
    %2 y;\r\n\
    %3 width;\r\n\
    %3 height;\r\n\
}ComImg_t;\r\n\
#define END_OF_IMG (0)\r\n\
#endif\r\n").arg(settings.keywordConst).arg(settings.keywordImgPos).arg(settings.keywordImgSize);
    return ret;
}

void ImgConvertor::test()
{

}


