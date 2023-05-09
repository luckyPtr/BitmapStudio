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


void MainWindow::init()
{
   //https://blog.csdn.net/wzz953200463/article/details/123643823

    ui->treeViewProject->setModel(pm.model());
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
    qDebug() << "Clicked" << index.data(ProjectMng::RoleId) << " Project:" << pm.getIndexProject(index);
    qDebug() << pm.getIndexDatabase(index).databaseName();
}

