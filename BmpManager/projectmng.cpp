#include "projectmng.h"
#include <string.h>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

void ProjectMng::addDataNodes(RawData *rd, const quint16 pid, TreeItem *parent, bool (*filter)(int))
{
    QList<BmFile> imgList;

    // 找出图片数据
    QMap<quint16, BmFile> dataMap = rd->getDataMap();
    foreach(const auto &key, dataMap.keys())
    {
        if(filter(dataMap.value(key).type))
        {
            imgList << dataMap.value(key);
        }
    }

    // 按照文件夹>文件，名称字母正序进行排序
    std::sort(imgList.begin(), imgList.end(), [](const BmFile &file1, const BmFile &file2){
        if(file1.type > file2.type) {
            return true;
        }
        else if(file1.type == file2.type) {
            return file1.name.toLower() < file2.name.toLower();
        }
        return false;
    });

    auto addNode = [=](auto&& self, const quint16 pid, TreeItem *parent) -> void{
        foreach(auto bf, imgList)
        {
            if(bf.pid == pid)
            {
                TreeItem *item = new TreeItem();
                item->setID(bf.id);
                item->setType(bf.type);
                item->setRawData(rd);
                parent->addChild(item);
                self(self, bf.id, item);
            }
        }
    };

    addNode(addNode, pid, parent);
}

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
    qDebug() << theModel->rowCount(treeView->rootIndex());

    for(int i = 0; i < theModel->rowCount(treeView->rootIndex()); i++)
    {
        QModelIndex root = theModel->index(i, 0, treeView->rootIndex());
        setExpandNode(root);
    }
}

void ProjectMng::initActions()
{
    actOpen = new QAction(tr("打开文件"), this);
    connect(actOpen, SIGNAL(triggered()), this, SLOT(on_ActOpen_Triggered()));

    actCloseProject = new QAction(tr("关闭项目"), this);
    connect(actCloseProject, SIGNAL(triggered()), this, SLOT(on_ActCloseProject_Triggered()));

    actDelete = new QAction(QIcon(":/Image/Toolbar/Delete.svg"), tr("删除"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(on_ActDelete_Triggered()));

    actRename = new QAction(tr("重命名"), this);
    connect(actRename, SIGNAL(triggered()), this, SLOT(on_ActRename_Triggered()));

    actProperties = new QAction(tr("属性"), this);
    connect(actProperties, SIGNAL(triggered()), this, SLOT(on_ActProperties_Triggered()));
}



ProjectMng::ProjectMng(QWidget *parent)
    : QWidget{parent}
{
    theModel = new TreeModel(this);
    initActions();
}

void ProjectMng::openProject(QString pro)
{
    // proList的类型从QVector更换为QList，打开三个工程崩溃的问题就没有出现
    // 但是可能并没有真正解决问题，可参考 https://zhidao.baidu.com/question/367115219524964612.html
    projList << pro;        // 惊，还可以这样？？？
}

void ProjectMng::closeProjcet(QModelIndex &index)
{
    projList.removeAt(index.row());
    theModel->removeRow(index);      // 删除model中的项目，否则后面保存TreeView中展开项的时候会崩溃
}

// 根据数据库初始化model
void ProjectMng::initModel()
{
    // TODO: 一次读取数据库，提高速度
    saveExpand();
    theModel->beginReset(); // 需要使用beginReset和endReset通知treeview刷新
    theModel->clear();
    for(int i = 0; i < projList.size(); i++)
    {
        TreeItem *proItem = new TreeItem();
        proItem->setType(RawData::TypeProject);
        proItem->setRawData(&projList[i]);
        proItem->setID(-1);
        theModel->root()->addChild(proItem);

        TreeItem *itemSettings = new TreeItem();
        itemSettings->setType(RawData::TypeClassSettings);
        itemSettings->setRawData(&projList[i]);
        itemSettings->setID(-2);
        proItem->addChild(itemSettings);

        TreeItem *itemImage = new TreeItem();
        itemImage->setType(RawData::TypeClassImg);
        itemImage->setRawData(&projList[i]);
        itemImage->setID(-3);
        proItem->addChild(itemImage);
        addDataNodes(&projList[i], 0, itemImage, RawData::isClassImgType);

        TreeItem *itemComImage = new TreeItem();
        itemComImage->setType(RawData::TypeClassComImg);
        itemComImage->setRawData(&projList[i]);
        itemComImage->setID(-4);
        proItem->addChild(itemComImage);
        addDataNodes(&projList[i], 0, itemComImage, RawData::isClassComImgType);

    }
    theModel->endReset();
    restoreExpand();
}

void ProjectMng::blindTreeView(QTreeView *treeView)
{
    this->treeView = treeView;
    treeView->setModel(theModel);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_CustomContextMenu(QPoint)));
}

void ProjectMng::setTabWidget(CustomTabWidget *tabWidget)
{
    this->tabWidget = tabWidget;
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

void ProjectMng::createImage(QModelIndex &index, QString name, quint16 width, quint16 height)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    int id = item->getID();
    int type = item->getType();

    if(type == RawData::TypeClassImg ||\
        type == RawData::TypeImgFolder ||\
        type == RawData::TypeImgGrpFolder ||\
        type == RawData::TypeImgFile)
    {
        rd->createBmp(id, name, width, height);
    }
    else if(type == RawData::TypeClassComImg ||\
            type == RawData::TypeComImgFolder ||\
            type == RawData::TypeComImgFolder)
    {
        rd->createComImg(id, name, QSize(width, height));
    }
}

void ProjectMng::createImage(QModelIndex &index, QString name, QImage &img)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->createBmp(item->getID(), name, img);
}

void ProjectMng::rename(QModelIndex &index, QString name)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->rename(item->getID(), name);
}

void ProjectMng::remove(QModelIndex &index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->remove(item->getID());
    rd->load();
}

void ProjectMng::imgFolderConvert(QModelIndex &index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->imgFolderConvert(item->getID());
}

QImage ProjectMng::getImage(QModelIndex index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    return rd->getImage(item->getID());
}

void ProjectMng::setImage(QModelIndex index, QImage &image)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->setImage(item->getID(), image);
}

ComImg ProjectMng::getComImg(QModelIndex index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    return rd->getComImg(item->getID());
}

void ProjectMng::setComImg(QModelIndex index, ComImg &comImg)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->setComImg(item->getID(), comImg);
}

QModelIndex ProjectMng::getModelIndex(QString project, int id)
{
    QModelIndex retIndex;

    auto getIndex = [&](auto&& self, QModelIndex &parentIndex) -> void {
        for(int i = 0; i < theModel->rowCount(parentIndex); i++)
        {
            QModelIndex index = theModel->index(i, 0, parentIndex);
            TreeItem *item = theModel->itemFromIndex(index);
            if(item->getID() == id)
            {
                retIndex = index;
                return;
            }
            self(self, index);
        }
    };

    QModelIndex rootIndex = treeView->rootIndex();
    for(int i = 0; i < theModel->rowCount(rootIndex); i++)
    {
        QModelIndex index = theModel->index(i, 0, rootIndex);
        TreeItem *item = theModel->itemFromIndex(index);
        if(item->getRawData()->getProject() == project)
        {
            getIndex(getIndex, rootIndex);
        }
    }

    return retIndex;
}

void ProjectMng::on_CustomContextMenu(QPoint point)
{
    currentIndex = treeView->indexAt(point);
    TreeItem *item = theModel->itemFromIndex(currentIndex);

    // 项目工程菜单
    auto menuProject = [=]() {
        QMenu menu;
        menu.addAction(actCloseProject);
        menu.exec(QCursor::pos());
    };

    // 图片文件菜单
    auto menuImgFile = [=]() {
        QMenu menu;
        menu.addAction(actOpen);
        menu.addAction(actDelete);
        menu.addAction(actRename);
        menu.addAction(actProperties);
        menu.exec(QCursor::pos());
    };


    switch (item->getType())
    {
    case RawData::TypeProject:
        menuProject();
        break;
    case RawData::TypeImgFile:
        menuImgFile();
        break;
    default:
        break;
    }



}

void ProjectMng::on_ActOpen_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    switch(item->getType())
    {
    case RawData::TypeImgFile:
    case RawData::TypeComImgFile:
        tabWidget->openTab(item);
        break;
    default:
        break;
    }
}

void ProjectMng::on_ActCloseProject_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    if(item->getType() == RawData::TypeProject)
    {
        QString s = QString(tr("关闭项目%1?")).arg(item->getRawData()->getProject());
        if(QMessageBox::question(this, tr("关闭项目"), s) == QMessageBox::Yes)
        {
            closeProjcet(currentIndex);
            initModel();
        }
    }

}

void ProjectMng::on_ActDelete_Triggered()
{
    qDebug() << "actDelete";
}

void ProjectMng::on_ActRename_Triggered()
{
qDebug() << "actRename";
}

void ProjectMng::on_ActProperties_Triggered()
{
qDebug() << "actProperties";
}
