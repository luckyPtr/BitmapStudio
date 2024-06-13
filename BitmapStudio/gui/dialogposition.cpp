#include "dialogposition.h"
#include "ui_dialogposition.h"

DialogPosition::DialogPosition(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogPosition)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？
}

DialogPosition::~DialogPosition()
{
    delete ui;
}

void DialogPosition::setDefaultPos(QPoint pos)
{
    ui->spinBoxX->setValue(pos.x());
    ui->spinBoxY->setValue(pos.y());
}

QPoint DialogPosition::getPos()
{
    return QPoint(ui->spinBoxX->value(), ui->spinBoxY->value());
}
