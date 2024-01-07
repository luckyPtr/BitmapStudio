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
    QImage outputImg;
    bool isNameLegal = false;
    void initClipboardText();
public:
    explicit DialogImportHex(QWidget *parent = nullptr);
    ~DialogImportHex();

    void setDefaultMode(int mode);
    QString getName();
    void setName(QString name);
    QString getBrief();
    void setBrief(QString brief);
    QImage getImg();

private slots:
    void on_textEdit_textChanged();
    void on_btnToggled(int btn, bool checked);

    void on_btnAddBrief_clicked();

    void on_spinBoxWidth_valueChanged(int arg1);

    void on_spinBoxHeight_valueChanged(int arg1);

    void on_lineEditName_textChanged(const QString &arg1);

private:
    Ui::DialogImportHex *ui;
    void updateImg();
    void updateBtnOK();
};

#endif // DIALOGIMPORTHEX_H
