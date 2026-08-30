#include "dialogsyssettings.h"
#include "ui_dialogsyssettings.h"

DialogSysSettings::DialogSysSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSysSettings)
{
    ui->setupUi(this);
}

DialogSysSettings::~DialogSysSettings()
{
    delete ui;
}
