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
#include <formpixeleditor.h>
#include <formcomimgeditor.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QClipboard>


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
    initStatusBar();
    treeItemDelegate = new TreeItemDelegate();
    ui->treeViewProject->setItemDelegate(treeItemDelegate);
    pm.blindTreeView(ui->treeViewProject);
}

void MainWindow::initStatusBar()
{
    labelPosition = new QLabel();
    labelSize = new QLabel();
    labelPositionIco = new QLabel();
    labelSizeIco_new = new QLabel();

    labelPositionIco->setPixmap(QPixmap(":/Image/StatusBar/Position.png"));
    labelSizeIco_new->setPixmap(QPixmap(":/Image/StatusBar/Size.png"));

    ui->statusbar->addWidget(labelPositionIco);
    ui->statusbar->addWidget(labelPosition);
    ui->statusbar->addWidget(labelSizeIco_new);
    ui->statusbar->addWidget(labelSize);

    labelPosition->setMinimumWidth(120);
    labelSize->setMinimumWidth(120);
}
void MainWindow::setStackedWidget(int index)
{
    //ui->stackedWidget->setCurrentIndex(index);
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
    if(item->getType() == RawData::TypeImgFile)
    {
        ui->tabWidget->addImgTab(item);
    }
    else if(item->getType() == RawData::TypeComImgFile)
    {
        ui->tabWidget->addComImgTab(item);
    }

    return;
//    BmFile bi = item->getRawData()->getBmFile(item->getID());
    qDebug() << "id" << item->getID();
    qDebug() << "type" << item->getType();
    QImage img = item->getRawData()->getImage(item->getID());
    QImage resultImg = img.scaled(ui->labelPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->labelPreview->setPixmap(QPixmap::fromImage(resultImg));
    ui->labelPreview->setAlignment(Qt::AlignCenter);
    ui->labelPreview->show();

    editedIndex = index;

    if(item->getType() == RawData::TypeImgFile)
    {
        emit selectItem(img);
        setStackedWidget(STACKED_WIDGET_IMG);
        on_UpdateStatusBarSize(img.size());
    }
    else if(item->getType() == RawData::TypeComImgFile)
    {
        ComImg ci = item->getRawData()->getComImg(item->getID());
        emit selectItem(ci, item->getRawData());
        setStackedWidget(STACKED_WIDGET_COMIMG);
        on_UpdateStatusBarSize(QSize(ci.width, ci.height));
    }
    else
    {
        setStackedWidget(STACKED_WIDGET_DEFAULT);
        on_UpdateStatusBarSize(QSize(-1, -1));
    }
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
    Q_UNUSED(pos)
    Q_UNUSED(index)
    ui->labelPreview->clear();
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    if(curIndex.isValid())
    {
        TreeItem *item = pm.model()->itemFromIndex(curIndex);
 //       BmFile bi = item->getRawData()->getBmFile(item->getID());
        QImage img = item->getRawData()->getImage(item->getID());
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
        if(item->getType() == RawData::TypeProject)
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

#include "imgconvertor.h"
void MainWindow::on_actTest_triggered()
{
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    //TreeItem *item = pm.model()->itemFromIndex(curIndex);
    //QModelIndex i = pm.getModelIndex(item->getRawData()->getProject(), item->getID());
    qDebug() << curIndex.column();
}

void MainWindow::on_SaveImage(QImage image)
{
    pm.setImage(editedIndex, image);
}

void MainWindow::on_SaveComImg(ComImg comImg)
{
    pm.setComImg(editedIndex, comImg);
}

void MainWindow::on_SaveImage(QString project, int id, QImage image)
{
    QModelIndex index = pm.getModelIndex(project, id);
    if(index.isValid())
    {
        pm.setImage(index, image);
    }
}

void MainWindow::on_SaveComImg(QString project, int id, ComImg comImg)
{
    QModelIndex index = pm.getModelIndex(project, id);
    if(index.isValid())
    {
        pm.setComImg(index, comImg);
    }
}


void MainWindow::on_actGrpImgTransform_triggered()
{
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    pm.imgFolderConvert(curIndex);
    pm.initModel();
}


void MainWindow::on_actRun_triggered()
{
    QString file_img_c;

    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    TreeItem *item = pm.model()->itemFromIndex(curIndex);
    ImgConvertor ic(item->getRawData()->getDataMap().values().toVector());

    QString path = item->getRawData()->getProject();
    QFileInfo fileInfo(path);
    path = fileInfo.path();

    if(!QDir(fileInfo.path() + "/Assets").exists())
    {
        QDir().mkdir(fileInfo.path() + "/Assets");
        qDebug() << fileInfo.path() + "/Assets";
    }


    QFile file(path + "/bm_img.c");
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

void MainWindow::on_UpdateStatusBarPos(QPoint point)
{
    if(point.x() >= 0 && point.y() >= 0)
    {
        labelPosition->setText(QString(tr("%1, %2像素")).arg(point.x()).arg(point.y()));
    }
    else
    {
        labelPosition->clear();
    }
}

void MainWindow::on_UpdateStatusBarSize(QSize size)
{
    if(size.width() >= 0 && size.height() >= 0)
    {
        labelSize->setText(QString(tr("%1 x %2像素")).arg(size.width()).arg(size.height()));
    }
    else
    {
        labelSize->clear();
    }
}


void MainWindow::on_actCopyName_triggered()
{
    QModelIndex curIndex = ui->treeViewProject->currentIndex();
    TreeItem *item = pm.model()->itemFromIndex(curIndex);
    ImgConvertor ic(item->getRawData()->getDataMap().values().toVector());

    QClipboard *clip = QApplication::clipboard();
    clip->setText(ic.getFullName(item->getRawData()->getDataMap()[item->getID()]));
}


void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->removeTab(index);
}

