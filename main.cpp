#include "mainwindow.h"
#include "singleapplication.h"
#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    // 高DPI（4K）屏幕缩放支持，必须在 QApplication 构造之前设置
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
                Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    SingleApplication a(argc, argv);

    a.setFont(QFont("Microsoft YaHei", 9));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "BitmapStudio_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    QString filePath = nullptr;
    if (argc >= 2)
    {
        filePath = QString::fromUtf8(argv[1]);
    }

    if (!a.isRunning())
    {
        MainWindow w;
        w.show();
        return a.exec();
    }
}
