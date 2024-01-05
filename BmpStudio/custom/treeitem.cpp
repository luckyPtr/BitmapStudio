#include "treeitem.h"

TreeItem::TreeItem(TreeItem *parent)
    : _parent(parent),
    _type(-1),
    _ptr(nullptr),
    _row(-1)
{

}

TreeItem::~TreeItem()
{
    removeChildren();
}

// 在本节点下添加子节点
void TreeItem::addChild(TreeItem *item)
{
    item->setParent(this);
    item->setRow(_children.size());
    _children.append(item);
}

// 清空所有子节点
void TreeItem::removeChildren()
{
    qDeleteAll(_children);
    _children.clear();
}

void TreeItem::removeChild(int row)
{
    delete _children[row];
    _children.removeAt(row);
}

// 获取本节点第column列的数据
QVariant TreeItem::data(int column) const
{
    if(column == Qt::DisplayRole)
    {
        switch(_type)
        {
        case RawData::TypeProject:
            return QFileInfo(rawData->getProject()).baseName();

        case RawData::TypeClassSettings:
            return QString("设置");

        case RawData::TypeClassImg:
            return QString("图片");

        case RawData::TypeClassComImg:
            return QString("组合图");

        case RawData::TypeImgFile:
        case RawData::TypeComImgFile:
        case RawData::TypeImgFolder:
        case RawData::TypeImgGrpFolder:
        case RawData::TypeComImgFolder:
            return rawData->getDataMap()[id].name;
        default:
            break;
        }
    }
    return QVariant();
}

QIcon TreeItem::icon() const
{
    switch (_type) {
    case RawData::TypeProject:
        return QIcon(":/Image/TreeIco/Project.svg");
    case RawData::TypeClassSettings:
        return QIcon(":/Image/TreeIco/Setting.svg");
    case RawData::TypeClassImg:
        return QIcon(":/Image/TreeIco/Image.svg");
    case RawData::TypeClassComImg:
        return QIcon(":/Image/TreeIco/CombiImage.svg");
    case RawData::TypeImgFolder:
    case RawData::TypeComImgFolder:
        return QIcon(":/Image/TreeIco/Folder.svg");
    case RawData::TypeImgGrpFolder:
        return QIcon(":/Image/TreeIco/FolderImgGroup.svg");
    case RawData::TypeImgFile:
        return QIcon(":/Image/TreeIco/ImageFile.svg");
    case RawData::TypeComImgFile:
        return QIcon(":/Image/TreeIco/ComImgFile.svg");
    default:
        return QIcon();
    }
}

QString TreeItem::getText()
{
    return data(Qt::DisplayRole).toString();
}

QString TreeItem::getNotes()
{
    return rawData->getDataMap()[id].brief;
}


