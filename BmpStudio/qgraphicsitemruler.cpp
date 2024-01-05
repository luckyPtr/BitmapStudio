#include "qgraphicsitemruler.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

QGraphicsItemRuler::QGraphicsItemRuler()
{
    this->setFlag(QGraphicsItem::ItemIsFocusable);
    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setAcceptHoverEvents(true);
}

void QGraphicsItemRuler::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    QPen pen(QColor(0, 0, 0));

    // https://blog.51cto.com/u_15692960/5406058
    // https://blog.csdn.net/weixin_43493507/article/details/122120695
    // https://blog.csdn.net/QQ1402369668/article/details/106747912
    if (option->state & QStyle::State_Selected)
    {
        pen.setColor(QColor(255, 0, 0));
    }
    painter->setPen(pen);
    painter->drawLine(0, 0, 200, 200);
}

QPainterPath QGraphicsItemRuler::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(10, 10, 180, 180));
    return path;
}


QRectF QGraphicsItemRuler::boundingRect() const
{
    return QRectF(0, 0, 200, 200);
}
