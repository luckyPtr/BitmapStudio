#ifndef PROJECTMNG_H
#define PROJECTMNG_H

#include <QWidget>
#include <QtSql>
#include <QStandardItem>
#include <QVector>
#include <QMap>


class ProjectMng : public QWidget
{
    Q_OBJECT
private:
    typedef struct
    {
        QSqlDatabase db;    // 项目数据库
        QString path;       // 项目数据库路径
        QString name;       // 项目名称(不含后缀)
    }Project_t;


    QVector<QSqlDatabase> proList;
//    QMap<QString, QSqlDatabase> proMap;
//    QVector<Project_t> proList; // 工程数据库
    void addSonToModel(QSqlDatabase db, int pid, QStandardItem *parent);
    QStandardItemModel *proModel;
public:
    explicit ProjectMng(QWidget *parent = nullptr);
    void openProject(QString pro);
    void initModel();   // 根据数据库重新初始化模型
    QStandardItemModel* model() { return proModel; }
    QSqlDatabase getIndexDatabase(QModelIndex index);
    QString getIndexProject(QModelIndex index);
    void createFolder(QModelIndex &index); // 新建文件夹
    void createBMP(QModelIndex &index);

    enum
    {
        RoleId = Qt::UserRole + 1,    // treeView节点名称
        RoleProjPath,                    // 节点所在项目
        RoleNodeType,                   // 节点类型 DIR FILE
        RoleStatus,                     // 节点状态 正常 隐藏 错误 未保存 名称重复
    };
    enum ItemStatus
    {
        Normal, // 正常状态
        Ignore, // 忽略
        Error,
        Unsaved,
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


    typedef struct
    {
        QString name;
        QString project;
        QString Type;
        enum TblType Table;
        enum ItemStatus Status;
    }ItemInfo_t;
signals:

};

#endif // PROJECTMNG_H
