#include "dialognewimgfile.h"
#include "ui_dialognewimgfile.h"
#include <QDebug>
#include <QTimer>

DialogNewImgFile::DialogNewImgFile(QWidget *parent, QSize defaultSize) :
    QDialog(parent),
    ui(new Ui::DialogNewImgFile)
{
    ui->setupUi(this);
    ui->spinBoxWidth->setValue(defaultSize.width());
    ui->spinBoxHeight->setValue(defaultSize.height());

    // 把焦点设置在图片名称的lineEdit上
    QTimer::singleShot(0, ui->lineEditImgFileName, SLOT(setFocus()));

    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 去掉标题栏上的问号图标
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口大小

    // 限制图片名称输入格式同C语言变量命名规则
//    QRegExp regx("^[a-zA-Z_][a-zA-Z0-9_]+$");
//    QValidator *validator = new QRegExpValidator(regx, ui->lineEditImgFileName);
//    ui->lineEditImgFileName->setValidator(validator);
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

QSize DialogNewImgFile::size()
{
    return QSize(width(), height());
}

QString DialogNewImgFile::imgFileName()
{
    return ui->lineEditImgFileName->text();
}

QString DialogNewImgFile::brief()
{
    return ui->textEdit->toPlainText();
}

void DialogNewImgFile::setExistFile(QVector<QString> files)
{
    existFile = files;
}

void DialogNewImgFile::setSize(QSize size)
{
    ui->spinBoxWidth->setValue(size.width());
    ui->spinBoxHeight->setValue(size.height());
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
    // 有字母数字下划线组成，长度不为0
    QRegExp regx("^[a-zA-Z0-9_]+$");
    if(regx.exactMatch(arg1))
    {
        ui->btnOK->setEnabled(true);
        ui->lineEditImgFileName->setStyleSheet("color:black;");
    }
    else
    {
        ui->btnOK->setEnabled(false);
        ui->lineEditImgFileName->setStyleSheet("color:#ff4040;");
    }
}



