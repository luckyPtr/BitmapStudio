#ifndef QGRAPHICSCOMIMGCANVANSITEM_H
#define QGRAPHICSCOMIMGCANVANSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include "rawdata.h"

class QGraphicsComImgCanvansItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

    enum Action
    {
        ActionNull,
        ActionSelect,
        ActionMove,
    };

    // https://blog.csdn.net/u013125105/article/details/100514290
    // Drag Drop参考


    QGraphicsView *view;
    QPoint startPoint;  // 左上角起始位置坐标
    ComImg comImg;
    RawData *rd = nullptr;
//    int dummy;  // RawData看起来有问题，程序关闭后会报错，可能是数组越界，这里添加应该变量后无报错 ？？？？？？？
    int selectedItemIndex = -1;
    Action action = ActionNull;
    QPoint currentPoint;
    QPoint currentPixel;    // 当前鼠标所在的坐标（图片像素）
    QPoint moveLastPixel;   // 移动图元上一次所在的坐标(像素)
    QPoint moveStartPixel;  // 开始移动画布的坐标(像素)
    int dragImgId;          // 拖入的图片ID
    bool isDragImg = false;
    
    void paintBackground(QPainter *painter);    // 绘制画布
    void paintItems(QPainter *painter);        // 绘制图形元素
    void paintGrid(QPainter *painter);          // 绘制网格
    void paintDragItem(QPainter *painter);      // 绘制拖入的图片
    QPoint pointToPixel(QPoint point);  // 坐标转换为画布上的像素坐标
    int getPointImgIndex(QPoint point);

public:
    explicit QGraphicsComImgCanvansItem(QObject *parent = nullptr);
    ~QGraphicsComImgCanvansItem() { rd = nullptr; }
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;

    void setComImg(ComImg &comImg);
    void setRawData(RawData *rd);
    ComImg getComImg() { return comImg; }

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;//处理drop
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;//添加QDrag




public slots:
    void on_MousePress(QPoint point);
    void on_MouseMove(QPoint point);
    void on_MouseRelease(QPoint point);

    void deleteSelectItem();    // 删除选中的图片

};

#endif // QGRAPHICSCOMIMGCANVANSITEM_H
