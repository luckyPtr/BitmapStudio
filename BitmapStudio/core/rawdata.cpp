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
            if(dataMap[id].pid != 0)
            {
                self(self, dataMap[id].pid);
            }
            if(dataMap[dataMap[id].pid].type == RawData::TypeImgGrpFolder)
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

// 名称规范化：去掉路径分隔符，同树同级冲突时自动加后缀
QString RawData::sanitizeName(const QString &name, quint16 pid, int type)
{
    QString n = name;
    n.replace('/', '_');
    n.replace('\\', '_');
    if (n.isEmpty()) n = "untitled";

    QString base = n;
    int suffix = 1;
    while (true)
    {
        bool clash = false;
        for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
        {
            // 仅同一棵树内的同名才算冲突（images/composites两树的根级节点互不影响）
            if (it.value().pid == pid && it.value().name == n &&
                RawData::isClassImgType(it.value().type) == RawData::isClassImgType(type))
            {
                clash = true;
                break;
            }
        }
        if (!clash) break;
        n = base + QString("_%1").arg(suffix++);
    }
    return n;
}

void RawData::load()
{
    dataMap.clear();
    pathIndex.clear();
    nextId = 1;
    valid = false;

    QFile file(project);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "工程文件打开失败:" << project;
        return;
    }
    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    file.close();
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "工程文件JSON解析失败:" << project << parseErr.errorString();
        return;
    }

    QJsonObject root = doc.object();
    if (root.value("format").toString() != "bms")
    {
        qWarning() << "不是有效的Bitmap Studio工程文件:" << project;
        return;
    }
    int version = root.value("version").toInt(1);
    if (version > 1)
    {
        qWarning() << "工程文件格式版本更新，按尽力而为方式解析:" << version;
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
    valid = true;
}

// 解析一层节点数组；同级先容器后叶子，组内保持数组顺序
void RawData::parseLevel(const QJsonArray &arr, quint16 pid, bool imgTree, const QString &parentPath)
{
    auto parsePngLeaf = [&](const QJsonObject &node, quint16 leafPid, const QString &leafParentPath)
    {
        QString name = sanitizeName(node.value("name").toString(), leafPid, TypeImgFile);
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
            qWarning() << "图片数据解码失败:" << path;
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
                qWarning() << "忽略非对象节点";
                continue;
            }
            QJsonObject node = v.toObject();
            bool container = node.contains("children") || node.contains("frames");
            if ((pass == 0) != container) continue;

            if (node.contains("png"))
            {
                parsePngLeaf(node, pid, parentPath);
                continue;
            }

            if (node.contains("items"))
            {
                // 组合图叶子
                QString name = sanitizeName(node.value("name").toString(), pid, TypeComImgFile);
                if (name.isEmpty()) name = "untitled";
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
                        qWarning() << "组合图" << name << "的悬空引用已忽略:" << ref;
                        continue;
                    }
                    QJsonArray pos = io.value("pos").toArray();
                    ComImgItem item((qint16)(pos.size() >= 1 ? pos.at(0).toInt() : 0),
                                    (qint16)(pos.size() >= 2 ? pos.at(1).toInt() : 0),
                                    pathIndex.value(ref));
                    bi.comImg.items.append(item);
                }
                dataMap.insert(id, bi);
                continue;
            }

            int nodeType = node.contains("frames") ? TypeImgGrpFolder
                                                   : (imgTree ? TypeImgFolder : TypeComImgFolder);
            QString name = sanitizeName(node.value("name").toString(), pid, nodeType);
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
                for (const QJsonValue &fv : node.value("frames").toArray())
                {
                    if (!fv.isObject() || !fv.toObject().contains("png"))
                    {
                        qWarning() << "图片组内忽略非图片节点:" << name;
                        continue;
                    }
                    parsePngLeaf(fv.toObject(), id, path);
                }
            }
            else
            {
                qWarning() << "无法识别的节点形状，已忽略:" << name;
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

void RawData::save()
{
    if (!valid)
    {
        // 无效工程（解析失败/旧格式）一律拒绝写回，防止覆盖原文件
        qWarning() << "工程文件无效，拒绝写回:" << project;
        return;
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

    QSaveFile file(project);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "工程文件写入失败:" << project;
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit())
    {
        qWarning() << "工程文件写入提交失败:" << project;
    }
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
    if(!expand.isEmpty())
    expand.clear();
    qDebug() << "~RawData:" << project;
}

void RawData::createFolder(int id, QString name, QString brief)
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
        save();
    }
}

void RawData::createBmp(int id, QString name, const QImage &img, const QString brief)
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
        save();
    }
}

void RawData::createBmp(int id, QString name, QSize size, const QString brief)
{
    QImage image(size, QImage::Format_RGBA8888);
    image.fill(Qt::white);
    createBmp(id, name, image, brief);
}

void RawData::createComImg(int id, QString name, QSize size, const QString brief)
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
        save();
    }
}

void RawData::rename(int id, QString name)
{
    if(dataMap.contains(id))
    {
        dataMap[id].name = sanitizeName(name, dataMap[id].pid, dataMap[id].type);
        updateFullName();
        save();
    }
}

QString RawData::getName(int id)
{
    return dataMap[id].name;
}

void RawData::remove(int id)
{
    if(!dataMap.contains(id)) return;

    // 先收集子节点再递归删除，避免删除过程中迭代失效
    QVector<quint16> children;
    for (auto it = dataMap.constBegin(); it != dataMap.constEnd(); ++it)
    {
        if(it.value().pid == id)
        {
            children << it.key();
        }
    }
    dataMap.remove(id);
    foreach (quint16 child, children)
    {
        remove(child);
    }
    updateFullName();
    save();
}

void RawData::imgFolderConvert(int id)
{
    if(dataMap.contains(id))
    {
        int type = dataMap[id].type;

        if(type == RawData::TypeImgFolder || type == RawData::TypeImgGrpFolder)
        {
            dataMap[id].type = type == RawData::TypeImgFolder ? RawData::TypeImgGrpFolder : RawData::TypeImgFolder;
            updateFullName();
            save();
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
        QImage image(dataMap[id].comImg.size, QImage::Format_RGB888);
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

void RawData::setImage(int id, QImage image)
{
    if(dataMap.contains(id))
    {
        dataMap[id].image = image;
        dataMap[id].png = encodePngBytes(image);
        save();
    }
}

QString RawData::getBrief(int id)
{
    return dataMap[id].brief;
}

void RawData::setBrief(int id, QString brief)
{
    if(dataMap.contains(id))
    {
        dataMap[id].brief = brief;
        save();
    }
}

ComImg RawData::getComImg(int id)
{
    return dataMap[id].comImg;
}

void RawData::setComImg(int id, ComImg ci)
{
    if(dataMap.contains(id))
    {
        // 尺寸发生变化说明是显式设定，不再跟随屏幕
        if(ci.size != dataMap[id].comImg.size)
        {
            dataMap[id].followScreen = false;
        }
        dataMap[id].comImg = ci;
        save();
    }
}



void RawData::saveSettings(Settings settings)
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
    save();
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
