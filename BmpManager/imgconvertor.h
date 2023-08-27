
#ifndef IMGCONVERTOR_H
#define IMGCONVERTOR_H

#include <QImage>
#include "rawdata.h"

class ImgConvertor
{
private:
    QVector<BmFile> dataList;
    QString getFullName(BmFile bf); // 获取包含文件夹的完整名称

public:
    ImgConvertor(QVector<BmFile> dataMap);
    QByteArray imgToByteArray(QImage &img);

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
