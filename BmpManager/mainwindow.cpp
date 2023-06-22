#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <treemodel.h>
#include <treeitem.h>
#include <rawdata.h>
#include <QDialog>
#include <QInputDialog>
#include <QPainter>
#include <QToolButton>
#include <dialognewimgfile.h>
#include <dialogimportimg.h>

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
    pm.blindTreeView(ui->treeViewProject);

//    QToolButton *toolBtn = new QToolButton(this);
//    toolBtn->setDefaultAction(ui->actNewImg);
//    toolBtn->setPopupMode(QToolButton::MenuButtonPopup);
//    QMenu *menu = new QMenu(this);
//    menu->addAction(ui->actNewImg);
//    menu->addAction(ui->actImportImg);
//    toolBtn->setMenu(menu);
//    ui->toolBar->addWidget(toolBtn);
}




void MainWindow::on_actOpenProject_triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, tr("打开工程"), "", tr("BmpManager工程(*.db)"));
    if(!aFile.isEmpty())
    {
        pm.openProject(aFile);
        pm.initModel();
    }
}


void MainWindow::on_treeViewProject_clicked(const QModelIndex &index)
{
    TreeItem *item = pm.model()->itemFromIndex(index);
    BmImg bi = item->getRawData()->getImgInfo(item->getID());
    QImage img = bi.file;
    QImage resultImg = img.scaled(ui->labelPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelPreview->setPixmap(QPixmap::fromImage(resultImg));
    ui->labelPreview->setAlignment(Qt::AlignCenter);
    ui->labelPreview->show();
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
    QString name = QInputDialog::getText(this, tr("重命名"), tr("名称"));
    if(!name.isEmpty())
    {
        QModelIndex curIndex = ui->treeViewProject->currentIndex();
        pm.rename(curIndex, name);
        pm.initModel();
    }
}


void MainWindow::on_actNewProject_triggered()
{
    QString aFile = QFileDialog::getSaveFileName(this, tr("保存工程"), "Untiled", tr("BmpManager工程(*.db)"));
    if(!aFile.isEmpty())
    {
        pm.openProject(aFile);
        pm.initModel();
    }
}


void MainWindow::on_actNewImg_triggered()
{
    DialogNewImgFile *dlgNewImg = new DialogNewImgFile(this);
    Qt::WindowFlags flags = dlgNewImg->windowFlags();
    dlgNewImg->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);
    int ret = dlgNewImg->exec();
    if(ret == QDialog::Accepted)
    {
        QModelIndex curIndex = ui->treeViewProject->currentIndex();
        pm.createImage(curIndex, dlgNewImg->imgFileName(), dlgNewImg->width(), dlgNewImg->height());
        pm.initModel();
    }
    delete dlgNewImg;
}


void MainWindow::on_splitter_splitterMoved(int pos, int index)
{
    ui->labelPreview->clear();
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    if(curIndex.isValid())
    {
        TreeItem *item = pm.model()->itemFromIndex(curIndex);
        BmImg bi = item->getRawData()->getImgInfo(item->getID());
        QImage img = bi.file;
        QImage resultImg = img.scaled(ui->labelPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->labelPreview->setPixmap(QPixmap::fromImage(resultImg));
    }
}



void MainWindow::on_actImportImg_triggered()
{
    QString aFile = QFileDialog::getOpenFileName(this, tr("导入图片"), "", tr("图片(*.jpg *.png *.bmp);;JPEG(*.jpg *.jpeg);;PNG(*.png);;BMP(*.bmp)"));
    if(!aFile.isEmpty())
    {
        QImage img(aFile);
        DialogImportImg *dlgImportImg = new DialogImportImg(img, this);
        dlgImportImg->setImgName(QFileInfo(aFile).baseName());
        int ret = dlgImportImg->exec();
        if(ret == QDialog::Accepted)
        {
            QModelIndex curIndex = ui->treeViewProject->currentIndex();
            QImage img = dlgImportImg->getMonoImg();
            pm.createImage(curIndex, dlgImportImg->getImgName(), img);
            pm.initModel();
        }
        delete dlgImportImg;
    }
}


void MainWindow::on_actDelete_triggered()
{
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    if(curIndex.isValid())
    {
        TreeItem *item = pm.model()->itemFromIndex(curIndex);
        if(item->getType() == TreeItem::PROJECT)
        {
            pm.closeProjcet(curIndex);
            pm.initModel();
        }
        else
        {
            pm.remove(curIndex);
            pm.initModel();
        }
    }
}


void MainWindow::on_actTest_triggered()
{
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    pm.closeProjcet(curIndex);
}

