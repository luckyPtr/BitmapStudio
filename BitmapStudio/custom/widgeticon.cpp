#include "widgeticon.h"
#include <QPainter>
#include <QDebug>

WidgetIcon::WidgetIcon(QWidget *parent)
    : QWidget{parent}
{
    setImage(QImage(":/Image/ico.png"));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_Timeout()));
    timer->start(5);
}


void WidgetIcon::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    int rectLen = qMin(width() / image.width(), height() / image.height());
    QPoint startPos = QPoint((width() - rectLen * image.width()) / 2, (height() - rectLen * image.height()) / 2);

    QPainter painter(this);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    for (int i = 0; i <= drawedCnt; i++)
    {
        int index = drawPixel.at(i);
        QPoint point(index / image.height(), index % image.height());
        brush.setColor(image.pixelColor(point));
        point *= rectLen;
        point += startPos;
        painter.fillRect(QRect(point, QSize(rectLen, rectLen)), brush);
    }
}

void WidgetIcon::setImage(QImage img)
{
    image = img;

    drawPixel.resize(img.width() * img.height());
    for (int i = 0; i < drawPixel.size(); i++)
    {
        drawPixel[i] = i;
    }
    for(int i = drawPixel.size() - 1; i > 0; --i)
    {
        int j = QRandomGenerator::global()->bounded(i + 1);
        int temp = drawPixel[i];
        drawPixel[i] = drawPixel[j];
        drawPixel[j] = temp;
    }
    drawedCnt = 0;
}

void WidgetIcon::on_Timeout()
{
    if(drawedCnt < drawPixel.size() - 30)
    {
        drawedCnt += QRandomGenerator::global()->bounded(10) + 15;
    }
    else
    {
        drawedCnt = drawPixel.size() - 1;
        timer->stop();
    }
    repaint();
}
