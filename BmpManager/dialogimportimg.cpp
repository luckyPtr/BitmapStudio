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
    rawImg = img;

    // 限制图片名称输入格式同C语言变量命名规则
    QRegExp regx("^[a-zA-Z_][a-zA-Z0-9_]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->lineEditName);
    ui->lineEditName->setValidator(validator);

    // 这里直接显示的话图片大小不对，延迟一段时间后显示没有这个问题，奇怪
    QTimer::singleShot(1, this, [=]{
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

QString DialogImportImg::getImgName()
{
    return ui->lineEditName->text();
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
            monoImg.setPixelColor(i, j, QColor(((grayscale >= this->grayscale) ^ invert) ? 0xFFFFFF : 0x000000));
        }
    }
}

void DialogImportImg::displayImg()
{

    QImage resultImg = monoImg.scaled(ui->labelPreviewImg->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelPreviewImg->setPixmap(QPixmap::fromImage(resultImg));
    qDebug() << resultImg.size();
}

void DialogImportImg::on_sliderGrayscale_valueChanged(int value)
{
    grayscale = value;
    ui->labelGrayscaleValue->setText(QString::number(value));
    imgTransform();
    displayImg();
}


void DialogImportImg::on_checkBoxInvert_toggled(bool checked)
{
    invert = checked;
    imgTransform();
    displayImg();
}

