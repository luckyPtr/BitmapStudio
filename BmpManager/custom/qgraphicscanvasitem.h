#ifndef QGRAPHICSCANVASITEM_H
#define QGRAPHICSCANVASITEM_H

#include <QGraphicsItem>
#include <QObject>

class QGraphicsCanvasItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
private:
    QGraphicsView *view;
    QImage image;
    QPoint startPoint;  // 左上角其实位置坐标
public:
    QGraphicsCanvasItem(QWidget *parent = nullptr);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;
    void setImage(QImage &image);
    QImage getImage();
};

#endif // QGRAPHICSCANVASITEM_H
