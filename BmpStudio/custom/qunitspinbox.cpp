#include "qunitspinbox.h"
#include <QBoxLayout>

QUnitSpinBox::QUnitSpinBox(QWidget *parent)
    : QSpinBox{parent}
{
    label = new QLabel(tr("像素 "), this);
    label->setStyleSheet("color: rgba(150, 150, 150, 1);");
    //单位加入布局
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addStretch();
    layout->setSpacing(0);
    //单位按钮的Margins
    layout->setContentsMargins(0, 0, 13, 0);
    layout->addWidget(label);
    this->setLayout(layout);
}

QUnitSpinBox::~QUnitSpinBox()
{
    delete label;
}

void QUnitSpinBox::setUnitText(const QString &unit)
{
    unitText = unit;
}
