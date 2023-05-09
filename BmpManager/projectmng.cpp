#include "projectmng.h"

void ProjectMng::addSonToModel(QSqlDatabase db, int pid, QStandardItem *parent)
{
    QString qreryString= QString("SELECT id,name,type FROM tbl_img where pid=%1 ").arg(pid);
    QSqlQuery query(qreryString, db);
    while (query.next())
    {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        QString type = query.value(2).toString();
        QStandardItem* item =new QStandardItem(name);
        item->setData(QVariant(id), RoleId);
        // 设置图标
        if(type == QString::fromLocal8Bit("DIR"))
        {
            item->setData(QVariant(NodeFolder), RoleNodeType);
            item->setIcon(QIcon(":/Image/TreeIco/Folder.png"));
        }
        else if(type == QString::fromLocal8Bit("BMP"))
        {
            item->setData(QVariant(NodeFile), RoleNodeType);
            item->setIcon(QIcon(":/Image/TreeIco/BMP.png"));
        }
        parent->appendRow(item);
        addSonToModel(db, id, item);
    }
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
    if(db.open())
    {
        proList << db;
    }
}

void ProjectMng::initModel()
{
    proModel->clear();

    foreach(QSqlDatabase db, proList)
    {
        QStandardItem *parent = new QStandardItem(QIcon(":/Image/TreeIco/Project.png"), QFileInfo(db.databaseName()).baseName());
        parent->setDragEnabled(false);
        parent->setDropEnabled(false);
        parent->setData(QVariant(db.databaseName()), RoleProjPath);
        parent->setData(QVariant(NodeProjRoot), RoleNodeType);
        proModel->appendRow(parent);

        QStandardItem* item = new QStandardItem(QIcon(":/Image/TreeIco/Setting.svg"), tr("设置"));
        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setData(QVariant(NodeSettings), RoleNodeType);
        parent->appendRow(item);

        item = new QStandardItem(QIcon(":/Image/TreeIco/Image.svg"), tr("图片"));
        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setData(QVariant(NodeImage), RoleNodeType);
        parent->appendRow(item);
        qDebug() << "Database" << db.databaseName();
        addSonToModel(db, 0, item);

        item = new QStandardItem(QIcon(":/Image/TreeIco/CombiImage.svg"), tr("组合图"));
        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setData(QVariant(NodeComImage), RoleNodeType);
        parent->appendRow(item);
    }


}

QSqlDatabase ProjectMng::getIndexDatabase(QModelIndex index)
{
    QString projectPath = getIndexProject(index);
    foreach (QSqlDatabase db, proList) {
        if(db.databaseName() == projectPath)
            return db;
    }
    return proList.at(0);
}

QString ProjectMng::getIndexProject(QModelIndex index)
{
    while(!index.data(RoleProjPath).isValid())
    {
        index = index.parent();
    }
    return index.data(RoleProjPath).toString();
}

void ProjectMng::createFolder(QModelIndex &index)
{

}





