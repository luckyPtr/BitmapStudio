#ifndef RAWDATA_H
#define RAWDATA_H
#include <QString>
#include <QList>
#include <QtSql>
#include <QPixmap>
#include <QMap>

struct BmImg
{
    quint32 id;
    quint32 pid;
    bool isFolder;
    QString name;
    QString details;
    quint16 wide;
    quint16 height;
    QImage file;
    bool isExpand;
    BmImg() {}
};

struct BmComImg
{
    quint32 id;
    quint32 pid;
    bool isFolder;
    QString name;
    QString details;
    quint16 wide;
    quint16 height;
    QString data;
    bool isExpand;
    BmComImg() {}
};

class RawData
{
private:
    QString project;        // 项目文件
    QSqlDatabase db;        // 项目数据库
    quint16 screenWith;     // 项目屏幕宽度(像素)
    quint16 screenHeight;   // 项目屏幕高度(像素)
    quint8 depth;           // 项目图片深度
    QMap<quint16, BmImg> imgMap;
    QMap<quint16, BmComImg> comImgMap;
    QVector<quint16> expand;

    void initDatabase();
    void load();    // 加载数据库数据
public:
    RawData(const QString path);
    ~RawData();

    QString getProject() const {return project;}
    QMap<quint16, BmImg> getImgMap() const {return imgMap;}
    void createFolder(quint16 id, QString name = "Untitled");
    void createBmp(quint16 id, QString name, const QImage &img);
    void createBmp(quint16 id, QString name, quint16 wide, quint16 height);

    void addExpandNode(quint16 id) { expand << id; };
    bool isExpandNode(quint16 id) { return expand.contains(id); }
    void clearExpandNode() { expand.clear(); }
};

#endif // RAWDATA_H
