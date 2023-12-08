#include "dialognewfolder.h"
#include "ui_dialognewfolder.h"
#include <QRegExpValidator>
#include <QDebug>

DialogNewFolder::DialogNewFolder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewFolder)
{
    ui->setupUi(this);

    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 去掉标题栏上的问号图标
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口大小

    // 限制图片名称输入格式同C语言变量命名规则
//    QRegExp regx("^[a-zA-Z0-9_]+$");
//    QValidator *validator = new QRegExpValidator(regx, ui->lineEditName);
//    ui->lineEditName->setValidator(validator);
}

DialogNewFolder::~DialogNewFolder()
{
    delete ui;
}

QString DialogNewFolder::name()
{
    return ui->lineEditName->text();
}

QString DialogNewFolder::brief()
{
    return ui->textEditBrief->toPlainText();
}

void DialogNewFolder::on_lineEditName_textChanged(const QString &arg1)
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

