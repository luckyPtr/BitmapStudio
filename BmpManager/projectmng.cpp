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
        item->setData(QVariant(id));
        item->setData(QVariant(type), RoleType);
        // 设置图标
        if(type == QString::fromLocal8Bit("DIR"))
        {
            item->setIcon(QIcon(":/Image/TreeIco/Folder.png"));
        }
        else if(type == QString::fromLocal8Bit("BMP"))
        {
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
    Project_t project;
    project.db = QSqlDatabase::addDatabase("QSQLITE");
    project.db.setDatabaseName(pro);
    project.path = pro;
    QFileInfo fileInfo(pro);
    project.name = fileInfo.baseName();
    if(project.db.open())
    {
        proList << project;
    }
}

void ProjectMng::initModel()
{
    proModel->clear();
    foreach (Project_t pro, proList) {
        QStandardItem *parent = new QStandardItem(QIcon(":/Image/TreeIco/Project.png"), pro.name);
        proModel->appendRow(parent);

        QStandardItem* item = new QStandardItem(QIcon(":/Image/TreeIco/Setting.svg"), tr("设置"));
        parent->appendRow(item);

        item = new QStandardItem(QIcon(":/Image/TreeIco/Image.svg"), tr("图片"));
        parent->appendRow(item);
        addSonToModel(pro.db, 0, item);

        item = new QStandardItem(QIcon(":/Image/TreeIco/CombiImage.svg"), tr("组合图"));
        parent->appendRow(item);
    }
}

