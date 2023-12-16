
#ifndef IMGCONVERTOR_H
#define IMGCONVERTOR_H

#include <QImage>
#include <QHash>
#include "rawdata.h"



class ImgConvertor
{
private:
    QVector<BmFile> dataList; 

    QHash<QString, QByteArray (*)(QImage)> funcImgToByteArray;    // 取模名称与方法映射表
    QHash<QString, QImage (*)(QByteArray, QSize)> funcByteArrayToImg;
    void initFunc();
public:
    ImgConvertor(QVector<BmFile> dataMap);

    QByteArray imgToByteArray(QImage &img, QString mode);
    QByteArray imgToByteArray(QImage &img);
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
