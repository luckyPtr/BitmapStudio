#include "qgraphicscomimgcanvansitem.h"
#include "qwgraphicsview.h"
#include "global.h"


void QGraphicsComImgCanvansItem::paintBackgrand(QPainter *painter)
{
    // 背景使用像素0的颜色填充
    QRect backgroudRect(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize, comImg.height * Global::pixelSize);
    painter->fillRect(backgroudRect, Global::pixelColor_0);

    // 外边框
    QPen pen(Qt::yellow);
    pen.setWidth(2);
    painter->setPen(pen);
    QRectF rect(startPoint.x(), startPoint.y(), comImg.width * Global::pixelSize + 1, comImg.height * Global::pixelSize + 1);
    painter->drawRect(rect);
}

void QGraphicsComImgCanvansItem::paintItems(QPainter *painter)
{
    // 在指定位置绘制单个图形
    auto paintItem = ([=](int x0, int y0, QImage img){
        for(int x = 0; x < img.width(); x++)
        {
            for(int y = 0; y < img.height(); y++)
            {
                QColor color = img.pixelColor(x, y);
                quint8 grayscale  = qGray(color.rgb());
                QRect rect(startPoint.x() + (x0 + x) * Global::pixelSize + 1, startPoint.y() + (y0 + y) * Global::pixelSize + 1, Global::pixelSize - 1, Global::pixelSize - 1);
                painter->fillRect(rect, grayscale < 128 ? Global::pixelColor_1 : Global::pixelColor_0);
            }
        }
    });

    // 绘制图形外框
    auto paintBound = ([=](int x0, int y0, QSize size, int id){
        QPen pen;
        pen.setColor(id == selectedItemId ? Global::selectedItemBoundColor : Global::itemBoundColor);
        painter->setPen(pen);
        QRect rect(startPoint.x() + x0 * Global::pixelSize, startPoint.y() + y0 * Global::pixelSize, Global::pixelSize * size.width(), Global::pixelSize * size.height());
        painter->drawRect(rect);
    });

    foreach (auto item, comImg.items) {
        QImage img = rd->getImage(item.id);
        paintItem(item.x, item.y, img);
        paintBound(item.x, item.y, img.size(), item.id);
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
}

QPoint QGraphicsComImgCanvansItem::pointToPixel(QPoint point)
{
    return QPoint((point.x() - startPoint.x()) / Global::pixelSize,
                  (point.y() - startPoint.y()) / Global::pixelSize);
}

void QGraphicsComImgCanvansItem::setComImg(ComImg &comImg)
{
    this->comImg = comImg;
}

void QGraphicsComImgCanvansItem::setRawData(RawData *rd)
{
    this->rd = rd;
    view->viewport()->update();
    qDebug() << "set rawdata";
}

void QGraphicsComImgCanvansItem::on_MousePress(QPoint point)
{
    bool isSelect = false;
    qDebug() << "Press:" << pointToPixel(point);
    QPoint p = pointToPixel(point);
    foreach(auto item, comImg.items)
    {
        QImage img = rd->getImage(item.id);
        QRect rect(item.x, item.y, img.width(), img.height());
        if(rect.contains(p))
        {
            isSelect = true;
            selectedItemId = item.id;
        }
    }
    if(!isSelect)
        selectedItemId = -1;
    view->viewport()->update();

}

void QGraphicsComImgCanvansItem::on_MouseMove(QPoint point)
{
    qDebug() << "Move:" << point;

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

    connect(view, SIGNAL(mousePress(QPoint)), this, SLOT(on_MousePress(QPoint)));
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
    paintBackgrand(painter);
    paintGrid(painter);
    paintItems(painter);

}
