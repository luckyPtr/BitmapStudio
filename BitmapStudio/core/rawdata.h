#ifndef RAWDATA_H
#define RAWDATA_H
#include <QString>
#include <QList>
#include <QImage>
#include <QPixmap>
#include <QMap>
#include <QVector>
#include <QHash>
#include <QJsonObject>



// 组合图单个元素
struct ComImgItem
{
    qint16 x;
    qint16 y;
    quint16 id;

    ComImgItem(){}
    ComImgItem(qint16 x, qint16 y, quint16 id) {
        this->x = x;
        this->y = y;
        this->id = id;
    }

};

struct ComImg
{
    QSize size;
    QVector<ComImgItem> items;
    ComImg() {}
    ComImg(QSize size)
    {
        this->size = size;
        items.clear();
    }

    ComImg & operator = (ComImg comimg)
    {
        this->size = comimg.size;
        this->items.clear();
        this->items.swap(comimg.items);

        return *this;
    }
};


struct BmFile
{
    quint32 id;
    quint32 pid;
    int type;
    QString name;
    QString fullName;
    QString brief;
    QImage image;
    QByteArray png;     // PNG字节缓存：未编辑的图片落盘时原样写回，保证字节级往返稳定并跳过重编码
    ComImg comImg;
    bool followScreen;  // 组合图尺寸是否跟随屏幕（size 省略时的内存标记，不落盘）

    bool isExpand;
    BmFile() { followScreen = false; }
};



/*
 * 工程文件为单文件 JSON（.bms），格式要点：
 * - 顶层：format("bms") / version / note / screen[w,h] / export / images / composites
 * - 节点形状即类型：children=文件夹，frames=图片组，png=图片，items=组合图
 * - 同级排序：容器在前、叶子在后；items 数组顺序 = 绘制顺序
 * - 组合图成员用图片树路径引用；默认值（note/keywords/customTypedef/size）省略不写
 */
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
    struct Settings
    {
        QSize size;             // 项目屏幕大小(像素)
        int mode;
        QString keywordConst;
        QString keywordImgPos;
        QString keywordImgSize;
        QString keywordImgAddr;
        QString path;
        QString format;
        QString brief;
        bool customTypedef;     // 自定义类型文件（勾选则不生成 bms_typedef.h）

        Settings()
        {
            size = QSize(128, 64);
            mode = 0;
            keywordConst = "const";
            keywordImgPos = "unsigned char";
            keywordImgSize = "unsigned char";
            keywordImgAddr = "unsigned int";
            path = nullptr;
            format = "C";
            customTypedef = false;  // 默认生成 bms_typedef.h
        }
    };

private:
    QString project;        // 项目文件
    Settings settings;
    QMap<quint16, BmFile> dataMap;
    quint16 nextId = 1;     // 内存id分配器，加载时按树的规范顺序（容器优先的深度优先）分配
    bool valid = true;      // 工程文件是否为有效的 bms JSON

    QHash<QString, quint16> pathIndex;      // 图片树路径 -> id，用于解析组合图成员引用

    void save();    // 序列化并原子写回JSON文件
    void parseLevel(const QJsonArray &arr, quint16 pid, bool imgTree, const QString &parentPath);
    QJsonArray serializeChildren(quint16 pid, bool imgTree, const QString &parentPath, QHash<quint16, QString> &idPath);
    QString sanitizeName(const QString &name, quint16 pid, int type);

    int getTypeFromId(int id);
    QString calFullName(int id);
    void updateFullName();
    static bool isContainerType(int type);
public:
    RawData(const QString path);
    ~RawData();

    QVector<quint16> expand;
    QString getProject() const {return project;}
    QMap<quint16, BmFile> getDataMap() const {return dataMap;}
    BmFile getBmFile(quint16 id) const { return dataMap[id]; }
    bool isValid() const { return valid; }
    void load();    // 从JSON文件重新加载整个工程（删除节点后由ProjectMng调用）
    void createFolder(int id, QString name = "Untitled", QString brief = "");
    void createBmp(int id, QString name, const QImage &img, const QString brief = "");
    void createBmp(int id, QString name, QSize size, const QString brief = "");
    void createComImg(int id, QString name, QSize size, const QString brief = "");
    void rename(int id, QString name);
    QString getName(int id);
    void remove(int id);
    void imgFolderConvert(int id);
    QImage getImage(int id);
    QImage getExportImage(int id);
    void setImage(int id, QImage image);
    QString getBrief(int id);
    void setBrief(int id, QString brief);


    ComImg getComImg(int id);
    void setComImg(int id, ComImg ci);

    void addExpandNode(quint16 id) { expand << id; };
    bool isExpandNode(quint16 id) { return expand.contains(id); }
    void clearExpandNode() { expand.clear(); }

    static bool isClassImgType(int type) { return type == TypeImgFolder || type == TypeImgGrpFolder || type == TypeImgFile; }
    static bool isClassComImgType(int type) { return type == TypeComImgFolder || type == TypeComImgFile; }

    Settings getSettings() { return settings; }
    void saveSettings(Settings settings);
    QSize getSize();
    bool haveSubFolder(int id); // 是否还有子文件夹

};

#endif // RAWDATA_H
