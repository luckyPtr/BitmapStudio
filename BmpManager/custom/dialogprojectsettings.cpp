#include "dialogprojectsettings.h"
#include "ui_dialogprojectsettings.h"
#include <QDebug>

DialogProjectSettings::DialogProjectSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProjectSettings)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    btnGroup1 = new QButtonGroup(this);
    btnGroup1->addButton(ui->radioButton_LSB, 0);
    btnGroup1->addButton(ui->radioButton_MSB, 1);

    btnGroup2 = new QButtonGroup(this);
    btnGroup2->addButton(ui->radioButton_ZL, 0);
    btnGroup2->addButton(ui->radioButton_ZH, 1);
    btnGroup2->addButton(ui->radioButton_LH, 2);
    btnGroup2->addButton(ui->radioButton_HL, 3);

    connect(btnGroup1, SIGNAL(idToggled(int,bool)), this, SLOT(on_btnToggled(int,bool)));
    connect(btnGroup2, SIGNAL(idToggled(int,bool)), this, SLOT(on_btnToggled(int,bool)));

    ui->radioButton_LSB->setChecked(true);
    ui->radioButton_ZL->setChecked(true);
}

DialogProjectSettings::~DialogProjectSettings()
{
    delete ui;
}

QString DialogProjectSettings::getConst()
{
    return ui->comboBox_Const->currentText();
}

QString DialogProjectSettings::getPosType()
{
    return ui->comboBox_PosTypedef->currentText();
}

QString DialogProjectSettings::getSizeType()
{
    return ui->comboBox_SizeTypedef->currentText();
}

QString DialogProjectSettings::getOutputPath()
{
    return ui->lineEdit_Output->text();
}

QString DialogProjectSettings::getOutputFormat()
{
    return ui->comboBox_Format->currentText();
}

void DialogProjectSettings::on_btnToggled(int btn, bool checked)
{
    switch(btnGroup2->checkedId())
    {
    case 0:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::ZL_LSB : ImgEncoderFactory::ZL_MSB;
        break;
    case 1:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::ZH_LSB : ImgEncoderFactory::ZH_MSB;
        break;
    case 2:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::LH_LSB : ImgEncoderFactory::LH_MSB;
        break;
    case 3:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::HL_LSB : ImgEncoderFactory::HL_MSB;
        break;
    }
    ui->widget->setMode(mode);
}
