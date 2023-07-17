#include "rawdata.h"
#include <QByteArray>

void RawData::initDatabase()
{
    db.transaction();
    QSqlQuery query(db);
    // 创建settings表
    query.prepare("CREATE TABLE tbl_settings (\
                  version INTEGER,\
                  depth   INTEGER,\
                  wide    INTEGER,\
                  height  INTEGER\
                  );");
    query.exec();
    // 创建tbl_img
    query.prepare("CREATE TABLE tbl_img (\
                  id      INTEGER PRIMARY KEY,\
                  pid     INTEGER DEFAULT (0),\
                  folder  INTEGER DEFAULT (0),\
                  name    TEXT,\
                  details TEXT,\
                  wide    INTEGER DEFAULT (0),\
                  height  INTEGER DEFAULT (0),\
                  data    BLOB\
                  );");
    query.exec();

    query.prepare("INSERT INTO tbl_settings (version) VALUES (1);");
    query.exec();
    db.commit();
}

void RawData::load()
{
    imgMap.clear();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM tbl_img");
    query.exec();
    while (query.next())
    {
        BmImg bi;
        bi.id = query.value("id").toUInt();
        bi.pid = query.value("pid").toUInt();
        bi.isFolder = query.value("folder").toBool();
        bi.name = query.value("name").toString();
        bi.details = query.value("details").toString();
        bi.wide = query.value("wide").toUInt();
        bi.height = query.value("height").toUInt();
        QByteArray ba = query.value("data").toByteArray();
        bi.file.loadFromData(ba);
        imgMap.insert(bi.id, bi);
    }
}

RawData::RawData(const QString path)
{
    qDebug() << "RawData:" << project;
    project = path;
    QFileInfo file(path);
    bool isFile = file.isFile();

    db = QSqlDatabase::addDatabase("QSQLITE", file.baseName());
    db.setDatabaseName(path);
    if(db.open())
    {
        qDebug() << "项目打开成功:" << path;
    }
    // 文件不存在则初始化文件
    if(!isFile)
    {
        initDatabase();
    }
    load();
}

RawData::~RawData()
{
//    db.close();
    expand.clear();
    qDebug() << "~RawData:" << project;
}

void RawData::createFolder(quint16 id, QString name)
{
    QSqlQuery query(db);
    query.prepare("SELECT pid, folder FROM tbl_img WHERE id=?");
    query.bindValue(0, id);
    query.exec();

    if(query.first() || id == 0)
    {
        if(id)
        {
            quint16 pid = query.value("pid").toUInt();
            bool isFolder = query.value("folder").toBool();

            if(!isFolder)
            {
                id = pid;
            }
        }

        query.prepare("INSERT INTO tbl_img (name,folder,pid) VALUES(:name,:folder,:pid)");
        query.bindValue(":name", name);
        query.bindValue(":folder", true);
        query.bindValue(":pid", id);
        query.exec();
        BmImg bi;
        bi.id = query.lastInsertId().toUInt();
        bi.pid = id;
        bi.isFolder = true;
        bi.name = name;
        imgMap.insert(bi.id, bi);
    }
}

void RawData::createBmp(quint16 id, QString name, const QImage &img)
{
    QByteArray byteArray = QByteArray();
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "png");

    QSqlQuery query(db);
    query.prepare("SELECT pid, folder FROM tbl_img WHERE id=?");
    query.bindValue(0, id);
    query.exec();

    if(query.first())
    {
        quint16 pid = query.value("pid").toUInt();
        bool isFolder = query.value("folder").toBool();

        if(!isFolder)
        {
            id = pid;
        }

        query.prepare("INSERT INTO tbl_img (name,folder,pid,data) VALUES(:name,:folder,:pid,:data)");
        query.bindValue(":name", name);
        query.bindValue(":folder", false);
        query.bindValue(":pid", id);
        query.bindValue(":data", byteArray);
        query.exec();


        BmImg bi;
        bi.id = query.lastInsertId().toUInt();
        bi.pid = id;
        bi.isFolder = false;
        bi.name = name;
        bi.wide = img.width();
        bi.height = img.height();
        bi.file = img;
        imgMap.insert(bi.id, bi);
    }
}

void RawData::createBmp(quint16 id, QString name, quint16 wide, quint16 height)
{
    QImage image(wide, height, QImage::Format_RGBA8888);
    image.fill(Qt::white);
    createBmp(id, name, image);
}

void RawData::rename(quint16 id, QString name)
{
    QSqlQuery query(db);
    query.prepare("UPDATE tbl_img SET name=:name WHERE id=:id");
    query.bindValue(":name", name);
    query.bindValue(":id", id);
    query.exec();

    if(imgMap.contains(id))
    {
        imgMap[id].name = name;
    }
}

void RawData::remove(quint16 id)
{
    // 删除对应id
    QSqlQuery query(db);
    query.prepare("DELETE FROM tbl_img WHERE id=:id");
    query.bindValue(":id", id);
    query.exec();

    // 递归删除该id下面的子项
    query.prepare("SELECT id FROM tbl_img WHERE pid=:pid");
    query.bindValue(":pid", id);
    query.exec();
    while (query.next())
    {
        quint16 childId = query.value("id").toInt();
        remove(childId);
    }
}

QImage RawData::getImage(quint16 id)
{
    if(imgMap.contains(id))
    {
        return imgMap[id].file;
    }
    return QImage();
}

void RawData::setImage(quint16 id, QImage image)
{
    QByteArray byteArray = QByteArray();
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "png");

    QSqlQuery query(db);
    query.prepare("UPDATE tbl_img SET data=:data WHERE id=:id");
    query.bindValue(":data", byteArray);
    query.bindValue(":id", id);
    query.exec();
}


