#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVariant>
#include <QPixmap>
#include <QIcon>
#include "rawdata.h"

// 工程配置信息

//struct BmData
//{
//    QString project;
//    quint32 version;
//    quint8 depth;
//    quint16 canvasWide;
//    quint16 canvasHeight;
//    quint32 id;
//    quint32 pid;
//    QString name;
//    QString details;
//    quint16 wide;
//    quint16 height;
//    QPixmap img;
//};


//struct BmConfig
//{
//    QString project;
//    quint32 version;
//    quint8 depth;
//    quint16 canvasWide;
//    quint16 canvasHeight;
//};
////
//struct BmImage
//{
//    quint32 id;
//    quint32 pid;
//    QString name;
//    QString details;
//    quint16 wide;
//    quint16 height;
//    QPixmap img;
//};
//Q_DECLARE_METATYPE(BmConfig);
//Q_DECLARE_METATYPE(BmImage);


class TreeItem
{
public:

    enum Type
    {
        UNKNOWN = -1,
        PROJECT,
        CLASS_SETTINGS,
        CLASS_IMAGE,
        CLASS_COMIMAGE,
        CLASS_FONT,
        FOLDER,
        FILE
    };
    enum
    {
        RoleDisplay,
        RoleData,
    };

private:
    QList<TreeItem*> _children;   // 子节点
    TreeItem *_parent;          // 父节点
    Type _type;      // 此节点保存的数据类型
    void* _ptr;     // 存储数据的指针
    int _row;       // 此item位于父节点中第几个
    bool isIngnore;
    quint16 id;

public:

    explicit TreeItem(TreeItem *parent = nullptr);
    ~TreeItem();

    void addChild(TreeItem *item);
    void removeChildren();

    TreeItem *child(int row) { return _children.value(row); }
    TreeItem *parent() { return _parent; }
    void setParent(TreeItem* parent) { _parent = parent; }

    int childCount() const { return _children.count(); }

    QVariant data(int column) const;
    QIcon icon() const;
    BmImg *bmImg() const;

    //设置、获取节点存的数据指针
    void setPtr(void* p) { _ptr = p; }
    void* ptr() const { return _ptr; }

    // 保存该节点是其父节点的第几个子节点，查询优化所用
    void setRow(int row) { _row = row; }
    // 返回本节点位于父节点下第几个子节点
    int row() const { return _row; }

    Type getType() const { return _type; }
    void setType(const Type &value) { _type = value; }
};

#endif // TREEITEM_H
