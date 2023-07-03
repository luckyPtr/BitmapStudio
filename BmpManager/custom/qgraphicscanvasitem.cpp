#include "qgraphicscanvasitem.h"
#include "qwgraphicsview.h"
#include "global.h"
#include <QDebug>
#include <QPen>

QGraphicsCanvasItem::QGraphicsCanvasItem(QWidget *parent)
{
    view = static_cast<QWGraphicsView*>(parent);
    // 初始化左上角0点坐标
    startPoint.setX(Global::scaleWidth + Global::scaleOffset);
    startPoint.setY(Global::scaleWidth + Global::scaleOffset);
}

QRectF QGraphicsCanvasItem::boundingRect() const
{
    QRectF rect(startPoint.x(), startPoint.y(), image.height() * Global::pixelSize, image.width() * Global::pixelSize);
    return rect;
}

QPainterPath QGraphicsCanvasItem::shape() const
{
//    QPainterPath path;
//    path.addRect()
    QPainterPath path;
    QRectF rect(startPoint.x(), startPoint.y(), image.height() * Global::pixelSize, image.width() * Global::pixelSize);
    path.addRect(rect);
    return path;
}

void QGraphicsCanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 绘制像素
    for(int x = 0; x < image.width(); x++)
    {
        for(int y = 0; y < image.height(); y++)
        {
            QColor color = image.pixelColor(x, y);
            quint8 grayscale  = qGray(color.rgb());
            QRect rect(startPoint.x() + x * Global::pixelSize, startPoint.y() + y * Global::pixelSize, Global::pixelSize, Global::pixelSize);
            painter->fillRect(rect, grayscale < 128 ? 0x9ce0ef : 0x495028);
        }
    }

    // 绘制网格
    QPen pen(0x303030);
    painter->setPen(pen);
    for(int x = 0; x < image.width(); x++)
    {
        painter->drawLine(startPoint.x() + x * Global::pixelSize, startPoint.y(), startPoint.x() + x * Global::pixelSize, startPoint.y() + image.height() * Global::pixelSize);
    }
    for(int y = 0; y < image.height(); y++)
    {
        painter->drawLine(startPoint.x(), startPoint.y() + y * Global::pixelSize, startPoint.x() + image.width() * Global::pixelSize, startPoint.y() + y * Global::pixelSize);
    }

    // 外边框
    pen.setWidth(2);
    pen.setColor(Qt::yellow);
    painter->setPen(pen);
    QRectF rect(startPoint.x(), startPoint.y(), image.height() * Global::pixelSize, image.width() * Global::pixelSize);
    painter->drawRect(rect);

    qDebug() << "Paint CanvasBound";
}

void QGraphicsCanvasItem::setImage(QImage &image)
{
    this->image = image;
}

QImage QGraphicsCanvasItem::getImage()
{
    return image;
}
