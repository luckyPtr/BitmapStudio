#include "rawdata.h"
#include <QByteArray>
#include <QImage>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QPainter>
#include "global.h"
#include "imgencoderfactory.h"

// QImage编码为PNG字节
static QByteArray encodePngBytes(const QImage &img)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return byteArray;
}

QString RawData::calFullName(int id)
{
    QString fullName;
    if (dataMap.contains(id))
    {
        if(RawData::isClassImgType(dataMap[id].type))
        {
            fullName = "IMG";
        }
        else if(RawData::isClassComImgType(dataMap[id].type))
        {
            fullName = "GRP";
        }

        auto appendParentName = [&](auto&& self, int id)->void {
            quint16 p = dataMap[id].pid;
            if(p != 0)
            {
                self(self, p);
            }
            // pid==0 是虚拟根，dataMap 中没有对应节点；非const的operator[]会把
            // 默认节点插入key=0，其type未初始化，恰好为图片类型时会导致建树无限递归
            if(p != 0 && dataMap[p].type == RawData::TypeImgGrpFolder)
            {
                fullName.append("[" + dataMap[id].name + "]");
            }
            else
            {
                fullName.append("_" + dataMap[id].name);
            }
        };

        appendParentName(appendParentName, id);
    }

    return fullName;
}

void RawData::updateFullName()
{
    for (auto it = dataMap.begin(); it != dataMap.end(); ++it) {
        it.value().fullName = calFullName(it.value().id);
    }
}

bool RawData::isContainerType(int type)
{
    return type == TypeImgFolder || type == TypeImgGrpFolder || type == TypeComImgFolder;
}

RawData::PathResolve RawData::resolvePath(const QString &path, quint16 *id) const
{
    bool inImg = pathIndex.contains(path);
    bool inCom = comPathIndex.contains(path);
    if (inImg && inCom) return PathAmbiguous;
    if (!inImg && !inCom) return PathNotFound;
    if (id) *id = inImg ? pathIndex.value(path) : comPathIndex.value(path);
    return PathOk;
}

QStringList RawData::takeLoadWarnings()
{
    QStringList warnings = loadWarnings;
    loadWarnings.clear();
    return warnings;
}

bool RawData::canMove(quint16 id, quint16 newPid) const
{
    if (!dataMap.contains(id)) return false;
    if (newPid != 0 && !dataMap.contains(newPid)) return false;

    if (newPid != 0)
    {
        int type = dataMap[id].type;
        int ptype = dataMap[newPid].type;
        // 目标必须是同树的文件夹；图片组只接受图片叶子
        if (isClassImgType(type))
        {
            if (ptype == TypeImgGrpFolder && type != TypeImgFile) return false;
            if (ptype != TypeImgFolder && ptype != TypeImgGrpFolder) return false;
        }
        else if (isClassComImgType(type))
        {
            if (ptype != TypeComImgFolder) return false;
        }
        else return false;

        // 不能移动到自身或自己的子孙下面
        quint16 p = newPid;
        while (p != 0)
        {
            if (p == id) return false;
            p = dataMap[p].pid;
        }
    }

    // 目标层级不得已有同名节点：组合图按路径引用，同级重名会让引用解析产生歧义
    //（同名重排自身排除，同树校验与sanitizeName一致）
    if (isSiblingNameClash(dataMap[id].name, newPid, dataMap[id].type, id)) return false;

    return true;
}

// pid下一层同树子节点的显示顺序（容器在前、叶子在后，组内按id升序即当前文件顺序）
QVector<quint16> RawData::childOrder(quint16 pid, int forType) const
{
    bool imgTree = isClassImgType(forType);
    QVector<quint16> containers, leaves;
    for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
    {
        const BmFile &bf = it.value();
        if (bf.pid != pid) continue;
        if (imgTree ? !isClassImgType(bf.type) : !isClassComImgType(bf.type)) continue;
        (isContainerType(bf.type) ? containers : leaves) << it.key();
    }
    return containers + leaves;
}

// 收集pid下一层的规范顺序（容器在前、叶子在后，组内按id升序即当前显示顺序），
// 并递归子级追加到order；movedId所在层级先把它从原位取出插到pos
//（pos为移除源后的插入下标，<0表示追加到末尾），再稳定重分区维持容器在前
void RawData::collectLevel(quint16 pid, bool imgTree, quint16 movedId, int pos, QVector<quint16> &order) const
{
    QVector<quint16> level = childOrder(pid, imgTree ? TypeImgFolder : TypeComImgFolder);

    if (level.contains(movedId))
    {
        level.removeAt(level.indexOf(movedId));
        int p = pos < 0 ? level.size() : pos;
        level.insert(qBound(0, p, level.size()), movedId);

        QVector<quint16> c2, l2;
        foreach (quint16 oid, level)
            (isContainerType(dataMap[oid].type) ? c2 : l2) << oid;
        level = c2 + l2;
    }

    foreach (quint16 oid, level)
    {
        order << oid;
        collectLevel(oid, imgTree, movedId, pos, order);
    }
}

bool RawData::move(quint16 id, quint16 newPid, int pos, QHash<quint16, quint16> *remap)
{
    if (!canMove(id, newPid)) return false;

    dataMap[id].pid = newPid;

    // 同级顺序完全由内存id顺序体现（序列化按id升序、容器在前），
    // 因此移动后按规范DFS序重编全部id，使文件顺序=显示顺序=想要的顺序
    QVector<quint16> order;
    collectLevel(0, true, id, pos, order);      // 图片树在前
    collectLevel(0, false, id, pos, order);     // 组合图树在后

    if (order.size() == dataMap.size())         // 防御：出现孤儿/环时退回仅改父节点的旧行为
    {
        QHash<quint16, quint16> oldToNew;
        for (int i = 0; i < order.size(); ++i)
            oldToNew.insert(order[i], i + 1);

        QMap<quint16, BmFile> newMap;
        foreach (quint16 oldId, order)
        {
            BmFile bf = dataMap[oldId];
            bf.id = oldToNew.value(oldId);
            bf.pid = bf.pid == 0 ? 0 : oldToNew.value(bf.pid);
            // 组合图成员引用的图片id同步重映射
            for (auto &item : bf.comImg.items)
                item.id = oldToNew.value(item.id, item.id);
            newMap.insert(bf.id, bf);
        }
        dataMap = newMap;

        for (quint16 &eid : expand)
            eid = oldToNew.value(eid, eid);
        nextId = order.size() + 1;
        if (remap)
            *remap = oldToNew;
    }

    updateFullName();
    return save();   // 引用在保存时按新树重新生成路径，自动级联
}

// 同树同级同名判定：仅同一棵树（images/composites两树根级互不影响）的同名才算冲突。
// 组合图按路径引用节点，同级重名会让路径解析产生歧义，必须杜绝
bool RawData::isSiblingNameClash(const QString &name, quint16 pid, int type, quint16 excludeId) const
{
    for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
    {
        if (it.key() == excludeId) continue;
        const BmFile &bf = it.value();
        if (bf.pid == pid && bf.name == name &&
            RawData::isClassImgType(bf.type) == RawData::isClassImgType(type))
            return true;
    }
    return false;
}

// 名称规范化：去掉路径分隔符，同树同级冲突时自动加后缀。
// report=true（加载路径）时对任何改名发出警告，让check能把静默规范化变成显式问题；
// 变更方法（createX/rename）不report——它们会把实际名称回显给调用方
QString RawData::sanitizeName(const QString &name, quint16 pid, int type, bool report)
{
    QString n = name;
    n.replace('/', '_');
    n.replace('\\', '_');
    if (n.isEmpty()) n = "untitled";

    QString base = n;
    int suffix = 1;
    while (isSiblingNameClash(n, pid, type))
        n = base + QString("_%1").arg(suffix++);

    if (report && n != name)
    {
        addWarning(QString("Node name normalized: \"%1\" -> \"%2\" (duplicate sibling name or invalid characters)").arg(name, n));
    }
    return n;
}

// 节点名称合法性：仅ASCII字母/数字/下划线。名称用于拼接导出的C数组名
// （IMG_/GRP_固定前缀+路径段），数字开头因有前缀而合法；中文/空格/连字符会导致生成的C无法编译。
bool RawData::isValidNodeName(const QString &name)
{
    if (name.isEmpty()) return false;
    for (const QChar &c : name)
    {
        const char ch = c.toLatin1();
        const bool ok = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_';
        if (!ok) return false;
    }
    return true;
}

void RawData::addWarning(const QString &msg)
{
    loadWarnings.append(msg);
    qWarning().noquote() << msg;
}

void RawData::load()
{
    dataMap.clear();
    pathIndex.clear();
    comPathIndex.clear();
    loadWarnings.clear();
    nextId = 1;
    valid = false;

    QFile file(project);
    if (!file.open(QIODevice::ReadOnly))
    {
        addWarning(QString("Failed to open project file: %1").arg(project));
        return;
    }
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    file.close();
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject())
    {
        addWarning(QString("Failed to parse project JSON: %1 (%2)").arg(project).arg(parseErr.errorString()));
        return;
    }

    QJsonObject root = doc.object();
    if (root.value("format").toString() != "bms")
    {
        addWarning(QString("Not a valid Bitmap Studio project file: %1").arg(project));
        return;
    }
    int version = root.value("version").toInt(1);
    if (version > 1)
    {
        addWarning(QString("Project file format version is newer (%1); parsing best-effort").arg(version));
    }

    settings = Settings();
    settings.brief = root.value("note").toString();
    QJsonArray screen = root.value("screen").toArray();
    if (screen.size() == 2)
    {
        settings.size = QSize(screen.at(0).toInt(), screen.at(1).toInt());
    }

    QJsonObject exp = root.value("export").toObject();
    // scan(ZH/ZL/HL/LH) + bitOrder(LSB/MSB) -> ImgEncoderFactory模式编号
    int mode = ImgEncoderFactory::ZH_LSB;
    QString scan = exp.value("scan").toString("ZH");
    if (scan == "ZL") mode = ImgEncoderFactory::ZL_LSB;
    else if (scan == "HL") mode = ImgEncoderFactory::HL_LSB;
    else if (scan == "LH") mode = ImgEncoderFactory::LH_LSB;
    if (exp.value("bitOrder").toString("LSB") == "MSB") mode += 4;
    settings.mode = mode;
    settings.format = exp.value("output").toString("C");
    settings.path = exp.value("outdir").toString();
    QJsonObject kw = exp.value("keywords").toObject();
    if (!kw.isEmpty())
    {
        if (kw.contains("const")) settings.keywordConst = kw.value("const").toString();
        if (kw.contains("pos")) settings.keywordImgPos = kw.value("pos").toString();
        if (kw.contains("size")) settings.keywordImgSize = kw.value("size").toString();
        if (kw.contains("addr")) settings.keywordImgAddr = kw.value("addr").toString();
    }
    settings.customTypedef = exp.value("customTypedef").toBool(false);

    // 先解析图片树（建立路径索引），再解析组合图树（解析成员引用）
    parseLevel(root.value("images").toArray(), 0, true, "");
    parseLevel(root.value("composites").toArray(), 0, false, "");

    updateFullName();

    // 存量名称的导出安全性检查：非法字符会让check显式暴露（不阻断加载/保存）
    for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
    {
        if (!isValidNodeName(it.value().name))
            addWarning(QString("Node name is not export-safe (only letters/digits/underscore allowed): \"%1\"").arg(it.value().name));
    }

    valid = true;
}

// 解析一层节点数组；同级先容器后叶子，组内保持数组顺序
void RawData::parseLevel(const QJsonArray &arr, quint16 pid, bool imgTree, const QString &parentPath)
{
    auto parsePngLeaf = [&](const QJsonObject &node, quint16 leafPid, const QString &leafParentPath)
    {
        QString name = sanitizeName(node.value("name").toString(), leafPid, TypeImgFile, true);
        if (name.isEmpty()) name = "untitled";
        QString path = leafParentPath.isEmpty() ? name : leafParentPath + "/" + name;

        quint16 id = nextId++;
        BmFile bi;
        bi.id = id;
        bi.pid = leafPid;
        bi.type = TypeImgFile;
        bi.name = name;
        bi.brief = node.value("note").toString();
        QByteArray png = QByteArray::fromBase64(node.value("png").toString().toLatin1());
        bi.png = png;       // 缓存原始字节，未编辑时落盘原样写回
        if (!bi.image.loadFromData(png))
        {
            addWarning(QString("Failed to decode image data: %1").arg(path));
        }
        dataMap.insert(id, bi);
        pathIndex.insert(path, id);
    };

    for (int pass = 0; pass < 2; ++pass)
    {
        for (const QJsonValue &v : arr)
        {
            if (!v.isObject())
            {
                addWarning("Ignoring non-object node");
                continue;
            }
            QJsonObject node = v.toObject();
            bool container = node.contains("children") || node.contains("frames");
            if ((pass == 0) != container) continue;

            if (node.contains("png"))
            {
                if (!imgTree)
                {
                    // 图片节点只能存在于images树；留在组合图树会在保存时被静默丢弃
                    addWarning(QString("Image node found in composite tree, ignored: %1").arg(node.value("name").toString()));
                    continue;
                }
                parsePngLeaf(node, pid, parentPath);
                continue;
            }

            if (node.contains("items"))
            {
                if (imgTree)
                {
                    addWarning(QString("Composite node found in image tree, ignored: %1").arg(node.value("name").toString()));
                    continue;
                }
                // 组合图叶子
                QString name = sanitizeName(node.value("name").toString(), pid, TypeComImgFile, true);
                if (name.isEmpty()) name = "untitled";
                QString path = parentPath.isEmpty() ? name : parentPath + "/" + name;
                quint16 id = nextId++;
                BmFile bi;
                bi.id = id;
                bi.pid = pid;
                bi.type = TypeComImgFile;
                bi.name = name;
                bi.brief = node.value("note").toString();
                if (node.contains("size"))
                {
                    QJsonArray s = node.value("size").toArray();
                    if (s.size() == 2) bi.comImg.size = QSize(s.at(0).toInt(), s.at(1).toInt());
                    bi.followScreen = false;
                }
                else
                {
                    bi.comImg.size = settings.size;     // 省略size = 跟随屏幕
                    bi.followScreen = true;
                }
                for (const QJsonValue &iv : node.value("items").toArray())
                {
                    QJsonObject io = iv.toObject();
                    QString ref = io.value("image").toString();
                    if (!pathIndex.contains(ref))
                    {
                        addWarning(QString("Dangling reference in composite %1 ignored: %2").arg(name).arg(ref));
                        continue;
                    }
                    QJsonArray pos = io.value("pos").toArray();
                    ComImgItem item((qint16)(pos.size() >= 1 ? pos.at(0).toInt() : 0),
                                    (qint16)(pos.size() >= 2 ? pos.at(1).toInt() : 0),
                                    pathIndex.value(ref));
                    bi.comImg.items.append(item);
                }
                dataMap.insert(id, bi);
                comPathIndex.insert(path, id);    // CLI寻址用
                continue;
            }

            if (node.contains("frames") && !imgTree)
            {
                // 图片组只能存在于images树
                addWarning(QString("Image group node found in composite tree, ignored: %1").arg(node.value("name").toString()));
                continue;
            }

            int nodeType = node.contains("frames") ? TypeImgGrpFolder
                                                   : (imgTree ? TypeImgFolder : TypeComImgFolder);
            QString name = sanitizeName(node.value("name").toString(), pid, nodeType, true);
            QString path = parentPath.isEmpty() ? name : parentPath + "/" + name;

            if (node.contains("children"))
            {
                quint16 id = nextId++;
                BmFile bi;
                bi.id = id;
                bi.pid = pid;
                bi.type = nodeType;
                bi.name = name;
                bi.brief = node.value("note").toString();
                dataMap.insert(id, bi);
                if (imgTree) pathIndex.insert(path, id);
                else comPathIndex.insert(path, id);      // CLI寻址用
                parseLevel(node.value("children").toArray(), id, imgTree, path);
            }
            else if (node.contains("frames"))
            {
                // 图片组：只包含图片叶子，帧路径同样纳入索引供组合图引用
                quint16 id = nextId++;
                BmFile bi;
                bi.id = id;
                bi.pid = pid;
                bi.type = TypeImgGrpFolder;
                bi.name = name;
                bi.brief = node.value("note").toString();
                dataMap.insert(id, bi);
                pathIndex.insert(path, id);
                for (const QJsonValue &fv : node.value("frames").toArray())
                {
                    if (!fv.isObject() || !fv.toObject().contains("png"))
                    {
                        addWarning(QString("Ignoring non-image node inside image group %1").arg(name));
                        continue;
                    }
                    parsePngLeaf(fv.toObject(), id, path);
                }
            }
            else
            {
                addWarning(QString("Unrecognized node shape, ignored: %1").arg(name));
            }
        }
    }
}

// 序列化pid下的一层子节点（dataMap按id升序迭代，容器在前），图片树节点同时登记路径索引
QJsonArray RawData::serializeChildren(quint16 pid, bool imgTree, const QString &parentPath, QHash<quint16, QString> &idPath)
{
    QVector<const BmFile *> containers, leaves;
    for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
    {
        const BmFile &bf = it.value();
        if (bf.pid != pid) continue;
        if (imgTree ? !RawData::isClassImgType(bf.type) : !RawData::isClassComImgType(bf.type)) continue;
        if (isContainerType(bf.type)) containers << &bf;
        else leaves << &bf;
    }

    auto nodeBase = [](const BmFile &bf) {
        QJsonObject o;
        o.insert("name", bf.name);
        if (!bf.brief.isEmpty()) o.insert("note", bf.brief);
        return o;
    };

    QJsonArray out;
    for (const BmFile *bf : containers)
    {
        QString path = parentPath.isEmpty() ? bf->name : parentPath + "/" + bf->name;
        QJsonObject o = nodeBase(*bf);
        if (bf->type == TypeImgGrpFolder)
        {
            QJsonArray frames;
            for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
            {
                const BmFile &f = it.value();
                if (f.pid != bf->id || f.type != TypeImgFile) continue;
                QJsonObject fo = nodeBase(f);
                fo.insert("png", QString::fromLatin1((f.png.isEmpty() ? encodePngBytes(f.image) : f.png).toBase64()));
                frames.append(fo);
                idPath.insert(f.id, path + "/" + f.name);
            }
            o.insert("frames", frames);
        }
        else
        {
            o.insert("children", serializeChildren(bf->id, imgTree, path, idPath));
        }
        out.append(o);
    }
    for (const BmFile *bf : leaves)
    {
        QJsonObject o = nodeBase(*bf);
        if (bf->type == TypeImgFile)
        {
            o.insert("png", QString::fromLatin1((bf->png.isEmpty() ? encodePngBytes(bf->image) : bf->png).toBase64()));
            idPath.insert(bf->id, parentPath.isEmpty() ? bf->name : parentPath + "/" + bf->name);
        }
        else
        {
            if (!bf->followScreen)
            {
                QJsonArray s;
                s << bf->comImg.size.width() << bf->comImg.size.height();
                o.insert("size", s);
            }
            QJsonArray items;
            foreach (const ComImgItem &item, bf->comImg.items)
            {
                if (!idPath.contains(item.id)) continue;    // 悬空引用不落盘
                QJsonObject io;
                io.insert("image", idPath.value(item.id));
                QJsonArray pos;
                pos << item.x << item.y;
                io.insert("pos", pos);
                items.append(io);
            }
            o.insert("items", items);
        }
        out.append(o);
    }
    return out;
}

bool RawData::save()
{
    if (!valid)
    {
        // 无效工程（解析失败/旧格式）一律拒绝写回，防止覆盖原文件
        qWarning() << "工程文件无效，拒绝写回:" << project;
        return false;
    }

    QJsonObject root;
    root.insert("format", "bms");
    root.insert("version", 1);
    if (!settings.brief.isEmpty()) root.insert("note", settings.brief);
    QJsonArray screen;
    screen << settings.size.width() << settings.size.height();
    root.insert("screen", screen);

    QJsonObject exp;
    int mode = settings.mode;
    if (mode < ImgEncoderFactory::ZH_LSB || mode > ImgEncoderFactory::LH_MSB) mode = ImgEncoderFactory::ZH_LSB;
    static const char *scans[4] = {"ZH", "ZL", "HL", "LH"};
    exp.insert("scan", scans[mode % 4]);
    exp.insert("bitOrder", mode < 4 ? "LSB" : "MSB");
    exp.insert("output", settings.format.isEmpty() ? "C" : settings.format);
    if (!settings.path.isEmpty()) exp.insert("outdir", settings.path);
    Settings def;
    if (settings.keywordConst != def.keywordConst || settings.keywordImgPos != def.keywordImgPos ||
        settings.keywordImgSize != def.keywordImgSize || settings.keywordImgAddr != def.keywordImgAddr)
    {
        // 仅当用户改过默认关键字时才落盘
        QJsonObject kw;
        kw.insert("const", settings.keywordConst);
        kw.insert("pos", settings.keywordImgPos);
        kw.insert("size", settings.keywordImgSize);
        kw.insert("addr", settings.keywordImgAddr);
        exp.insert("keywords", kw);
    }
    if (settings.customTypedef) exp.insert("customTypedef", true);
    root.insert("export", exp);

    QHash<quint16, QString> idPath;
    root.insert("images", serializeChildren(0, true, "", idPath));
    root.insert("composites", serializeChildren(0, false, "", idPath));

    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);

    // 首选原子写盘（临时文件+重命名替换）。Windows下若原文件被文本编辑器等程序占用
    // （其打开方式不含共享删除），重命名替换会被系统拒绝，此时降级为一次性直写
    QSaveFile file(project);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(bytes);
        if (file.commit())
        {
            return true;
        }
    }
    qWarning() << "原子写盘失败，尝试直接写入:" << project << file.errorString();

    // 降级直写：占用方通常允许共享写。此路径无原子性，写入窗口内异常退出可能损坏文件
    QFile direct(project);
    if (direct.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qint64 written = direct.write(bytes);
        direct.flush();
        direct.close();
        if (written == bytes.size())
        {
            qWarning() << "工程文件已通过非原子方式写入:" << project;
            return true;
        }
    }
    qWarning() << "工程文件写入失败:" << project << direct.errorString();
    return false;
}

RawData::RawData(const QString path)
{
    project = path;
    QFileInfo info(path);
    if (!info.isFile())
    {
        save();      // 新建工程：写入默认内容
    }
    else
    {
        load();
    }
}

RawData::~RawData()
{
    qDebug() << "~RawData:" << project;
}

bool RawData::createFolder(int id, QString name, QString brief)
{
    int type = TypeUnknow;
    quint16 pid = 0;

    if(id == -3)
    {
        type = RawData::TypeImgFolder;
    }
    else if(id == -4)
    {
        type = RawData::TypeComImgFolder;
    }
    else if(dataMap.contains(id))
    {
        pid = dataMap[id].pid;
        int curType = dataMap[id].type;

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

    if(type != TypeUnknow)
    {
        BmFile bi;
        bi.id = nextId++;
        bi.pid = pid;
        bi.type = type;
        bi.name = sanitizeName(name, pid, type);
        bi.brief = brief;
        dataMap.insert(bi.id, bi);
        updateFullName();
        return save();
    }
    return false;
}

bool RawData::createBmp(int id, QString name, const QImage &img, const QString brief)
{
    bool isVaild = false;
    quint16 pid = 0;

    if(id == -3)    // 如果是图片类
    {
        isVaild = true;
    }
    else if(dataMap.contains(id))
    {
        pid = dataMap[id].pid;
        int type = dataMap[id].type;

        if(type == RawData::TypeImgFolder ||\
           type == RawData::TypeImgGrpFolder)
        {
            pid = id;
        }
        isVaild = true;
    }

    if(isVaild)
    {
        BmFile bi;
        bi.id = nextId++;
        bi.pid = pid;
        bi.type = RawData::TypeImgFile;
        bi.name = sanitizeName(name, pid, TypeImgFile);
        bi.image = img;
        bi.png = encodePngBytes(img);
        bi.brief = brief;
        dataMap.insert(bi.id, bi);
        updateFullName();
        return save();
    }
    return false;
}

bool RawData::createBmp(int id, QString name, QSize size, const QString brief)
{
    QImage image(size, QImage::Format_RGBA8888);
    image.fill(Qt::white);
    return createBmp(id, name, image, brief);
}

bool RawData::createComImg(int id, QString name, QSize size, const QString brief)
{
    bool isVaild = false;
    quint16 pid = 0;

    if(id == -4)
    {
        isVaild = true;
    }
    else if(dataMap.contains(id))
    {
        pid = dataMap[id].pid;
        if(dataMap[id].type == RawData::TypeComImgFolder)
        {
            pid = id;
        }
        isVaild = true;
    }

    if(isVaild)
    {
        BmFile bi;
        bi.id = nextId++;
        bi.pid = pid;
        bi.type = RawData::TypeComImgFile;
        bi.name = sanitizeName(name, pid, TypeComImgFile);
        bi.brief = brief;
        // 尺寸与屏幕一致时视为跟随屏幕（size不落盘，改屏幕尺寸时自动跟随）
        bi.followScreen = (size == settings.size);
        bi.comImg = ComImg(size);
        dataMap.insert(bi.id, bi);
        updateFullName();
        return save();
    }
    return false;
}

bool RawData::rename(int id, QString name)
{
    if(dataMap.contains(id))
    {
        dataMap[id].name = sanitizeName(name, dataMap[id].pid, dataMap[id].type);
        updateFullName();
        return save();
    }
    return false;
}

QString RawData::getName(int id)
{
    return dataMap[id].name;
}

bool RawData::remove(int id)
{
    if(!dataMap.contains(id)) return false;

    // 迭代收集整棵子树后一次性删除：避免递归remove在每个子孙节点上
    // 都执行一次全表fullName重算+全文件序列化写盘（删除大文件夹呈平方级开销）
    QVector<quint16> subtree;
    QVector<quint16> stack;
    stack << id;
    while (!stack.isEmpty())
    {
        quint16 cur = stack.takeLast();
        subtree << cur;
        for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
        {
            if (it.value().pid == cur) stack << it.key();
        }
    }
    foreach (quint16 sid, subtree)
    {
        dataMap.remove(sid);
    }
    updateFullName();
    return save();
}

bool RawData::imgFolderConvert(int id)
{
    if(dataMap.contains(id))
    {
        int type = dataMap[id].type;

        if(type == RawData::TypeImgFolder || type == RawData::TypeImgGrpFolder)
        {
            dataMap[id].type = type == RawData::TypeImgFolder ? RawData::TypeImgGrpFolder : RawData::TypeImgFolder;
            updateFullName();
            return save();
        }
    }
    return false;
}



// 将img2合并到img1，img2的位置在x,y（越界部分裁剪；起点钳制到画布内，绘制结果与逐像素实现一致）
static void mergeImage(QImage &img1, const QImage &img2, int x, int y)
{
    int sx = qMax(0, x);
    int sy = qMax(0, y);
    for(int i = sx; i < img1.width() && i < x + img2.width(); i++)
    {
        for(int j = sy; j < img1.height() && j < y + img2.height(); j++)
        {
            img1.setPixelColor(i, j, img2.pixelColor(i-x, j-y));
        }
    }
}

// 离屏合成：白底RGB888画布，图片与绘图原语按items顺序混排绘制（图片粘贴与GUI导出路径语义一致）；
// 不涉及工程节点变更。图片成员id不存在时跳过（与getImage行为一致）
QImage RawData::renderCompose(QSize size, const QVector<ComDrawItem> &items)
{
    QImage image(size, QImage::Format_RGB888);
    image.fill(Qt::white);
    QPainter p(&image);
    foreach (const ComDrawItem &it, items)
    {
        switch (it.kind)
        {
        case ComDrawItem::Image:
            if (dataMap.contains(it.id))
            {
                mergeImage(image, dataMap[it.id].image, it.x, it.y);
            }
            break;
        case ComDrawItem::Line:
            p.setPen(it.white ? Qt::white : Qt::black);
            p.drawLine(it.x, it.y, it.x2, it.y2);
            break;
        case ComDrawItem::FillRect:
            p.fillRect(it.x, it.y, it.w, it.h, it.white ? Qt::white : Qt::black);
            break;
        case ComDrawItem::InvertRect:
        {
            QRect r = QRect(it.x, it.y, it.w, it.h).intersected(image.rect());
            if (!r.isEmpty())
            {
                QImage region = image.copy(r);
                region.invertPixels(QImage::InvertRgb);
                p.drawImage(r.topLeft(), region);
            }
            break;
        }
        case ComDrawItem::Points:
            p.setPen(it.white ? Qt::white : Qt::black);
            foreach (const QPoint &pt, it.pts)
            {
                p.drawPoint(pt);
            }
            break;
        }
    }
    return image;
}

QImage RawData::getImage(int id)
{
    if(dataMap.contains(id))
    {
        if(dataMap[id].type == RawData::TypeImgFile)
        {
            return dataMap[id].image;
        }
        else if(dataMap[id].type == RawData::TypeComImgFile)
        {
            QVector<ComDrawItem> items;
            foreach(auto item, dataMap[id].comImg.items)
            {
                items.append(ComDrawItem::imageItem(item.x, item.y, item.id));
            }
            return renderCompose(dataMap[id].comImg.size, items);
        }
    }
    return QImage();
}

// 获取导出的图片
QImage RawData::getExportImage(int id)
{
    QImage image = getImage(id);
    if (image.isNull())
        return image;

    // 遍历图片的每一个像素
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);

            // 计算像素的亮度
            int brightness = (color.red() + color.green() + color.blue()) / 3;

            if (brightness < 128) {
                image.setPixelColor(x, y, QColor(Global::exportImgColor_1));
            } else {
                image.setPixelColor(x, y, QColor(Global::exportImgColor_0));
            }
        }
    }

    return image;
}

bool RawData::setImage(int id, QImage image)
{
    if(dataMap.contains(id))
    {
        dataMap[id].image = image;
        dataMap[id].png = encodePngBytes(image);
        return save();
    }
    return false;
}

QString RawData::getBrief(int id)
{
    return dataMap[id].brief;
}

bool RawData::setBrief(int id, QString brief)
{
    if(dataMap.contains(id))
    {
        dataMap[id].brief = brief;
        return save();
    }
    return false;
}

ComImg RawData::getComImg(int id)
{
    return dataMap[id].comImg;
}

bool RawData::setComImg(int id, ComImg ci)
{
    if(dataMap.contains(id))
    {
        // 尺寸发生变化说明是显式设定，不再跟随屏幕
        if(ci.size != dataMap[id].comImg.size)
        {
            dataMap[id].followScreen = false;
        }
        dataMap[id].comImg = ci;
        return save();
    }
    return false;
}



bool RawData::saveSettings(Settings settings)
{
    this->settings = settings;
    // 跟随屏幕的组合图同步到新屏幕尺寸
    for (auto it = dataMap.begin(); it != dataMap.end(); ++it)
    {
        if (it.value().type == TypeComImgFile && it.value().followScreen)
        {
            it.value().comImg.size = settings.size;
        }
    }
    return save();
}

QSize RawData::getSize()
{
    return settings.size;
}

bool RawData::haveSubFolder(int id)
{
    foreach (auto i, dataMap) {
        if(i.pid == id && (i.type == TypeImgFolder || i.type == TypeImgGrpFolder))
        {
            return true;
        }
    }
    return false;
}
