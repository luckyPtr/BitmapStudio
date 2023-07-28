#include "qgraphicscomimgcanvansitem.h"
#include "qwgraphicsview.h"
#include "global.h"


QGraphicsComImgCanvansItem::QGraphicsComImgCanvansItem(QObject *parent)
    : QObject{parent}
{
    view = static_cast<QWGraphicsView*>(parent);
    // 初始化左上角0点坐标
    startPoint.setX(Global::scaleWidth + Global::scaleOffset);
    startPoint.setY(Global::scaleWidth + Global::scaleOffset);
    comImg.height = 200;
    comImg.width = 200;
}

QRectF QGraphicsComImgCanvansItem::boundingRect() const
{
    return QRectF(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize, comImg.height * Global::pixelSize);
}

QPainterPath QGraphicsComImgCanvansItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void QGraphicsComImgCanvansItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen(0xFF00FF);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    painter->drawLine(0, 0, 100, 100);

    qDebug() << "painter";
}
