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
}

DialogProjectSettings::~DialogProjectSettings()
{
    delete ui;
}

void DialogProjectSettings::init(RawData::Settings settings)
{
    switch(settings.mode)
    {
    case ImgEncoderFactory::ZH_LSB:
        ui->radioButton_LSB->setChecked(true);
        ui->radioButton_ZH->setChecked(true);
        break;
    case ImgEncoderFactory::ZH_MSB:
        ui->radioButton_MSB->setChecked(true);
        ui->radioButton_ZH->setChecked(true);
        break;
    case ImgEncoderFactory::ZL_LSB:
        ui->radioButton_LSB->setChecked(true);
        ui->radioButton_ZL->setChecked(true);
        break;
    case ImgEncoderFactory::ZL_MSB:
        ui->radioButton_MSB->setChecked(true);
        ui->radioButton_ZL->setChecked(true);
        break;
    case ImgEncoderFactory::LH_LSB:
        ui->radioButton_LSB->setChecked(true);
        ui->radioButton_LH->setChecked(true);
        break;
    case ImgEncoderFactory::LH_MSB:
        ui->radioButton_MSB->setChecked(true);
        ui->radioButton_LH->setChecked(true);
        break;
    case ImgEncoderFactory::HL_LSB:
        ui->radioButton_LSB->setChecked(true);
        ui->radioButton_HL->setChecked(true);
        break;
    case ImgEncoderFactory::HL_MSB:
        ui->radioButton_MSB->setChecked(true);
        ui->radioButton_HL->setChecked(true);
        break;
    default:
        break;
    }
    mode = settings.mode;
    ui->widget->setMode(mode);

    ui->comboBox_Const->setCurrentText(settings.keywordConst);
    ui->comboBox_PosTypedef->setCurrentText(settings.keywordImgPos);
    ui->comboBox_SizeTypedef->setCurrentText(settings.keywordImgSize);
    ui->lineEdit_Output->setText(settings.path);
    ui->comboBox_Format->setCurrentText(settings.format);
}

RawData::Settings DialogProjectSettings::getResult()
{
    RawData::Settings settings;
    settings.mode = mode;
    settings.keywordConst = ui->comboBox_Const->currentText();
    settings.keywordImgPos = ui->comboBox_PosTypedef->currentText();
    settings.keywordImgSize = ui->comboBox_SizeTypedef->currentText();
    settings.path = ui->lineEdit_Output->text();
    settings.format = ui->comboBox_Format->currentText();
    return settings;
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
