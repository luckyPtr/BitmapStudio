#ifndef PROJECTMNG_H
#define PROJECTMNG_H

#include <QWidget>
#include <QtSql>
#include <QStandardItem>
#include <QTreeView>
#include <QVector>
#include <QSet>


class ProjectMng : public QWidget
{
    Q_OBJECT
private:
    QVector<QSqlDatabase> proList;  // 项目列表
    QStandardItemModel *proModel;
    QTreeView *treeView;

    void addSonToModel(QSqlDatabase db, QString table, int pid, QStandardItem *parent);
    void getExpandNode(QModelIndex root, QVector<int> &expandNode);      // 保存展开的节点
    void setExpandNode(QModelIndex root, QVector<int> &expandNode);   // 恢复展开的节点
    void saveExpand();
    void restoreExpand();
    QString getTblName(int id);
public:
    explicit ProjectMng(QWidget *parent = nullptr);
    void openProject(QString pro);
    void initModel();   // 根据数据库重新初始化模型
    QStandardItemModel* model() { return proModel; }
    void blindTreeView(QTreeView *treeView);
    QSqlDatabase getIndexDatabase(QModelIndex index);
    QString getIndexProject(QModelIndex index);
    void createFolder(QModelIndex index); // 新建文件夹
    void createBMP(QModelIndex &index);

    enum
    {
        RoleId = Qt::UserRole + 1,    // treeView节点名称
        RoleProject,
        RoleNodeType,                   // 节点类型 DIR FILE
        RoleStatus,                     // 节点状态 正常 隐藏 错误 未保存 名称重复
    };

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
