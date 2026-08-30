#include "dialogimportimg.h"
#include "ui_dialogimportimg.h"
#include <QDialog>
#include <QDebug>
#include <QTimer>
#include <QRegExp>
#include <QRegExpValidator>

DialogImportImg::DialogImportImg(QImage &img, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogImportImg)
{
    ui->setupUi(this);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    rawImg = img;

    // 这里直接显示的话图片大小不对，延迟一段时间后显示没有这个问题，奇怪
    QTimer::singleShot(0, this, [=]{
        imgTransform();
        displayImg();
    });
}

DialogImportImg::~DialogImportImg()
{
    delete ui;
}

void DialogImportImg::setImgName(QString name)
{
    ui->lineEditName->setText(name);
}

void DialogImportImg::setBrief(QString breif)
{
    ui->textEditBrief->setText(breif);
}

QString DialogImportImg::getImgName()
{
    return ui->lineEditName->text();
}

QString DialogImportImg::getBrief()
{
    return ui->textEditBrief->toPlainText();
}



void DialogImportImg::imgTransform()
{
    if (rawImg.format() != QImage::Format_RGB888) {
        monoImg = QImage(rawImg).convertToFormat(QImage::Format_RGB888);
    } else {
        monoImg = QImage(rawImg);
    }

    for(int i = 0; i < monoImg.width(); i++)
    {
        for(int j = 0; j < monoImg.height(); j++)
        {
            QColor color = monoImg.pixelColor(i, j);
            //quint8 grayscale = (quint8)(color.red() * 0.3f + color.green() * 0.59f + color.blue() * 0.11f);
            quint8 grayscale  = qGray(color.rgb());
            monoImg.setPixelColor(i, j, QColor((grayscale >= this->grayscale) ? 0xFFFFFF : 0x000000));
        }
    }
}

void DialogImportImg::displayImg()
{
    QImage resultImg = monoImg.scaled(ui->labelPreviewImg->size(), Qt::KeepAspectRatio);
    ui->labelPreviewImg->setPixmap(QPixmap::fromImage(resultImg));
}

void DialogImportImg::on_sliderGrayscale_valueChanged(int value)
{
    grayscale = value;
    ui->labelGrayscaleValue->setText(QString::number(value));
    imgTransform();
    displayImg();
}



void DialogImportImg::on_lineEditName_textChanged(const QString &arg1)
{
    // 有字母数字下划线组成，长度不为0
    QRegExp regx("^[a-zA-Z0-9_]+$");
    if(regx.exactMatch(arg1))
    {
        ui->btnOK->setEnabled(true);
        ui->lineEditName->setStyleSheet("color:black;");
    }
    else
    {
        ui->btnOK->setEnabled(false);
        ui->lineEditName->setStyleSheet("color:#ff4040;");
    }
}

