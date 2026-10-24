#ifndef DIALOGLOADING_H
#define DIALOGLOADING_H

#include <QDialog>

namespace Ui {
class DialogLoading;
}

class DialogLoading : public QDialog
{
    Q_OBJECT

    const QSize rectSize = QSize(10, 10);
    const int rectGap = 5;
    int timer;
    int step = 0;
public:
    explicit DialogLoading(QWidget *parent = nullptr);
    ~DialogLoading();

private:
    Ui::DialogLoading *ui;

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent* event);
};

#endif // DIALOGLOADING_H
