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


    scene->addItem(comImgCanvansItem);
    scene->addItem(scaleItem);
    QBrush brush;
    brush.setColor(0xcbd4e4);
    brush.setStyle(Qt::SolidPattern);
    scene->setBackgroundBrush(brush);

    ui->graphicsView->setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);

    initScrollerPos();
    initAction();

    connect(scaleItem, SIGNAL(createAuxLine(Qt::Orientation)), comImgCanvansItem, SLOT(on_CreateAuxLine(Qt::Orientation)));
    connect(ui->actDelete, SIGNAL(triggered()), comImgCanvansItem, SLOT(deleteSelectItem()));
    connect(ui->actSave, &QAction::triggered, this, [=]{
        emit saveComImg(comImgCanvansItem->getComImg());
    });
    connect(ui->actMoveUp, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveUp()));
    connect(ui->actMoveDown, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveDown()));
    connect(ui->actMoveTop, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveTop()));
    connect(ui->actMoveBottom, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveBottom()));

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

void FormComImgEditor::initAction()
{
    ui->toolButtonDelete->setDefaultAction(ui->actDelete);
    ui->toolButtonSave->setDefaultAction(ui->actSave);
    ui->toolButtonMoveUp->setDefaultAction(ui->actMoveUp);
    ui->toolButtonMoveDown->setDefaultAction(ui->actMoveDown);
    ui->toolButtonMoveTop->setDefaultAction(ui->actMoveTop);
    ui->toolButtonMoveBottom->setDefaultAction(ui->actMoveBottom);
}

void FormComImgEditor::leaveEvent(QEvent *event)
{
    emit updataStatusBarPos(QPoint(-1, -1));
}


void FormComImgEditor::on_LoadComImg(ComImg &comImg, RawData *rd)
{
    comImgCanvansItem->setComImg(comImg);
    comImgCanvansItem->setRawData(rd);
}




