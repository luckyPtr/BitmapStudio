#ifndef FORMCOMIMGEDITOR_H
#define FORMCOMIMGEDITOR_H

#include <QWidget>
#include <QGraphicsScene>
#include "custom/qgraphicsscaleitem.h"
#include "custom/qgraphicscomimgcanvansitem.h"


namespace Ui {
class FormComImgEditor;
}

class FormComImgEditor : public QWidget
{
    Q_OBJECT

public:
    explicit FormComImgEditor(QWidget *parent = nullptr);
    ~FormComImgEditor();

private:
    Ui::FormComImgEditor *ui;

    QGraphicsScene *scene;
    QGraphicsScaleItem *scaleItem;
    QGraphicsComImgCanvansItem * comImgCanvansItem;


    void initScrollerPos();   // 初始化滚轮位置为左上角

protected:
//    void dragEnterEvent(QDragEnterEvent *event) override;
//    void dropEvent(QDropEvent *event) override;
//    void dragMoveEvent(QDragMoveEvent *event) override;

public slots:
    void on_LoadComImg(ComImg &comImg, RawData *rd);

};

#endif // FORMCOMIMGEDITOR_H
