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

    QAction *actOpen;
    QAction *actCloseProject;
    QAction *actDelete;
    QAction *actRename;
    QAction *actProperties;

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
    void createFolder(QModelIndex index); // 新建文件夹
    void createImage(QModelIndex &index, QString name, quint16 width, quint16 height);
    void createImage(QModelIndex &index, QString name, QImage &img);
    void rename(QModelIndex &index, QString name);
    void remove(QModelIndex &index);
    void imgFolderConvert(QModelIndex &index);
    QImage getImage(QModelIndex index);
    void setImage(QModelIndex index, QImage &image);
    ComImg getComImg(QModelIndex index);
    void setComImg(QModelIndex index, ComImg &comImg);
    QModelIndex getModelIndex(QString project, int id);   // 从项目名称和id获取ModelIndex

public slots:
    void on_CustomContextMenu(QPoint point);
    void on_ActOpen_Triggered();
    void on_ActCloseProject_Triggered();
    void on_ActDelete_Triggered();
    void on_ActRename_Triggered();
    void on_ActProperties_Triggered();
signals:

};

#endif // PROJECTMNG_H
