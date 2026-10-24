#ifndef DIALOGSYSSETTINGS_H
#define DIALOGSYSSETTINGS_H

#include <QDialog>

namespace Ui {
class DialogSysSettings;
}

class DialogSysSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSysSettings(QWidget *parent = nullptr);
    ~DialogSysSettings();

private:
    Ui::DialogSysSettings *ui;
};

#endif // DIALOGSYSSETTINGS_H
