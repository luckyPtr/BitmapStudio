#include "treeview.h"
#include <QImage>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

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

TreeView::TreeView(QWidget *parent)
    : QTreeView{parent}
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
}

void TreeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count() == 1 && urls.first().toLocalFile().endsWith(".bs")) {
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
            QTreeView::dragEnterEvent(event); // 调用基类方法处理其他情况
        }
    }
    else
    {
        QTreeView::dragMoveEvent(event);
    }

}

void TreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count() == 1 && urls.first().toLocalFile().endsWith(".bs")) {
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
        QTreeView::dragMoveEvent(event);
    }
}

void TreeView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count() == 1 && urls.first().toLocalFile().endsWith(".bs")) {
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
        QTreeView::dropEvent(event);
    }
}
