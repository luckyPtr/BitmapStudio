#include "formpixeleditor.h"
#include "ui_formpixeleditor.h"
#include <QGraphicsRectItem>
#include <QDebug>
#include "qgraphicsitemruler.h"
#include <global.h>
#include <QScrollBar>

FormPixelEditor::FormPixelEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPixelEditor)
{
    ui->setupUi(this);

    ui->graphicsView->setCursor(Qt::BitmapCursor);
    QRectF rect(0, 0, 800, 600);
    scene = new QGraphicsScene(rect);
    ui->graphicsView->setScene(scene);

    scaleItem = new QGraphicsScaleItem(ui->graphicsView);
    scaleItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    scene->addItem(scaleItem);

    connect(ui->graphicsView, SIGNAL(paint()), this, SLOT(paintView()));
}

FormPixelEditor::~FormPixelEditor()
{
    delete ui;
}

void FormPixelEditor::drawRuler()
{
    QGraphicsItemRuler *itemRuler = new QGraphicsItemRuler();
    itemRuler->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    scene->addItem(itemRuler);
    itemRuler->setPos(200, 100);

    QGraphicsLineItem *line = new QGraphicsLineItem(0, 0, 200, 100);
    line->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    scene->addItem(line);


//    QGraphicsLineItem *rulerLineX = new QGraphicsLineItem(16, 16, this->width(), 16);
//    QGraphicsLineItem *rulerLineY = new QGraphicsLineItem(16, 16, 16, this->height());
//    QPen pen(Qt::gray);
//    pen.setWidth(1);
//    pen.setJoinStyle(Qt::RoundJoin);
//    rulerLineX->setPen(pen);
//    rulerLineY->setPen(pen);
//    scene->addItem(rulerLineX);
//    scene->addItem(rulerLineY);

//    for(int i = 18; i < this->width(); i += 6)
//    {
//        QGraphicsLineItem *line = new QGraphicsLineItem();
//        if((i - 18) % 60 == 0)
//            line->setLine(i, 0, i, 16);
//        else
//            line->setLine(i, 12, i, 16);
//        line->setPen(pen);
//        scene->addItem(line);
//    }

//    int a = 0;
//    QBrush brush(Qt::gray);

//    pen.setBrush(brush);
//    for(int x = 18; x < width(); x += 10)
//    {
//        for(int y = 18; y < height(); y += 10)
//        {
//            QGraphicsRectItem *item = new QGraphicsRectItem();
//            item->setRect(x, y, 8, 8);
//            item->setPen(pen);
//            scene->addItem(item);
//            a++;
//        }
//    }
    //    qDebug() << "item cnt = " << a;
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
//    scene->clear();
//    drawRuler();
}





