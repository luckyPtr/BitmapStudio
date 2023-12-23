#include "dialogimporthex.h"
#include "ui_dialogimporthex.h"
#include "QDebug"
#include <QRegExp>
#include <QSet>

DialogImportHex::DialogImportHex(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogImportHex)
{
    ui->setupUi(this);

}

DialogImportHex::~DialogImportHex()
{
    delete ui;
}

void DialogImportHex::on_textEdit_textChanged()
{
    QString text = ui->textEdit->toPlainText();
    QRegExp rx("0(x|X)[0-9a-fA-F]{1,2}");

    QSet<QString> set;
    int pos = 0;
    while((pos = rx.indexIn(text, pos)) != -1)
    {
        set << rx.cap(0);
        pos += rx.matchedLength();
    }

    foreach(auto s, set)
    {
        text.replace(s, QString("<font style='background-color: #cff5ff;' >%1</font>").arg(s));
    }
    text.replace("\n", "<br>");
    text.replace("\t", "    ");

    ui->textEdit->blockSignals(true);
    ui->textEdit->setText(text);
    ui->textEdit->blockSignals(false);
}


void DialogImportHex::on_pushButton_clicked()
{
    QString text = ui->textEdit->toPlainText();
    QRegExp rx("0(x|X)[0-9a-fA-F]{1,2}");

    QSet<QString> set;
    int pos = 0;
    while((pos = rx.indexIn(text, pos)) != -1)
    {
        set << rx.cap(0);
        pos += rx.matchedLength();
    }

    foreach(auto s, set)
    {
        text.replace(s, QString("<font style='background-color: #cff5ff;' >%1</font>").arg(s));
    }
    text.replace("\n", "<br>");
    text.replace("\t", "    ");
    qDebug() << text;

   ui->textEdit->setText(text);
}

