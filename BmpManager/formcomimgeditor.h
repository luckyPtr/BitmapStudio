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
    QGraphicsComImgCanvansItem * comImgCanvansItem;

private:
    Ui::FormComImgEditor *ui;

    QGraphicsScene *scene;
    QGraphicsScaleItem *scaleItem;


    void initScrollerPos();   // 初始化滚轮位置为左上角
    void initAction();
protected:
//    void dragEnterEvent(QDragEnterEvent *event) override;
//    void dropEvent(QDropEvent *event) override;
//    void dragMoveEvent(QDragMoveEvent *event) override;

public slots:
    void on_LoadComImg(ComImg &comImg, RawData *rd);

signals:
    void saveComImg(ComImg comImg);

};

#endif // FORMCOMIMGEDITOR_H
