#ifndef FORMPIXELEDITOR_H
#define FORMPIXELEDITOR_H

#include <QWidget>
#include <QGraphicsScene>
#include "custom/qgraphicsscaleitem.h"
#include "custom/qgraphicscanvasitem.h"

namespace Ui {
class FormPixelEditor;
}

class FormPixelEditor : public QWidget
{
    Q_OBJECT

public:
    explicit FormPixelEditor(QWidget *parent = nullptr);
    ~FormPixelEditor();

private:
    Ui::FormPixelEditor *ui;

    QImage *image;      // 编辑的图片
    QGraphicsScene *scene;
    QGraphicsScaleItem *scaleItem;
    QGraphicsCanvasItem *scanvasItem;

    void initScrollerPos();   // 初始化滚轮位置为左上角
    void initAction();
public slots:
    void paintView();
private slots:
    void on_actFlipHorizontal_triggered();
    void on_actFlipVerital_triggered();
    void on_actRotateLeft_triggered();
    void on_actRotateRight_triggered();
    void on_actReserve_triggered();

signals:
    void imgReserve();
};

#endif // FORMPIXELEDITOR_H
