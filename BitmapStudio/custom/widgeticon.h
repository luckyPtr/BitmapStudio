#ifndef WIDGETICON_H
#define WIDGETICON_H

#include <QWidget>
#include <QTimer>
#include <QRandomGenerator>

class WidgetIcon : public QWidget
{
    Q_OBJECT

    QImage image;
    QVector<int> drawPixel;
    int drawedCnt;
    QTimer *timer;

public:
    explicit WidgetIcon(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);
public:
    void setImage(QImage img);
signals:
private slots:
    void on_Timeout();
};

#endif // WIDGETICON_H
