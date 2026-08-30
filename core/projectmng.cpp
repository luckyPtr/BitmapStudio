#include "projectmng.h"
#include <string.h>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QFileInfo>
#include <gui/dialognewimgfile.h>
#include <QFileDialog>
#include <gui/dialognewfolder.h>
#include <gui/dialogprojectsettings.h>
#include <gui/dialogimporthex.h>
#include <gui/dialogimportimg.h>
#include <core/imgconvertor.h>
#include "core/exportrunner.h"
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>
#include "gui/dialognotice.h"
#include "gui/dialogloading.h"
#include "gui/dialognewproject.h"
#include "custom/customtab.h"

void ProjectMng::addDataNodes(RawData *rd, const quint16 pid, TreeItem *parent, bool (*filter)(int))
{
    QList<BmFile> imgList;

    // 找出图片数据
    QMap<quint16, BmFile> dataMap = rd->getDataMap();
    foreach(const auto &key, dataMap.keys())
    {
        // id=0 是虚拟根；防御异常产生的幽灵节点(key=0)进入建树递归导致栈溢出
        if(dataMap.value(key).id != 0 && filter(dataMap.value(key).type))
        {
            imgList << dataMap.value(key);
        }
    }

    // 同级排序：容器（文件夹/图片组）在前、叶子在后，组内按id升序（即工程文件中的顺序）
    std::sort(imgList.begin(), imgList.end(), [](const BmFile &file1, const BmFile &file2){
        bool container1 = (file1.type == RawData::TypeImgFolder ||
                           file1.type == RawData::TypeImgGrpFolder ||
                           file1.type == RawData::TypeComImgFolder);
        bool container2 = (file2.type == RawData::TypeImgFolder ||
                           file2.type == RawData::TypeImgGrpFolder ||
                           file2.type == RawData::TypeComImgFolder);
        if(container1 != container2) {
            return container1;
        }
        return file1.id < file2.id;
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

    actImportFromImg = new QAction(tr("图片"), this);
    connect(actImportFromImg, SIGNAL(triggered()), this, SLOT(on_ActImportFromImg_Triggered()));

    actImportFromHex = new QAction(tr("字模"), this);
    connect(actImportFromHex, SIGNAL(triggered()), this, SLOT(on_ActImportFromHex_Triggered()));

    actNewFolder = new QAction(tr("文件夹"), this);
    connect(actNewFolder, SIGNAL(triggered()), this, SLOT(on_ActNewFolder_Triggered()));

    actOpen = new QAction(tr("打开文件"), this);
    connect(actOpen, SIGNAL(triggered()), this, SLOT(on_ActOpen_Triggered()));

    actCloseProject = new QAction(tr("关闭项目"), this);
    connect(actCloseProject, SIGNAL(triggered()), this, SLOT(on_ActCloseProject_Triggered()));

    actDelete = new QAction(QIcon(":/Image/TreeIco/Delete.svg"), tr("删除"), this);
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

    actExportImg = new QAction(tr("保存图片到本地"));
    connect(actExportImg, SIGNAL(triggered()), this, SLOT(on_ActExportImg_Triggered()));

    actCopyImg = new QAction(tr("复制图片到剪贴板"));
    connect(actCopyImg, SIGNAL(triggered()), this, SLOT(on_ActCopyImg_Triggered()));

    actCopyName = new QAction(tr("复制名称到剪贴板"));
    connect(actCopyName, SIGNAL(triggered()), this, SLOT(on_ActCopyName_Triggeded()));


    actReplaceFromImg = new QAction(tr("从图片"));
    connect(actReplaceFromImg, SIGNAL(triggered()), this, SLOT(on_ActReplaceFromImg_Triggered()));

    actReplaceFromHex = new QAction(tr("从字模"));
    connect(actReplaceFromHex, SIGNAL(triggered()), this, SLOT(on_ActReplaceFromHex_Triggered()));
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
    RawData rd(pro);
    if (!rd.isValid())
    {
        // 无效工程（非JSON、损坏或旧版SQLite格式），统一提示；
        // 用模态错误框而非toast：这类错误阻止后续操作，需要用户明确知晓
        QMessageBox errBox;
        errBox.setIcon(QMessageBox::Critical);
        errBox.setWindowTitle(tr("无法打开"));
        errBox.setText(tr("%1 不是有效的 Bitmap Studio 工程文件\n(旧版工程请使用 tools/convert_legacy.py 转换)").arg(QFileInfo(pro).fileName()));
        errBox.addButton(QObject::tr("确定"), QMessageBox::AcceptRole);
        errBox.exec();
        return;
    }
    projList << rd;
}

void ProjectMng::newProject(QString pro, RawData::Settings settings)
{
    // 防止在已有工程文件（含旧版SQLite工程）路径上新建导致原文件被覆盖
    QFileInfo info(pro);
    if (info.isFile())
    {
        QMessageBox warnBox;
        warnBox.setIcon(QMessageBox::Warning);
        warnBox.setWindowTitle(tr("无法新建"));
        warnBox.setText(tr("工程文件已存在，请直接打开或更换名称/路径"));
        warnBox.addButton(QObject::tr("确定"), QMessageBox::AcceptRole);
        warnBox.exec();
        return;
    }
    RawData newProject(pro);
    warnSaveFailed(newProject.saveSettings(settings));
    projList << newProject;}

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

//        TreeItem *itemSettings = new TreeItem();
//        itemSettings->setType(RawData::TypeClassSettings);
//        itemSettings->setRawData(&projList[i]);
//        itemSettings->setID(-2);
//        proItem->addChild(itemSettings);

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
    connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(on_Clicked_Triggered(QModelIndex)));
}

void ProjectMng::setTabWidget(CustomTabWidget *tabWidget)
{
    this->tabWidget = tabWidget;
}




void ProjectMng::warnSaveFailed(bool saved)
{
    if (!saved)
    {
        // 模态错误框而非toast：保存失败意味着编辑可能丢失，需用户手动确认
        QMessageBox errBox;
        errBox.setIcon(QMessageBox::Critical);
        errBox.setWindowTitle(tr("保存失败"));
        errBox.setText(tr("保存工程文件失败（磁盘已满/文件被占用/目录只读），当前编辑仅存在于内存中，请检查后重试"));
        errBox.addButton(QObject::tr("确定"), QMessageBox::AcceptRole);
        errBox.exec();
    }
}

void ProjectMng::createFolder(QModelIndex index, QString name, QString brief)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    warnSaveFailed(rd->createFolder(item->getID(), name, brief));
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
        warnSaveFailed(rd->createBmp(id, name, size, brief));
    }
    else if(type == RawData::TypeClassComImg ||\
            type == RawData::TypeComImgFolder ||\
            type == RawData::TypeComImgFolder)
    {
        warnSaveFailed(rd->createComImg(id, name, size));
    }
}

void ProjectMng::createImage(QModelIndex &index, QString name, QImage &img, QString brief)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    warnSaveFailed(rd->createBmp(item->getID(), name, img, brief));
}

void ProjectMng::createComImg(QModelIndex &index, QString name, QSize size, QString brief)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    warnSaveFailed(rd->createComImg(item->getID(), name, size, brief));
}

void ProjectMng::rename(QModelIndex &index, QString name)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    warnSaveFailed(rd->rename(item->getID(), name));
}

QString ProjectMng::getName(QModelIndex &index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    return rd->getName(item->getID());
}

void ProjectMng::remove(QModelIndex &index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    tabWidget->closeTab(item);
    // RawData::remove已同步维护内存dataMap，不再整体load()——
    // 重载会按文件位置重编所有内存id，导致已打开编辑器标签页持有的id错位
    warnSaveFailed(rd->remove(item->getID()));
}

void ProjectMng::imgFolderConvert(QModelIndex &index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    warnSaveFailed(rd->imgFolderConvert(item->getID()));
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
    warnSaveFailed(rd->setImage(item->getID(), image));
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
    warnSaveFailed(rd->setComImg(item->getID(), comImg));
}

QString ProjectMng::getBrief(QModelIndex index)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    return rd->getBrief(item->getID());
}

void ProjectMng::setBrief(QModelIndex index, QString brief)
{
    TreeItem *item = theModel->itemFromIndex(index);
    RawData *rd = item->getRawData();
    warnSaveFailed(rd->setBrief(item->getID(), brief));
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

void ProjectMng::on_Clicked_Triggered(QModelIndex index)
{
    currentIndex = index;
    TreeItem *item = theModel->itemFromIndex(currentIndex);

    emit updateSelectProject(item->getRawData()->getProject());
}




void ProjectMng::on_CustomContextMenu(QPoint point)
{
    currentIndex = treeView->indexAt(point);

    if(!currentIndex.isValid())
    {
        return;
    }
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    emit updateSelectProject(item->getRawData()->getProject());
    // 项目工程菜单
    auto menuProject = [=]() {
        QMenu menu;
        menu.addAction(actCloseProject);
        menu.addAction(actNewProject);
        menu.addAction(actOpenProject);
        menu.addAction(actRun);
        menu.addAction(actSettings);
        menu.addSeparator();
//        menu.addAction(actProperties);

        menu.exec(QCursor::pos());
    };

    // 图片菜单
    auto menuClassImg = [=]() {
        QMenu menu;
        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);

        QMenu menuImport(tr("导入"));
        menuImport.addAction(actImportFromImg);
        menuImport.addAction(actImportFromHex);
        menu.addMenu(&menuImport);

        menu.exec(QCursor::pos());
    };

    // 图片文件夹菜单
    auto menuImgFolder = [=]() {
        QMenu menu;

        QMenu menuNew(tr("新建"));
        menuNew.addAction(actNewImgFile);
        menuNew.addAction(actNewFolder);
        menu.addMenu(&menuNew);

        QMenu menuImport(tr("导入"));
        menuImport.addAction(actImportFromImg);
        menuImport.addAction(actImportFromHex);
        menu.addMenu(&menuImport);

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
//        menu.addAction(actProperties);

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

        QMenu menuReplace(tr("替换"));
        menuReplace.addAction(actReplaceFromImg);
        menuReplace.addAction(actReplaceFromHex);
        menu.addMenu(&menuReplace);

        QMenu menuExport(tr("导出"));
        menuExport.addAction(actExportImg);
        menuExport.addAction(actCopyImg);
        menuExport.addAction(actCopyName);
        menu.addMenu(&menuExport);

        menu.addAction(actDelete);
        menu.addAction(actRename);
//        menu.addAction(actProperties);
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
//        menu.addAction(actProperties);

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

        QMenu menuExport(tr("导出"));
        menuExport.addAction(actExportImg);
        menuExport.addAction(actCopyImg);
        menuExport.addAction(actCopyName);
        menu.addMenu(&menuExport);

        menu.addSeparator();
//        menu.addAction(actProperties);

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
    DialogNewProject *dlg = new DialogNewProject(this);
    if(dlg->exec() == QDialog::Accepted)
    {
        QString projectFile = dlg->getNewProject();
        RawData::Settings settings = dlg->getSettings();
        if(!projectFile.isEmpty())
        {
            newProject(projectFile, settings);
            initModel();
        }
    }

    delete dlg;
}

void ProjectMng::on_ActOpenProject_Triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, tr("打开工程"), "", tr("Bitmap Studio工程(*.bms *.bs)"));
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
            // 关闭所有该工程的tab
            for(int i = tabWidget->count() - 1; i >= 0; i--)
            {
                if(item->getRawData()->getProject() == static_cast<CustomTab *>(tabWidget->widget(i))->getProject())
                {
                    if(tabWidget->currentIndex() == 1)
                    {
                        // 清除预览图片
                    }
                    tabWidget->removeTab(i);
                }
            }

            // 关闭项目管理器的项目
            closeProjcet(currentIndex);
            initModel();
            emit updateSelectProject(nullptr);  // 关闭后选中的工程为空
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
        warnSaveFailed(item->getRawData()->saveSettings(dlgSettings->getResult()));
    }
    delete dlgSettings;
}

void ProjectMng::on_ActRun_Triggered()
{
    DialogLoading *dlgLoading = new DialogLoading;
    dlgLoading->show();

    TreeItem *item = theModel->itemFromIndex(currentIndex);
    bool ok = ExportRunner::run(*item->getRawData());

    dlgLoading->close();
    DialogNotice *dlg = new DialogNotice(ok ? "字模转换完成!" : "字模转换失败，请检查输出目录权限");
    dlg->exec();
}

void ProjectMng::on_ActImportFromImg_Triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, tr("导入图片"), "", tr("图片(*.jpg *.jpeg *.png *.bmp);;JPEG(*.jpg *.jpeg);;PNG(*.png);;BMP(*.bmp)"));
    if(!aFile.isEmpty())
    {
        QImage img(aFile);
        DialogImportImg *dlgImportImg = new DialogImportImg(img, this);
        dlgImportImg->setImgName(QFileInfo(aFile).baseName());
        int ret = dlgImportImg->exec();
        if(ret == QDialog::Accepted)
        {
            QImage img = dlgImportImg->getMonoImg();
            QString brief = dlgImportImg->getBrief();
            createImage(currentIndex, dlgImportImg->getImgName(), img, brief);
            initModel();
        }
        delete dlgImportImg;
    }
}

void ProjectMng::on_ActImportFromHex_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    RawData::Settings settings = item->getRawData()->getSettings();
    DialogImportHex *dlgImportHex = new DialogImportHex(this);
    dlgImportHex->setDefaultMode(settings.mode);
    if(dlgImportHex->exec() == QDialog::Accepted)
    {
        QImage img = dlgImportHex->getImg();
        QString brief = dlgImportHex->getBrief();
        createImage(currentIndex, dlgImportHex->getName(), img, brief);
        initModel();
    }
    delete dlgImportHex;
}

void ProjectMng::on_ActReplaceFromImg_Triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, tr("导入图片"), "", tr("图片(*.jpg *.png *.bmp);;JPEG(*.jpg *.jpeg);;PNG(*.png);;BMP(*.bmp)"));
    if(!aFile.isEmpty())
    {
        QImage img(aFile);
        DialogImportImg *dlgImportImg = new DialogImportImg(img, this);
        dlgImportImg->setImgName(getName(currentIndex));
        QString brief = getBrief(currentIndex);
        if(!brief.isEmpty())
        {
            dlgImportImg->setBrief(brief);
        }
        int ret = dlgImportImg->exec();
        if(ret == QDialog::Accepted)
        {
            QImage img = dlgImportImg->getMonoImg();
            QString brief = dlgImportImg->getBrief();
            setImage(currentIndex, img);
            if(!brief.isEmpty())
            {
                setBrief(currentIndex, brief);
            }
            on_ActOpen_Triggered();
        }
        delete dlgImportImg;
    }
}

void ProjectMng::on_ActReplaceFromHex_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    RawData::Settings settings = item->getRawData()->getSettings();
    DialogImportHex *dlgImportHex = new DialogImportHex(this);
    dlgImportHex->setDefaultMode(settings.mode);
    dlgImportHex->setName(getName(currentIndex));
    dlgImportHex->setBrief(getBrief(currentIndex));
    if(dlgImportHex->exec() == QDialog::Accepted)
    {
        QImage img = dlgImportHex->getImg();
        QString brief = dlgImportHex->getBrief();
        setImage(currentIndex, img);
        if(!brief.isEmpty())
        {
            setBrief(currentIndex, brief);
        }
        on_ActOpen_Triggered();
    }
    delete dlgImportHex;
}

void ProjectMng::on_ActExportImg_Triggered()
{

    TreeItem *item = theModel->itemFromIndex(currentIndex);
    int id = item->getID();
    QImage img = item->getRawData()->getExportImage(id);
    QString name = item->getRawData()->getName(id);

    QString fileName = QFileDialog::getSaveFileName(this, "保存图片", name, "BMP(*.bmp);;PNG(*.png);;JPG(*jpg)");
    if(!fileName.isEmpty())
    {
        img.save(fileName);
    }
}

void ProjectMng::on_ActCopyImg_Triggered()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    QImage img = item->getRawData()->getExportImage(item->getID());
    QClipboard *clip = QApplication::clipboard();
    clip->setImage(img);

    DialogNotice *dlgNotice = new DialogNotice("已复制到剪贴板");
    dlgNotice->exec();
}

void ProjectMng::on_ActCopyName_Triggeded()
{
    TreeItem *item = theModel->itemFromIndex(currentIndex);
    QString name = item->getRawData()->getDataMap()[item->getID()].fullName;
    if(!name.isEmpty())
    {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(name);

        DialogNotice *dlg = new DialogNotice("图片名称已复制到剪贴板");
        dlg->exec();
    }
}

void ProjectMng::on_OpenProjectUrl(QString url)
{
    if(!url.isEmpty())
    {
        foreach (auto i, projList)
        {
            if (i.getProject() == url)
            {
                QMessageBox infoBox;
                infoBox.setIcon(QMessageBox::Information); // 设置图标为警告
                infoBox.setWindowTitle(tr("提示")); // 设置窗口标题
                infoBox.setText(tr("工程已打开")); // 设置主要文本内容
                // 添加按钮并设置文本为中文
                infoBox.addButton(QObject::tr("确定"), QMessageBox::AcceptRole);
                infoBox.exec();

                return;
            }
        }

        openProject(url);
        initModel();
    }
}

void ProjectMng::on_ImportImg(QModelIndex index, QString file)
{
    TreeItem *item = theModel->itemFromIndex(index);
    int type = item->getType();
    if (type == RawData::TypeImgFile || type == RawData::TypeImgFolder || type == RawData::TypeImgGrpFolder || type == RawData::TypeClassImg)
    {
        QImage img(file);
        DialogImportImg *dlgImportImg = new DialogImportImg(img, this);
        dlgImportImg->setImgName(QFileInfo(file).baseName());
        int ret = dlgImportImg->exec();
        if(ret == QDialog::Accepted)
        {
            QImage img = dlgImportImg->getMonoImg();
            QString brief = dlgImportImg->getBrief();
            createImage(index, dlgImportImg->getImgName(), img, brief);
            initModel();
        }
        delete dlgImportImg;
    }
    else
    {
        QMessageBox warningBox;
        warningBox.setIcon(QMessageBox::Critical); // 设置图标为警告
        warningBox.setWindowTitle(tr("错误")); // 设置窗口标题
        warningBox.setText(tr("无法导入图片")); // 设置主要文本内容
        // 添加按钮并设置文本为中文
        warningBox.addButton(QObject::tr("确定"), QMessageBox::AcceptRole);

        warningBox.exec();
    }
}
