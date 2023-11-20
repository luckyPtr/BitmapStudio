#include "dialogresize.h"
#include "ui_dialogresize.h"

DialogResize::DialogResize(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogResize)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    QPixmap pixemapWidth = QPixmap(":/Image/PixelEditor/Width.svg");
    pixemapWidth = pixemapWidth.scaled(QSize(48, 48), Qt::KeepAspectRatio);
    ui->labelWidth->setPixmap(pixemapWidth);

    QPixmap pixemapHeight = QPixmap(":/Image/PixelEditor/Height.svg");
    pixemapHeight = pixemapHeight.scaled(QSize(48, 48), Qt::KeepAspectRatio);
    ui->labelHeight->setPixmap(pixemapHeight);
}

DialogResize::~DialogResize()
{
    delete ui;
}

void DialogResize::setDefaultSize(QSize size)
{
    ui->spinBoxWidth->setValue(size.width());
    ui->spinBoxHeitght->setValue(size.height());
}

QSize DialogResize::getSize()
{
    QSize size(ui->spinBoxWidth->value(), ui->spinBoxHeitght->value());
    return size;
}
