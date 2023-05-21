#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <treemodel.h>
#include <treeitem.h>
#include <rawdata.h>
#include <QDialog>
#include <QInputDialog>

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
    qDebug() << ui->treeViewProject->isExpanded(index);

    TreeItem *item = pm.model()->itemFromIndex(index);
    qDebug() <<item->getID();
}


void MainWindow::on_actNewFolder_triggered()
{
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    pm.createFolder(curIndex);
    ui->treeViewProject->expand(curIndex);
    pm.initModel();
}

void MainWindow::on_actRename_triggered()
{
    pm.initModel();
}

