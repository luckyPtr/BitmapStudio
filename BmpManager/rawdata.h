#ifndef RAWDATA_H
#define RAWDATA_H
#include <QString>
#include <QList>
#include <QtSql>
#include <QPixmap>
#include <QMap>
#include <QVector>
#include <QJsonObject>

// 组合图单个元素
struct ComImgItem
{
    qint16 x;
    qint16 y;
    quint16 z;
    quint16 id;

    ComImgItem(){}
    ComImgItem(quint16 x, quint16 y, quint16 id) {
        this->x = x;
        this->y = y;
        this->id = id;
    }


};

struct ComImg
{
    quint16 width;
    quint16 height;
    QVector<ComImgItem> items;
    ComImg() {}
    ComImg(quint16 width, quint16 height)
    {
        this->width = width;
        this->height = height;
        items.clear();
    }

    ComImg & operator = (ComImg comimg)
    {
        this->width = comimg.width;
        this->height = comimg.height;
        this->items.clear();
        this->items.swap(comimg.items);

        return *this;
    }
};


// TODO 二合一 FileMap
struct BmFile
{
    quint32 id;
    quint32 pid;
    bool isFolder;
    QString name;
    QString details;
    QImage image;
    ComImg comImg;

    bool isExpand;
    BmFile() {}
};



class RawData
{
private:
    QString project;        // 项目文件
    QSqlDatabase db;        // 项目数据库
    quint16 screenWith;     // 项目屏幕宽度(像素)
    quint16 screenHeight;   // 项目屏幕高度(像素)
    quint8 depth;           // 项目图片深度
    QMap<quint16, BmFile> imgMap;

    void initDatabase();
    void convertComImgToImage(BmFile &file);
public:


    RawData(const QString path);
    ~RawData();

    QVector<quint16> expand;

    QString getProject() const {return project;}
    QMap<quint16, BmFile> getImgMap() const {return imgMap;}
//    QMap<quint16, BmComImg> getComImgMap() const {return comImgMap;}
    BmFile getBmFile(quint16 id) const { return imgMap[id]; }
    void load();    // 加载数据库数据
    void createFolder(quint16 id, QString name = "Untitled");
    void createBmp(quint16 id, QString name, const QImage &img);
    void createBmp(quint16 id, QString name, quint16 width, quint16 height);
    void createComImg(quint16 id, QString name, QSize size);
    void rename(quint16 id, QString name);
    void remove(quint16 id);
    QImage getImage(quint16 id);
    void setImage(quint16 id, QImage image);

    ComImg getComImg(quint16 id);
    void setComImg(quint16 id, ComImg ci);

    void addExpandNode(quint16 id) { expand << id; };
    bool isExpandNode(quint16 id) { return expand.contains(id); }
    void clearExpandNode() { expand.clear(); }
};

#endif // RAWDATA_H
