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

    //image = new QImage("C:\\Users\\Naive\\Desktop\\1\\AMQ图标\\大-日期.bmp");

    //QRectF rect(0, 0, image->width() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset, image->height() * Global::pixelSize + Global::scaleWidth + Global::scaleOffset);
    scene = new QGraphicsScene(QRectF(0, 0, 1, 1));
    ui->graphicsView->setScene(scene);

    scaleItem = new QGraphicsScaleItem(ui->graphicsView);
    scaleItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    scanvasItem = new QGraphicsCanvasItem(ui->graphicsView);
    scanvasItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
   // scanvasItem->setImage(*image);

    scene->addItem(scaleItem);
    scene->addItem(scanvasItem);
    QBrush brush;
    brush.setColor(0xcbd4e4);
    brush.setStyle(Qt::SolidPattern);
    scene->setBackgroundBrush(brush);

    ui->graphicsView->setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);



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


    //connect(parent->parent(), SIGNAL(selectItem(QImage&)), this, SLOT(on_LoadImage(QImage&)));
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
    ui->toolBntSelect->setDefaultAction(ui->actSelect);
    ui->toolBntEdit->setDefaultAction(ui->actEdit);
    ui->toolBntMeasure->setDefaultAction(ui->actMeasure);
    ui->toolBtnSave->setDefaultAction(ui->actSave);
}

void FormPixelEditor::paintView()
{
    initScrollerPos();
}

void FormPixelEditor::on_LoadImage(QImage &image)
{
    scanvasItem->setImage(image);
}




void FormPixelEditor::on_actSelect_triggered()
{
    ui->actSelect->setChecked(true);
    ui->actEdit->setChecked(false);
    ui->actMeasure->setChecked(false);
    scanvasItem->setMode(QGraphicsCanvasItem::SelectMode);
}


void FormPixelEditor::on_actEdit_triggered()
{
    ui->actSelect->setChecked(false);
    ui->actEdit->setChecked(true);
    ui->actMeasure->setChecked(false);
    scanvasItem->setMode(QGraphicsCanvasItem::EditMode);
}


void FormPixelEditor::on_actMeasure_triggered()
{
    ui->actSelect->setChecked(false);
    ui->actEdit->setChecked(false);
    ui->actMeasure->setChecked(true);
    scanvasItem->setMode(QGraphicsCanvasItem::MeasureMode);
}


void FormPixelEditor::on_actSave_triggered()
{
    emit saveImage(scanvasItem->getImage());
}

