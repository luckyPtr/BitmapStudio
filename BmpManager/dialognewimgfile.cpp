#include "dialognewimgfile.h"
#include "ui_dialognewimgfile.h"
#include <QDebug>

DialogNewImgFile::DialogNewImgFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewImgFile)
{
    ui->setupUi(this);

    ui->btnOK->setEnabled(false);
    connect(ui->spinBoxWidth, SIGNAL(valueChanged(int)), this, SLOT(on_inputUpdate()));
    connect(ui->spinBoxHeight, SIGNAL(valueChanged(int)), this, SLOT(on_inputUpdate()));
    connect(ui->lineEditImgFileName, SIGNAL(textChanged(QString)), this, SLOT(on_inputUpdate()));
}

DialogNewImgFile::~DialogNewImgFile()
{
    delete ui;
}

quint16 DialogNewImgFile::width()
{
    return ui->spinBoxWidth->value();
}

quint16 DialogNewImgFile::height()
{
    return ui->spinBoxHeight->value();
}

QString DialogNewImgFile::imgFileName()
{
    return ui->lineEditImgFileName->text();
}

void DialogNewImgFile::setExistFile(QVector<QString> files)
{
    existFile = files;
}

bool DialogNewImgFile::checkImgFileName()
{
    if(existFile.contains(imgFileName()))
    {
        return false;
    }

    return true;
}

void DialogNewImgFile::on_lineEditImgFileName_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    if(!checkImgFileName())
    {
        ui->lineEditImgFileName->setStyleSheet("color:red");
    }
    else
    {
        ui->lineEditImgFileName->setStyleSheet("color:black");
    }
}

void DialogNewImgFile::on_inputUpdate()
{
    bool isDisable = ui->lineEditImgFileName->text().isEmpty() || ui->spinBoxWidth->value() == 0 || ui->spinBoxHeight == 0;
    ui->btnOK->setEnabled(!isDisable);
}

