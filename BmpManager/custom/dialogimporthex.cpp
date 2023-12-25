#include "dialogimporthex.h"
#include "ui_dialogimporthex.h"
#include "QDebug"
#include <QRegExp>
#include <QSet>
#include "imgencoderfactory.h"

DialogImportHex::DialogImportHex(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogImportHex)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    ui->plainTextEditBrief->setVisible(false);
    ui->label_Brief->setVisible(false);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

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

DialogImportHex::~DialogImportHex()
{
    delete ui;
}

void DialogImportHex::setDefaultMode(int mode)
{
    this->mode = mode;
    switch(mode)
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
    ui->widget->setMode(mode);
}

QString DialogImportHex::getName()
{
    return ui->lineEditName->text();
}

QString DialogImportHex::getBrief()
{
    return ui->plainTextEditBrief->toPlainText();
}

QImage DialogImportHex::getImg()
{
    return outputImg;
}

void DialogImportHex::on_textEdit_textChanged()
{
    updateImg();
    updateBtnOK();
}

void DialogImportHex::on_btnToggled(int btn, bool checked)
{
    Q_UNUSED(btn)
    Q_UNUSED(checked)

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



void DialogImportHex::on_btnAddBrief_clicked()
{
    bool visible = !ui->plainTextEditBrief->isVisible();
    ui->plainTextEditBrief->setVisible(visible);
    ui->label_Brief->setVisible(visible);
    if(!visible)
    {
        ui->plainTextEditBrief->clear();
    }
}

void DialogImportHex::updateImg()
{
    QString text = ui->textEdit->toPlainText();
    QRegExp rx("0(x|X)[0-9a-fA-F]{1,2}");

    inputByteArray.clear();
    QSet<QString> set;
    int pos = 0;
    while((pos = rx.indexIn(text, pos)) != -1)
    {
        set << rx.cap(0);
        bool ok;
        quint8 hex = rx.cap(0).toInt(&ok, 16);
        inputByteArray.append(hex);
        pos += rx.matchedLength();
    }

    foreach(auto s, set)
    {
        text.replace(s, QString("<font style='background-color: #cff5ff;'>%1</font>").arg(s));
    }
    text.replace("\n", "<br>");
    text.replace("\t", "    ");

    ui->textEdit->blockSignals(true);   // 防止textChanged被反复触发
    ui->textEdit->setText(text);
    ui->textEdit->blockSignals(false);

    ImgEncoder *imgEncoder = ImgEncoderFactory::create(mode);
    outputImg = imgEncoder->decode(inputByteArray, QSize(ui->spinBoxWidth->value(), ui->spinBoxHeight->value()));
    QImage resultImg = outputImg.scaled(ui->labelPreview->size(), Qt::KeepAspectRatio);
    ui->labelPreview->setPixmap(QPixmap::fromImage(resultImg));

    delete imgEncoder;
}

void DialogImportHex::updateBtnOK()
{
    if(!outputImg.isNull() && isNameLegal)
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}


void DialogImportHex::on_spinBoxWidth_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    updateImg();
    updateBtnOK();
}


void DialogImportHex::on_spinBoxHeight_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    updateImg();
    updateBtnOK();
}


void DialogImportHex::on_lineEditName_textChanged(const QString &arg1)
{
    // 有字母数字下划线组成，长度不为0
    QRegExp regx("^[a-zA-Z0-9_]+$");
    if(regx.exactMatch(arg1))
    {
        isNameLegal = true;
        ui->lineEditName->setStyleSheet("color:black;");
    }
    else
    {
        isNameLegal = false;
        ui->lineEditName->setStyleSheet("color:#ff4040;");
    }
    updateBtnOK();
}

