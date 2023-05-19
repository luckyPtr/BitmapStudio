#ifndef PROJECTDATA_H
#define PROJECTDATA_H

#include <QObject>
#include <QtSql>
#include <QPixmap>
#include <vector>

class ProjectData : public QObject
{
    Q_OBJECT
public:
    enum Type
    {
        CLASS,
        FOLDER,
        FILE
    };

    struct Settings
    {
        qint16 version; // 数据库版本
        qint16 wide;    // 图片宽度
        qint16 height;  // 图片高度
        qint8 depth;    // 图片深度
    };
    struct ImageData
    {
        int id;
        int pid;
        Type type;
        QString name;
        qint16 wide;
        qint16 height;
        QPixmap imgFile;
        QString note;
        bool isExpand;
    };

private:
    QString path;
    QSqlDatabase db;
    Settings settings;
    QVector<ImageData> imgList;
    void initDatabase(QSqlDatabase &db);    // 初始化数据库
public:
    explicit ProjectData(QString path, QObject *parent = nullptr);

    void pull();
    void push();
signals:

};

#endif // PROJECTDATA_H
