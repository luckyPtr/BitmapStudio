#include "treeitem.h"

TreeItem::TreeItem(TreeItem *parent)
    : _parent(parent),
    _type(UNKNOWN),
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

// 获取本节点第column列的数据
QVariant TreeItem::data(int column) const
{
    if(_type == PROJECT)
    {
        if(column == Qt::DisplayRole)
        {
            return QString("项目");// config.project;
        }
    }
    else if(_type == CLASS_SETTINGS)
    {
        return QString("设置");
    }
    else if(_type == CLASS_IMAGE)
    {
        if(column == RoleDisplay)
        {
            return QString("图片");
        }
    }
    else if(_type == CLASS_COMIMAGE)
    {
        return QString("组合图");
    }
    else if(_type == FOLDER)
    {
//        if(column == RoleDisplay)
//        {
//            return image.name;
//        }
    }
    else if(_type == FILE)
    {
//        if(column == RoleDisplay)
//        {
//            return image.name;
//        }
//        else
//        {
//            return QVariant::fromValue(image);
//        }
    }

    return QVariant();
}

QIcon TreeItem::icon() const
{
    switch (_type) {
    case PROJECT:
        return QIcon(":/Image/TreeIco/Project.svg");
    case CLASS_SETTINGS:
        return QIcon(":/Image/TreeIco/Setting.svg");
    case CLASS_IMAGE:
        return QIcon(":/Image/TreeIco/Image.svg");
    case CLASS_COMIMAGE:
        return QIcon(":/Image/TreeIco/CombiImage.svg");
    default:
        return QIcon();
    }
}

//BmImg *TreeItem::bmImg() const
//{

//}
