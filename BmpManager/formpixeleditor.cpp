#include "formpixeleditor.h"
#include "ui_formpixeleditor.h"
#include <QGraphicsRectItem>
#include <QDebug>
#include <global.h>
#include <QScrollBar>



FormPixelEditor::FormPixelEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPixelEditor)
{
    ui->setupUi(this);

    image = new QImage("C:\\Users\\Naive\\Desktop\\1\\AMQ图标\\大-日期.bmp");
    qDebug() << image->size();

    ui->graphicsView->setCursor(Qt::BitmapCursor);
    QRectF rect(0, 0, image->width() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset, image->height() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset);
    scene = new QGraphicsScene(rect);
    ui->graphicsView->setScene(scene);

    scaleItem = new QGraphicsScaleItem(ui->graphicsView);
    scaleItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    scanvasItem = new QGraphicsCanvasItem(ui->graphicsView);
    scanvasItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    scanvasItem->setImage(*image);

    scene->addItem(scaleItem);
    scene->addItem(scanvasItem);
    QBrush brush;
    brush.setColor(0xcbd4e4);
    brush.setStyle(Qt::SolidPattern);
    scene->setBackgroundBrush(brush);

//    QPen pen;
//    brush.setColor(QColor(180, 180, 180, 60));
//    pen.setColor(QColor(180, 180, 180, 60));
//    QGraphicsRectItem *rectSItem = new QGraphicsRectItem;
//    rectSItem->setRect(QRect(26, 26, 620, 420));
//    rectSItem->setBrush(brush);
//    rectSItem->setPen(pen);
//    scene->addItem(rectSItem);

//    QGraphicsRectItem *rectItem = new QGraphicsRectItem;
//    QRectF rect1(20, 20, 620, 420);
//    brush.setColor(Qt::white);
//    rectItem->setBrush(brush);

//    pen.setColor(Qt::white);
//    rectItem->setPen(pen);
//    rectItem->setRect(rect1);
//    scene->addItem(rectItem);




    connect(ui->graphicsView, SIGNAL(paint()), this, SLOT(paintView()));
}

FormPixelEditor::~FormPixelEditor()
{
    delete ui;
}



void FormPixelEditor::initScrollerPos()
{
    // 将scene的初始滚动条位置设置为左上角
    // 必须要QGraphicsView绘制过一次后，重新设置滚轮位置才有效
    static bool flag = true;
    if(flag)
    {
        flag = false;
        ui->graphicsView->horizontalScrollBar()->setSliderPosition(0);
        ui->graphicsView->verticalScrollBar()->setSliderPosition(0);
    }
}

void FormPixelEditor::paintView()
{
    initScrollerPos();
}





