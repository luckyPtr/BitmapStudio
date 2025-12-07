#include "dialogprojectsettings.h"
#include "ui_dialogprojectsettings.h"
#include <QDebug>
#include <QPushButton>

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

    // 选择bin输出的时候显示图片地址类型
    connect(ui->comboBox_Format, &QComboBox::currentTextChanged, [=](QString text) {
        bool visible = text == "bin";
        ui->lineEdit_Addr->setVisible(visible);
        ui->label_Addr->setVisible(visible);
    });
    emit ui->comboBox_Format->currentTextChanged(ui->comboBox_Format->currentText());


    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));

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

    ui->lineEdit_Const->setText(settings.keywordConst);
    ui->lineEdit_PosType->setText(settings.keywordImgPos);
    ui->lineEdit_Addr->setText(settings.keywordImgAddr);
    ui->lineEdit_Output->setText(settings.path);
    ui->comboBox_Format->setCurrentText(settings.format);
    ui->lineEdit_Brief->setText(settings.brief);
}

RawData::Settings DialogProjectSettings::getResult()
{
    RawData::Settings settings;
    settings.mode = mode;
    settings.keywordConst = ui->lineEdit_Const->text();
    settings.keywordImgPos = ui->lineEdit_PosType->text();
    settings.keywordImgSize = ui->lineEdit_PosType->text();
    settings.keywordImgAddr = ui->lineEdit_Addr->text();
    settings.path = ui->lineEdit_Output->text();
    settings.format = ui->comboBox_Format->currentText();
    settings.brief = ui->lineEdit_Brief->text();
    return settings;
}

QString DialogProjectSettings::getConst()
{
    return ui->lineEdit_Const->text();
}

QString DialogProjectSettings::getPosType()
{
    return ui->lineEdit_PosType->text();
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
