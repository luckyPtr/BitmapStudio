
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

    QString ImgFileToString(BmFile bf);
    QString ImgArrayToString(BmFile bf);
    QString ComImgFileToString(BmFile bf);
    int getParentType(BmFile bf);
public:
    ImgConvertor(QVector<BmFile> dataMap, RawData::Settings settings);
    ~ImgConvertor();

    bool generateImgC(const QString &outputPath);
    bool generateImgH(const QString &outputPath);
    bool generateImgBin(const QString &outputPath);     // 生成图片字模Bin文件
    bool generateImgBinH(const QString &outoutPath);    // 生成图片字模Bin文件的地址描述头文件
    bool generateComImgC(const QString &outputPath);
    bool generateComImgH(const QString &outputPath);
    bool generateTypedefH(const QString &outputPath);
    QString generateDeclareH();


};

#endif // IMGCONVERTOR_H
