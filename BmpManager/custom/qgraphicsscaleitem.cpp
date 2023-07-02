#include "qgraphicsscaleitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsView>
#include <QDebug>
#include "global.h"

void QGraphicsScaleItem::drawScale(QPainter *painter)
{
    // 绘制轮廓喝背景
    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(142, 156, 175));
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);

    QBrush brush;
    brush.setColor(QColor(241, 243, 248));
    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);

    int width = qMax(view->width(), (int)view->scene()->width());
    int height = qMax(view->height(), (int)view->scene()->height());

    QPoint points[] = {
        QPoint(-1, -1),
        QPoint(width, 0),
        QPoint(width, Global::scaleWidth),
        QPoint(Global::scaleWidth, Global::scaleWidth),
        QPoint(Global::scaleWidth, height),
        QPoint(0, height)
    };
    painter->drawPolygon(points, 6);

    // 绘制刻度线
    // 水平刻度线
    for(int i = 0, x = 0; x < width; i++)
    {
        x = Global::scaleWidth + Global::scaleOffset + i * Global::pixelSize;
        // 每10像素绘制长刻度线和数值
        if(i % 10 == 0)
        {
            QLine line(x, 0, x, Global::scaleWidth);
            painter->drawLine(line);
            QFont font(QStringLiteral("微软雅黑"), 8);
            painter->setFont(font);

            painter->drawText(x + 2, Global::scaleWidth / 2 + 2, QString::number(i));
        }
        else
        {
            QLine line(x, Global::scaleWidth - 4, x, Global::scaleWidth);
            painter->drawLine(line);
        }
    }
    // 垂直刻度线
    for(int i = 0, y = 0; y < height; i++)
    {
        y = Global::scaleWidth + Global::scaleOffset + i * Global::pixelSize;
        // 每10像素绘制长刻度线和数值
        if(i % 10 == 0)
        {
            QLine line(0, y, Global::scaleWidth, y);
            painter->drawLine(line);
        }
        else
        {
            QLine line(Global::scaleWidth - 4, y, Global::scaleWidth, y);
            painter->drawLine(line);
        }
    }

    painter->save();
    painter->rotate(90);
    for(int i = 0, y = 0; y < height; i++)
    {
        y = Global::scaleWidth + Global::scaleOffset + i * Global::pixelSize;
        if(i % 10 == 0)
        {
            painter->drawText(y + 2, Global::scaleWidth / 2 - 10, QString::number(i));
        }
    }


    painter->restore();
}

QGraphicsScaleItem::QGraphicsScaleItem(QWidget *parent)
{
    view = static_cast<QGraphicsView*>(parent);

}

QRectF QGraphicsScaleItem::boundingRect() const
{
    int width = qMax(view->width(), (int)view->scene()->width());
    int height = qMax(view->height(), (int)view->scene()->height());
    return QRectF(0, 0, width, height);
}

void QGraphicsScaleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    drawScale(painter);

    if (option->state & QStyle::State_Selected)
    {

    }
    qDebug() << "paint scale";
}

QPainterPath QGraphicsScaleItem::shape() const
{
    int width = qMax(view->width(), (int)view->scene()->width());
    int height = qMax(view->height(), (int)view->scene()->height());
    QPainterPath path;

    QVector<QPointF> points;
    points.append(QPointF(0, 0));
    points.append(QPointF(width, 0));
    points.append(QPointF(width, Global::scaleWidth));
    points.append(QPointF(Global::scaleWidth, Global::scaleWidth));
    points.append(QPointF(Global::scaleWidth, height));
    points.append(QPointF(0, height));
    path.addPolygon(QPolygonF(points));


    return path;
}
