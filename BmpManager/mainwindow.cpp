#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

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

    pm.blindTreeView(ui->treeViewProject);
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
    pm.createFolder(ui->treeViewProject->currentIndex());
    pm.initModel();
//    getExpandNode(pm.model()->invisibleRootItem()->index());

//    ui->treeViewProject->setUpdatesEnabled(false);
//    //pm.createFolder(ui->treeViewProject->currentIndex());
//    pm.initModel();
//    restoreExpand();
//    qDebug() << "expand-----" << expandNode.size();
//    ui->treeViewProject->setUpdatesEnabled(true);

}

