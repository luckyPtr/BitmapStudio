#include "treeitemdelegate.h"
#include <QPainter>
#include "treeitem.h"
#include "treemodel.h"

TreeItemDelegate::TreeItemDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void TreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const TreeModel *model = dynamic_cast<const TreeModel*>(index.model());
    TreeItem *item = dynamic_cast<TreeItem*>(model->itemFromIndex(index));

    QString notes = item->getNotes();
    QString text = item->getText();

//    opt.state |= QStyle::State_HasFocus;
    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();
    // Coords
    int x = option.rect.x() + option.fontMetrics.horizontalAdvance(text) + 15 + option.decorationSize.width();
    int y = opt.rect.y() + (opt.rect.height() - opt.fontMetrics.descent() - opt.fontMetrics.ascent()) / 2 + opt.fontMetrics.ascent();

    // Paint
    if(!notes.isEmpty())
    {
        QPen pen(QColor(128, 128, 0));
        painter->setPen(pen);
        painter->setFont(opt.font);
        painter->drawText(QPoint(x, y), QString("(%1)").arg(notes));
    }

    painter->restore();
}
