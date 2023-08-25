
#include "imgconvertor.h"
#include <QPoint>
#include <QRgb>
#include <QDebug>

QString ImgConvertor::byteArrayToString(QByteArray ba)
{
    QString str;
    for(int i = 0; i < ba.size(); i++)
    {

    }
    return str;
}

QString ImgConvertor::encodeImg(QImage &img)
{
    QByteArray ba;
    ba.resize(img.width() * ((img.height() + 7) / 8));

    qDebug() << "Size:" << img.size();

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


    QString str = "";
    for(int i = 0; i < ba.size(); i++)
    {
        if(i % 32 == 0 && i)
        {
            str.append("\n\t");
        }
        str.append(QString::asprintf("0x%02X, ", (quint8)ba[i]));
    }
    return str;
}

ImgConvertor::ImgConvertor()
{

}

