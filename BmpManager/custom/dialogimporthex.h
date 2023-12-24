#ifndef DIALOGIMPORTHEX_H
#define DIALOGIMPORTHEX_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class DialogImportHex;
}

class DialogImportHex : public QDialog
{
    Q_OBJECT

    int mode = 0;
    QButtonGroup *btnGroup1;
    QButtonGroup *btnGroup2;
    QByteArray inputByteArray;
public:
    explicit DialogImportHex(QWidget *parent = nullptr);
    ~DialogImportHex();

    void setDefaultMode(int mode);

private slots:
    void on_textEdit_textChanged();
    void on_btnToggled(int btn, bool checked);

private:
    Ui::DialogImportHex *ui;
};

#endif // DIALOGIMPORTHEX_H
