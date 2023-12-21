#include "projectmng.h"
#include <string.h>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <dialognewimgfile.h>
#include <QFileDialog>
#include <custom/dialognewfolder.h>
#include <custom/dialogprojectsettings.h>
#include <imgconvertor.h>

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
    actNewProject = new QAction(tr("新建项目"), this);
    connect(actNewProject, SIGNAL(triggered()), this, SLOT(on_ActNewProject_Triggered()));

    actOpenProject = new QAction(tr("载入项目"), this);
    connect(actOpenProject, SIGNAL(triggered()), this, SLOT(on_ActOpenProject_Triggered()));

    actNewImgFile = new QAction(tr("图片"), this);
    connect(actNewImgFile, SIGNAL(triggered()), this, SLOT(on_ActNewImg_Triggered()));

    actNewComImgFile = new QAction(tr("组合图"), this);
    connect(actNewComImgFile, SIGNAL(triggered()), this, SLOT(on_ActNewComImg_Triggered()));

    actNewFolder = new QAction(tr("文件夹"), this);
    connect(actNewFolder, SIGNAL(triggered()), this, SLOT(on_ActNewFolder_Triggered()));

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

    actImgGrpFolder = new QAction(tr("转换为图片组文件夹"), this);
    connect(actImgGrpFolder, SIGNAL(triggered()), this, SLOT(on_ActImgGrpFolder_Triggered()));

    actRename = new QAction(tr("重命名"), this);
    connect(actRename, SIGNAL(triggered()), this, SLOT(on_ActRename_Triggered()));

    actSettings = new QAction(tr("设置"), this);
    connect(actSettings, SIGNAL(triggered()), this, SLOT(on_ActSettings_Triggered()));

    actRun = new QAction(tr("运行"), this);
    connect(actRun, SIGNAL(triggered()), this, SLOT(on_ActRun_Triggered()));
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




void ProjectMng::createFolder(QModelIndex index, QString name, QString brief)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->createFolder(item->getID(), name, brief);
}

void ProjectMng::createImage(QModelIndex &index, QString name, QSize size, QString brief)
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
        rd->createBmp(id, name, size, brief);
    }
    else if(type == RawData::TypeClassComImg ||\
            type == RawData::TypeComImgFolder ||\
            type == RawData::TypeComImgFolder)
    {
        rd->createComImg(id, name, size);
    }
}

void ProjectMng::createImage(QModelIndex &index, QString name, QImage &img)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->createBmp(item->getID(), name, img);
}

void ProjectMng::createComImg(QModelIndex &index, QString name, QSize size, QString brief)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    rd->createComImg(item->getID(), name, size, brief);
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
    tabWidget->closeTab(item);
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
        menu.addAction(actNewProject);
        menu.addAction(actOpenProject);
        menu.addAction(actRun);
        menu.addAction(actSettings);
        menu.addSeparator();
        menu.addAction(actProperties);

        menu.exec(QCursor::pos());
    };

    // 图片菜单
    auto menuClassImg = [=]() {
        QMenu menu;
        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);
        menu.exec(QCursor::pos());
    };

    // 图片文件夹菜单
    auto menuImgFolder = [=]() {
        QMenu menu;

        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);

        menu.addSeparator();

        menu.addAction(actDelete);

        menu.addAction(actRename);

        menu.addAction(actImgGrpFolder);

        // 非最后一级菜单
        if(item->getRawData()->haveSubFolder(item->getID()))
        {
            actImgGrpFolder->setEnabled(false);
        }
        else
        {
            actImgGrpFolder->setEnabled(true);
            if(item->getType() == RawData::TypeImgGrpFolder)
            {
                actImgGrpFolder->setText(tr("转换为普通文件夹"));
            }
            else
            {
                actImgGrpFolder->setText(tr("转换为图片组"));
            }
        }

        menu.addSeparator();
        menu.addAction(actProperties);

        menu.exec(QCursor::pos());
    };

    // 图片文件菜单
    auto menuImgFile = [=]() {
        QMenu menu;

        menu.addAction(actOpen);

        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);

        menu.addAction(actDelete);
        menu.addAction(actRename);
        menu.addAction(actProperties);
        menu.exec(QCursor::pos());
    };

    // 组合图
    auto menuClassComImg = [=]() {
        QMenu menu;
        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewComImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);
        menu.exec(QCursor::pos());
    };

    // 组合图文件夹菜单
    auto menuComImgFolder = [=]() {
        QMenu menu;

        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewComImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);

        menu.addSeparator();
        menu.addAction(actRename);
        menu.addAction(actDelete);

        menu.addSeparator();
        menu.addAction(actProperties);

        menu.exec(QCursor::pos());
    };

    // 组合图文件
    auto menuComImgFile = [=]() {
        QMenu menu;
        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewComImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);

        menu.addSeparator();
        menu.addAction(actRename);
        menu.addAction(actDelete);

        menu.addSeparator();
        menu.addAction(actProperties);

        menu.exec(QCursor::pos());
    };


    switch (item->getType())
    {
    case RawData::TypeProject:
        menuProject();
        break;
    case RawData::TypeClassImg:
        menuClassImg();
        break;
    case RawData::TypeImgFolder:
    case RawData::TypeImgGrpFolder:
        menuImgFolder();
        break;
    case RawData::TypeImgFile:
        menuImgFile();
        break;
    case RawData::TypeClassComImg:
        menuClassComImg();
        break;
    case RawData::TypeComImgFolder:
        menuComImgFolder();
        break;
    case RawData::TypeComImgFile:
        menuComImgFile();
        break;
    default:
        break;
    }
}

void ProjectMng::on_ActNewProject_Triggered()
{
    QString aFile = QFileDialog::getSaveFileName(this, tr("保存工程"), "Untiled", tr("BmpManager工程(*.db)"));
    if(!aFile.isEmpty())
    {
        openProject(aFile);
        initModel();
    }
}

void ProjectMng::on_ActOpenProject_Triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, tr("打开工程"), "", tr("BmpManager工程(*.db)"));
    if(!aFile.isEmpty())
    {
        openProject(aFile);
        initModel();
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
        QString s = QString(tr("关闭项目%1?")).arg(item->getText());
        if(QMessageBox::question(this, tr("关闭项目"), s) == QMessageBox::Yes)
        {
            closeProjcet(currentIndex);
            initModel();
        }
    }

}

void ProjectMng::on_ActDelete_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);

    switch(item->getType())
    {
    case RawData::TypeImgFolder:
    case RawData::TypeImgGrpFolder:
    case RawData::TypeComImgFolder:
        if(QMessageBox::question(this, tr("删除文件夹"), QString(tr("删除文件夹%1及其所有子文件?")).arg(item->getText())) == QMessageBox::Yes)
        {
            remove(currentIndex);
            initModel();
        }
        break;
    case RawData::TypeImgFile:
    case RawData::TypeComImgFile:
        if(QMessageBox::question(this, tr("删除文件"), QString(tr("删除文件%1?")).arg(item->getText())) == QMessageBox::Yes)
        {
            remove(currentIndex);
            initModel();
        }
        break;
    default:
        break;
    }
}

void ProjectMng::on_ActRename_Triggered()
{
    QString name = QInputDialog::getText(this, tr("重命名"), tr("名称"));
    if(!name.isEmpty())
    {
        rename(currentIndex, name);
    }
}

void ProjectMng::on_ActProperties_Triggered()
{
qDebug() << "actProperties";
}

void ProjectMng::on_ActNewImg_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);

    DialogNewImgFile *dlgNewImg = new DialogNewImgFile(this, item->getRawData()->getSize());
    int ret = dlgNewImg->exec();
    if(ret == QDialog::Accepted)
    {
        createImage(currentIndex, dlgNewImg->imgFileName(), dlgNewImg->size(), dlgNewImg->brief());
        initModel();
    }
    delete dlgNewImg;
}

void ProjectMng::on_ActNewComImg_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);

    DialogNewImgFile *dlgNewComImg = new DialogNewImgFile(this, item->getRawData()->getSize());
    dlgNewComImg->setWindowTitle(tr("新建组合图"));
    int ret = dlgNewComImg->exec();
    if(ret == QDialog::Accepted)
    {
        createComImg(currentIndex, dlgNewComImg->imgFileName(), dlgNewComImg->size(), dlgNewComImg->brief());
        initModel();
    }
    delete dlgNewComImg;
}

void ProjectMng::on_ActNewFolder_Triggered()
{
    DialogNewFolder *dlgNewFolder = new DialogNewFolder(this);
    if(dlgNewFolder->exec() == QDialog::Accepted)
    {
        QString name = dlgNewFolder->name();
        QString brief = dlgNewFolder->brief();
        if(!name.isEmpty())
        {
            createFolder(currentIndex, name, brief);
            treeView->expand(currentIndex);
            initModel();
        }
    }
    delete dlgNewFolder;
}

void ProjectMng::on_ActImgGrpFolder_Triggered()
{
    imgFolderConvert(currentIndex);
    initModel();
}

void ProjectMng::on_ActSettings_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    DialogProjectSettings *dlgSettings = new DialogProjectSettings(this);
    dlgSettings->init(item->getRawData()->getSettings());
    if(dlgSettings->exec() == QDialog::Accepted)
    {
        item->getRawData()->saveSettings(dlgSettings->getResult());
    }
    delete dlgSettings;
}

void ProjectMng::on_ActRun_Triggered()
{
    QString file_img_c;

    TreeItem *item = theModel->itemFromIndex(currentIndex);
    RawData::Settings settings = item->getRawData()->getSettings();
    ImgConvertor ic(item->getRawData()->getDataMap().values().toVector(), settings);

    QFileInfo fileInfo(item->getRawData()->getProject());
    QString path = fileInfo.path() + "/" + settings.path;  // 输出目录;

    if(!settings.path.isEmpty())
    {
        if(!QDir(path).exists())
        {
            QDir().mkdir(path);
        }
    }

    QFile file(path + "/bm_typedef.h");
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(ic.generateTypedefH().toUtf8());
        file.close();
    }

    file.setFileName(path + "/bm_img.c");
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(ic.generateImgC().toUtf8());
        file.close();
    }

    file.setFileName(path + "/bm_img.h");
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(ic.generateImgH().toUtf8());
        file.close();
    }

    file.setFileName(path + "/bm_com_img.c");
    if(file.open(QIODevice::WriteOnly))
    {

        file.write(ic.generateComImgC().toUtf8());
        file.close();
    }

    file.setFileName(path + "/bm_com_img.h");
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(ic.generateComImgH().toUtf8());
        file.close();
    }

    QMessageBox::information(this, "","字模转换完成");
}
