#ifndef DIALOGPROJECTSETTINGS_H
#define DIALOGPROJECTSETTINGS_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class DialogProjectSettings;
}

class DialogProjectSettings : public QDialog
{
    Q_OBJECT

    QButtonGroup *btnGroup1;
    QButtonGroup *btnGroup2;
    int mode = 0;       // 取模方式
public:
    explicit DialogProjectSettings(QWidget *parent = nullptr);
    ~DialogProjectSettings();

    int getMode();
    QString getConst();
    QString getPosType();
    QString getSizeType();
    QString getOutputPath();
    QString getOutputFormat();

private:
    Ui::DialogProjectSettings *ui;

private slots:
    void on_btnToggled(int btn, bool checked);
};

#endif // DIALOGPROJECTSETTINGS_H
