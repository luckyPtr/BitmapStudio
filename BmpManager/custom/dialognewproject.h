#ifndef DIALOGNEWPROJECT_H
#define DIALOGNEWPROJECT_H

#include <QDialog>
#include <QButtonGroup>
#include "rawdata.h"

namespace Ui {
class DialogNewProject;
}

class DialogNewProject : public QDialog
{
    Q_OBJECT

    int mode;
    QButtonGroup *btnGroup1;
    QButtonGroup *btnGroup2;
    QString dirPath = nullptr;
public:
    explicit DialogNewProject(QWidget *parent = nullptr);
    ~DialogNewProject();

    QString getNewProject();
    RawData::Settings getSettings();
private slots:
    void on_btnOK1_clicked();

    void on_btnLastStep_clicked();

    void on_btnToggled(int,bool);

    void on_btnBrowse_clicked();

    void on_lineEditProjectName_textChanged(const QString &arg1);

private:
    Ui::DialogNewProject *ui;

    void updateBtnStatus();
};

#endif // DIALOGNEWPROJECT_H
