#ifndef RAWDATA_H
#define RAWDATA_H
#include <QString>
#include <QList>
#include <QtSql>
#include <QPixmap>
#include <QMap>
#include <QJsonObject>

// TODO 二合一 FileMap
struct BmFile
{
    quint32 id;
    quint32 pid;
    bool isFolder;
    QString name;
    QString details;
    quint16 wide;
    quint16 height;
    QImage image;
    QString data;
    QJsonObject jsonComImg;
    bool isExpand;
    BmFile() {}
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
    QJsonObject jsonImg;
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
    QMap<quint16, BmFile> imgMap;
 //   QMap<quint16, BmComImg> comImgMap;


    void initDatabase();

public:
    RawData(const QString path);
    ~RawData();

    QVector<quint16> expand;

    QString getProject() const {return project;}
    QMap<quint16, BmFile> getImgMap() const {return imgMap;}
//    QMap<quint16, BmComImg> getComImgMap() const {return comImgMap;}
    BmFile getImgInfo(quint16 id) const { return imgMap[id]; }
    void load();    // 加载数据库数据
    void createFolder(quint16 id, QString name = "Untitled");
    void createBmp(quint16 id, QString name, const QImage &img);
    void createBmp(quint16 id, QString name, quint16 wide, quint16 height);
    void rename(quint16 id, QString name);
    void remove(quint16 id);
    QImage getImage(quint16 id);
    void setImage(quint16 id, QImage image);

    void addExpandNode(quint16 id) { expand << id; };
    bool isExpandNode(quint16 id) { return expand.contains(id); }
    void clearExpandNode() { expand.clear(); }

    void test() {qDebug()<<"Test";}
};

#endif // RAWDATA_H
