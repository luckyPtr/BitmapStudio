#include "projectdata.h"

void ProjectData::initDatabase(QSqlDatabase &db)
{
    db.transaction();
    QSqlQuery query(db);
    // 创建settings表
    query.prepare("CREATE TABLE tbl_settings (\
                  version INTEGER,\
                  depth   INTEGER,\
                  wide    INTEGER,\
                  height  INTEGER,\
                  expand  BLOB\
                  );");
    query.exec();
    // 创建tbl_img
    query.prepare("CREATE TABLE tbl_img (\
                  id     INTEGER PRIMARY KEY,\
                  pid    INTEGER DEFAULT (0),\
                  type   TEXT,\
                  name   TEXT,\
                  note   TEXT,\
                  wide   INTEGER DEFAULT (0),\
                  height INTEGER DEFAULT (0),\
                  data   BLOB,\
                  expand INTEGER DEFAULT (0)\
                  );");
    query.exec();

    query.prepare("INSERT INTO tbl_settings (version) VALUES (1);");
    query.exec();
    db.commit();
}

ProjectData::ProjectData(QString path, QObject *parent)
    : QObject{parent}
{
    this->path = path;
    QFileInfo file(path);
    bool isFile = file.isFile();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", file.baseName());
    db.setDatabaseName(path);
    if(db.open())
    {
        qDebug() << "项目打开成功:" << path;
    }
    // 文件不存在则初始化文件
    if(!isFile)
    {
        initDatabase(db);
    }
}

void ProjectData::pull()
{

}
