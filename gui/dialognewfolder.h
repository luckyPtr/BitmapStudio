#ifndef DIALOGNEWFOLDER_H
#define DIALOGNEWFOLDER_H

#include <QDialog>

namespace Ui {
class DialogNewFolder;
}

class DialogNewFolder : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewFolder(QWidget *parent = nullptr);
    ~DialogNewFolder();

    QString name();
    QString brief();
private slots:
    void on_lineEditName_textChanged(const QString &arg1);

private:
    Ui::DialogNewFolder *ui;
};

#endif // DIALOGNEWFOLDER_H
