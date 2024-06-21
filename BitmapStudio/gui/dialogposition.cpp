#include "dialogposition.h"
#include "ui_dialogposition.h"
#include <QPushButton>
#include <QTimer>
#include <QKeyEvent>

DialogPosition::DialogPosition(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogPosition)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus(Qt::ActiveWindowFocusReason); // 打开窗口后设置焦点
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
