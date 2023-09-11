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
    QGraphicsCanvasItem *scanvasItem;
private:
    Ui::FormPixelEditor *ui;

    QImage *image;      // 编辑的图片
    QGraphicsScene *scene;
    QGraphicsScaleItem *scaleItem;


    void initScrollerPos();   // 初始化滚轮位置为左上角
    void initAction();

protected:
    void leaveEvent(QEvent *event) override;

public slots:
    void paintView();
    void on_LoadImage(QImage &image);
    void on_actSelect_triggered();
    void on_actEdit_triggered();
    void on_actMeasure_triggered();
    void on_actSave_triggered();

signals:
    void updataStatusBarPos(QPoint);
    void saveImage(QImage image);

};

#endif // FORMPIXELEDITOR_H
