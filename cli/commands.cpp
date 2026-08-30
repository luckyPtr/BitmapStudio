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
        err(QString("Failed to load project: %1 (file corrupted or legacy SQLite format; convert it with tools/convert_legacy.py first)").arg(file));
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
        err(QString("Path not found: /%1").arg(p));
        return false;
    }
    if (r == RawData::PathAmbiguous)
    {
        err(QString("Path /%1 exists in both the image tree and the composite tree; rename one of them first").arg(p));
        return false;
    }
    return true;
}

static QString kindLabel(int type)
{
    switch (type)
    {
    case RawData::TypeImgFile:      return "image";
    case RawData::TypeComImgFile:   return "composite";
    case RawData::TypeImgFolder:    return "folder";
    case RawData::TypeImgGrpFolder: return "image group";
    case RawData::TypeComImgFolder: return "folder";
    default:                        return "unknown";
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
        err("Usage: bms-cli init <new_project.bms> [--screen WxH] [--scan ZH|ZL|HL|LH] [--bit MSB|LSB]\n"
            "                       [--format C|bin] [--outdir dir] [--note text]");
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
        err("Invalid --scan: " + scan + " (expected ZH|ZL|HL|LH)");
        return 1;
    }
    if (bit != "MSB" && bit != "LSB")
    {
        err("Invalid --bit: " + bit + " (expected MSB|LSB)");
        return 1;
    }
    if (format != "C" && format != "bin")
    {
        err("Invalid --format: " + format + " (expected C|bin)");
        return 1;
    }
    QStringList wh = screenStr.split('x');
    if (wh.size() != 2 || wh.at(0).toInt() <= 0 || wh.at(1).toInt() <= 0)
    {
        err("Invalid --screen: " + screenStr + " (expected e.g. 128x64)");
        return 1;
    }
    QSize screen(wh.at(0).toInt(), wh.at(1).toInt());

    QString file = args.at(0);
    if (QFileInfo(file).isFile())
    {
        err("Project file already exists: " + file);
        return 1;
    }

    RawData rd(file);       // 文件不存在 -> 写入默认JSON
    if (!rd.isValid())
    {
        err("Failed to initialize project: " + file);
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
        err("Failed to write project file: " + file + " (directory read-only or disk error)");
        return 1;
    }

    out(QString("Project created: %1  screen %2x%3  scan %4 %5  output %6")
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
        err("Usage: bms-cli check <project.bms> [--json]");
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
        foreach (const QString &w, warnings) err("Error: " + w);
        return 2;
    }
    if (warnings.isEmpty())
    {
        out("OK");
        return 0;
    }
    foreach (const QString &w, warnings)
    {
        out("Warning: " + w);
    }
    out(QString("%1 warning(s) (dangling refs and similar issues were auto-handled on load per project rules; they will be persisted on save)").arg(warnings.size()));
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
        err("Usage: bms-cli info <project.bms> [--json]");
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
        out(QString("Screen: %1x%2    Scan: %3 %4    Output: %5")
            .arg(st.size.width()).arg(st.size.height()).arg(scan, bitOrder, st.format));
        if (!st.brief.isEmpty()) out("Note: " + st.brief);

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
                    line += QString("  %1x%2 (%3 member(s))").arg(bf.comImg.size.width()).arg(bf.comImg.size.height()).arg(bf.comImg.items.size());
                if (!bf.brief.isEmpty()) line += "  \"" + bf.brief + "\"";
                out(line);
                if (bf.type == RawData::TypeImgFolder || bf.type == RawData::TypeComImgFolder || bf.type == RawData::TypeImgGrpFolder)
                    self(self, bf.id, depth + 1, imgTree);
            }
        };
        out("Images:");
        printLevel(printLevel, 0, 1, true);
        out("Composites:");
        printLevel(printLevel, 0, 1, false);
    }
    delete rd;
    return 0;
}

// ---------- render ----------

// render/compose 共用的输出：--ascii 打印文本视图；非ascii模式（或指定了-o）时近邻放大保存PNG，
// 保存名缺省用 defaultName（ascii且未指定-o时只打印不保存）
static int writeRenderOutput(const QImage &img, QString outFile, const QString &defaultName, int scale, bool ascii)
{
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
            outFile = defaultName;
        int s = qMax(1, scale);
        QImage big = img.scaled(img.width() * s, img.height() * s, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        if (!big.save(outFile, "PNG"))
        {
            err("Failed to save image: " + outFile);
            return 1;
        }
        out("Saved: " + QFileInfo(outFile).absoluteFilePath());
    }
    return 0;
}

int render(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 2)
    {
        err("Usage: bms-cli render <project.bms> <path> [-o output.png] [-s scale] [--ascii]");
        return 1;
    }
    QString outFile = takeOpt(args, "-o");
    QString scaleStr = takeOpt(args, "-s", "1");
    bool ascii = hasOpt(args, "--ascii");
    bool invert = hasOpt(args, "--invert");

    int code = 0;
    QString file = args.at(0);
    RawData *rd = openProject(file, &code);
    if (!rd) return code;

    quint16 id = 0;
    if (!resolve(rd, args.at(1), &id)) { delete rd; return 1; }

    QImage img = rd->getImage(id);    // 组合图自动按成员合成（绘制序/跟随屏幕/裁剪语义一致）
    if (img.isNull())
    {
        err("Rendered image is empty");
        delete rd;
        return 1;
    }

    if (invert)
    {
        img.invertPixels(QImage::InvertRgb);    // 反色预览，与invertrect同语义（通道取反）
    }

    int rc = writeRenderOutput(img, outFile, rd->getBmFile(id).name + ".png", scaleStr.toInt(), ascii);
    delete rd;
    return rc;
}

// ---------- compose（渲染工程中不存在的组合图：演示/测试用） ----------

// 读取 [x, y] 形式的点；格式错误时输出错误并返回false
static bool readPoint2(const QJsonValue &v, qint16 *x, qint16 *y, const QString &field)
{
    QJsonArray a = v.toArray();
    if (a.size() != 2)
    {
        err("Invalid \"" + field + "\": expected [x, y]");
        return false;
    }
    *x = (qint16)a.at(0).toInt();
    *y = (qint16)a.at(1).toInt();
    return true;
}

// 读取可选前景色 "color":"black"(默认)|"white"
static bool readColor(const QJsonObject &io, bool *white)
{
    QString c = io.value("color").toString("black");
    if (c == "black") { *white = false; return true; }
    if (c == "white") { *white = true; return true; }
    err("Invalid \"color\": " + c + " (expected black|white)");
    return false;
}

// 解析单个绘制项（type: image默认/line/fillrect/invertrect/points）；失败时已输出错误并返回false
static bool parseDrawItem(const QJsonObject &io, RawData *rd, ComDrawItem *di)
{
    QString type = io.value("type").toString("image");

    if (type == "image")
    {
        QString ref = io.value("image").toString();
        if (ref.isEmpty())
        {
            err("Item is missing \"image\" path");
            return false;
        }
        quint16 imgId = 0;
        RawData::PathResolve r = rd->resolvePath(normPath(ref), &imgId);
        if (r == RawData::PathNotFound)
        {
            err("Member image not found: /" + normPath(ref));
            return false;
        }
        if (r == RawData::PathAmbiguous)
        {
            err("Path /" + normPath(ref) + " exists in both the image tree and the composite tree; rename one of them first");
            return false;
        }
        if (rd->getBmFile(imgId).type != RawData::TypeImgFile)
        {
            err("Member must be an image (nested composites are not supported): /" + normPath(ref));
            return false;
        }
        *di = ComDrawItem::imageItem(0, 0, imgId);
        return !io.contains("pos") || readPoint2(io.value("pos"), &di->x, &di->y, "pos");
    }

    if (type == "line")
    {
        di->kind = ComDrawItem::Line;
        if (!io.contains("pos")) { err("Item type \"line\" requires \"pos\" [x1, y1]"); return false; }
        if (!readPoint2(io.value("pos"), &di->x, &di->y, "pos")) return false;
        if (!io.contains("end")) { err("Item type \"line\" requires \"end\" [x2, y2]"); return false; }
        if (!readPoint2(io.value("end"), &di->x2, &di->y2, "end")) return false;
        return readColor(io, &di->white);
    }

    if (type == "fillrect" || type == "invertrect")
    {
        di->kind = (type == "fillrect") ? ComDrawItem::FillRect : ComDrawItem::InvertRect;
        if (!io.contains("pos")) { err("Item type \"" + type + "\" requires \"pos\" [x, y]"); return false; }
        if (!readPoint2(io.value("pos"), &di->x, &di->y, "pos")) return false;
        if (!io.contains("size")) { err("Item type \"" + type + "\" requires \"size\" [width, height]"); return false; }
        QJsonArray s = io.value("size").toArray();
        if (s.size() != 2 || s.at(0).toInt() < 0 || s.at(1).toInt() < 0)
        {
            err("Invalid \"size\": expected [width, height] (>= 0)");
            return false;
        }
        di->w = (qint16)s.at(0).toInt();
        di->h = (qint16)s.at(1).toInt();
        if (type == "fillrect") return readColor(io, &di->white);
        return true;
    }

    if (type == "points")
    {
        di->kind = ComDrawItem::Points;
        if (!io.contains("points") || !io.value("points").isArray())
        {
            err("Item type \"points\" requires \"points\" array, e.g. [[10,10],[20,20]]");
            return false;
        }
        QJsonArray pts = io.value("points").toArray();
        if (pts.isEmpty())
        {
            err("\"points\" is empty");
            return false;
        }
        foreach (const QJsonValue &pv, pts)
        {
            qint16 px, py;
            if (!readPoint2(pv, &px, &py, "points entry")) return false;
            di->pts.append(QPoint(px, py));
        }
        return readColor(io, &di->white);
    }

    err("Unknown item type: " + type + " (expected image, line, fillrect, invertrect, points)");
    return false;
}

// 解析组合图描述JSON，与.bms里组合图节点同构，items支持图片与绘图原语混排：
// {"size":[W,H]可选（省略=跟随屏幕）, "items":[{"image":"路径","pos":[x,y]} | {"type":"line",...} | ...]}
// 失败时已输出错误并返回false。
static bool parseComposeJson(const QString &text, RawData *rd, QSize *size, QVector<ComDrawItem> *items)
{
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &parseErr);
    if (!doc.isObject())
    {
        err("Invalid composite JSON: " + QString(parseErr.error != QJsonParseError::NoError ? parseErr.errorString().toUtf8() : "top level must be an object"));
        return false;
    }
    QJsonObject o = doc.object();

    if (o.contains("size"))
    {
        QJsonArray s = o.value("size").toArray();
        if (s.size() != 2 || s.at(0).toInt() <= 0 || s.at(1).toInt() <= 0)
        {
            err("Invalid \"size\": expected [width, height]");
            return false;
        }
        *size = QSize(s.at(0).toInt(), s.at(1).toInt());
    }
    else
    {
        *size = rd->getSettings().size;     // 省略size = 跟随屏幕（与工程语义一致）
    }

    if (!o.contains("items") || !o.value("items").isArray())
    {
        err("Missing \"items\" array");
        return false;
    }
    foreach (const QJsonValue &v, o.value("items").toArray())
    {
        ComDrawItem di;
        if (!parseDrawItem(v.toObject(), rd, &di)) return false;
        items->append(di);
    }
    if (items->isEmpty())
    {
        err("\"items\" is empty");
        return false;
    }
    return true;
}

int compose(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 2)
    {
        err("Usage: bms-cli compose <project.bms> <json-file|json> [-o output.png] [-s scale] [--ascii]\n"
            "       json example: {\"size\":[128,64],\"items\":[{\"image\":\"icons/logo\",\"pos\":[0,0]}]}  (\"size\" omitted = follow screen)\n"
            "       item types: image (default) / line (pos+end) / fillrect, invertrect (pos+size) / points (points array); optional \"color\":\"black\"|\"white\"");
        return 1;
    }
    QString outFile = takeOpt(args, "-o");
    QString scaleStr = takeOpt(args, "-s", "1");
    bool ascii = hasOpt(args, "--ascii");

    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    // 输入判别：存在的文件路径读文件内容，否则视为JSON字符串（以'{'开头，无歧义）
    QString text;
    if (QFileInfo(args.at(1)).isFile())
    {
        QFile fh(args.at(1));
        if (!fh.open(QIODevice::ReadOnly))
        {
            err("Failed to read JSON file: " + args.at(1));
            delete rd;
            return 1;
        }
        text = QString::fromUtf8(fh.readAll());
    }
    else if (args.at(1).trimmed().startsWith('{'))
    {
        text = args.at(1);
    }
    else
    {
        err("Input is neither an existing file nor a JSON string: " + args.at(1));
        delete rd;
        return 1;
    }

    QSize size;
    QVector<ComDrawItem> drawItems;
    if (!parseComposeJson(text, rd, &size, &drawItems))
    {
        delete rd;
        return 1;
    }

    QImage img = rd->renderCompose(size, drawItems);        // 合成语义与GUI导出路径一致；此命令绝不修改工程文件
    delete rd;

    return writeRenderOutput(img, outFile, "compose.png", scaleStr.toInt(), ascii);
}

// ---------- export ----------

int exportCmd(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.isEmpty())
    {
        err("Usage: bms-cli export <project.bms> [-o dir] [--json]");
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
        err("Export did not fully succeed; check output directory permissions");
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
        err("Usage: bms-cli rename <project.bms> <path> <new name>");
        return 1;
    }
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    quint16 id = 0;
    if (!resolve(rd, args.at(1), &id)) { delete rd; return 1; }

    if (!rd->rename(id, args.at(2)))   // 同名/非法字符会被自动规范化
    {
        err("Failed to write project file: " + args.at(0));
        delete rd;
        return 1;
    }
    out(QString("Renamed to: %1 (composite refs updated automatically)").arg(rd->getBmFile(id).name));
    delete rd;
    return 0;
}

// ---------- move ----------

int move(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 3)
    {
        err("Usage: bms-cli move <project.bms> <path> <target folder|/>");
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
        err("Move failed: target must be a folder in the same tree, the node must not be moved into its own subtree (image groups only accept images); or writing the project file failed");
        delete rd;
        return 1;
    }
    out(QString("Moved %1 -> %2 (composite refs updated automatically)").arg(normPath(args.at(1)), dest.isEmpty() ? "/" : "/" + dest));
    delete rd;
    return 0;
}

// ---------- delete ----------

int del(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 2)
    {
        err("Usage: bms-cli delete <project.bms> <path>");
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
                out(QString("Warning: composite %1 will lose member %2").arg(bf.name).arg(rd->getBmFile(it.id).name));
            }
        }
    }

    if (!rd->remove(id))
    {
        err("Failed to write project file: " + args.at(0));
        delete rd;
        return 1;
    }
    out(QString("Deleted /%1").arg(normPath(args.at(1))));
    delete rd;
    return 0;
}

// ---------- add ----------

int add(const QStringList &rawArgs)
{
    QStringList args = rawArgs;
    if (args.size() < 3)
    {
        err(QString("Usage: bms-cli add <project.bms> <parent|/> <name> [--kind img|folder|group|composite]\n")
            + "                 [--png image.png] [--size WxH] [--note text]");
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
            err("Adding an image requires --png <image file>");
            delete rd;
            return 1;
        }
        QImage img(pngFile);
        if (img.isNull())
        {
            err("Failed to load image file: " + pngFile);
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
        err("Unknown --kind: " + kind + " (expected img|folder|group|composite)");
        delete rd;
        return 1;
    }

    if (!saved)
    {
        err("Failed to write project file: " + args.at(0));
        delete rd;
        return 1;
    }

    // createX内部会做名称规范化（同级去重/过滤'/'），回显实际结果
    quint16 newId = rd->getDataMap().lastKey();
    BmFile bf = rd->getBmFile(newId);
    QString actualPath = parent.isEmpty() ? bf.name : parent + "/" + bf.name;
    out(QString("Added [%1] /%2").arg(kindLabel(bf.type), actualPath));
    if (bf.name != name)
    {
        out(QString("Note: name normalized to %1 (original clashed with a sibling or contained invalid characters)").arg(bf.name));
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
        err("Usage: bms-cli item-add <project.bms> <composite path> <image path> <x> <y>");
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
        err("Target is not a composite: /" + normPath(args.at(1)));
        delete rd;
        return 1;
    }
    if (rd->getBmFile(imgId).type != RawData::TypeImgFile)
    {
        err("Member must be an image (nested composites are not supported): /" + normPath(args.at(2)));
        delete rd;
        return 1;
    }

    ComImg ci = rd->getComImg(compId);
    ci.items.append(ComImgItem((qint16)args.at(3).toInt(), (qint16)args.at(4).toInt(), imgId));
    if (!rd->setComImg(compId, ci))
    {
        err("Failed to write project file: " + args.at(0));
        delete rd;
        return 1;
    }

    out(QString("Added member %1 @(%2,%3); composite %4 now has %5 member(s)")
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
        err("Usage: bms-cli item-rm <project.bms> <composite path> <index>");
        return 1;
    }
    int code = 0;
    RawData *rd = openProject(args.at(0), &code);
    if (!rd) return code;

    quint16 compId = 0;
    if (!resolve(rd, args.at(1), &compId)) { delete rd; return 1; }
    if (rd->getBmFile(compId).type != RawData::TypeComImgFile)
    {
        err("Target is not a composite: /" + normPath(args.at(1)));
        delete rd;
        return 1;
    }

    ComImg ci = rd->getComImg(compId);
    int idx = args.at(2).toInt();
    if (idx < 0 || idx >= ci.items.size())
    {
        err(QString("Index out of range: %1 (composite has %2 members, 0-based)").arg(idx).arg(ci.items.size()));
        delete rd;
        return 1;
    }

    QHash<quint16, QString> idPath = buildIdPath(rd->getDataMap());
    QString removedPath = idPath.value(ci.items.at(idx).id);
    ci.items.removeAt(idx);
    if (!rd->setComImg(compId, ci))
    {
        err("Failed to write project file: " + args.at(0));
        delete rd;
        return 1;
    }
    out(QString("Removed member #%1 (%2); %3 remaining").arg(idx).arg(removedPath).arg(ci.items.size()));
    delete rd;
    return 0;
}

} // namespace BmsCli
