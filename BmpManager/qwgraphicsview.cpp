#include "qwgraphicsview.h"
#include <QPoint>
#include <QMouseEvent>
#include <QDebug>

void QWGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug() << "mouse move" << event->pos();
    QPoint point = event->pos();
    emit mouseMovePoint(point);
    QGraphicsView::mouseMoveEvent(event);
}

void QWGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit mousePress(event->pos());
    else if(event->button() == Qt::MiddleButton)
        emit mousePressMiddle(event->pos());
    else if(event->button() == Qt::RightButton)
        emit mousePressRight(event->pos());

    QGraphicsView::mousePressEvent(event);
}

void QWGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    emit mouseRelease(event->pos());
    QGraphicsView::mouseReleaseEvent(event);
}

void QWGraphicsView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void QWGraphicsView::paintEvent(QPaintEvent *event)
{
    static int a;
    qDebug() << "Paint" << a++;
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
