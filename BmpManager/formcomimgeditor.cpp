#include "formcomimgeditor.h"
#include "ui_formcomimgeditor.h"
#include <QScrollBar>
#include <QDragMoveEvent>
#include <QDragEnterEvent>

FormComImgEditor::FormComImgEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormComImgEditor)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(QRectF(0, 0, 1, 1));
    ui->graphicsView->setScene(scene);

    scaleItem = new QGraphicsScaleItem(ui->graphicsView);
    scaleItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    comImgCanvansItem = new QGraphicsComImgCanvansItem(ui->graphicsView);
    comImgCanvansItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
//    scanvasItem = new QGraphicsCanvasItem(ui->graphicsView);
//    scanvasItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    scene->addItem(scaleItem);
    scene->addItem(comImgCanvansItem);
    QBrush brush;
    brush.setColor(0xcbd4e4);
    brush.setStyle(Qt::SolidPattern);
    scene->setBackgroundBrush(brush);

    ui->graphicsView->setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);

    initScrollerPos();
}

FormComImgEditor::~FormComImgEditor()
{
    delete ui;
}

void FormComImgEditor::initScrollerPos()
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

//void FormComImgEditor::dragEnterEvent(QDragEnterEvent *event)
//{
//    event->accept();
//    qDebug() << "Drop Enter";
//}

//void FormComImgEditor::dropEvent(QDropEvent *event)
//{
//    event->accept();
//    qDebug() << "Drop event";
//}

//void FormComImgEditor::dragMoveEvent(QDragMoveEvent *event)
//{
//    event->ignore();
//    qDebug() << "Drag mvoe";
//}

void FormComImgEditor::on_LoadComImg(ComImg &comImg, RawData *rd)
{
    comImgCanvansItem->setComImg(comImg);
    comImgCanvansItem->setRawData(rd);
    qDebug() << "Load ComImg:" << rd->getProject();
}
