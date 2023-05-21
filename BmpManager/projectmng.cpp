#include "projectmng.h"
#include <string.h>
#include <QInputDialog>

void ProjectMng::addImgNode(RawData *rd, const quint16 pid, TreeItem *parent)
{
    QMap<quint16, BmImg> imgMap = rd->getImgMap();
    foreach(auto bi, imgMap)
    {
        if(bi.pid == pid)
        {
            TreeItem *item = new TreeItem();
            item->setID(bi.id);
            item->setProject(rd->getProject());
            item->setType(bi.isFolder ? TreeItem::FOLDER : TreeItem::FILE);
            item->setRawData(rd);
            parent->addChild(item);
            addImgNode(rd, bi.id, item);
        }
    }
}

//void ProjectMng::addSonToModel(QSqlDatabase db, QString table, int pid, QStandardItem *parent)
//{
//    QString qreryString= QString("SELECT id,name,type FROM %1 where pid=%2 ").arg(table).arg(pid);
//    QSqlQuery query(qreryString, db);

//    while (query.next())
//    {
//        int id = query.value(0).toInt();
//        QString name = query.value(1).toString();
//        QString type = query.value(2).toString();
//        QStandardItem* item =new QStandardItem(name);
//        item->setData(QVariant(id), RoleId);
//        // 设置图标
//        if(type == QString::fromLocal8Bit("CLASS"))
//        {
//            if(id == 10000)
//                item->setIcon(QIcon(":/Image/TreeIco/Image.svg"));
//            else if(id == 20000)
//                item->setIcon(QIcon(":/Image/TreeIco/CombiImage.svg"));
//        }
//        else if(type == QString::fromLocal8Bit("FOLDER"))
//        {
//            item->setData(QVariant(NodeFolder), RoleNodeType);
//            item->setIcon(QIcon(":/Image/TreeIco/Folder.png"));
//        }
//        else if(type == QString::fromLocal8Bit("FILE"))
//        {
//            item->setData(QVariant(NodeFile), RoleNodeType);
//            item->setIcon(QIcon(":/Image/TreeIco/BMP.png"));
//        }
//        parent->appendRow(item);
//        addSonToModel(db, table, id, item);
//    }
//}

void ProjectMng::getExpandNode(QModelIndex root)
{
    TreeItem *item = theModel->itemFromIndex(root);
    quint16 id = item->getID();
    if(treeView->isExpanded(root))
    {
        item->getRawData()->addExpandNode(id);
    }
    for(int i = 0; i < theModel->rowCount(root); i++)
    {
        QModelIndex index = theModel->index(i, 0, root);
        getExpandNode(index);
    }
}

void ProjectMng::setExpandNode(QModelIndex root)
{
    TreeItem *item = theModel->itemFromIndex(root);
    quint16 id = item->getID();
    if(item->getRawData()->isExpandNode(id))
    {
        treeView->expand(root);
    }
    for(int i = 0; i < theModel->rowCount(root); i++)
    {
        QModelIndex index = theModel->index(i, 0, root);
        setExpandNode(index);
    }
}

void ProjectMng::saveExpand()
{
    for(int i = 0; i < theModel->rowCount(treeView->rootIndex()); i++)
    {
        QModelIndex root = theModel->index(i, 0, treeView->rootIndex());
        TreeItem *item = theModel->itemFromIndex(root);
        item->getRawData()->clearExpandNode();
        getExpandNode(root);
    }
}

void ProjectMng::restoreExpand()
{
    for(int i = 0; i < theModel->rowCount(treeView->rootIndex()); i++)
    {
        QModelIndex root = theModel->index(i, 0, treeView->rootIndex());
        setExpandNode(root);
    }
}



ProjectMng::ProjectMng(QWidget *parent)
    : QWidget{parent}
{
    theModel = new TreeModel(this);

}

void ProjectMng::openProject(QString pro)
{
    //RawData rd(pro);
    projList << pro;        // 惊，还可以这样？？？
}

// 根据数据库初始化model
void ProjectMng::initModel()
{
    // TODO: 一次读取数据库，提高速度
    QElapsedTimer timer;
    timer.start();
    saveExpand();
    theModel->beginReset(); // 需要使用beginReset和endReset通知treeview刷新
    theModel->clear();
    for(int i = 0; i < projList.size(); i++)
    {
        TreeItem *proItem = new TreeItem();
        proItem->setType(TreeItem::PROJECT);
        proItem->setRawData(&projList[i]);
        proItem->setID(-1);
        theModel->root()->addChild(proItem);

        TreeItem *itemSettings = new TreeItem();
        itemSettings->setType(TreeItem::CLASS_SETTINGS);
        itemSettings->setRawData(&projList[i]);
        itemSettings->setID(-2);
        proItem->addChild(itemSettings);

        TreeItem *itemImage = new TreeItem();
        itemImage->setType(TreeItem::CLASS_IMAGE);
        itemImage->setRawData(&projList[i]);
        itemImage->setID(0);
        proItem->addChild(itemImage);
        addImgNode(&projList[i], 0, itemImage);

        TreeItem *itemComImage = new TreeItem();
        itemComImage->setType(TreeItem::CLASS_COMIMAGE);
        itemComImage->setRawData(&projList[i]);
        itemComImage->setID(10000);
        proItem->addChild(itemComImage);
    }
    theModel->endReset();
    restoreExpand();

    qDebug() << "耗时" << timer.elapsed();
}

void ProjectMng::blindTreeView(QTreeView *treeView)
{
    this->treeView = treeView;
    treeView->setModel(theModel);
}


void ProjectMng::createFolder(QModelIndex index)
{
    bool ok = false;
    QString text = QInputDialog::getText(this, tr("新建文件夹"), tr("文件夹名"), QLineEdit::Normal, "", &ok);
    if(ok && !text.isEmpty())
    {
        TreeItem *item = theModel->itemFromIndex(index);
        RawData *rd = item->getRawData();
        rd->createFolder(item->getID(), text);
    }
}





