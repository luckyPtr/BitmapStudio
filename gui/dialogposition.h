#ifndef DIALOGPOSITION_H
#define DIALOGPOSITION_H

#include <QDialog>

namespace Ui {
class DialogPosition;
}

class DialogPosition : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPosition(QWidget *parent = nullptr);
    ~DialogPosition();

private:
    Ui::DialogPosition *ui;
    QPoint position;

public:
    void setDefaultPos(QPoint);
    QPoint getPos();
};

#endif // DIALOGPOSITION_H
