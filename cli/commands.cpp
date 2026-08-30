#include "commands.h"
#include "rawdata.h"
#include "exportrunner.h"
#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <cstdio>

namespace BmsCli {

// ---------- 基础输出/工具 ----------

static void out(const QString &s)
{
    QByteArray b = s.toUtf8();
    b += '\n';
    fwrite(b.constData(), 1, b.size(), stdout);
    fflush(stdout);
}

static void err(const QString &s)
{
    QByteArray b = s.toUtf8();
    b += '\n';
    fwrite(b.constData(), 1, b.size(), stderr);
    fflush(stderr);
}

// 路径归一化：去掉前导'/'；空串表示根
static QString normPath(QString p)
{
    while (p.startsWith('/')) p.remove(0, 1);
    return p;
}

// 取出args里的选项值：找到flag返回其值并从args移除两项；未找到返回defaultValue
static QString takeOpt(QStringList &args, const QString &flag, const QString &defaultValue = QString())
{
    int i = args.indexOf(flag);
    if (i < 0 || i + 1 >= args.size()) return defaultValue;
    QString v = args.at(i + 1);
    args.removeAt(i + 1);
    args.removeAt(i);
    return v;
}

static bool hasOpt(QStringList &args, const QString &flag)
{
    int i = args.indexOf(flag);
    if (i < 0) return false;
    args.removeAt(i);
    return true;
}

static RawData *openProject(const QString &file, int *exitCode)
{
    RawData *rd = new RawData(file);
    if (!rd->isValid())
    {
        err(QString("无法加载工程: %1（文件损坏或为旧版SQLite格式，旧版请用 tools/convert_legacy.py 转换）").arg(file));
        *exitCode = 2;
        delete rd;
        return nullptr;
    }
    return rd;
}

// 路径解析，失败时已打印错误并返回false
static bool resolve(const RawData *rd, const QString &path, quint16 *id)
{
    QString p = normPath(path);
    RawData::PathResolve r = rd->resolvePath(p, id);
    if (r == RawData::PathNotFound)
    {
        err(QString("路径不存在: /%1").arg(p));
        return false;
    }
    if (r == RawData::PathAmbiguous)
    {
        err(QString("路径 /%1 在图片树和组合图树中都存在，无法区分，请先重命名其一").arg(p));
        return false;
    }
    return true;
}

static QString kindLabel(int type)
{
    switch (type)
    {
    case RawData::TypeImgFile:      return "图片";
    case RawData::TypeComImgFile:   return "组合图";
    case RawData::TypeImgFolder:    return "文件夹";
    case RawData::TypeImgGrpFolder: return "图片组";
    case RawData::TypeComImgFolder: return "文件夹";
    default:                        return "未知";
    }
}

// mode -> scan/bitOrder 字符串（与RawData::save的映射保持一致）
static void modeToStrings(int mode, QString *scan, QString *bitOrder)
{
    static const char *scans[4] = {"ZH", "ZL", "HL", "LH"};
    if (mode < 0 || mode > 7) mode = 0;
    *scan = scans[mode % 4];
    *bitOrder = mode < 4 ? "LSB" : "MSB";
}

// ---------- init ----------

// scan(ZH/ZL/HL/LH) + bit(MSB/LSB) -> ImgEncoderFactory模式编号（与RawData::load的映射一致）
static int scanBitToMode(const QString &scan, const QString &bit)
{
    int mode = 0;
    if (scan == "ZL") mode = 1;
    else if (scan == "HL") mode = 2;
    else if (scan == "LH") mode = 3;
    if (bit == "MSB") mode += 4;
    return mode;
}

int init(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.isEmpty())
    {
        err("用法: bms-cli init <新工程.bms> [--screen 宽x高] [--scan ZH|ZL|HL|LH] [--bit MSB|LSB]\n"
            "                       [--format C|bin] [--outdir 目录] [--note 备注]");
        return 1;
    }
    QString screenStr = takeOpt(args, "--screen", "128x64");
    QString scan = takeOpt(args, "--scan", "ZH");
    QString bit = takeOpt(args, "--bit", "MSB");
    QString format = takeOpt(args, "--format", "C");
    QString outdir = takeOpt(args, "--outdir");
    QString note = takeOpt(args, "--note");

    if (scan != "ZH" && scan != "ZL" && scan != "HL" && scan != "LH")
    {
        err("无效的 --scan: " + scan + "（可选 ZH|ZL|HL|LH）");
        return 1;
    }
    if (bit != "MSB" && bit != "LSB")
    {
        err("无效的 --bit: " + bit + "（可选 MSB|LSB）");
        return 1;
    }
    if (format != "C" && format != "bin")
    {
        err("无效的 --format: " + format + "（可选 C|bin）");
        return 1;
    }
    QStringList wh = screenStr.split('x');
    if (wh.size() != 2 || wh.at(0).toInt() <= 0 || wh.at(1).toInt() <= 0)
    {
        err("无效的 --screen: " + screenStr + "（格式如 128x64）");
        return 1;
    }
    QSize screen(wh.at(0).toInt(), wh.at(1).toInt());

    QString file = args.at(0);
    if (QFileInfo(file).isFile())
    {
        err("工程文件已存在: " + file + "（请直接打开或更换路径）");
        return 1;
    }

    RawData rd(file);       // 文件不存在 -> 写入默认JSON
    if (!rd.isValid())
    {
        err("工程初始化失败: " + file);
        return 1;
    }
    RawData::Settings st = rd.getSettings();
    st.size = screen;
    st.mode = scanBitToMode(scan, bit);
    st.format = format;
    st.path = outdir;
    st.brief = note;
    if (!rd.saveSettings(st))
    {
        err("写入工程文件失败: " + file + "（目录只读或磁盘错误）");
        return 1;
    }

    out(QString("已创建工程: %1  屏幕 %2x%3  取模 %4 %5  输出 %6")
        .arg(QFileInfo(file).absoluteFilePath())
        .arg(screen.width()).arg(screen.height())
        .arg(scan).arg(bit).arg(format));
    return 0;
}

// ---------- check ----------

int check(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    bool asJson = hasOpt(args, "--json");
    if (args.isEmpty())
    {
        err("用法: bms-cli check <工程.bms> [--json]");
        return 1;
    }
    RawData rd(args.at(0));
    QStringList warnings = rd.takeLoadWarnings();

    if (asJson)
    {
        QJsonObject doc;
        doc.insert("valid", rd.isValid());
        QJsonArray arr;
        foreach (const QString &w, warnings) arr.append(w);
        doc.insert("warnings", arr);
        out(QString::fromUtf8(QJsonDocument(doc).toJson(QJsonDocument::Compact)));
        if (!rd.isValid()) return 2;
        return warnings.isEmpty() ? 0 : 1;
    }

    if (!rd.isValid())
    {
        foreach (const QString &w, warnings) err("错误: " + w);
        return 2;
    }
    if (warnings.isEmpty())
    {
        out("OK");
        return 0;
    }
    foreach (const QString &w, warnings)
    {
        out("警告: " + w);
    }
    out(QString("共 %1 项警告（悬空引用等异常项已在加载时按规则处理，保存后将固化）").arg(warnings.size()));
    return 1;
}

// ---------- info ----------

// 构建内存id->树路径的反查表（组合图成员以路径形式对外展示）
static QHash<quint16, QString> buildIdPath(const QMap<quint16, BmFile> &m)
{
    QHash<quint16, QString> idPath;
    foreach (quint16 id, m.keys())
    {
        QStringList parts;
        quint16 p = id;
        while (p != 0)
        {
            parts.prepend(m[p].name);
            p = m[p].pid;
        }
        idPath.insert(id, parts.join('/'));
    }
    return idPath;
}

static QJsonArray treeJson(const QMap<quint16, BmFile> &m, quint16 pid, bool imgTree,
                           const QHash<quint16, QString> &idPath)
{
    QList<BmFile> kids;
    foreach (const BmFile &bf, m)
    {
        if (bf.pid != pid) continue;
        if (imgTree ? !RawData::isClassImgType(bf.type) : !RawData::isClassComImgType(bf.type)) continue;
        kids << bf;
    }
    // 同级排序与GUI一致：容器在前、组内按id
    std::sort(kids.begin(), kids.end(), [](const BmFile &a, const BmFile &b) {
        bool ca = a.type == RawData::TypeImgFolder || a.type == RawData::TypeImgGrpFolder || a.type == RawData::TypeComImgFolder;
        bool cb = b.type == RawData::TypeImgFolder || b.type == RawData::TypeImgGrpFolder || b.type == RawData::TypeComImgFolder;
        if (ca != cb) return ca;
        return a.id < b.id;
    });

    QJsonArray arr;
    foreach (const BmFile &bf, kids)
    {
        QJsonObject o;
        o.insert("name", bf.name);
        if (!bf.brief.isEmpty()) o.insert("note", bf.brief);
        if (bf.type == RawData::TypeImgFile)
        {
            QJsonArray s; s << bf.image.width() << bf.image.height();
            o.insert("size", s);
        }
        else if (bf.type == RawData::TypeComImgFile)
        {
            QJsonArray s; s << bf.comImg.size.width() << bf.comImg.size.height();
            o.insert("size", s);
            QJsonArray items;
            foreach (const ComImgItem &it, bf.comImg.items)
            {
                QJsonObject io;
                io.insert("image", idPath.value(it.id, QString()));
                QJsonArray pos; pos << it.x << it.y;
                io.insert("pos", pos);
                items.append(io);
            }
            o.insert("items", items);
        }
        else if (bf.type == RawData::TypeImgGrpFolder)
        {
            o.insert("frames", treeJson(m, bf.id, imgTree, idPath));
        }
        else
        {
            o.insert("children", treeJson(m, bf.id, imgTree, idPath));
        }
        arr.append(o);
    }
    return arr;
}

int info(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.isEmpty())
    {
        err("用法: bms-cli info <工程.bms> [--json]");
        return 1;
    }
    bool asJson = hasOpt(args, "--json");
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    RawData::Settings st = rd->getSettings();
    QString scan, bitOrder;
    modeToStrings(st.mode, &scan, &bitOrder);

    if (asJson)
    {
        QJsonObject doc;
        QJsonArray screen; screen << st.size.width() << st.size.height();
        doc.insert("screen", screen);
        QJsonObject exp;
        exp.insert("scan", scan);
        exp.insert("bitOrder", bitOrder);
        exp.insert("output", st.format);
        if (!st.path.isEmpty()) exp.insert("outdir", st.path);
        doc.insert("export", exp);
        doc.insert("images", treeJson(rd->getDataMap(), 0, true, buildIdPath(rd->getDataMap())));
        doc.insert("composites", treeJson(rd->getDataMap(), 0, false, buildIdPath(rd->getDataMap())));
        out(QString::fromUtf8(QJsonDocument(doc).toJson(QJsonDocument::Indented)));
    }
    else
    {
        out(QString("屏幕: %1x%2    取模: %3 %4    输出: %5")
            .arg(st.size.width()).arg(st.size.height()).arg(scan, bitOrder, st.format));
        if (!st.brief.isEmpty()) out("备注: " + st.brief);

        // 按树打印（缩进体现层级）
        QMap<quint16, BmFile> m = rd->getDataMap();
        auto printLevel = [&m](auto &&self, quint16 pid, int depth, bool imgTree) -> void {
            foreach (const BmFile &bf, m)
            {
                if (bf.pid != pid) continue;
                if (imgTree ? !RawData::isClassImgType(bf.type) : !RawData::isClassComImgType(bf.type)) continue;
                QString line = QString("  ").repeated(depth) + "[" + kindLabel(bf.type) + "] " + bf.name;
                if (bf.type == RawData::TypeImgFile)
                    line += QString("  %1x%2").arg(bf.image.width()).arg(bf.image.height());
                else if (bf.type == RawData::TypeComImgFile)
                    line += QString("  %1x%2 (%3个成员)").arg(bf.comImg.size.width()).arg(bf.comImg.size.height()).arg(bf.comImg.items.size());
                if (!bf.brief.isEmpty()) line += "  \"" + bf.brief + "\"";
                out(line);
                if (bf.type == RawData::TypeImgFolder || bf.type == RawData::TypeComImgFolder || bf.type == RawData::TypeImgGrpFolder)
                    self(self, bf.id, depth + 1, imgTree);
            }
        };
        out("图片:");
        printLevel(printLevel, 0, 1, true);
        out("组合图:");
        printLevel(printLevel, 0, 1, false);
    }
    delete rd;
    return 0;
}

// ---------- render ----------

int render(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 2)
    {
        err("用法: bms-cli render <工程.bms> <路径> [-o 输出.png] [-s 放大倍数] [--ascii]");
        return 1;
    }
    QString outFile = takeOpt(args, "-o");
    QString scaleStr = takeOpt(args, "-s", "1");
    bool ascii = hasOpt(args, "--ascii");

    int code = 0;
    QString file = args.at(0);
    RawData *rd = openProject(file, &code);
    if (!rd) return code;

    quint16 id = 0;
    if (!resolve(rd, args.at(1), &id)) { delete rd; return 1; }

    QImage img = rd->getImage(id);    // 组合图自动按成员合成（绘制序/跟随屏幕/裁剪语义一致）
    if (img.isNull())
    {
        err("渲染结果为空图像");
        delete rd;
        return 1;
    }

    if (ascii)
    {
        for (int y = 0; y < img.height(); ++y)
        {
            QString line;
            for (int x = 0; x < img.width(); ++x)
                line += qGray(img.pixel(x, y)) < 128 ? '#' : '.';
            out(line);
        }
    }

    if (!ascii || !outFile.isEmpty())
    {
        if (outFile.isEmpty())
            outFile = rd->getBmFile(id).name + ".png";
        int s = qMax(1, scaleStr.toInt());
        QImage big = img.scaled(img.width() * s, img.height() * s, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        if (!big.save(outFile, "PNG"))
        {
            err("图片保存失败: " + outFile);
            delete rd;
            return 1;
        }
        out("已保存: " + QFileInfo(outFile).absoluteFilePath());
    }
    delete rd;
    return 0;
}

// ---------- export ----------

int exportCmd(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.isEmpty())
    {
        err("用法: bms-cli export <工程.bms> [-o 目录] [--json]");
        return 1;
    }
    QString outdir = takeOpt(args, "-o");
    bool asJson = hasOpt(args, "--json");

    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    QStringList generated;
    bool ok = ExportRunner::run(*rd, outdir, &generated);
    delete rd;

    if (asJson)
    {
        // 机器可读产物清单（CI可用sha256比对检测资产漂移）
        QJsonObject doc;
        doc.insert("ok", ok);
        QJsonArray files;
        foreach (const QString &f, generated)
        {
            QJsonObject fo;
            fo.insert("path", QFileInfo(f).absoluteFilePath());
            QFile fh(f);
            if (fh.open(QIODevice::ReadOnly))
            {
                fo.insert("bytes", (double)fh.size());
                fo.insert("sha256", QString::fromLatin1(QCryptographicHash::hash(fh.readAll(), QCryptographicHash::Sha256).toHex()));
            }
            files.append(fo);
        }
        doc.insert("files", files);
        out(QString::fromUtf8(QJsonDocument(doc).toJson(QJsonDocument::Indented)));
    }
    else
    {
        foreach (const QString &f, generated) out(f);
    }
    if (!ok)
    {
        err("导出未全部成功，请检查输出目录权限");
        return 1;
    }
    return 0;
}

// ---------- rename ----------

int rename(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 3)
    {
        err("用法: bms-cli rename <工程.bms> <路径> <新名称>");
        return 1;
    }
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    quint16 id = 0;
    if (!resolve(rd, args.at(1), &id)) { delete rd; return 1; }

    if (!rd->rename(id, args.at(2)))   // 同名/非法字符会被自动规范化
    {
        err("写入工程文件失败: " + args.at(0));
        delete rd;
        return 1;
    }
    out(QString("已重命名为: %1（组合图引用已自动更新）").arg(rd->getBmFile(id).name));
    delete rd;
    return 0;
}

// ---------- move ----------

int move(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 3)
    {
        err("用法: bms-cli move <工程.bms> <路径> <目标文件夹|/>");
        return 1;
    }
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    quint16 id = 0, destId = 0;
    if (!resolve(rd, args.at(1), &id)) { delete rd; return 1; }

    QString dest = normPath(args.at(2));
    if (!dest.isEmpty() && !resolve(rd, dest, &destId)) { delete rd; return 1; }

    if (!rd->move(id, destId))
    {
        err("移动失败：目标必须是同树的文件夹，且不能移动到自身子树下（图片组内只接受图片）；或写入工程文件失败");
        delete rd;
        return 1;
    }
    out(QString("已移动 %1 -> %2（组合图引用已自动更新）").arg(normPath(args.at(1)), dest.isEmpty() ? "/" : "/" + dest));
    delete rd;
    return 0;
}

// ---------- delete ----------

int del(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 2)
    {
        err("用法: bms-cli delete <工程.bms> <路径>");
        return 1;
    }
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    quint16 id = 0;
    if (!resolve(rd, args.at(1), &id)) { delete rd; return 1; }

    // 收集子树，报告将被悬空的组合图引用
    QSet<quint16> subtree;
    subtree << id;
    bool grew = true;
    while (grew)
    {
        grew = false;
        foreach (const BmFile &bf, rd->getDataMap())
        {
            if (subtree.contains(bf.pid) && !subtree.contains(bf.id))
            {
                subtree << bf.id;
                grew = true;
            }
        }
    }
    foreach (const BmFile &bf, rd->getDataMap())
    {
        if (bf.type != RawData::TypeComImgFile) continue;
        foreach (const ComImgItem &it, bf.comImg.items)
        {
            if (subtree.contains(it.id))
            {
                out(QString("警告: 组合图 %1 将丢失成员 %2").arg(bf.name).arg(rd->getBmFile(it.id).name));
            }
        }
    }

    if (!rd->remove(id))
    {
        err("写入工程文件失败: " + args.at(0));
        delete rd;
        return 1;
    }
    out(QString("已删除 /%1").arg(normPath(args.at(1))));
    delete rd;
    return 0;
}

// ---------- add ----------

int add(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 3)
    {
        err(QString("用法: bms-cli add <工程.bms> <父路径|/> <名称> [--kind img|folder|group|composite]\n")
            + "                 [--png 图片.png] [--size 宽x高] [--note 备注]");
        return 1;
    }
    QString kind = takeOpt(args, "--kind", "img");
    QString pngFile = takeOpt(args, "--png");
    QString sizeStr = takeOpt(args, "--size");
    QString note = takeOpt(args, "--note");

    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    QString parent = normPath(args.at(1));
    QString name = args.at(2);

    // 父节点：根用类别哨兵（-3图片树 -4组合图树），否则解析路径
    int parentId;
    if (parent.isEmpty())
    {
        parentId = (kind == "composite") ? -4 : -3;
    }
    else
    {
        quint16 pid = 0;
        if (!resolve(rd, parent, &pid)) { delete rd; return 1; }
        parentId = pid;
    }

    bool saved = false;
    if (kind == "img")
    {
        if (pngFile.isEmpty())
        {
            err("添加图片需要 --png 指定图片文件");
            delete rd;
            return 1;
        }
        QImage img(pngFile);
        if (img.isNull())
        {
            err("无法加载图片文件: " + pngFile);
            delete rd;
            return 1;
        }
        saved = rd->createBmp(parentId, name, img, note);
    }
    else if (kind == "folder")
    {
        saved = rd->createFolder(parentId, name, note);
    }
    else if (kind == "group")
    {
        saved = rd->createFolder(parentId, name, note) &&
                rd->imgFolderConvert(rd->getDataMap().lastKey());   // 新建的文件夹转为图片组
    }
    else if (kind == "composite")
    {
        QSize size = rd->getSize();     // 缺省跟随屏幕
        if (!sizeStr.isEmpty())
        {
            QStringList wh = sizeStr.split('x', QString::SkipEmptyParts);
            if (wh.size() == 2) size = QSize(wh.at(0).toInt(), wh.at(1).toInt());
        }
        saved = rd->createComImg(parentId, name, size, note);
    }
    else
    {
        err("未知的 --kind: " + kind + "（可选 img|folder|group|composite）");
        delete rd;
        return 1;
    }

    if (!saved)
    {
        err("写入工程文件失败: " + args.at(0));
        delete rd;
        return 1;
    }

    // createX内部会做名称规范化（同级去重/过滤'/'），回显实际结果
    quint16 newId = rd->getDataMap().lastKey();
    BmFile bf = rd->getBmFile(newId);
    QString actualPath = parent.isEmpty() ? bf.name : parent + "/" + bf.name;
    out(QString("已添加 [%1] /%2").arg(kindLabel(bf.type), actualPath));
    if (bf.name != name)
    {
        out(QString("注意: 名称已规范化为 %1（原名与同级冲突或含非法字符）").arg(bf.name));
    }
    delete rd;
    return 0;
}

// ---------- item-add / item-rm（组合图成员管理） ----------

int itemAdd(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 5)
    {
        err("用法: bms-cli item-add <工程.bms> <组合图路径> <图片路径> <x> <y>");
        return 1;
    }
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    quint16 compId = 0, imgId = 0;
    if (!resolve(rd, args.at(1), &compId)) { delete rd; return 1; }
    if (!resolve(rd, args.at(2), &imgId)) { delete rd; return 1; }

    if (rd->getBmFile(compId).type != RawData::TypeComImgFile)
    {
        err("目标不是组合图: /" + normPath(args.at(1)));
        delete rd;
        return 1;
    }
    if (rd->getBmFile(imgId).type != RawData::TypeImgFile)
    {
        err("成员必须是图片（不支持组合图嵌套）: /" + normPath(args.at(2)));
        delete rd;
        return 1;
    }

    ComImg ci = rd->getComImg(compId);
    ci.items.append(ComImgItem((qint16)args.at(3).toInt(), (qint16)args.at(4).toInt(), imgId));
    if (!rd->setComImg(compId, ci))
    {
        err("写入工程文件失败: " + args.at(0));
        delete rd;
        return 1;
    }

    out(QString("已添加成员 %1 @(%2,%3)，组合图 %4 现有 %5 个成员")
        .arg(normPath(args.at(2)), args.at(3), args.at(4), normPath(args.at(1)))
        .arg(ci.items.size()));
    delete rd;
    return 0;
}

int itemRm(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 3)
    {
        err("用法: bms-cli item-rm <工程.bms> <组合图路径> <序号>");
        return 1;
    }
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    quint16 compId = 0;
    if (!resolve(rd, args.at(1), &compId)) { delete rd; return 1; }
    if (rd->getBmFile(compId).type != RawData::TypeComImgFile)
    {
        err("目标不是组合图: /" + normPath(args.at(1)));
        delete rd;
        return 1;
    }

    ComImg ci = rd->getComImg(compId);
    int idx = args.at(2).toInt();
    if (idx < 0 || idx >= ci.items.size())
    {
        err(QString("序号越界: %1（组合图现有 %2 个成员，序号从0起）").arg(idx).arg(ci.items.size()));
        delete rd;
        return 1;
    }

    QHash<quint16, QString> idPath = buildIdPath(rd->getDataMap());
    QString removedPath = idPath.value(ci.items.at(idx).id);
    ci.items.removeAt(idx);
    if (!rd->setComImg(compId, ci))
    {
        err("写入工程文件失败: " + args.at(0));
        delete rd;
        return 1;
    }
    out(QString("已移除成员 #%1（%2），剩余 %3 个").arg(idx).arg(removedPath).arg(ci.items.size()));
    delete rd;
    return 0;
}

} // namespace BmsCli
