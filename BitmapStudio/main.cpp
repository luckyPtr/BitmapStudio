#include "mainwindow.h"
#include "singleapplication.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv);

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
