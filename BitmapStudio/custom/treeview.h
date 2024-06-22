#ifndef TREEVIEW_H
#define TREEVIEW_H
#include <QTreeView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

class TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if (event->mimeData()->hasUrls()) {
            const QList<QUrl> urls = event->mimeData()->urls();
            if (urls.count() == 1 && urls.first().toLocalFile().endsWith(".bs")) {
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
            event->ignore();
        }

    }

    void dragMoveEvent(QDragMoveEvent* event) override
    {
        if (event->mimeData()->hasUrls()) {
            const QList<QUrl> urls = event->mimeData()->urls();
            if (urls.count() == 1 && urls.first().toLocalFile().endsWith(".bs")) {
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
            event->ignore();
        }
    }

    void dropEvent(QDropEvent *event) override
    {
        if (event->mimeData()->hasUrls()) {
            const QList<QUrl> urls = event->mimeData()->urls();
            if (urls.count() == 1 && urls.first().toLocalFile().endsWith(".bs")) {
                event->setDropAction(Qt::CopyAction);
                event->accept();
                emit openProject(urls.first().toLocalFile());
            }
            else
            {
                event->setDropAction(Qt::MoveAction);
                QTreeView::dropEvent(event); // 调用基类方法处理其他情况
            }
        }
        else
        {
            event->ignore();
        }
    }

signals:
    void openProject(QString);
};

#endif // TREEVIEW_H
