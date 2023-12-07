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
private:
    Ui::DialogNewFolder *ui;
};

#endif // DIALOGNEWFOLDER_H
