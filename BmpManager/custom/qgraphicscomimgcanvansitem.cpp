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
                QRect rect(startPoint.x() + (x0 + x) * Global::pixelSize, startPoint.y() + (y0 + y) * Global::pixelSize, Global::pixelSize, Global::pixelSize);
                painter->fillRect(rect, grayscale < 128 ? Global::pixelColor_1 : Global::pixelColor_0);
            }
        }
    });

    foreach (auto item, comImg.items) {
        QImage img = rd->getImage(item.id);
        paintItem(item.x, item.y, img);
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

void QGraphicsComImgCanvansItem::setComImg(ComImg &comImg)
{
    this->comImg = comImg;
}

void QGraphicsComImgCanvansItem::setRawData(RawData *rd)
{
    this->rd = rd;
    view->viewport()->update();
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
    paintItems(painter);
    paintGrid(painter);
}
