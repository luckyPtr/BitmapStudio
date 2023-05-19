#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <treemodel.h>
#include <treeitem.h>
#include <rawdata.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveExpand(QModelIndex root)
{

    for(int i = 0; i < pm.model()->rowCount(root); i++)
    {
        QModelIndex index = pm.model()->index(i, 0, root);
        if(ui->treeViewProject->isExpanded(index))
        {

        }
        saveExpand(index);
    }
}

void MainWindow::restoreExpand()
{
    for(QModelIndex index : expandNode)
    {
        if(index.isValid())
        {
            //ui->treeViewProject->expand(index);
        }
        qDebug() << index;
    }
    expandNode.clear();
}


void MainWindow::init()
{
   //https://blog.csdn.net/wzz953200463/article/details/123643823

    //pm.blindTreeView(ui->treeViewProject);
    QStringList headers;
    TreeModel* model = new TreeModel(headers, ui->treeViewProject);
    TreeItem* root = model->root();
    TreeItem* province = new TreeItem();
    province->setType(TreeItem::PROJECT);
    root->addChild(province);

    TreeItem *itemSettings = new TreeItem();
    itemSettings->setType(TreeItem::CLASS_SETTINGS);
    province->addChild(itemSettings);

    TreeItem *itemImage = new TreeItem();
    itemImage->setType(TreeItem::CLASS_IMAGE);
    province->addChild(itemImage);

    TreeItem *itemComImage = new TreeItem();
    itemComImage->setType(TreeItem::CLASS_COMIMAGE);
    province->addChild(itemComImage);

    ui->treeViewProject->setModel(model);

//    foreach (auto pro, proList)
//    {
//        TreeItem* province = new TreeItem(root);
//        province->setPtr(pro); // 保存数据指针
//        province->setType(TreeItem::PROVINCE); // 设置节点类型为PROVINCE
//        root->addChild(province);

//        foreach (auto per, pro->people)
//        {
//            TreeItem* person = new TreeItem(province);
//            person->setPtr(per);    // 保存数据指针
//            person->setType(TreeItem::PERSON);  // 设置节点类型为PERSON
//            province->addChild(person);
//        }
//    }
}




void MainWindow::on_actOpenProject_triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, "数据库", "", tr("SQLite(*.db)"));
    if(!aFile.isEmpty())
    {
        pm.openProject(aFile);
        pm.initModel();
    }
}


void MainWindow::on_treeViewProject_clicked(const QModelIndex &index)
{
    qDebug() << "Clicked" << index.data(ProjectMng::RoleId);
    qDebug() << ui->treeViewProject->isExpanded(index);
}


void MainWindow::on_actNewFolder_triggered()
{
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    pm.createFolder(curIndex);
    ui->treeViewProject->expand(curIndex);
    pm.initModel();
}

#include <projectdata.h>

void MainWindow::on_actRename_triggered()
{
    RawData rd("C:\\Users\\Naive\\Desktop\\1\\a.db");
    QImage img(":/Image/TreeIco/Project.png");
    QPixmap pixmap(64, 64);

    //pixmap.transformed()
    rd.createBmp(5, "New File", 128, 64);
    qDebug() << img.depth();
    qDebug() << pixmap.depth();
}

