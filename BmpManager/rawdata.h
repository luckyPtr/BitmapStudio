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


struct BmFile
{
    quint32 id;
    quint32 pid;
    quint8 isFolder;    // 0-文件 1-文件夹 2-图片组文件夹
    int type;
    QString name;
    QString brief;
    QImage image;
    ComImg comImg;

    bool isExpand;
    BmFile() {}
};



class RawData
{
public:
    enum
    {
        TypeUnknow = -1,
        TypeImgFile,    // 图片文件
        TypeComImgFile, // 组图文件
        TypeImgFolder,  // 图片文件夹
        TypeImgGrpFolder,   // 图片组文件夹
        TypeComImgFolder,   // 组图文件夹

        TypeProject = 0xFF,
        TypeClassSettings,
        TypeClassImg,
        TypeClassComImg,
    };
private:
    QString project;        // 项目文件
    QSqlDatabase db;        // 项目数据库
    quint8 depth;           // 项目图片深度
    QSize size;             // 项目屏幕大小(像素)
    QMap<quint16, BmFile> dataMap;

    void initDatabase();
    void convertComImgToImage(BmFile &file);

    int getTypeFromId(int id);
public:
    RawData(const QString path);
    ~RawData();

    QVector<quint16> expand;
    QString getProject() const {return project;}
    QMap<quint16, BmFile> getDataMap() const {return dataMap;}
    BmFile getBmFile(quint16 id) const { return dataMap[id]; }
    void load();    // 加载数据库数据
    void sortDataMap(); // 按名称字母顺序排列
    void createFolder(int id, QString name = "Untitled");
    void createBmp(int id, QString name, const QImage &img, const QString brief = "");
    void createBmp(int id, QString name, QSize size, const QString brief = "");
    void createComImg(int id, QString name, QSize size);
    void rename(int id, QString name);
    void remove(int id);
    void imgFolderConvert(int id);
    QImage getImage(int id);
    void setImage(int id, QImage image);

    ComImg getComImg(int id);
    void setComImg(int id, ComImg ci);

    void addExpandNode(quint16 id) { expand << id; };
    bool isExpandNode(quint16 id) { return expand.contains(id); }
    void clearExpandNode() { expand.clear(); }

    static bool isClassImgType(int type) { return type == TypeImgFolder || type == TypeImgGrpFolder || type == TypeImgFile; }
    static bool isClassComImgType(int type) { return type == TypeComImgFolder || type == TypeComImgFile; }

    QSize getSize();
};

#endif // RAWDATA_H
