#ifndef DIALOGNOTICE_H
#define DIALOGNOTICE_H

#include <QDialog>

namespace Ui {
class DialogNotice;
}

class DialogNotice : public QDialog
{
    Q_OBJECT

    const QColor BackGroundColor = QColor("#227022");   // 背景颜色
    const qreal StartAlpha = 0.95;                      // 起始的透明度
    const qreal EndAlpha = 0.0;                         // 消失的透明度
    const quint16 ShowTime = 800;                      // 显示的时间(ms)
    const quint16 DisappearTime = 500;                  // 逐渐消失的时间(ms)

    QString text;   // 显示的文字
    qreal alphaStepValue;
    int timer;
    QColor backgroundColor;
public:
    explicit DialogNotice(QString text = "", QWidget *parent = nullptr);
    ~DialogNotice();

private:
    Ui::DialogNotice *ui;

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent* event);
};

#endif // DIALOGNOTICE_H
