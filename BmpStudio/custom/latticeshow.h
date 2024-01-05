#ifndef LATTICESHOW_H
#define LATTICESHOW_H

#include <QWidget>
#include "imgencoderfactory.h"

class LatticeShow : public QWidget
{
    Q_OBJECT

    ImgEncoder *imgEncoder = nullptr;
    const int muduleSize = 16;  // 演示取模方式的尺寸大小(像素)
    int showStep;
public:
    explicit LatticeShow(QWidget *parent = nullptr);

    void setMode(int mode);
protected:
    void paintEvent(QPaintEvent *event);
signals:

};

#endif // LATTICESHOW_H
