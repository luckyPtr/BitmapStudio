#include "treeview.h"

TreeView::TreeView(QWidget *parent)
    : QTreeView{parent}
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
}
