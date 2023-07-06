#include "qgraphicscanvasitem.h"
#include "qwgraphicsview.h"
#include "global.h"
#include <QDebug>
#include <QPen>

bool QGraphicsCanvasItem::isInSizeVerArea(QPoint point)
{
    QRect rect(startPoint.x() + image.height() * Global::pixelSize / 2 - 6, startPoint.y() + image.width() * Global::pixelSize, 12, 12);
    return rect.contains(point);
}

bool QGraphicsCanvasItem::isInSizeHorArea(QPoint point)
{
    QRect rect(startPoint.x() + image.height() * Global::pixelSize, startPoint.y() + image.width() * Global::pixelSize / 2 - 6, 12, 12);
    return rect.contains(point);
}

bool QGraphicsCanvasItem::isInSizeFDiagArea(QPoint point)
{
    QRect rect(startPoint.x() + image.height() * Global::pixelSize, startPoint.y() + image.width() * Global::pixelSize, 12, 12);
    return rect.contains(point);
}

QGraphicsCanvasItem::QGraphicsCanvasItem(QWidget *parent)
{
    view = static_cast<QWGraphicsView*>(parent);
    // 初始化左上角0点坐标
    startPoint.setX(Global::scaleWidth + Global::scaleOffset);
    startPoint.setY(Global::scaleWidth + Global::scaleOffset);
    ResizeStep = ResizeNull;

    connect(view, SIGNAL(mouseMovePoint(QPoint)), this, SLOT(on_ResizeMouseMove(QPoint)));
    connect(view, SIGNAL(mousePress(QPoint)), this, SLOT(on_ResizeMousePress(QPoint)));
    connect(view, SIGNAL(mouseRelease(QPoint)), this, SLOT(on_ResizeMouseRelease(QPoint)));
}

QRectF QGraphicsCanvasItem::boundingRect() const
{
    QRectF rect(startPoint.x(), startPoint.y(), image.height() * Global::pixelSize, image.width() * Global::pixelSize);
    return rect;
}

QPainterPath QGraphicsCanvasItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void QGraphicsCanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    // 绘制像素
    for(int x = 0; x < image.width(); x++)
    {
        for(int y = 0; y < image.height(); y++)
        {
            QColor color = image.pixelColor(x, y);
            quint8 grayscale  = qGray(color.rgb());
            QRect rect(startPoint.x() + x * Global::pixelSize, startPoint.y() + y * Global::pixelSize, Global::pixelSize, Global::pixelSize);
            painter->fillRect(rect, grayscale < 128 ? 0x9ce0ef : 0x495028);
        }
    }

    // 绘制网格
    QPen pen(0x303030);
    painter->setPen(pen);
    for(int x = 0; x < image.width(); x++)
    {
        painter->drawLine(startPoint.x() + x * Global::pixelSize, startPoint.y(), startPoint.x() + x * Global::pixelSize, startPoint.y() + image.height() * Global::pixelSize);
    }
    for(int y = 0; y < image.height(); y++)
    {
        painter->drawLine(startPoint.x(), startPoint.y() + y * Global::pixelSize, startPoint.x() + image.width() * Global::pixelSize, startPoint.y() + y * Global::pixelSize);
    }

    // 外边框
    pen.setWidth(2);
    pen.setColor(Qt::yellow);
    painter->setPen(pen);
    QRectF rect(startPoint.x(), startPoint.y(), image.width() * Global::pixelSize, image.height() * Global::pixelSize);
    painter->drawRect(rect);

    // 调整画布大小
    //if(isResezeMode)
    {
        pen.setWidth(1);
        pen.setColor(Qt::black);
        painter->setPen(pen);
        QBrush brush(Qt::white);
        brush.setStyle(Qt::SolidPattern);
        painter->setBrush(brush);
        painter->drawRect(QRect(startPoint.x() + image.height() * Global::pixelSize, startPoint.y() + image.width() * Global::pixelSize, 4, 4));
        painter->drawRect(QRect(startPoint.x() + image.height() * Global::pixelSize / 2 - 2, startPoint.y() + image.width() * Global::pixelSize, 4, 4));
        painter->drawRect(QRect(startPoint.x() + image.height() * Global::pixelSize, startPoint.y() + image.width() * Global::pixelSize / 2 - 2, 4, 4));
    }



    // 画布预览

    // 校准画布大小到像素点对应的大小
    auto calibrate = ([=](QPoint point){
        return QPoint(((point.x() - startPoint.x()) / Global::pixelSize) * Global::pixelSize + startPoint.x(),
                      ((point.y() - startPoint.y()) / Global::pixelSize) * Global::pixelSize + startPoint.y());
    });

    if(ResizeStep != ResizeNull)
    {
        QBrush brush;
        brush.setStyle(Qt::NoBrush);
        painter->setBrush(brush);
        pen.setWidth(2);
        pen.setStyle(Qt::DotLine);
        pen.setColor(Qt::yellow);
        painter->setPen(pen);


        if(ResizeStep == ResizeFDiag)
        {
            painter->drawRect(QRect(startPoint, calibrate(currentPoint)));
        }
        else if(ResizeStep == ResizeVer)
        {
            QPoint point(startPoint.x() + image.width() * Global::pixelSize, currentPoint.y());
            painter->drawRect(QRect(startPoint, calibrate(point)));
        }
        else if(ResizeStep == ResizeHor)
        {
            QPoint point(currentPoint.x(), startPoint.y() + image.height() * Global::pixelSize);
            painter->drawRect(QRect(startPoint, calibrate(point)));
        }
    }
}

void QGraphicsCanvasItem::setImage(QImage &image)
{
    this->image = image;
}

QImage QGraphicsCanvasItem::getImage()
{
    return image;
}

void QGraphicsCanvasItem::on_ResizeMouseMove(QPoint point)
{
    if(ResizeStep == ResizeNull)
    {
        Qt::CursorShape cursor = Qt::ArrowCursor;
        if(isInSizeFDiagArea(point))
        {
            cursor = Qt::SizeFDiagCursor;
        }
        else if(isInSizeVerArea(point))
        {
            cursor = Qt::SizeVerCursor;
        }
        else if(isInSizeHorArea(point))
        {
            cursor = Qt::SizeHorCursor;
        }
        view->setCursor(cursor);
    }
    currentPoint = point;
}

void QGraphicsCanvasItem::on_ResizeMousePress(QPoint point)
{
    if(ResizeStep == ResizeNull)
    {
        if(isInSizeFDiagArea(point))
        {
            ResizeStep = ResizeFDiag;
        }
        else if(isInSizeVerArea(point))
        {
            ResizeStep = ResizeVer;
        }
        else if(isInSizeHorArea(point))
        {
            ResizeStep = ResizeHor;
        }
    }
}

void QGraphicsCanvasItem::on_ResizeMouseRelease(QPoint point)
{
    if(ResizeStep != ResizeNull)
    {
        ResizeStep = ResizeNull;
        view->setCursor(Qt::ArrowCursor);
    }
    view->viewport()->update();
}
