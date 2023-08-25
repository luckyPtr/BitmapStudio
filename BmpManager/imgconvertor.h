
#ifndef IMGCONVERTOR_H
#define IMGCONVERTOR_H

#include <QImage>


class ImgConvertor
{
private:
    QString byteArrayToString(QByteArray ba);

public:
    ImgConvertor();
    QString encodeImg(QImage &img);
    static QString encode(int id);
};

#endif // IMGCONVERTOR_H
