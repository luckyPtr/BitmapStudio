#include "qwgraphicsview.h"
#include <QPoint>
#include <QMouseEvent>
#include <QDebug>

void QWGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug() << "mouse move" << mapToScene(event->pos());
    QPoint point = mapToScene(event->pos()).toPoint();

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

void QWGraphicsView::wheelEvent(QWheelEvent *event)
{
    QGraphicsView::wheelEvent(event);
}

QWGraphicsView::QWGraphicsView(QWidget *parent)
{
    Q_UNUSED(parent)
    setMouseTracking(true);    // 只有打开鼠标追踪功能，鼠标移动才能触发mouseMoveEvent
}
