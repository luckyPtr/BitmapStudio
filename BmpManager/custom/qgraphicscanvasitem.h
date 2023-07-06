#ifndef QGRAPHICSCANVASITEM_H
#define QGRAPHICSCANVASITEM_H

#include <QGraphicsItem>
#include <QObject>

enum ResizeStep
{
    ResizeNull,
    ResizeVer,
    ResizeHor,
    ResizeFDiag,
};

class QGraphicsCanvasItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
private:
    QGraphicsView *view;
    QImage image;
    QPoint startPoint;  // 左上角其实位置坐标
    QPoint currentPoint;
    bool isResezeMode;  // 是否为重新设置画布大小的模式
    bool isInSizeVerArea(QPoint point); // 是否处于垂直调整画布大小的区域内
    bool isInSizeHorArea(QPoint point); // 是否处于水平调整画布大小的区域内
    bool isInSizeFDiagArea(QPoint point);
    quint8 ResizeStep = ResizeNull;  // 调整画布大小的步骤 0-初始状态 1-按下
public:
    QGraphicsCanvasItem(QWidget *parent = nullptr);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;
    void setImage(QImage &image);
    QImage getImage();

public slots:
    void on_ResizeMouseMove(QPoint point);
    void on_ResizeMousePress(QPoint point);
    void on_ResizeMouseRelease(QPoint point);

};

#endif // QGRAPHICSCANVASITEM_H
