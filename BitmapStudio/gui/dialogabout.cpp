#include "dialogabout.h"
#include "ui_dialogabout.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    ui->buttonBox->button(QDialogButtonBox::Close)->setText(tr("关闭"));

    ui->labelProject->setText(qApp->applicationName());
    ui->labelVersion->setText(APP_VERSION);
    ui->labelBuildTime->setText(QString("%1 %2").arg(__DATE__).arg(__TIME__));

    connect(ui->toolButtonAboutQt, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
    connect(ui->toolButtonGitHub, &QToolButton::clicked, [](){
        QDesktopServices::openUrl(QUrl("https://github.com/luckyPtr/BmpStudio"));
    });
}

DialogAbout::~DialogAbout()
{
    delete ui;
}
