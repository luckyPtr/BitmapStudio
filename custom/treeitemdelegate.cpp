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

    const TreeModel *model = static_cast<const TreeModel*>(index.model());
    TreeItem *item = static_cast<TreeItem*>(model->itemFromIndex(index));

    QString brief = item->getNotes();
    QString briefTitle = brief.left(brief.indexOf('\n'));

    QString text = item->getText();


//    opt.state |= QStyle::State_HasFocus;
    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();
    // Coords
    int x = option.rect.x() + option.fontMetrics.horizontalAdvance(text) + 15 + option.decorationSize.width();
    int y = opt.rect.y() + (opt.rect.height() - opt.fontMetrics.descent() - opt.fontMetrics.ascent()) / 2 + opt.fontMetrics.ascent();

    // Paint
    if(!briefTitle.isEmpty())
    {
        QPen pen(QColor(128, 128, 0));
        painter->setPen(pen);
        painter->setFont(opt.font);
        painter->drawText(QPoint(x, y), QString("(%1)").arg(briefTitle));
    }

    painter->restore();
}
