#ifndef RAWDATA_H
#define RAWDATA_H
#include <QString>
#include <QList>
#include <QImage>
#include <QPixmap>
#include <QMap>
#include <QVector>
#include <QHash>
#include <QPoint>
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
};

// 离屏合成（renderCompose）用的绘制项：图片或绘图原语。仅用于预览/演示合成，不落盘
struct ComDrawItem
{
    enum Kind { Image, Line, FillRect, InvertRect, Points };
    Kind kind;
    qint16 x, y;        // Image: 粘贴位置; Line: 起点; Rect: 左上角
    qint16 x2, y2;      // Line: 终点
    qint16 w, h;        // Rect: 宽高
    quint16 id;         // Image: 成员图片id
    bool white;         // Line/FillRect/Points 前景色: false=黑(点亮) true=白
    QVector<QPoint> pts;    // Points: 点集

    ComDrawItem() : kind(Image), x(0), y(0), x2(0), y2(0), w(0), h(0), id(0), white(false) {}
    static ComDrawItem imageItem(qint16 x, qint16 y, quint16 id)
    {
        ComDrawItem d;
        d.kind = Image;
        d.x = x;
        d.y = y;
        d.id = id;
        return d;
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

    BmFile()
    {
        id = 0;
        pid = 0;
        type = -1;  // -1 = RawData::TypeUnknow：确定值，误用非const operator[]产生的空节点不会匹配任何类型过滤器
        followScreen = false;
    }
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

    QHash<QString, quint16> pathIndex;      // 图片树路径 -> id（含文件夹，供组合图引用解析与CLI寻址）
    QHash<QString, quint16> comPathIndex;   // 组合图树路径 -> id（仅供CLI寻址）
    QStringList loadWarnings;               // 加载过程中的警告（悬空引用/自动改名等），供check命令收集

    bool save();       // 序列化并原子写回JSON文件；失败（无效工程/打不开/提交失败）返回false
    void parseLevel(const QJsonArray &arr, quint16 pid, bool imgTree, const QString &parentPath);
    QJsonArray serializeChildren(quint16 pid, bool imgTree, const QString &parentPath, QHash<quint16, QString> &idPath);
    QString sanitizeName(const QString &name, quint16 pid, int type, bool report = false);
    void addWarning(const QString &msg);    // qWarning的同时收集进loadWarnings

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
    void load();    // 从JSON文件重新加载整个工程（会按文件顺序重编所有内存id）

    // 路径寻址（供CLI）：路径在两棵树中都存在时返回PathAmbiguous。
    // 注意：索引只在加载后有效，任何变更操作后需重新load才能继续寻址。
    enum PathResolve { PathNotFound, PathOk, PathAmbiguous };
    PathResolve resolvePath(const QString &path, quint16 *id = nullptr) const;

    QStringList takeLoadWarnings();     // 取走并清空加载警告
    // 变更方法返回值 = 是否已成功落盘（false表示编辑仅存在于内存，需提示用户）
    bool move(quint16 id, quint16 newPid);  // 移动节点（引用在保存时按新树自动级联）
    bool createFolder(int id, QString name = "Untitled", QString brief = "");
    bool createBmp(int id, QString name, const QImage &img, const QString brief = "");
    bool createBmp(int id, QString name, QSize size, const QString brief = "");
    bool createComImg(int id, QString name, QSize size, const QString brief = "");
    bool rename(int id, QString name);
    QString getName(int id);
    bool remove(int id);
    bool imgFolderConvert(int id);
    QImage getImage(int id);
    QImage renderCompose(QSize size, const QVector<ComDrawItem> &items);    // 离屏合成（图片+绘图原语按items顺序混排），不涉及工程节点变更/落盘
    QImage getExportImage(int id);
    bool setImage(int id, QImage image);
    QString getBrief(int id);
    bool setBrief(int id, QString brief);


    ComImg getComImg(int id);
    bool setComImg(int id, ComImg ci);

    void addExpandNode(quint16 id) { expand << id; };
    bool isExpandNode(quint16 id) { return expand.contains(id); }
    void clearExpandNode() { expand.clear(); }

    static bool isClassImgType(int type) { return type == TypeImgFolder || type == TypeImgGrpFolder || type == TypeImgFile; }
    static bool isClassComImgType(int type) { return type == TypeComImgFolder || type == TypeComImgFile; }

    Settings getSettings() const { return settings; }
    bool saveSettings(Settings settings);
    QSize getSize();
    bool haveSubFolder(int id); // 是否还有子文件夹

};

#endif // RAWDATA_H
