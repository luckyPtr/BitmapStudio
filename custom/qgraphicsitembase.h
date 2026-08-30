#ifndef QGRAPHICSITEMBASE_H
#define QGRAPHICSITEMBASE_H

#include <QGraphicsItem>
#include <QObject>
#include <QVector>


class QGraphicsItemBase : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    struct AuxiliaryLine
    {
        Qt::Orientation dir;
        int scale;
        static bool lock;  // 锁定辅助线
        static bool hide;  // 隐藏辅助线
        AuxiliaryLine(Qt::Orientation dir, int scale)
        {
            this->dir = dir;
            this->scale = scale;
        }
    };


    int selectedAuxiliaryLine = -1;
    static QVector<AuxiliaryLine> auxiliaryLines;

    explicit QGraphicsItemBase(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent) {}

};

#endif // QGRAPHICSITEMBASE_H
