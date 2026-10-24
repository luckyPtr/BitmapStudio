#include "treeview.h"
#include "treemodel.h"
#include "treeitem.h"
#include <QImage>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QPaintEvent>

bool TreeView::isImageFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        return false;
    }

    // 尝试加载图像文件
    QImage image;
    bool isValid = image.load(filePath);
    return isValid;
}

bool TreeView::isProjectFile(const QString &filePath)
{
    return filePath.endsWith(".bms") || filePath.endsWith(".bs");
}

TreeView::TreeView(QWidget *parent)
    : QTreeView{parent}
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDragDropOverwriteMode(false);    // 用插入线指示落点，而不是整项覆盖高亮
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(false);       // 内置指示的OnItem画的是行底横线，与“移入”语义相悖，改为自绘

    autoExpandTimer = new QTimer(this);
    autoExpandTimer->setSingleShot(true);
    autoExpandTimer->setInterval(700);
    connect(autoExpandTimer, &QTimer::timeout, this, [this](){
        if(state() == DraggingState && autoExpandIndex.isValid() && !isExpanded(autoExpandIndex))
            expand(autoExpandIndex);
    });
}

// 按行内位置自算落点区域：文件夹/类别根上1/4=插前、下1/4=插后、中间1/2=移入；
// 文件上1/4=插前、其余=插后。
// 空文件夹没有子行可“接住”行下方的落点（那里是下一个兄弟节点），因此移入模式
// 必须靠整行高亮的自绘反馈引导用户瞄准到文件夹行本身
TreeView::DropZone TreeView::zoneAt(const QPoint &pos, const QModelIndex &idx)
{
    QRect r = visualRect(idx);
    if (!idx.isValid() || r.isNull() || r.height() <= 0)
        return ZoneBelow;

    TreeModel *m = static_cast<TreeModel *>(model());
    int type = m->itemFromIndex(idx)->getType();
    bool folderLike = (type == RawData::TypeImgFolder || type == RawData::TypeImgGrpFolder ||
                       type == RawData::TypeComImgFolder || type == RawData::TypeClassImg ||
                       type == RawData::TypeClassComImg);

    int band = qMax(2, r.height() / 4);
    if (pos.y() - r.top() < band)
        return ZoneAbove;
    if (r.bottom() - pos.y() < band)
        return ZoneBelow;
    return folderLike ? ZoneInto : ZoneBelow;
}

// 解析树内拖放的落点。不合法（虚拟节点、跨工程、跨树、拖入自身子树、图片组只收图片等）返回valid=false
TreeView::DropTarget TreeView::resolveDropTarget(const QPoint &pos, const QMimeData *mime)
{
    DropTarget t;

    bool ok = false;
    int id = mime->data("bm/id").toInt(&ok);
    if (!ok || id <= 0)
        return t;

    TreeModel *m = static_cast<TreeModel *>(model());
    QModelIndex srcIndex = m->findIndex(QString::fromUtf8(mime->data("bm/proj")), id);
    if (!srcIndex.isValid())
        return t;
    TreeItem *srcItem = m->itemFromIndex(srcIndex);

    QModelIndex idx = indexAt(pos);
    if (!idx.isValid())
        return t;
    TreeItem *item = m->itemFromIndex(idx);

    t.hover = idx;
    t.zone = zoneAt(pos, idx);

    QModelIndex parent;
    int row = -1;
    switch (item->getType())
    {
    case RawData::TypeImgFolder:
    case RawData::TypeImgGrpFolder:
    case RawData::TypeComImgFolder:
        if (t.zone == ZoneInto)
        {
            parent = idx;   // 拖到文件夹行中部：移入该文件夹（追加到末尾）
            row = -1;
        }
        else
        {
            parent = idx.parent();
            row = idx.row() + (t.zone == ZoneBelow ? 1 : 0);
        }
        break;
    case RawData::TypeClassImg:
    case RawData::TypeClassComImg:
        if (t.zone != ZoneInto)
            return t;   // 类别根没有同级插入位置
        parent = idx;
        row = -1;
        break;
    case RawData::TypeImgFile:
    case RawData::TypeComImgFile:
        // 拖到文件行：上1/4插到其前，其余插到其后
        parent = idx.parent();
        row = idx.row() + (t.zone == ZoneAbove ? 0 : 1);
        break;
    default:
        return t;
    }

    TreeItem *parentItem = m->itemFromIndex(parent);
    if (parentItem->getRawData() != srcItem->getRawData())
        return t;   // 跨工程

    // 所在树必须匹配：图片树节点只能落在图片类目标下，组合图同理
    int ptype = parentItem->getType();
    bool parentIsImg = (ptype == RawData::TypeClassImg ||
                        ptype == RawData::TypeImgFolder || ptype == RawData::TypeImgGrpFolder);
    if (RawData::isClassImgType(srcItem->getType()) != parentIsImg)
        return t;

    // 图片组文件夹只接受图片文件
    if (ptype == RawData::TypeImgGrpFolder && srcItem->getType() != RawData::TypeImgFile)
        return t;

    // 不能移动到自身或自身的子孙下面
    for (QModelIndex p = parent; p.isValid(); p = p.parent())
    {
        if (p == srcIndex)
            return t;
    }

    // 同级重排时把插入行换算成“目标层级移除源节点后”的下标（与RawData::move的pos语义一致）
    if (row >= 0 && parent == srcIndex.parent() && srcIndex.row() < row)
        row--;

    t.valid = true;
    t.source = srcIndex;
    t.parent = parent;
    t.row = row;
    return t;
}

void TreeView::setDropHint(DropZone zone, const QRect &rect)
{
    if (hintZone == zone && hintRect == rect)
        return;
    hintZone = zone;
    hintRect = rect;
    viewport()->update();
}

void TreeView::clearDropHint()
{
    if (hintZone == ZoneNone)
        return;
    hintZone = ZoneNone;
    hintRect = QRect();
    viewport()->update();
}

void TreeView::updateAutoExpand(const QModelIndex &idx)
{
    bool expandable = idx.isValid() && !isExpanded(idx) && model()->rowCount(idx) > 0;
    if (!expandable)
    {
        stopAutoExpand();
        return;
    }
    if (idx != autoExpandIndex)
    {
        autoExpandIndex = idx;
        autoExpandTimer->start();
    }
}

void TreeView::stopAutoExpand()
{
    autoExpandTimer->stop();
    autoExpandIndex = QModelIndex();
}

void TreeView::dragEnterEvent(QDragEnterEvent *event)
{
    clearDropHint();
    stopAutoExpand();

    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count() == 1 && isProjectFile(urls.first().toLocalFile())) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else if (urls.count() == 1 && isImageFile(urls.first().toLocalFile())) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else
        {
            event->setDropAction(Qt::MoveAction);
            QTreeView::dragEnterEvent(event); // 调用基类方法处理其他情况（树内节点拖动等）
        }
    }
    else
    {
        event->setDropAction(Qt::MoveAction);
        QTreeView::dragEnterEvent(event);
    }

}

void TreeView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count() == 1 && isProjectFile(urls.first().toLocalFile())) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else if (urls.count() == 1 && isImageFile(urls.first().toLocalFile())) {
            QModelIndex index = indexAt(event->pos());
            setCurrentIndex(index);
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else
        {
            event->setDropAction(Qt::MoveAction);
            QTreeView::dragMoveEvent(event); // 调用基类方法处理其他情况
        }
    }
    else
    {
        // 树内节点拖动：先走基类维护自动滚动，再按落点合法性决定接受与否，
        // 并更新自绘落点提示与悬停自动展开
        QTreeView::dragMoveEvent(event);
        DropTarget t = resolveDropTarget(event->pos(), event->mimeData());
        if (t.valid)
        {
            setDropHint(t.zone, visualRect(t.hover));
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            clearDropHint();
            event->ignore();
        }
        updateAutoExpand(indexAt(event->pos()));
    }
}

void TreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    clearDropHint();
    stopAutoExpand();
    QTreeView::dragLeaveEvent(event);
}

void TreeView::dropEvent(QDropEvent *event)
{
    clearDropHint();
    stopAutoExpand();

    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count() == 1 && isProjectFile(urls.first().toLocalFile())) {
            event->setDropAction(Qt::CopyAction);
            event->accept();

            emit openProject(urls.first().toLocalFile());
        }
        else if (urls.count() == 1 && isImageFile(urls.first().toLocalFile())) {
            QModelIndex index = indexAt(event->pos());
            emit importImageFile(index, urls.first().toLocalFile());
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else
        {
            event->setDropAction(Qt::MoveAction);
            QTreeView::dropEvent(event); // 调用基类方法处理其他情况
        }
    }
    else
    {
        // 树内节点拖动：按松手位置现算落点（不依赖拖动过程中的残留状态），
        // 交由ProjectMng执行移动（改顺序/改父级），模型随后整体重建
        DropTarget t = resolveDropTarget(event->pos(), event->mimeData());
        if (t.valid)
        {
            emit moveNode(t.source, t.parent, t.row);
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}

// 自绘树内拖放的落点提示：移入=整行半透明高亮+边框，插前/插后=行上/下边缘横线。
// 内置指示已关闭（见构造函数），两者不会叠加
void TreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);

    if (state() != DraggingState || hintZone == ZoneNone || hintRect.isNull())
        return;

    QPainter p(viewport());
    QColor base = palette().color(QPalette::Highlight);
    if (hintZone == ZoneInto)
    {
        QColor fill = base;
        fill.setAlpha(80);
        p.fillRect(hintRect, fill);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(base.darker(140), 1));
        p.drawRoundedRect(hintRect.adjusted(0, 0, -1, -1), 3, 3);
    }
    else
    {
        int y = hintZone == ZoneAbove ? hintRect.top() : hintRect.bottom() - 1;
        p.fillRect(QRect(hintRect.left(), y, hintRect.width(), 2), base.darker(120));
    }
}
