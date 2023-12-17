#include "imgencoderfactory.h"

ImgEncoderFactory::ImgEncoderFactory()
{

}

ImgEncoder *ImgEncoderFactory::create(int mode)
{
    switch(mode)
    {
    case LH_MSB:
        return new ImgEncoder_LH_MSB;
    case LH_LSB:
        return new ImgEncoder_LH_LSB;
    }
    return nullptr;
}

QByteArray ImgEncoder_LH_LSB::encode(QImage img)
{
    QByteArray ba;
    ba.resize(img.width() * ((img.height() + 7) / 8));

    for(int y = 0; y < (img.height() + 7) / 8; y++)
    {
        for(int x = 0; x < img.width(); x++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x, y * 8 + n);
                if(!img.rect().contains(point))
                {
                    break;
                }
                temp |= ((qGray(img.pixel(point)) < 128) << n);
            }
            ba[y * img.width() + x] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_LH_LSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);
    for(int y = 0; y < (img.height() + 7) / 8; y++)
    {
        for(int x = 0; x < img.width(); x++)
        {
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x, y * 8 + n);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(ba[y * img.width() + x] & (1 << n))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}

QByteArray ImgEncoder_LH_MSB::encode(QImage img)
{
    QByteArray ba;
    ba.resize(img.width() * ((img.height() + 7) / 8));

    for(int y = 0; y < (img.height() + 7) / 8; y++)
    {
        for(int x = 0; x < img.width(); x++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x, y * 8 + n);
                if(!img.rect().contains(point))
                {
                    break;
                }
                temp |= ((qGray(img.pixel(point)) < 128) << (7 - n));
            }
            ba[y * img.width() + x] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_LH_MSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);
    for(int y = 0; y < (img.height() + 7) / 8; y++)
    {
        for(int x = 0; x < img.width(); x++)
        {
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x, y * 8 + n);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(ba[y * img.width() + x] & (1 << (7 - n)))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}
