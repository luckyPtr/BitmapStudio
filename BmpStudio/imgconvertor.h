
#ifndef IMGCONVERTOR_H
#define IMGCONVERTOR_H

#include <QImage>
#include <QHash>
#include "rawdata.h"
#include "imgencoderfactory.h"


class ImgConvertor
{
private:
    QVector<BmFile> dataList;
    RawData::Settings settings;
    ImgEncoder *imgEncoder;
    QHash<QString, QByteArray (*)(QImage)> funcImgToByteArray;    // 取模名称与方法映射表
    QHash<QString, QImage (*)(QByteArray, QSize)> funcByteArrayToImg;
    void initFunc();
public:
    ImgConvertor(QVector<BmFile> dataMap, RawData::Settings settings);
    ~ImgConvertor();

    QImage byteArrayToImg(QByteArray ba, QSize, QString mode);

    QString getFullName(BmFile bf); // 获取包含文件夹的完整名称
    QString encodeImgFile(BmFile bf);
    QString encodeImgArray(BmFile bf);
    QString encodeComImgFile(BmFile bf);
    QString encode(BmFile bf);
    QString generateImgC();
    QString generateImgH();
    QString generateComImgC();
    QString generateComImgH();
    QString generateTypedefH();
    QString generateDeclareH();
    void test();

};

#endif // IMGCONVERTOR_H
