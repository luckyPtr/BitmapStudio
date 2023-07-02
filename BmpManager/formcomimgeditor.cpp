#include "formcomimgeditor.h"
#include "ui_formcomimgeditor.h"

FormComImgEditor::FormComImgEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormComImgEditor)
{
    ui->setupUi(this);
}

FormComImgEditor::~FormComImgEditor()
{
    delete ui;
}
