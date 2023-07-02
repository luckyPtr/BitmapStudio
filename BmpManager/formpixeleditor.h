#ifndef FORMPIXELEDITOR_H
#define FORMPIXELEDITOR_H

#include <QWidget>
#include <QGraphicsScene>
#include "custom/qgraphicsscaleitem.h"

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

    QGraphicsScene *scene;
    QGraphicsItemGroup *rulerGroup; // 标尺图形图元组
    int pixelSize;      // 像素大小，单位像素
    const int rulerWith = 30;   // 标尺宽度
    QGraphicsScaleItem *scaleItem;

    void drawRuler();
    void initScrollerPos();   // 初始化滚轮位置为左上角
public slots:
    void paintView();
};

#endif // FORMPIXELEDITOR_H
