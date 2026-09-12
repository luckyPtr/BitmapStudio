#include "treemodel.h"
#include "TreeItem.h"
#include <QMimeData>

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    //_headers = headers;
    _rootItem = new TreeItem();
}

TreeModel::~TreeModel()
{
    delete _rootItem;
}

TreeItem *TreeModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {

        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item;
    }
    return _rootItem;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);
    // 项目/类别根等虚拟节点不可拖动；所有节点保持可放置（具体落点合法性由TreeView按类型判定）
    TreeItem *item = itemFromIndex(index);
    int type = item->getType();
    if (type == RawData::TypeImgFile || type == RawData::TypeComImgFile ||
        type == RawData::TypeImgFolder || type == RawData::TypeImgGrpFolder || type == RawData::TypeComImgFolder)
    {
        flag |= Qt::ItemIsDragEnabled;
    }
    return flag | Qt::ItemIsDropEnabled;				//允许Model拖动，这个很重要
}

Qt::DropActions TreeModel::supportedDropActions() const
{
    // 基类默认只支持CopyAction，会禁掉树内节点的Move拖放
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeD = QAbstractItemModel::mimeData(indexes);				//先获取原始的mimeData;
    if (indexes.size() > 0)
    {
        QModelIndex index = indexes.at(0);

        TreeItem *item = itemFromIndex(index);
        int id = data(index, RoleID).toInt();
        mimeD->setData("bm/id", QByteArray::number(id));							//将自己需要的数据 存入到MimeData中
        mimeD->setData("bm/proj", item->getRawData()->getProject().toUtf8());	//工程路径用于恢复源节点（多工程打开时id不唯一）
        if(item->getType() == RawData::TypeImgFile)
        {
            mimeD->setData("bm/type", "image");
        }
    }
    else
        mimeD->setData("bm/id", "NULL");

    return mimeD;
}

// 递归查找工程+id对应的节点索引
QModelIndex TreeModel::findIndex(const QString &project, int id) const
{
    auto find = [&](auto&& self, const QModelIndex &parentIndex) -> QModelIndex {
        for(int i = 0; i < rowCount(parentIndex); i++)
        {
            QModelIndex idx = index(i, 0, parentIndex);
            TreeItem *item = itemFromIndex(idx);
            if(item->getID() == id && item->getRawData()->getProject() == project)
                return idx;
            QModelIndex ret = self(self, idx);
            if(ret.isValid())
                return ret;
        }
        return QModelIndex();
    };

    return find(find, QModelIndex());
}



TreeItem *TreeModel::root()
{
    return _rootItem;
}

// 获取表头数据
QVariant TreeModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return _headers.at(section);
        }
    }
    return QVariant();
}

// 获取index.row行，index.column列数据
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = itemFromIndex(index);
    if (role == Qt::DisplayRole)
    {
        return item->data(index.column());
    }
    else if(role == Qt::DecorationRole)
    {
        if(index.column() == 0)
        return item->icon();
    }
    else if(role == RoleID)
    {
        return item->getID();
    }
    else if(role == RoleProjPath)
    {
        return item->getRawData()->getProject();
    }

    return QVariant();
}

// 在parent节点下，第row行，第column列位置上创建索引
QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem = itemFromIndex(parent);
    TreeItem *item = parentItem->child(row);
    if (item)
        return createIndex(row, column, item);
    else
        return QModelIndex();
}

// 创建index的父索引
QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *item = itemFromIndex(index);
    TreeItem *parentItem = item->parent();

    if (parentItem == _rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

// 获取索引parent下有多少行
int TreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    TreeItem* item = itemFromIndex(parent);
    return item->childCount();
}

// 返回索引parent下有多少列
int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1; //_headers.size();
}

void TreeModel::clear()
{
    if(_rootItem)
    {
        _rootItem->removeChildren();
    }
}

void TreeModel::removeRow(const QModelIndex &index)
{
    itemFromIndex(index.parent())->removeChild(index.row());
}

