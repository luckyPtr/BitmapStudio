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
    qDebug() << "mouse clicked!";
    QGraphicsView::mousePressEvent(event);
}

void QWGraphicsView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void QWGraphicsView::paintEvent(QPaintEvent *event)
{
    emit paint();
    QGraphicsView::paintEvent(event);
}

QWGraphicsView::QWGraphicsView(QWidget *parent)
{
    setMouseTracking(true);    // 只有打开鼠标追踪功能，鼠标移动才能触发mouseMoveEvent
}
