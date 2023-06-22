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


class ProjectMng : public QWidget
{
    Q_OBJECT
private:
    QList<RawData> projList;
    TreeModel *theModel;
    QTreeView *treeView;

    void addImgNode(RawData *rd, const quint16 pid, TreeItem *parent);
    void getExpandNode(QModelIndex root);      // 保存展开的节点
    void setExpandNode(QModelIndex root);   // 恢复展开的节点
    void saveExpand();
    void restoreExpand();

public:
    explicit ProjectMng(QWidget *parent = nullptr);
    void openProject(QString pro);
    void closeProjcet(QModelIndex &index);
    void initModel();   // 根据数据库重新初始化模型
    TreeModel* model() { return theModel; }
    void blindTreeView(QTreeView *treeView);
    QString getIndexProject(QModelIndex index);
    bool contain(QString project);
    void createFolder(QModelIndex index); // 新建文件夹
    void createImage(QModelIndex &index, QString name, quint16 width, quint16 height);
    void createImage(QModelIndex &index, QString name, QImage &img);
    void rename(QModelIndex &index, QString name);
    void remove(QModelIndex &index);

    enum NodeType
    {
        NodeProjRoot,   // 项目根目录
        NodeSettings,   // 设置
        NodeImage,      // 图片
        NodeComImage,   // 组合图
        NodeFont,       // 字体
        NodeFile,       // 文件
        NodeFolder,     // 文件夹
    };

signals:

};

#endif // PROJECTMNG_H
