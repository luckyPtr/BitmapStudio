#include "rawdata.h"
#include <QByteArray>
#include <QImage>

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
                  type    INTEGER DEFAULT (0),\
                  name    TEXT,\
                  brief   TEXT,\
                  data    BLOB\
                  );");
    query.exec();

    query.prepare("INSERT INTO tbl_settings (version) VALUES (1);");
    query.exec();
    db.commit();
}

void RawData::convertComImgToImage(BmFile &file)
{
    QImage img(file.comImg.width, file.comImg.height, QImage::Format_RGB888);
}

int RawData::getTypeFromId(int id)
{
    switch(id)
    {
    case -1:
        return RawData::TypeProject;
    case -2:
        return RawData::TypeClassSettings;
    case -3:
        return RawData::TypeClassImg;
    case -4:
        return RawData::TypeClassComImg;
    default:
        return getDataMap()[id].type;
        break;
    }
}

void RawData::load()
{
    // QString转QJsonObject
    auto stringToJson = [](QString jsonString){
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data());
        QJsonObject jsonObject = jsonDocument.object();
        return jsonObject;
    };


    dataMap.clear();

    QSqlQuery query(db);

    query.prepare("SELECT * FROM tbl_settings");
    query.exec();
    if(query.first())
    {
        depth = query.value("depth").toInt();
        int width = query.value("width").toInt();
        int height = query.value("height").toInt();
        size.setWidth(width);
        size.setHeight(height);
    }

    query.prepare("SELECT * FROM tbl_img");
    query.exec();
    while (query.next())
    {
        BmFile bi;
        bi.id = query.value("id").toInt();
        bi.pid = query.value("pid").toInt();
        bi.type = query.value("type").toInt();
        bi.name = query.value("name").toString();
        bi.brief = query.value("brief").toString();
        if(bi.type == RawData::TypeImgFile)
        {
            QByteArray ba = query.value("data").toByteArray();
            bi.image.loadFromData(ba);
        }
        else if(bi.type == RawData::TypeComImgFile)
        {
            QString s = query.value("data").toString();
            QJsonObject jsonComImg = stringToJson(s);
            bi.comImg.width = jsonComImg.value("width").toInt();
            bi.comImg.height = jsonComImg.value("height").toInt();
            QJsonArray array = jsonComImg.value("images").toArray();

            for(auto obj : array)
            {
                ComImgItem item;
                item.x = obj.toObject().value("x").toInt();
                item.y = obj.toObject().value("y").toInt();
                item.id = obj.toObject().value("id").toInt();

                bi.comImg.items.append(item);
            }
        }
        dataMap.insert(bi.id, bi);
    }
}

void RawData::sortDataMap()
{
    QMap<quint16, BmFile> newMap;
    QVector<BmFile> temp = dataMap.values().toVector();

}

RawData::RawData(const QString path)
{
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
    if(!expand.isEmpty())
    expand.clear();
    qDebug() << "~RawData:" << project;
}

void RawData::createFolder(int id, QString name)
{
    int type = TypeUnknow;
    int pid = 0;
    QSqlQuery query(db);

    if(id == -3)
    {
        type = RawData::TypeImgFolder;
    }
    else if(id == -4)
    {
        type = RawData::TypeComImgFolder;
    }
    else
    {
        query.prepare("SELECT pid, type FROM tbl_img WHERE id=?");
        query.bindValue(0, id);
        query.exec();

        if(query.first())
        {
            pid = query.value("pid").toUInt();
            int curType = query.value("type").toInt();

            // 如果选择的是文件夹，新建的文件夹在此文件夹下；如果选择的是文件，新建的文件夹在该文件所在的文件夹下
            if(curType == RawData::TypeImgFolder || curType == RawData::TypeComImgFolder)
            {
                pid = id;
            }

            if(curType == RawData::TypeImgFile || curType == RawData::TypeImgFolder)
            {
                type = RawData::TypeImgFolder;
            }
            else if(curType == RawData::TypeComImgFile || curType == RawData::TypeComImgFolder)
            {
                type = RawData::TypeComImgFolder;
            }
        }
    }

    if(type != RawData::TypeUnknow)
    {
        query.prepare("INSERT INTO tbl_img (name,type,pid) VALUES(:name,:type,:pid)");
        query.bindValue(":name", name);
        query.bindValue(":type", type);
        query.bindValue(":pid", pid);
        query.exec();

        BmFile bi;
        bi.id = query.lastInsertId().toUInt();
        bi.pid = pid;
        bi.type = type;
        bi.name = name;
        dataMap.insert(bi.id, bi);
    }
}

void RawData::createBmp(int id, QString name, const QImage &img, const QString brief)
{
    bool isVaild = false;
    int pid = 0;

    QByteArray byteArray = QByteArray();
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "png");

    QSqlQuery query(db);

    if(id == -3)    // 如果是图片类
    {
        isVaild = true;
    }
    else
    {
        query.prepare("SELECT pid, type FROM tbl_img WHERE id=?");
        query.bindValue(0, id);
        query.exec();

        if(query.first())
        {
            pid = query.value("pid").toUInt();
            int type = query.value("type").toInt();

            if(type == RawData::TypeImgFolder ||\
               type == RawData::TypeImgGrpFolder)
            {
                pid = id;
            }
            isVaild = true;
        }
    }

    if(isVaild)
    {
        query.prepare("INSERT INTO tbl_img (name,type,pid,data,brief) VALUES(:name,:type,:pid,:data,:brief)");
        query.bindValue(":name", name);
        query.bindValue(":type", RawData::TypeImgFile);
        query.bindValue(":pid", pid);
        query.bindValue(":data", byteArray);
        query.bindValue(":brief", brief);
        query.exec();

        BmFile bi;
        bi.id = query.lastInsertId().toUInt();
        bi.pid = pid;
        bi.type = RawData::TypeImgFile;
        bi.name = name;
        bi.image = img;
        bi.brief = brief;
        dataMap.insert(bi.id, bi);
    }
}

void RawData::createBmp(int id, QString name, QSize size, const QString brief)
{
    QImage image(size, QImage::Format_RGBA8888);
    image.fill(Qt::white);
    createBmp(id, name, image, brief);
}

void RawData::createComImg(int id, QString name, QSize size)
{
    bool isVaild = false;
    int pid = 0;
    QSqlQuery query(db);

    if(id == -4)
    {
        isVaild = true;
    }
    else
    {
        query.prepare("SELECT pid, type FROM tbl_img WHERE id=?");
        query.bindValue(0, id);
        query.exec();

        if(query.first())
        {
            pid = query.value("pid").toInt();
            int type = query.value("type").toInt();

            if(type == RawData::TypeComImgFolder)
            {
                pid = id;
            }
            isVaild = true;
        }
    }

    if(isVaild)
    {
        QJsonObject ciObj;
        ciObj.insert("width", 128);
        ciObj.insert("height", 64);

        query.prepare("INSERT INTO tbl_img (name,type,pid,data) VALUES(:name,:type,:pid,:data)");
        query.bindValue(":name", name);
        query.bindValue(":type", RawData::TypeComImgFile);
        query.bindValue(":pid", pid);
        query.bindValue(":data", QString(QJsonDocument(ciObj).toJson()).toUtf8());
        query.exec();

        BmFile bi;
        bi.id = query.lastInsertId().toUInt();
        bi.pid = pid;
        bi.type = RawData::TypeComImgFile;
        bi.name = name;

        bi.image = QImage();
        bi.comImg = ComImg(128, 64);
        dataMap.insert(bi.id, bi);
    }
}

void RawData::rename(int id, QString name)
{
    QSqlQuery query(db);

    query.prepare("UPDATE tbl_img SET name=:name WHERE id=:id");
    query.bindValue(":name", name);
    query.bindValue(":id", id);
    query.exec();

    if(dataMap.contains(id))
    {
        dataMap[id].name = name;
    }
}

void RawData::remove(int id)
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
        int childId = query.value("id").toInt();
        remove(childId);
    }
}

void RawData::imgFolderConvert(int id)
{
    if(dataMap.contains(id))
    {
        int type = dataMap[id].type;

        if(type == RawData::TypeImgFolder || type == RawData::TypeImgGrpFolder)
        {
            dataMap[id].type = type == RawData::TypeImgFolder ? RawData::TypeImgGrpFolder : RawData::TypeImgFolder;
            QSqlQuery query(db);
            query.prepare("UPDATE tbl_img SET type=:type WHERE id=:id");
            query.bindValue(":type", dataMap[id].type);
            query.bindValue(":id", id);
            query.exec();
        }
    }
}




QImage RawData::getImage(int id)
{
    // 将img2合并到img1，img2的位置在x,y
    auto merge = [](QImage &img1, QImage &img2, int x, int y) ->void {
        for(int i = x; i < img1.width() && i < x + img2.width(); i++)
        {
            for(int j = y; j < img1.height() && j < y + img2.height(); j++)
            {
                img1.setPixelColor(i, j, img2.pixelColor(i-x, j-y));
            }
        }
    };

    auto comImgToImage = [=](){
        QImage image(dataMap[id].comImg.width, dataMap[id].comImg.height, QImage::Format_RGB888);
        image.fill(Qt::white);
        foreach(auto item, dataMap[id].comImg.items)
        {
            if(dataMap.contains(item.id))
            {
                merge(image, dataMap[item.id].image, item.x, item.y);
            }
        }
        return image;
    };

    if(dataMap.contains(id))
    {
        if(dataMap[id].type == RawData::TypeImgFile)
        {
            return dataMap[id].image;
        }
        else if(dataMap[id].type == RawData::TypeComImgFile)
        {
            return comImgToImage();
        }
    }
    return QImage();
}

void RawData::setImage(int id, QImage image)
{
    if(dataMap.contains(id))
    {
        dataMap[id].image = image;

        QByteArray byteArray = QByteArray();
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "png");

        QSqlQuery query(db);
        query.prepare("UPDATE tbl_img SET data=:data WHERE id=:id AND type=:type");
        query.bindValue(":data", byteArray);
        query.bindValue(":id", id);
        query.bindValue(":type", RawData::TypeImgFile);
        query.exec();
        buffer.close();
    }
}

ComImg RawData::getComImg(int id)
{
    return dataMap[id].comImg;
}

void RawData::setComImg(int id, ComImg ci)
{
    // JSON Object -> QString
    auto jsonToString = [](QJsonObject jsonObj){
        return QString(QJsonDocument(jsonObj).toJson());
    };

    if(dataMap.contains(id))
    {
        dataMap[id].comImg = ci;

        QJsonArray imgList;
        foreach(auto item, ci.items)
        {
            QJsonObject obj;
            obj.insert("id", item.id);
            obj.insert("x", item.x);
            obj.insert("y", item.y);
            imgList.append(obj);
        }

        QJsonObject ciObj;
        ciObj.insert("width", ci.width);
        ciObj.insert("height", ci.height);
        ciObj.insert("images", QJsonValue(imgList));

        QSqlQuery query(db);
        query.prepare("UPDATE tbl_img SET data=:data WHERE id=:id");
        query.bindValue(":data", jsonToString(ciObj).toUtf8());
        query.bindValue(":id", id);
        query.exec();
    }
}

QSize RawData::getSize()
{
    return size;
}




