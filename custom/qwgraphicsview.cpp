#include "qwgraphicsview.h"
#include <QPoint>
#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>
#include <QCursor>
#include "qgraphicscanvasitem.h"
#include "qgraphicscomimgcanvansitem.h"
#include "qgraphicsscaleitem.h"
#include "global.h"

void QWGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint point = mapToScene(event->pos()).toPoint();  // 把坐标映射到Scene上的坐标，防止有ScrollBar的时候像素画图坐标不对
    emit mouseMovePoint(point);
    QGraphicsView::mouseMoveEvent(event);
}

void QWGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QPoint point = mapToScene(event->pos()).toPoint();

    if(event->button() == Qt::LeftButton)
        emit mousePress(point);
    else if(event->button() == Qt::MiddleButton)
        emit mousePressMiddle(point);
    else if(event->button() == Qt::RightButton)
        emit mousePressRight(point);

    QGraphicsView::mousePressEvent(event);
}

void QWGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint point = mapToScene(event->pos()).toPoint();
    emit mouseRelease(point);
    QGraphicsView::mouseReleaseEvent(event);
}

void QWGraphicsView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void QWGraphicsView::paintEvent(QPaintEvent *event)
{
    //emit paint();
    QGraphicsView::paintEvent(event);
}

void QWGraphicsView::zoomPixelSize(int notches)
{
    if (!scene())
        return;
    const int step = qMax(1, Global::pixelSize / 4);   // 格子越大步长越大
    const int newSize = qBound(4, Global::pixelSize + notches * step, 64);
    if (newSize == Global::pixelSize)
        return;

    // 记录光标处的图片像素坐标（视图无缩放变换，场景坐标与像素坐标线性对应）
    const QPointF scenePos = mapToScene(mapFromGlobal(QCursor::pos()));
    const double pixelX = (double)scenePos.x() / Global::pixelSize;
    const double pixelY = (double)scenePos.y() / Global::pixelSize;

    Global::pixelSize = newSize;
    // 注意：三个自定义 item 都未声明 Type，type() 同为 UserType，
    // qgraphicsitem_cast 会全部误判命中，必须用 dynamic_cast 区分
    const QList<QGraphicsItem *> items = scene()->items();
    for (QGraphicsItem *item : items)
    {
        if (auto *canvas = dynamic_cast<QGraphicsCanvasItem *>(item))
            canvas->refreshPixelSize();
        else if (auto *comCanvas = dynamic_cast<QGraphicsComImgCanvansItem *>(item))
            comCanvas->refreshPixelSize();
        else if (auto *scaleItem = dynamic_cast<QGraphicsScaleItem *>(item))
            scaleItem->refreshPixelSize();
    }
    Global::savePixelSize(newSize);

    // 以光标为锚点调整滚动条，使光标下的像素格缩放前后不动
    const QPoint viewPos = mapFromGlobal(QCursor::pos());
    horizontalScrollBar()->setValue(qRound(pixelX * newSize) - viewPos.x());
    verticalScrollBar()->setValue(qRound(pixelY * newSize) - viewPos.y());
}

void QWGraphicsView::wheelEvent(QWheelEvent *event)
{
    // Ctrl+滚轮：调整像素格子大小（全局生效），光标处的像素格保持不动
    if ((event->modifiers() & Qt::ControlModifier) && scene())
    {
        int notches = event->angleDelta().y() / 120;
        if (notches == 0)
            notches = event->angleDelta().y() > 0 ? 1 : -1;
        zoomPixelSize(notches);
        event->accept();
        return;
    }
    QGraphicsView::wheelEvent(event);
}

QWGraphicsView::QWGraphicsView(QWidget *parent)
{
    Q_UNUSED(parent)
    setMouseTracking(true);    // 只有打开鼠标追踪功能，鼠标移动才能触发mouseMoveEvent
    setAcceptDrops(true);
}
