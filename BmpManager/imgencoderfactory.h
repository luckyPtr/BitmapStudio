#ifndef IMGENCODERFACTORY_H
#define IMGENCODERFACTORY_H
#include <QImage>

class ImgEncoder
{
public:
    virtual QByteArray encode(QImage) = 0;          // 图片转数组
    virtual QImage decode(QByteArray, QSize) = 0;   // 数组转图片

    ImgEncoder() {}
    ~ImgEncoder() {}
};

// 逐列 逆向
class ImgEncoder_ZL_LSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};

// 逐列 顺向
class ImgEncoder_ZL_MSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};


// 逐行 逆向
class ImgEncoder_ZH_LSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};

// 逐行 顺向
class ImgEncoder_ZH_MSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};


// 列行 顺向
class ImgEncoder_LH_MSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};

// 列行 逆向
class ImgEncoder_LH_LSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};

// 行列 逆向
class ImgEncoder_HL_LSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};


// 行列 顺向
class ImgEncoder_HL_MSB : public ImgEncoder
{
public:
    QByteArray encode(QImage);
    QImage decode(QByteArray, QSize);
};



class ImgEncoderFactory
{
public:
    enum
    {
        ZH_LSB,
        ZL_LSB,
        HL_LSB,
        LH_LSB,
        ZH_MSB,
        ZL_MSB,
        HL_MSB,
        LH_MSB,
        // ...
    };


    ImgEncoderFactory();
    static ImgEncoder *create(int mode);
};

#endif // IMGENCODERFACTORY_H
