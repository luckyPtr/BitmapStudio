#ifndef IMGPREVIREWER_H
#define IMGPREVIREWER_H

#include <QWidget>

class ImgPrevirewer : public QWidget
{
    Q_OBJECT
public:
    explicit ImgPrevirewer(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);

signals:

};

#endif // IMGPREVIREWER_H
