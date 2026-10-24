#ifndef QGRAPHICSITEMRULER_H
#define QGRAPHICSITEMRULER_H

#include <QGraphicsItem>

class QGraphicsItemRuler : public QObject, public QGraphicsItem
{
    Q_OBJECT
private:

public:
    QGraphicsItemRuler();
    QRectF boundingRect() const  Q_DECL_OVERRIDE;;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    QPainterPath shape() const Q_DECL_OVERRIDE;;
};

#endif // QGRAPHICSITEMRULER_H
