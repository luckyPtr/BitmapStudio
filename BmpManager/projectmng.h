#ifndef PROJECTMNG_H
#define PROJECTMNG_H

#include <QWidget>
#include <QtSql>
#include <QStandardItem>
#include <QVector>

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
    typedef struct
    {
        QString name;
        QString project;
        QString Type;
        QString Table;
        QString Status;
    }ItemInfo_t;


    QVector<Project_t> proList; // 工程数据库
    void addSonToModel(QSqlDatabase db, int pid, QStandardItem *parent);
    QStandardItemModel *proModel;
public:
    explicit ProjectMng(QWidget *parent = nullptr);
    void openProject(QString pro);
    void initModel();   // 根据数据库重新初始化模型
    QStandardItemModel* model() { return proModel; }
    enum
    {
        RoleName = Qt::UserRole + 1,    // treeView节点名称
        RoleProject,                    // 节点所在项目
        RoleType,                       // 节点类型 DIR FILE
        RoleTable,                      // 节点表 Setting Image ComImage Font...
        RoleStatus,                     // 节点状态 正常 隐藏 错误 未保存 名称重复
    };
    enum TblType
    {
        TblSetting,
        TblImage,
        TblComImage,
        TblFont,
    };
signals:

};

#endif // PROJECTMNG_H
