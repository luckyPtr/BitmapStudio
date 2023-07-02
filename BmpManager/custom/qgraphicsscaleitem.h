#ifndef QGRAPHICSSCALEITEM_H
#define QGRAPHICSSCALEITEM_H

#include <QGraphicsItem>
#include <QObject>

class QGraphicsScaleItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
private:
    QGraphicsView *view;
    void drawScale(QPainter *painter);
public:
    QGraphicsScaleItem(QWidget *parent = nullptr);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
};

#endif // QGRAPHICSSCALEITEM_H
