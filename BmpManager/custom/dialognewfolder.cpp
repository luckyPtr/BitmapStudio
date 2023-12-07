#include "dialognewfolder.h"
#include "ui_dialognewfolder.h"

DialogNewFolder::DialogNewFolder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewFolder)
{
    ui->setupUi(this);
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
