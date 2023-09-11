#include "qgraphicscanvasitem.h"
#include "qwgraphicsview.h"
#include "global.h"
#include <QDebug>
#include <QPen>
#include <QImage>
#include <QtConcurrent/QtConcurrent>

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

bool QGraphicsCanvasItem::isInImgArea(QPoint point)
{
    QRectF rect(startPoint.x(), startPoint.y(), image.width() * Global::pixelSize, image.height() * Global::pixelSize);
    return rect.contains(point);
}

void QGraphicsCanvasItem::resizeImage(QImage &img, QSize size)
{
    QImage newImage(size.width(), size.height(), QImage::Format_RGB888);
    newImage.fill(Qt::white);

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
    QImage newImg(img.size(), QImage::Format_RGB888);
    newImg.fill(Qt::white);

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

void QGraphicsCanvasItem::reserveImage(QImage &img)
{
    QImage newImg(img.size(), QImage::Format_RGB888);
    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            QColor color = image.pixelColor(x, y);
            color.setRgb(0xff - color.red(), 0xff - color.green(), 0xff - color.blue());
            newImg.setPixelColor(x, y, color);
        }
    }
    img = newImg;
}

void QGraphicsCanvasItem::flipHor(QImage &img)
{
    QImage newImg(img.size(), QImage::Format_RGB888);
    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            newImg.setPixelColor(img.width() - 1 - x, y, img.pixelColor(x, y));
        }
    }
    img = newImg;
}

void QGraphicsCanvasItem::flipVer(QImage &img)
{
    QImage newImg(img.size(), QImage::Format_RGB888);
    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            newImg.setPixelColor(x, img.height() - 1 - y, img.pixelColor(x, y));
        }
    }
    img = newImg;
}

void QGraphicsCanvasItem::rotateLeft(QImage &img)
{
    QImage newImg(img.height(), img.width(), QImage::Format_RGB888);
    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            newImg.setPixelColor(y, img.width() - 1 - x, img.pixelColor(x, y));
        }
    }
    img = newImg;
}

void QGraphicsCanvasItem::rotateRight(QImage &img)
{
    QImage newImg(img.height(), img.width(), QImage::Format_RGB888);
    for(int x = 0; x < img.width(); x++)
    {
        for(int y = 0; y < img.height(); y++)
        {
            newImg.setPixelColor(img.height() - 1 - y, x, img.pixelColor(x, y));
        }
    }
    img = newImg;
}

void QGraphicsCanvasItem::getMargin(int &up, int &down, int &left, int &right)
{
    auto isEmptyRow = ([=](int row){
        for(int i = 0; i < image.width(); i++)
        {
            if(qGray(image.pixelColor(i, row).rgb()) < 128)
                return false;
        }
        return true;
    });

    auto isEmptyCol = ([=](int col){
        for(int i = 0; i < image.height(); i++)
        {

            if(qGray(image.pixelColor(col, i).rgb()) < 128)
                return false;
        }
        return true;
    });

    for(int i = 0; i < image.height(); i++)
    {
        if(!isEmptyRow(i))
        {
            up = i;
            break;
        }
        // 全部为空，则直接退出
        if(i == image.height() - 1)
        {
            return;
        }
    }

    for(int i = 0; i < image.height(); i++)
    {
        if(!isEmptyRow(image.height() - 1 -i))
        {
            down = i;
            break;
        }
    }

    for(int i = 0; i < image.width(); i++)
    {
        if(!isEmptyCol(i))
        {
            left = i;
            break;
        }
    }

    for(int i = 0; i < image.width(); i++)
    {
        if(!isEmptyCol(image.width() - 1 - i))
        {
            right = i;
            break;
        }
    }
}

void QGraphicsCanvasItem::drawPoint(QImage &img, int x, int y, bool dot)
{
    if(x < img.width() && y < img.height())
    {
        img.setPixelColor(x, y, QColor(dot ? Qt::black : Qt::white));
    }
}

void QGraphicsCanvasItem::drawPoint(QImage &img, QPoint point, bool dot)
{
    if(point.x() < img.width() && point.y() < img.height())
    {
        img.setPixelColor(point, QColor(dot ? Qt::black : Qt::white));
    }
}

QGraphicsCanvasItem::QGraphicsCanvasItem(QWidget *parent)
{
    view = static_cast<QWGraphicsView*>(parent);
    // 初始化左上角0点坐标
    startPoint.setX(Global::scaleWidth + Global::scaleOffset);
    startPoint.setY(Global::scaleWidth + Global::scaleOffset);
    action = ActionNull;
    mode = SelectMode;

    connect(view, SIGNAL(mousePress(QPoint)), this, SLOT(on_MousePressLeft(QPoint)));
    connect(view, SIGNAL(mousePressMiddle(QPoint)), this, SLOT(on_MousePressMiddle(QPoint)));
    connect(view, SIGNAL(mousePressRight(QPoint)), this, SLOT(on_MousePressRight(QPoint)));
    connect(view, SIGNAL(mouseMovePoint(QPoint)), this, SLOT(on_MouseMove(QPoint)));
    connect(view, SIGNAL(mouseRelease(QPoint)), this, SLOT(on_MouseRelease(QPoint)));
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

    if(image.isNull())
        return;

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
            painter->fillRect(rect, grayscale < 128 ? Global::pixelColor_1 : Global::pixelColor_0);
        }
    }

    // 绘制网格
    QPen pen(Global::gridColor);
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
    if(image.isNull())
    {
        //return;
    }
    // 虽然是单色的，转为为RGB888，像素处理的时候方便一点
    this->image = image.convertToFormat(QImage::Format_RGB888);
    view->viewport()->update();
}

QImage QGraphicsCanvasItem::getImage()
{
    return image;
}

void QGraphicsCanvasItem::setMode(quint8 mode)
{
    this->mode = mode;
}

void QGraphicsCanvasItem::on_MouseMove(QPoint point)
{
    currentPoint = point;
    currentPixel.setX((currentPoint.x() - startPoint.x()) / Global::pixelSize);
    currentPixel.setY((currentPoint.y() - startPoint.y()) / Global::pixelSize);

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
    else if(action == ActionWrite)
    {
        static QPoint pointLast;
        QPoint pixelPoint = pointToPixel(point);
        if(pointLast != pixelPoint)
        {
            pointLast = pixelPoint;
            drawPoint(image, pointToPixel(point), true);
        }
        //view->viewport()->update();
    }
    else if(action == ActionErase)
    {
        drawPoint(image, pointToPixel(point), false);
    }
    else if(action == ActionResizeFDiag)
    {
        newSize = QSize(currentPixel.x(), currentPixel.y());
        emit updataStatusBarSize(newSize);
    }
    else if(action == ActionResizeVer)
    {
        newSize = QSize(image.size().width(), currentPixel.y());
        emit updataStatusBarSize(newSize);
    }
    else if(action == ActionResizeHor)
    {
        newSize = QSize(currentPixel.x(), image.size().height());
        emit updataStatusBarSize(newSize);
    }

    emit updataStatusBarPos(currentPixel);
}

void QGraphicsCanvasItem::on_MousePressLeft(QPoint point)
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
        else
        {
            if(mode == EditMode)
            {
                action = ActionWrite;
                drawPoint(image, pointToPixel(point), true);
                view->viewport()->update();
            }
        }
    }
}

void QGraphicsCanvasItem::on_MousePressMiddle(QPoint point)
{
    if(isInImgArea(point))
    {
        action = ActionMove;
        moveStartPixel = pointToPixel(point);
        view->setCursor(Qt::SizeAllCursor);
    }
}

void QGraphicsCanvasItem::on_MousePressRight(QPoint point)
{
    if(mode == EditMode)
    {
        action = ActionErase;
        drawPoint(image, pointToPixel(point), false);
        view->viewport()->update();
    }
}

void QGraphicsCanvasItem::on_MouseRelease(QPoint point)
{
    Q_UNUSED(point);
    if(action != ActionNull)
    {
        if(action == ActionResizeFDiag || action == ActionResizeVer || action == ActionResizeHor)
        {
            resizeImage(image, newSize);
            view->scene()->setSceneRect(QRectF(0, 0, image.width() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset, image.height() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset));
        }
        else if(action == ActionMove)
        {
            moveImage(image, currentPixel.x() - moveStartPixel.x(), currentPixel.y() - moveStartPixel.y());
        }
        view->setCursor(Qt::ArrowCursor);
        action = ActionNull;
    }
    view->viewport()->update();
}



void QGraphicsCanvasItem::on_Reserve()
{
    reserveImage(image);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_Center()
{
    int upMargin, downMargin, leftMargin, rightMargin;  // 图片离画布边缘的距离
    getMargin(upMargin, downMargin, leftMargin, rightMargin);
    moveImage(image, (leftMargin + rightMargin) / 2 - leftMargin, (upMargin + downMargin) / 2 - upMargin);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_AutoResize()
{
    int upMargin, downMargin, leftMargin, rightMargin;  // 图片离画布边缘的距离
    getMargin(upMargin, downMargin, leftMargin, rightMargin);
    moveImage(image, -leftMargin, -upMargin);   // 图形移到左上角
    resizeImage(image, QSize(image.width() - leftMargin - rightMargin, image.height() - upMargin - downMargin));
    view->scene()->setSceneRect(QRectF(0, 0, image.width() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset, image.height() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset));
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_MoveUp()
{
    moveImage(image, 0, -1);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_MoveDown()
{
    moveImage(image, 0, 1);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_MoveLeft()
{
    moveImage(image, -1, 0);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_MoveRight()
{
    moveImage(image, 1, 0);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_FlipHor()
{
    flipHor(image);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_FlipVer()
{
    flipVer(image);
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_RotateLeft()
{
    rotateLeft(image);
    view->scene()->setSceneRect(QRectF(0, 0, image.width() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset, image.height() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset));
    view->viewport()->update();
}

void QGraphicsCanvasItem::on_RotateRight()
{
    rotateRight(image);
    view->scene()->setSceneRect(QRectF(0, 0, image.width() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset, image.height() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset));
    view->viewport()->update();
}
