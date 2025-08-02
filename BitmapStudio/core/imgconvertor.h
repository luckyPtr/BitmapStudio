
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
    QMap<int, BmFile> dataMap;
    RawData::Settings settings;
    ImgEncoder *imgEncoder;

    QString ImgFileToString(BmFile bf);
    QPair<QString, int> ImgArrayToString(BmFile bf);
    QString ComImgFileToString(BmFile bf);
    int getParentType(BmFile bf);
    quint32 getOffset(BmFile bf);
public:
    ImgConvertor(QVector<BmFile> dataMap, RawData::Settings settings);
    ~ImgConvertor();

    bool generateImgC(const QString &outputPath);
    bool generateImgBin(const QString &outputPath);     // 生成图片字模Bin文件
    bool generateComImg(const QString &outputPath);
    bool generateTypedef(const QString &outputPath);
    QString generateDeclareH();


};

#endif // IMGCONVERTOR_H
