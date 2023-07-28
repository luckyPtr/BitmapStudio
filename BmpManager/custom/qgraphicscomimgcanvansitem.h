#ifndef QGRAPHICSCOMIMGCANVANSITEM_H
#define QGRAPHICSCOMIMGCANVANSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include "rawdata.h"

class QGraphicsComImgCanvansItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

    QGraphicsView *view;
    QPoint startPoint;  // 左上角起始位置坐标
    ComImg comImg;

public:
    explicit QGraphicsComImgCanvansItem(QObject *parent = nullptr);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;
signals:

};

#endif // QGRAPHICSCOMIMGCANVANSITEM_H
