#include "projectmng.h"
#include <string.h>

void ProjectMng::addSonToModel(QSqlDatabase db, QString table, int pid, QStandardItem *parent)
{
    QString qreryString= QString("SELECT id,name,type FROM %1 where pid=%2 ").arg(table).arg(pid);
    QSqlQuery query(qreryString, db);

    while (query.next())
    {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        QString type = query.value(2).toString();
        QStandardItem* item =new QStandardItem(name);
        item->setData(QVariant(id), RoleId);
        // 设置图标
        if(type == QString::fromLocal8Bit("CLASS"))
        {
            if(id == 10000)
                item->setIcon(QIcon(":/Image/TreeIco/Image.svg"));
            else if(id == 20000)
                item->setIcon(QIcon(":/Image/TreeIco/CombiImage.svg"));
        }
        else if(type == QString::fromLocal8Bit("FOLDER"))
        {
            item->setData(QVariant(NodeFolder), RoleNodeType);
            item->setIcon(QIcon(":/Image/TreeIco/Folder.png"));
        }
        else if(type == QString::fromLocal8Bit("FILE"))
        {
            item->setData(QVariant(NodeFile), RoleNodeType);
            item->setIcon(QIcon(":/Image/TreeIco/BMP.png"));
        }
        parent->appendRow(item);
        addSonToModel(db, table, id, item);
    }
}

void ProjectMng::getExpandNode(QModelIndex root, QVector<int> &expandNode)
{
    if(treeView->isExpanded(root))
    {
        expandNode << root.data(RoleId).toInt();
    }
    for(int i = 0; i < proModel->rowCount(root); i++)
    {
        QModelIndex index = proModel->index(i, 0, root);
        getExpandNode(index, expandNode);
    }
}

void ProjectMng::setExpandNode(QModelIndex root, QVector<int> &expandNode)
{
    if(expandNode.contains(root.data(RoleId).toInt()))
    {
      treeView->expand(root);
    }
    for(int i = 0; i < proModel->rowCount(root); i++)
    {
        QModelIndex index = proModel->index(i, 0, root);
        setExpandNode(index, expandNode);
    }
}

void ProjectMng::saveExpand()
{
    for(int i = 0; i < proModel->rowCount(); i++)
    {
        QModelIndex root = proModel->item(i)->index();
        QVector<int> expandNode;    // 展开的节点ID列表
        getExpandNode(root, expandNode);
        QByteArray byteArray;
        byteArray.resize(expandNode.size() * sizeof(int));
        for(int i = 0; i < expandNode.size(); i++)
        {
            int id = expandNode.at(i);
            memcpy(byteArray.data() + i * sizeof(int), &id, sizeof(int));
        }

        QSqlQuery query(getIndexDatabase(root));
        query.prepare("UPDATE tbl_settings SET expand = ?");
        query.bindValue(0, byteArray);
        query.exec();
    }
}

void ProjectMng::restoreExpand()
{
    for(int i = 0; i < proModel->rowCount(); i++)
    {
        QModelIndex root = proModel->item(i)->index();

        QSqlQuery query(getIndexDatabase(root));
        query.prepare("SELECT expand FROM tbl_settings");
        query.exec();

        if(query.first())
        {
            QByteArray byteArray = query.value(0).toByteArray();
            QVector<int> expandNode;    // 展开的节点ID列表
            for(int i = 0; i < byteArray.size(); i += sizeof(int))
            {
                int id;
                memcpy(&id, byteArray.data() + i, sizeof(int));
                expandNode << id;
            }
            setExpandNode(root, expandNode);
        }
    }
}

QString ProjectMng::getTblName(int id)
{
    if(id >= 10000 && id < 20000)
    {
        return "tbl_img";
    }
    else if(id >= 20000 && id < 30000)
    {
        return "tbl_comimg";
    }
    else if(id >= 30000)
    {
        return "tbl_font";
    }
    return "tbl_settings";
}


ProjectMng::ProjectMng(QWidget *parent)
    : QWidget{parent}
{
    proModel = new QStandardItemModel(this);
}

void ProjectMng::openProject(QString pro)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", QFileInfo(pro).baseName());
    db.setDatabaseName(pro);
//   db.exec("PRAGMA synchronous = OFF");
//    db.exec("PRAGMA journal_mode = MEMORY");
    if(db.open())
    {
        proList << db;
    }
}

// 根据数据库初始化model
void ProjectMng::initModel()
{
    // TODO: 一次读取数据库，提高速度

    treeView->setUpdatesEnabled(false);
    saveExpand();
    proModel->clear();
    for(int i = 0; i < proList.size(); i++)
    {
        QStandardItem *parent = new QStandardItem(QIcon(":/Image/TreeIco/Project.svg"), QFileInfo(proList[i].databaseName()).baseName());
        parent->setDragEnabled(false);
        parent->setDropEnabled(false);
        parent->setData(QVariant(0), RoleId);
        proModel->invisibleRootItem()->appendRow(parent);

        addSonToModel(proList[i], "tbl_img", 0, parent);
        addSonToModel(proList[i], "tbl_comimg", 0, parent);
    }

    restoreExpand();
    treeView->setUpdatesEnabled(true);

}

void ProjectMng::blindTreeView(QTreeView *treeView)
{
    this->treeView = treeView;
    treeView->setModel(proModel);
}

QSqlDatabase ProjectMng::getIndexDatabase(QModelIndex index)
{
    while(index.data(RoleId).toInt() != 0)
    {
        index = index.parent();
    }
    return proList[index.row()];
}


void ProjectMng::createFolder(QModelIndex index)
{
    QSqlDatabase db = getIndexDatabase(index);
    QSqlQuery query(db);

    int id = index.data(RoleId).toInt();
    query.prepare("SELECT pid,type FROM tbl_img WHERE id=?");
    query.bindValue(0, id);
    query.exec();
    if(query.first())
    {
        int pid = query.value(0).toInt();
        QString type = query.value(1).toString();

        if(type == QString::fromUtf8("FILE"))
        {
            id = pid;
        }

        query.prepare("INSERT INTO tbl_img (name,type,pid) VALUES(:name,:type,:pid)");
        query.bindValue(":name", "Untiled");
        query.bindValue(":type", "FOLDER");
        query.bindValue(":pid", id);
        query.exec();
    }
}





