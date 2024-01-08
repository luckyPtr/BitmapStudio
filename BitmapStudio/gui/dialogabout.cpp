#include "dialogabout.h"
#include "ui_dialogabout.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QDateTime>


QString DialogAbout::buildTime()
{
    QString dateTime;
    dateTime.clear();
    dateTime += __DATE__;
    dateTime += __TIME__;
    //注意" "是两个空格，用于日期为单数时需要转成“空格+0”
    dateTime.replace("  "," 0");

    QDateTime buildDateTime = QLocale(QLocale::English).toDateTime(dateTime,"MMM dd yyyyhh:mm:ss");

    return QString::asprintf("%d-%02d-%02d %02d:%02d:%02d", buildDateTime.date().year(),
                             buildDateTime.date().month(),
                             buildDateTime.date().day(),
                             buildDateTime.time().hour(),
                             buildDateTime.time().minute(),
                             buildDateTime.time().second());
}

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    ui->buttonBox->button(QDialogButtonBox::Close)->setText(tr("关闭"));

    ui->labelProject->setText(qApp->applicationName());
    ui->labelVersion->setText(QString(tr("版本：%1")).arg(APP_VERSION));
    ui->labelBuildTime->setText(buildTime());

    connect(ui->toolButtonAboutQt, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
    connect(ui->toolButtonGitHub, &QToolButton::clicked, [](){
        QDesktopServices::openUrl(QUrl("https://github.com/luckyPtr/BitmapStudio"));
    });
}

DialogAbout::~DialogAbout()
{
    delete ui;
}
