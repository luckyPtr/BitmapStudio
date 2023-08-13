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
    RawData *rd = nullptr;
//    int dummy;  // RawData看起来有问题，程序关闭后会报错，可能是数组越界，这里添加应该变量后无报错 ？？？？？？？
    int selectedItemId = -1; // 选择的图片 -1：未选择


    void paintBackgrand(QPainter *painter);    // 绘制画布
    void paintItems(QPainter *painter);        // 绘制图形元素
    void paintGrid(QPainter *painter);          // 绘制网格
    QPoint pointToPixel(QPoint point);  // 坐标转换为画布上的像素坐标

public:
    explicit QGraphicsComImgCanvansItem(QObject *parent = nullptr);
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;

    void setComImg(ComImg &comImg);
    void setRawData(RawData *rd);
signals:

public slots:
    void on_MousePress(QPoint point);
    void on_MouseMove(QPoint point);
};

#endif // QGRAPHICSCOMIMGCANVANSITEM_H
