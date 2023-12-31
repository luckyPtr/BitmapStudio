#ifndef PROJECTMNG_H
#define PROJECTMNG_H

#include <QWidget>
#include <QtSql>
#include <QStandardItem>
#include <QTreeView>
#include <QVector>
#include <QSet>
#include <rawdata.h>
#include <treemodel.h>
#include <treeitem.h>
#include <customtabwidget.h>


class ProjectMng : public QWidget
{
    Q_OBJECT
private:
    QList<RawData> projList;
    TreeModel *theModel;
    QTreeView *treeView;
    QModelIndex currentIndex;
    CustomTabWidget *tabWidget;

    QAction *actNewProject;
    QAction *actOpenProject;
    QAction *actNewImgFile;
    QAction *actNewComImgFile;
    QAction *actNewFolder;
    QAction *actImportFromImg;
    QAction *actImportFromHex;
    QAction *actReplaceFromImg;
    QAction *actReplaceFromHex;
    QAction *actExportImg;
    QAction *actCopyImg;
    QAction *actOpen;
    QAction *actCloseProject;
    QAction *actDelete;
    QAction *actImgGrpFolder; // 普通文件夹和图片组文件夹互相转换
    QAction *actRename;
    QAction *actProperties;
    QAction *actSettings;
    QAction *actRun;

    void addDataNodes(RawData *rd, const quint16 pid, TreeItem *parent, bool(*filter)(int));
    void getExpandNode(QModelIndex root);      // 保存展开的节点
    void setExpandNode(QModelIndex root);   // 恢复展开的节点
    void saveExpand();
    void restoreExpand();
    void initActions();

public:
    explicit ProjectMng(QWidget *parent = nullptr);
    void openProject(QString pro);
    void closeProjcet(QModelIndex &index);
    void initModel();   // 根据数据库重新初始化模型
    TreeModel* model() { return theModel; }
    void blindTreeView(QTreeView *treeView);
    void setTabWidget(CustomTabWidget *tabWidget);
    void createFolder(QModelIndex index, QString name, QString brief = ""); // 新建文件夹
    void createImage(QModelIndex &index, QString name, QSize size, QString brief = "");
    void createImage(QModelIndex &index, QString name, QImage &img, QString brief = "");
    void createComImg(QModelIndex &index, QString name, QSize size, QString brief = "");
    void rename(QModelIndex &index, QString name);
    void remove(QModelIndex &index);
    void imgFolderConvert(QModelIndex &index);
    QImage getImage(QModelIndex index);
    void setImage(QModelIndex index, QImage &image);
    ComImg getComImg(QModelIndex index);
    void setComImg(QModelIndex index, ComImg &comImg);
    QString getBrief(QModelIndex index);
    void setBrief(QModelIndex index, QString brief);
    QModelIndex getModelIndex(QString project, int id);   // 从项目名称和id获取ModelIndex

public slots:
    void on_CustomContextMenu(QPoint point);
    void on_ActNewProject_Triggered();
    void on_ActOpenProject_Triggered();
    void on_ActOpen_Triggered();
    void on_ActCloseProject_Triggered();
    void on_ActDelete_Triggered();
    void on_ActRename_Triggered();
    void on_ActProperties_Triggered();
    void on_ActNewImg_Triggered();
    void on_ActNewComImg_Triggered();
    void on_ActNewFolder_Triggered();
    void on_ActImgGrpFolder_Triggered();
    void on_ActSettings_Triggered();
    void on_ActRun_Triggered();
    void on_ActImportFromImg_Triggered();
    void on_ActImportFromHex_Triggered();
    void on_ActReplaceFromImg_Triggered();
    void on_ActReplaceFromHex_Triggered();
    void on_ActExportImg_Triggered();
    void on_ActCopyImg_Triggered();
signals:

};

#endif // PROJECTMNG_H
