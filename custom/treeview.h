#ifndef TREEVIEW_H
#define TREEVIEW_H
#include <QTreeView>
#include <QTimer>



class TreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit TreeView(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dragMoveEvent(QDragMoveEvent* event) override;

    void dragLeaveEvent(QDragLeaveEvent *event) override;

    void dropEvent(QDropEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

signals:
    void openProject(QString);
    void importImageFile(QModelIndex, QString);
    void moveNode(QModelIndex source, QModelIndex targetParent, int row);  // 树内拖动节点（改顺序/改父级）

private:
    bool isImageFile(const QString &filePath);
    bool isProjectFile(const QString &filePath);

    // 树内拖放的落点区域（按行内位置自算分带，不依赖Qt内置dropIndicatorPosition：
    // 其OnItem指示画行底横线，视觉上像“插到后面”，无法区分“移入文件夹”）
    enum DropZone
    {
        ZoneNone,
        ZoneInto,   // 移入文件夹/类别根（追加到末尾）
        ZoneAbove,  // 插到目标行之前（同级）
        ZoneBelow,  // 插到目标行之后（同级）
    };

    struct DropTarget
    {
        bool valid = false;
        QModelIndex source;     // 被拖动节点
        QModelIndex parent;     // 目标父节点
        int row = -1;           // 插入下标（-1=追加到末尾）
        QModelIndex hover;      // 悬停的行
        DropZone zone = ZoneNone;
    };

    DropZone zoneAt(const QPoint &pos, const QModelIndex &idx);
    DropTarget resolveDropTarget(const QPoint &pos, const QMimeData *mime);

    void setDropHint(DropZone zone, const QRect &rect);
    void clearDropHint();
    void updateAutoExpand(const QModelIndex &idx);
    void stopAutoExpand();

    DropZone hintZone = ZoneNone;    // 自绘落点提示（viewport坐标）
    QRect hintRect;
    QTimer *autoExpandTimer;         // 拖动时悬停收起的文件夹自动展开
    QModelIndex autoExpandIndex;
};

#endif // TREEVIEW_H
