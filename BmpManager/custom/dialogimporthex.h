#ifndef DIALOGIMPORTHEX_H
#define DIALOGIMPORTHEX_H

#include <QDialog>

namespace Ui {
class DialogImportHex;
}

class DialogImportHex : public QDialog
{
    Q_OBJECT

public:
    explicit DialogImportHex(QWidget *parent = nullptr);
    ~DialogImportHex();

private slots:
    void on_textEdit_textChanged();

    void on_pushButton_clicked();

private:
    Ui::DialogImportHex *ui;
};

#endif // DIALOGIMPORTHEX_H
