#include "qgraphicscomimgcanvansitem.h"
#include "qwgraphicsview.h"
#include "global.h"
#include <QGraphicsSceneDragDropEvent>


void QGraphicsComImgCanvansItem::paintBackground(QPainter *painter)
{
    // 背景使用像素0的颜色填充
    QRect backgroudRect(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize, comImg.height * Global::pixelSize);
    painter->fillRect(backgroudRect, Global::pixelColor_0);


}

void QGraphicsComImgCanvansItem::paintItems(QPainter *painter)
{
    // 在指定位置绘制单个图形
    auto paintItem = ([=](int x0, int y0, QImage img){
        QRect canvasRect(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize, comImg.height * Global::pixelSize);

        QRect rect(startPoint.x() + x0 * Global::pixelSize, startPoint.y() + y0 * Global::pixelSize, Global::pixelSize * img.width(), Global::pixelSize * img.height());
        painter->fillRect(rect, Global::gridColor);
        for(int x = 0; x < img.width(); x++)
        {
            for(int y = 0; y < img.height(); y++)
            {
                QColor color = img.pixelColor(x, y);
                quint8 grayscale  = qGray(color.rgb());
                QRect rect(startPoint.x() + (x0 + x) * Global::pixelSize + 1, startPoint.y() + (y0 + y) * Global::pixelSize + 1, Global::pixelSize - 1, Global::pixelSize - 1);
                if(canvasRect.contains(rect.topLeft()))
                    painter->fillRect(rect, grayscale < 128 ? Global::pixelColor_1 : Global::pixelColor_0);
            }
        }
    });

    // 绘制图形外框
    auto paintBound = ([=](int x0, int y0, QSize size, int index){
        QPen pen;
        pen.setColor(index == selectedItemIndex ? Global::selectedItemBoundColor : Global::itemBoundColor);
        pen.setWidth(2);
        painter->setPen(pen);

        QRect rect(startPoint.x() + x0 * Global::pixelSize, startPoint.y() + y0 * Global::pixelSize, Global::pixelSize * size.width(), Global::pixelSize * size.height());
        painter->drawRect(rect);
    });

    for(int i = 0; i < comImg.items.size(); i++)
    {
        ComImgItem item = comImg.items[i];
        QImage img = rd->getImage(item.id);
        paintItem(item.x, item.y, img);
        paintBound(item.x, item.y, img.size(), i);
    }
}

void QGraphicsComImgCanvansItem::paintGrid(QPainter *painter)
{
    // 网格
    QPen pen(Global::gridColor);
    painter->setPen(pen);
    for(int x = 0; x < comImg.width; x++)
    {
        painter->drawLine(startPoint.x() + x * Global::pixelSize, startPoint.y(), startPoint.x() + x * Global::pixelSize, startPoint.y() + comImg.height * Global::pixelSize);
    }
    for(int y = 0; y < comImg.height; y++)
    {
        painter->drawLine(startPoint.x(), startPoint.y() + y * Global::pixelSize, startPoint.x() + comImg.width * Global::pixelSize, startPoint.y() + y * Global::pixelSize);
    }

    // 外边框
    pen.setColor(Qt::yellow);
    pen.setWidth(2);
    painter->setPen(pen);
    QRectF rect(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize + 1, comImg.height * Global::pixelSize + 1);
    painter->drawRect(rect);
}

void QGraphicsComImgCanvansItem::paintDragItem(QPainter *painter)
{
    // 在指定位置绘制单个图形
    auto paintItem = ([&](int x0, int y0, QImage img){
        QRect canvasRect(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize, comImg.height * Global::pixelSize);
        for(int x = 0; x < img.width(); x++)
        {
            for(int y = 0; y < img.height(); y++)
            {
                QColor color = img.pixelColor(x, y);
                quint8 grayscale  = qGray(color.rgb());
                QRect rect(startPoint.x() + (x0 + x) * Global::pixelSize + 1, startPoint.y() + (y0 + y) * Global::pixelSize + 1, Global::pixelSize - 1, Global::pixelSize - 1);
                if(canvasRect.contains(rect.topLeft()))
                    painter->fillRect(rect, grayscale < 128 ? Global::pixelColor_1 : Global::pixelColor_0);
            }
        }
        QPen pen(Global::selectedItemBoundColor);
        pen.setWidth(2);
        painter->setPen(pen);
        QRect rect(startPoint.x() + x0 * Global::pixelSize, startPoint.y() + y0 * Global::pixelSize, Global::pixelSize * img.width(), Global::pixelSize * img.height());
        painter->drawRect(rect);
    });


    if(isDragImg)
    {
        QImage img = rd->getImage(dragImgId);
        paintItem(currentPixel.x() - img.width() / 2, currentPixel.y() - img.height() / 2, img);
    }
}

QPoint QGraphicsComImgCanvansItem::pointToPixel(QPoint point)
{
    return QPoint((point.x() - startPoint.x()) / Global::pixelSize,
                  (point.y() - startPoint.y()) / Global::pixelSize);
}



int QGraphicsComImgCanvansItem::getPointImgIndex(QPoint point)
{
    int index = -1;
    for(int i = 0; i < comImg.items.size(); i++)
    {
        QImage img = rd->getImage(comImg.items[i].id);
        QRect rect(comImg.items[i].x, comImg.items[i].y, img.width(), img.height());
        if(rect.contains(pointToPixel(point)))
        {
            index = i;
        }
    }
    return index;
}

void QGraphicsComImgCanvansItem::setComImg(ComImg &comImg)
{
    this->comImg = comImg;
}

void QGraphicsComImgCanvansItem::setRawData(RawData *rd)
{
    this->rd = rd;
    view->viewport()->update();
}

void QGraphicsComImgCanvansItem::deleteSelectItem()
{
    if(selectedItemIndex != -1)
    {
        comImg.items.removeAt(selectedItemIndex);
        selectedItemIndex = -1;
        view->viewport()->update();
    }
}




void QGraphicsComImgCanvansItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if(event->mimeData()->hasFormat("bm/type"))
    {
        // 仅接收图片
        if(event->mimeData()->data("bm/type") == "image")
        {
            isDragImg = true;
            dragImgId = event->mimeData()->data("bm/id").toInt();
            selectedItemIndex = -1;
            event->setAccepted(true);
            return;
        }
    }
    event->setAccepted(false);
}

void QGraphicsComImgCanvansItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    isDragImg = false;
    view->viewport()->update();
}

void QGraphicsComImgCanvansItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    isDragImg = false;
    QImage img = rd->getImage(dragImgId);
    ComImgItem item(currentPixel.x() - img.width() / 2, currentPixel .y() - img.height() / 2, dragImgId);
    comImg.items << item;
}

void QGraphicsComImgCanvansItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    currentPoint = event->pos().toPoint();
    currentPixel = pointToPixel(currentPoint);
    view->viewport()->update();
}

void QGraphicsComImgCanvansItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
//    qDebug() << "mouseMoveEvent";
}

void QGraphicsComImgCanvansItem::on_MousePress(QPoint point)
{
    selectedItemIndex = getPointImgIndex(point);

    if(action == ActionNull)
    {
        if(selectedItemIndex != -1)
        {
            action = ActionSelect;
            moveStartPixel = currentPixel;
        }
    }

    view->viewport()->update();
}

void QGraphicsComImgCanvansItem::on_MouseMove(QPoint point)
{
    auto itemMove([&](){
        if(currentPixel != moveLastPixel)
        {
            // TODO：获取指定ID的图片
            if(selectedItemIndex < comImg.items.size())
            {
                comImg.items[selectedItemIndex].x += currentPixel.x() - moveLastPixel.x();
                comImg.items[selectedItemIndex].y += currentPixel.y() - moveLastPixel.y();
            }
            moveLastPixel = currentPixel;
        }
    });

    currentPoint = point;
    currentPixel = pointToPixel(point);

    if(action == ActionSelect)
    {
        action = ActionMove;
        moveLastPixel = currentPixel;
        view->setCursor(Qt::SizeAllCursor);
    }
    else if(action == ActionMove)
    {
        itemMove();
    }
}

void QGraphicsComImgCanvansItem::on_MouseRelease(QPoint point)
{
    if(action == ActionSelect || action == ActionMove)
    {
        action = ActionNull;
        view->setCursor(Qt::ArrowCursor);
    }
}

QGraphicsComImgCanvansItem::QGraphicsComImgCanvansItem(QObject *parent)
    : QObject{parent}
{
    view = static_cast<QWGraphicsView*>(parent);
    // 初始化左上角0点坐标
    startPoint.setX(Global::scaleWidth + Global::scaleOffset);
    startPoint.setY(Global::scaleWidth + Global::scaleOffset);
    comImg.height = 64;
    comImg.width = 128;

    this->setAcceptHoverEvents(true);
    this->setAcceptDrops(true);



    connect(view, SIGNAL(mousePress(QPoint)), this, SLOT(on_MousePress(QPoint)));
    connect(view, SIGNAL(mouseMovePoint(QPoint)), this, SLOT(on_MouseMove(QPoint)));
    connect(view, SIGNAL(mouseRelease(QPoint)), this, SLOT(on_MouseRelease(QPoint)));
}

QRectF QGraphicsComImgCanvansItem::boundingRect() const
{
    return QRectF(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize, comImg.height * Global::pixelSize);
}

QPainterPath QGraphicsComImgCanvansItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void QGraphicsComImgCanvansItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    paintBackground(painter);
    paintGrid(painter);
    paintItems(painter);
    paintDragItem(painter);
}
