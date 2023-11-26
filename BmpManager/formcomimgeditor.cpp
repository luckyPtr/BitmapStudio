#include "formcomimgeditor.h"
#include "ui_formcomimgeditor.h"
#include <QScrollBar>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMenu>
#include <custom/qcustommenu.h>
#include <custom/dialogresize.h>


FormComImgEditor::FormComImgEditor(QWidget *parent) :
    CustomTab(parent),
    ui(new Ui::FormComImgEditor)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(QRectF(0, 0, 1, 1));
    ui->graphicsView->setScene(scene);

    scaleItem = new QGraphicsScaleItem(ui->graphicsView);
    scaleItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    comImgCanvansItem = new QGraphicsComImgCanvansItem(ui->graphicsView);
    comImgCanvansItem->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    scene->addItem(comImgCanvansItem);
    scene->addItem(scaleItem);
    QBrush brush;
    brush.setColor(0xcbd4e4);
    brush.setStyle(Qt::SolidPattern);
    scene->setBackgroundBrush(brush);

    ui->graphicsView->setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);

//    initScrollerPos();

    connect(scaleItem, SIGNAL(createAuxLine(Qt::Orientation)), comImgCanvansItem, SLOT(on_CreateAuxLine(Qt::Orientation)));
    connect(ui->actDelete, SIGNAL(triggered()), comImgCanvansItem, SLOT(deleteSelectItem()));
    connect(ui->actClean, SIGNAL(triggered()), comImgCanvansItem, SLOT(deleteAll()));
    connect(ui->actSave, &QAction::triggered, this, [=]{
        emit saveComImg(comImgCanvansItem->getComImg());
    });
    connect(ui->actForward, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_Forward()));
    connect(ui->actBackward, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_Backward()));
    connect(ui->actTop, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_Top()));
    connect(ui->actBottom, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_Bottom()));
    connect(ui->actAlignHCenter, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_AlignVCenter()));
    connect(ui->actAlignVCenter, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_AlignHCenter()));
    connect(ui->actAlignCenter, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_AlignCenter()));
    connect(ui->actMoveUp, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveUp()));
    connect(ui->actMoveDown, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveDown()));
    connect(ui->actMoveLeft, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveLeft()));
    connect(ui->actMoveRight, SIGNAL(triggered()), comImgCanvansItem, SLOT(on_MoveRight()));


    connect(this->comImgCanvansItem, SIGNAL(updateStatusBarPos(QPoint)), this->parent()->parent()->parent(), SLOT(on_UpdateStatusBarPos(QPoint)));
    connect(this->comImgCanvansItem, SIGNAL(updateStatusBarSize(QSize)), this->parent()->parent()->parent(), SLOT(on_UpdateStatusBarSize(QSize)));

    connect(this, SIGNAL(saveComImg(QString,int,ComImg)), this->parent()->parent()->parent(), SLOT(on_SaveComImg(QString,int,ComImg)));

    addAction(ui->actSave);
    addAction(ui->actMoveUp);
    addAction(ui->actMoveDown);
    addAction(ui->actMoveLeft);
    addAction(ui->actMoveRight);
    addAction(ui->actDelete);
}

FormComImgEditor::~FormComImgEditor()
{
    delete comImgCanvansItem;
    delete scaleItem;
    delete scene;
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




void FormComImgEditor::leaveEvent(QEvent *event)
{
    emit updataStatusBarPos(QPoint(-1, -1));
}

void FormComImgEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;

    QCustomMenu menuMove;
    menuMove.setTitle(tr("移动"));
    menuMove.addAction(ui->actMoveUp);
    menuMove.addAction(ui->actMoveDown);
    menuMove.addAction(ui->actMoveLeft);
    menuMove.addAction(ui->actMoveRight);
    menu.addMenu(&menuMove);

    QMenu menuLayer(tr("图层"));
    menuLayer.addAction(ui->actTop);
    menuLayer.addAction(ui->actBottom);
    menuLayer.addAction(ui->actForward);
    menuLayer.addAction(ui->actBackward);
    menu.addMenu(&menuLayer);

    QMenu menuLayout(tr("布局"));
    menuLayout.addAction(ui->actAlignHCenter);
    menuLayout.addAction(ui->actAlignVCenter);
    menuLayout.addAction(ui->actAlignCenter);
    menu.addMenu(&menuLayout);

    menu.addAction(ui->actDelete);
    menu.addAction(ui->actClean);

    menu.addAction(ui->actResize);

    menu.exec(QCursor::pos());
}


void FormComImgEditor::on_LoadComImg(ComImg &comImg, RawData *rd)
{
    comImgCanvansItem->setComImg(comImg);
    comImgCanvansItem->setRawData(rd);
    setSize(QSize(comImg.width, comImg.height));
}





void FormComImgEditor::on_actSave_triggered()
{
    emit saveComImg(getProject(), getId(), comImgCanvansItem->getComImg());
}


void FormComImgEditor::on_actResize_triggered()
{
    DialogResize *dlgResize = new DialogResize(this);
    QSize defaultSize = QSize(comImgCanvansItem->getComImg().width, comImgCanvansItem->getComImg().height);
    dlgResize->setDefaultSize(defaultSize);
    int ret = dlgResize->exec();
    if(ret == QDialog::Accepted)
    {
        QSize size = dlgResize->getSize();
        comImgCanvansItem->resize(size);
    }
    delete dlgResize;
}

