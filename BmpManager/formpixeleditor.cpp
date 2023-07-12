#include "formpixeleditor.h"
#include "ui_formpixeleditor.h"
#include <QGraphicsRectItem>
#include <QDebug>
#include <global.h>
#include <QScrollBar>
#include "custom/qcustommenu.h"


FormPixelEditor::FormPixelEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPixelEditor)
{
    ui->setupUi(this);
    initAction();

    image = new QImage("C:\\Users\\Naive\\Desktop\\1\\AMQ图标\\大-日期.bmp");

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

    ui->graphicsView->setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);


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




    connect(ui->graphicsView, SIGNAL(paint()), this, SLOT(paintView()));    // todo Lambda
    connect(ui->actReserve, SIGNAL(triggered()), scanvasItem, SLOT(on_Reserve()));
    connect(ui->actCenter, SIGNAL(triggered()), scanvasItem, SLOT(on_Center()));
    connect(ui->actAutoResize, SIGNAL(triggered()), scanvasItem, SLOT(on_AutoResize()));
    connect(ui->actMoveUp, SIGNAL(triggered()), scanvasItem, SLOT(on_MoveUp()));
    connect(ui->actMoveDown, SIGNAL(triggered()), scanvasItem, SLOT(on_MoveDown()));
    connect(ui->actMoveLeft, SIGNAL(triggered()), scanvasItem, SLOT(on_MoveLeft()));
    connect(ui->actMoveRight, SIGNAL(triggered()), scanvasItem, SLOT(on_MoveRight()));
    connect(ui->actFlipHorizontal, SIGNAL(triggered()), scanvasItem, SLOT(on_FlipHor()));
    connect(ui->actFlipVerital, SIGNAL(triggered()), scanvasItem, SLOT(on_FlipVer()));
    connect(ui->actRotateLeft, SIGNAL(triggered()), scanvasItem, SLOT(on_RotateLeft()));
    connect(ui->actRotateRight, SIGNAL(triggered()), scanvasItem, SLOT(on_RotateRight()));
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

void FormPixelEditor::initAction()
{
    ui->toolBtnRotateFlip->setIcon(QIcon(":/Image/PixelEditor/FlipHorizontal.svg"));
    QCustomMenu *menu = new QCustomMenu(this);
    menu->addAction(ui->actFlipHorizontal);
    menu->addAction(ui->actFlipVerital);
    menu->addAction(ui->actRotateLeft);
    menu->addAction(ui->actRotateRight);
    ui->toolBtnRotateFlip->setMenu(menu);

    ui->toolBtnMove->setIcon(QIcon(":/Image/PixelEditor/Move.svg"));
    QCustomMenu *menuMove = new QCustomMenu(this);
    menuMove->addAction(ui->actMoveUp);
    menuMove->addAction(ui->actMoveDown);
    menuMove->addAction(ui->actMoveLeft);
    menuMove->addAction(ui->actMoveRight);
    menuMove->setToolTip("移动");
    ui->toolBtnMove->setMenu(menuMove);

    ui->toolBtnReserve->setDefaultAction(ui->actReserve);
    ui->toolBtnCenter->setDefaultAction(ui->actCenter);
    ui->toolBtnAutoResize->setDefaultAction(ui->actAutoResize);

}

void FormPixelEditor::paintView()
{
    initScrollerPos();
}










