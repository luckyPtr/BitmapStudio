#include "latticeshow.h"
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include "global.h"
#include "qrgb.h"

LatticeShow::LatticeShow(QWidget *parent)
    : QWidget{parent}
{
    QTimer *t = new QTimer(this);
    t->start(50);
    connect(t, &QTimer::timeout, this, [=]{
        if(++showStep > muduleSize * muduleSize)
        {
            showStep = 0;
        }
        this->repaint();
    });

    setMode(ImgEncoderFactory::LH_LSB);
}

void LatticeShow::setMode(int mode)
{
    if(imgEncoder != nullptr)
    {
        delete imgEncoder;
    }

    imgEncoder = ImgEncoderFactory::create(mode);
    showStep = 0;
}

void LatticeShow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    int len = qMin(width(), height()) - 1;  // 取长宽最小值作为绘制正方形的宽
    len /= muduleSize;

    QPainter painter(this);
    QPen pen;
    QBrush brush;

    // 绘制底色
    brush.setColor(Global::pixelColor_0);
    brush.setStyle(Qt::SolidPattern);
    painter.fillRect(QRect(0, 0, len * muduleSize, len * muduleSize), brush);

    // 绘制填充的像素
    brush.setColor(Global::pixelColor_1);
    QByteArray ba(muduleSize * muduleSize / 8, 0);

    int i = 0;
    while(i < showStep / 8)
    {
        ba[i++] = 0xFF;
    }
    for(int j = 0; j < showStep % 8; j++)
    {
        ba[i] = (ba[i] << 1) | 1;
    }

    if(imgEncoder)
    {
        QImage img = imgEncoder->decode(ba, QSize(muduleSize, muduleSize));
        for(int x = 0; x < muduleSize; x++)
        {
            for(int y = 0; y < muduleSize; y++)
            {
                QColor color = img.pixelColor(x, y);
                if(qGray(color.rgb()) < 128)
                {
                    painter.fillRect(x * len, y * len, len, len, brush);
                }
            }
        }
    }

    // 绘制网格
    pen.setColor(QColor(Global::gridColor));
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    for(int x = 0; x <= muduleSize; x++)
    {
        for(int y = 0; y <= muduleSize; y++)
        {
            painter.drawLine(0, y * len, muduleSize * len, y * len);
            painter.drawLine(x * len, 0, x * len, muduleSize * len);
        }
    }
}
