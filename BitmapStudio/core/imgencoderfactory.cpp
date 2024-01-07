#include "imgencoderfactory.h"

ImgEncoderFactory::ImgEncoderFactory()
{

}

ImgEncoder *ImgEncoderFactory::create(int mode)
{
    switch(mode)
    {
    case ZL_LSB:
        return new ImgEncoder_ZL_LSB;
    case ZL_MSB:
        return new ImgEncoder_ZL_MSB;
    case ZH_LSB:
        return new ImgEncoder_ZH_LSB;
    case ZH_MSB:
        return new ImgEncoder_ZH_MSB;
    case LH_MSB:
        return new ImgEncoder_LH_MSB;
    case LH_LSB:
        return new ImgEncoder_LH_LSB;
    case HL_LSB:
        return new ImgEncoder_HL_LSB;
    case HL_MSB:
        return new ImgEncoder_HL_MSB;
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

QByteArray ImgEncoder_HL_LSB::encode(QImage img)
{
    QByteArray ba(img.width() * ((img.height() + 7) / 8), 0);

    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + n, y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                temp |= ((qGray(img.pixel(point)) < 128) << n);
            }
            ba[x * img.height() + y] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_HL_LSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);
    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + n, y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(ba[x * img.height() + y] & (1 << n))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}

QByteArray ImgEncoder_HL_MSB::encode(QImage img)
{
    QByteArray ba(((img.width() + 7) / 8) * img.height(), 0);

    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + n, y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                temp |= ((qGray(img.pixel(point)) < 128) << (7 - n));
            }
            ba[x * img.height() + y] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_HL_MSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);
    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + n, y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(ba[x * img.height() + y] & (1 << (7 - n)))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}

QByteArray ImgEncoder_ZL_LSB::encode(QImage img)
{
    QByteArray ba(img.width() * ((img.height() + 7) / 8), 0);

    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < (img.height() + 7) / 8; y++)
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
            ba[x * ((img.height() + 7) / 8) + y] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_ZL_LSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);

    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < (img.height() + 7) / 8; y++)
        {
            quint8 temp = ba[x * ((img.height() + 7) / 8) + y];
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x, y * 8 + n);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(temp & (1 << n))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}


QByteArray ImgEncoder_ZL_MSB::encode(QImage img)
{
    QByteArray ba(img.width() * ((img.height() + 7) / 8), 0);

    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < (img.height() + 7) / 8; y++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x, y * 8 + (7 - n));
                if(!img.rect().contains(point))
                {
                    break;
                }
                temp |= ((qGray(img.pixel(point)) < 128) << n);
            }
            ba[x * ((img.height() + 7) / 8) + y] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_ZL_MSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);

    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < (img.height() + 7) / 8; y++)
        {
            quint8 temp = ba[x * ((img.height() + 7) / 8) + y];
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x, y * 8 + n);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(temp & (1 << (7 - n)))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}

QByteArray ImgEncoder_ZH_LSB::encode(QImage img)
{
    QByteArray ba(((img.width() + 7) / 8) * img.height(), 0);

    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + n, y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                temp |= ((qGray(img.pixel(point)) < 128) << n);
            }
            ba[x + y * ((img.width() + 7) / 8)] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_ZH_LSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);

    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = ba[x + y * ((img.width() + 7) / 8)];
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + n, y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(temp & (1 << n))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}

QByteArray ImgEncoder_ZH_MSB::encode(QImage img)
{
    QByteArray ba(((img.width() + 7) / 8) * img.height(), 0);

    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = 0;
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + (7 - n), y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                temp |= ((qGray(img.pixel(point)) < 128) << n);
            }
            ba[x + y * ((img.width() + 7) / 8)] = temp;
        }
    }
    return ba;
}

QImage ImgEncoder_ZH_MSB::decode(QByteArray ba, QSize size)
{
    QImage img(size, QImage::Format_RGB888);
    img.fill(Qt::white);

    for(int x = 0; x < (img.width() + 7) / 8; x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            quint8 temp = ba[x + y * ((img.width() + 7) / 8)];
            for(int n = 0; n < 8; n++)
            {
                QPoint point(x * 8 + n, y);
                if(!img.rect().contains(point))
                {
                    break;
                }
                if(temp & (1 << (7 - n)))
                {
                    img.setPixelColor(point, Qt::black);
                }
            }
        }
    }
    return img;
}
