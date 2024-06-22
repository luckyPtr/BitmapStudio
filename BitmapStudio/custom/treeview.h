#ifndef TREEVIEW_H
#define TREEVIEW_H
#include <QTreeView>




class TreeView : public QTreeView
{
    Q_OBJECT

    bool isImageFile(const QString &filePath);
public:
    explicit TreeView(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dragMoveEvent(QDragMoveEvent* event) override;

    void dropEvent(QDropEvent *event) override;

signals:
    void openProject(QString);
    void importImageFile(QModelIndex, QString);
};

#endif // TREEVIEW_H
