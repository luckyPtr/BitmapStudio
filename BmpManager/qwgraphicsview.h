#ifndef QWGRAPHICSVIEW_H
#define QWGRAPHICSVIEW_H

#include <QGraphicsView>


class QWGraphicsView : public QGraphicsView
{
    Q_OBJECT
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
public:
    QWGraphicsView(QWidget *parent = nullptr);
signals:
    void mouseMovePoint(QPoint point);
    void mouseClicked(QPoint point);
    void resized();
    void paint();
};

#endif // QWGRAPHICSVIEW_H