#ifndef DIALOGNOTICE_H
#define DIALOGNOTICE_H

#include <QDialog>

namespace Ui {
class DialogNotice;
}

class DialogNotice : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNotice(QWidget *parent = nullptr);
    ~DialogNotice();

private:
    Ui::DialogNotice *ui;

    int timer;
    QColor backgroundColor;
protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent* event);
};

#endif // DIALOGNOTICE_H
