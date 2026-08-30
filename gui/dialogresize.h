#ifndef DIALOGRESIZE_H
#define DIALOGRESIZE_H

#include <QDialog>

namespace Ui {
class DialogResize;
}

class DialogResize : public QDialog
{
    Q_OBJECT

public:
    explicit DialogResize(QWidget *parent = nullptr);
    ~DialogResize();

private:
    Ui::DialogResize *ui;

public:
    void setDefaultSize(QSize size);
    QSize getSize();
};

#endif // DIALOGRESIZE_H
