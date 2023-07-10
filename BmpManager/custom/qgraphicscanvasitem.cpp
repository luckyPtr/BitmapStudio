#include "qgraphicscanvasitem.h"
#include "qwgraphicsview.h"
#include "global.h"
#include <QDebug>
#include <QPen>
#include <QImage>

QPoint QGraphicsCanvasItem::pointToPixel(QPoint point)
{
    return QPoint((point.x() - startPoint.x()) / Global::pixelSize,
                  (point.y() - startPoint.y()) / Global::pixelSize);
}

bool QGraphicsCanvasItem::isInSizeVerArea(QPoint point)
{
    QRect rect(startPoint.x() + image.width() * Global::pixelSize / 2 - 6, startPoint.y() + image.height() * Global::pixelSize, 12, 12);
    return rect.contains(point);
}

bool QGraphicsCanvasItem::isInSizeHorArea(QPoint point)
{
    QRect rect(startPoint.x() + image.width() * Global::pixelSize, startPoint.y() + image.height() * Global::pixelSize / 2 - 6, 12, 12);
    return rect.contains(point);
}

bool QGraphicsCanvasItem::isInSizeFDiagArea(QPoint point)
{
    QRect rect(startPoint.x() + image.width() * Global::pixelSize, startPoint.y() + image.height() * Global::pixelSize, 12, 12);
    return rect.contains(point);
}

void QGraphicsCanvasItem::resizeImage(QImage &img, int width, int heighy)
{
    QImage newImage(width, heighy, QImage::Format_RGB888);

    for(int x = 0; x < img.width() && x < newImage.width(); x++)
    {
        for(int y = 0; y < img.height() && y < newImage.height(); y++)
        {
            QPoint point(x, y);
            newImage.setPixelColor(point, image.pixelColor(point));
        }
    }

    img = newImage;
}

void QGraphicsCanvasItem::moveImage(QImage &img, int OffsetX, int OffsetY)
{
    QImage newImg(img.width(), img.height(), QImage::Format_RGB888);

    auto isContainPoint([=](QPoint point){
        return (point.x() >= 0 && point.x() < img.width() && point.y() >= 0 && point.y() < img.height());
    });

    for(int x = 0; x < newImg.width(); x++)
    {
        for(int y = 0; y < newImg.height(); y++)
        {
            QPoint point(x + OffsetX, y + OffsetY);
            if(isContainPoint(point))
            {
                newImg.setPixelColor(point, img.pixelColor(x, y));
            }
        }
    }
    img = newImg;
}

QGraphicsCanvasItem::QGraphicsCanvasItem(QWidget *parent)
{
    view = static_cast<QWGraphicsView*>(parent);
    // 初始化左上角0点坐标
    startPoint.setX(Global::scaleWidth + Global::scaleOffset);
    startPoint.setY(Global::scaleWidth + Global::scaleOffset);
    action = ActionNull;

    connect(view, SIGNAL(mouseMovePoint(QPoint)), this, SLOT(on_ResizeMouseMove(QPoint)));
    connect(view, SIGNAL(mousePress(QPoint)), this, SLOT(on_ResizeMousePress(QPoint)));
    connect(view, SIGNAL(mouseRelease(QPoint)), this, SLOT(on_ResizeMouseRelease(QPoint)));
    connect(view, SIGNAL(mouseMovePoint(QPoint)), this, SLOT(on_MoveMouseMove(QPoint)));
    connect(view, SIGNAL(mousePressMiddle(QPoint)), this, SLOT(on_MoveMousePress(QPoint)));
    connect(view, SIGNAL(mouseRelease(QPoint)), this, SLOT(on_MoveMouseRelease(QPoint)));
}

QRectF QGraphicsCanvasItem::boundingRect() const
{
    QRectF rect(startPoint.x(), startPoint.y(), image.width() * Global::pixelSize, image.height() * Global::pixelSize);
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
    QImage imageShow = image;
    if(action == ActionMove)   // 如果是移动画布，对图片位置进行移动
    {
        moveImage(imageShow, currentPixel.x() - moveStartPixel.x(), currentPixel.y() - moveStartPixel.y());
    }
    for(int x = 0; x < imageShow.width(); x++)
    {
        for(int y = 0; y < imageShow.height(); y++)
        {
            QColor color = imageShow.pixelColor(x, y);
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
    QRectF rect(startPoint.x(), startPoint.y(), image.width() * Global::pixelSize + 1, image.height() * Global::pixelSize + 1);
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
        painter->drawRect(QRect(startPoint.x() + image.width() * Global::pixelSize, startPoint.y() + image.height() * Global::pixelSize, 4, 4));
        painter->drawRect(QRect(startPoint.x() + image.width() * Global::pixelSize / 2 - 2, startPoint.y() + image.height() * Global::pixelSize, 4, 4));
        painter->drawRect(QRect(startPoint.x() + image.width() * Global::pixelSize, startPoint.y() + image.height() * Global::pixelSize / 2 - 2, 4, 4));
    }



    // 画布预览

    // 校准画布大小到像素点对应的大小
    auto calibrate = ([=](QPoint point){
        return QPoint(((point.x() - startPoint.x()) / Global::pixelSize) * Global::pixelSize + startPoint.x(),
                      ((point.y() - startPoint.y()) / Global::pixelSize) * Global::pixelSize + startPoint.y());
    });

    if(action != ActionNull)
    {
        QBrush brush;
        brush.setStyle(Qt::NoBrush);
        painter->setBrush(brush);
        pen.setWidth(2);
        pen.setStyle(Qt::DotLine);
        pen.setColor(Qt::yellow);
        painter->setPen(pen);


        if(action == ActionResizeFDiag)
        {
            painter->drawRect(QRect(startPoint, calibrate(currentPoint)));
        }
        else if(action == ActionResizeVer)
        {
            QPoint point(startPoint.x() + image.width() * Global::pixelSize, currentPoint.y());
            painter->drawRect(QRect(startPoint, calibrate(point)));
        }
        else if(action == ActionResizeHor)
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
    if(action == ActionNull)
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

    currentPixel.setX((currentPoint.x() - startPoint.x()) / Global::pixelSize);
    currentPixel.setY((currentPoint.y() - startPoint.y()) / Global::pixelSize);
}

void QGraphicsCanvasItem::on_ResizeMousePress(QPoint point)
{
    if(action == ActionNull)
    {
        if(isInSizeFDiagArea(point))
        {
            action = ActionResizeFDiag;
        }
        else if(isInSizeVerArea(point))
        {
            action = ActionResizeVer;
        }
        else if(isInSizeHorArea(point))
        {
            action = ActionResizeHor;
        }
    }
}

void QGraphicsCanvasItem::on_ResizeMouseRelease(QPoint point)
{
    if(action != ActionNull)
    {
        if(action == ActionResizeFDiag)
        {
            resizeImage(image, currentPixel.x(), currentPixel.y());
        }
        else if(action == ActionResizeVer)
        {
            resizeImage(image, image.width(), currentPixel.y());
        }
        else if(action == ActionResizeHor)
        {
            resizeImage(image, currentPixel.x(), image.height());
        }
        else
        {
            return;
        }
        action = ActionNull;
        view->setCursor(Qt::ArrowCursor);
        view->scene()->setSceneRect(QRectF(0, 0, image.width() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset, image.height() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset));
    }
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_MoveMouseMove(QPoint point)
{

}

void QGraphicsCanvasItem::on_MoveMousePress(QPoint point)
{
    QRectF rect(startPoint.x(), startPoint.y(), image.width() * Global::pixelSize, image.height() * Global::pixelSize);

    if(rect.contains(point))
    {
        action = ActionMove;
        moveStartPixel = pointToPixel(point);
        view->setCursor(Qt::SizeAllCursor);
    }
}

void QGraphicsCanvasItem::on_MoveMouseRelease(QPoint point)
{
    if(action == ActionMove)
    {
        action = ActionNull;
        view->setCursor(Qt::ArrowCursor);
        moveImage(image, currentPixel.x() - moveStartPixel.x(), currentPixel.y() - moveStartPixel.y());
    }
}

void QGraphicsCanvasItem::on_Reserve()
{
    QImage newImg(image.width(), image.height(), QImage::Format_RGB888);
    for(int x = 0; x < image.width(); x++)
    {
        for(int y = 0; y < image.height(); y++)
        {
            QColor color = image.pixelColor(x, y);
            color.setRed(255 - color.red());
            color.setGreen(255 - color.green());
            color.setBlue(255 - color.blue());
            newImg.setPixelColor(x, y, color);
        }
    }
    image = newImg;
    update();
}