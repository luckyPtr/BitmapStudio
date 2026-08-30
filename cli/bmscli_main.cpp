#include <QCoreApplication>
#include <cstdio>
#include "commands.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static void printUsage()
{
    fputs(
        "Bitmap Studio CLI (bms-cli)\n"
        "用法: bms-cli <命令> [参数]\n"
        "\n"
        "  init    <新工程.bms> [--screen 宽x高] [--scan ZH|ZL|HL|LH] [--bit MSB|LSB]\n"
        "          [--format C|bin] [--outdir 目录] [--note 备注]    创建新工程\n"
        "  check   <工程.bms> [--json]                   校验工程（悬空引用/坏数据/自动改名）\n"
        "                                                  退出码: 0干净 1有问题 2工程无效\n"
        "  info    <工程.bms> [--json]                   查看树结构与导出设置\n"
        "  render  <工程.bms> <路径> [-o 输出.png] [-s 倍数] [--ascii]\n"
        "                                                  渲染图片/组合图为PNG（近邻放大）\n"
        "  export  <工程.bms> [-o 目录] [--json]         按工程设置导出字模（--json输出产物清单含sha256）\n"
        "  rename  <工程.bms> <路径> <新名称>             重命名（组合图引用自动级联）\n"
        "  move    <工程.bms> <路径> <目标文件夹|/>       移动节点（引用自动级联）\n"
        "  delete  <工程.bms> <路径>                     删除节点（先报告受影响的组合图）\n"
        "  add     <工程.bms> <父路径|/> <名称> [--kind img|folder|group|composite]\n"
        "          [--png 图片.png] [--size 宽x高] [--note 备注]    添加节点\n"
        "  item-add <工程.bms> <组合图路径> <图片路径> <x> <y>       组合图添加成员\n"
        "  item-rm  <工程.bms> <组合图路径> <序号>                    移除组合图成员\n"
        "\n"
        "路径以 / 分隔，/ 表示根。低风险编辑（重排/note/导出设置）建议直接修改JSON文件后用 check 校验。\n",
        stdout);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    SetConsoleOutputCP(65001);      // 控制台UTF-8输出，避免中文乱码
#endif
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments();
    if (args.size() < 2)
    {
        printUsage();
        return 1;
    }

    const QString cmd = args.at(1);
    const QStringList rest = args.mid(2);

    if (cmd == "help" || cmd == "--help" || cmd == "-h") { printUsage(); return 0; }
    if (cmd == "version" || cmd == "--version" || cmd == "-v")
    {
        printf("bms-cli %s\n", APP_VERSION);
        fflush(stdout);
        return 0;
    }
    if (cmd == "init")     return BmsCli::init(rest);
    if (cmd == "check")   return BmsCli::check(rest);
    if (cmd == "info")    return BmsCli::info(rest);
    if (cmd == "render")  return BmsCli::render(rest);
    if (cmd == "export")  return BmsCli::exportCmd(rest);
    if (cmd == "rename")  return BmsCli::rename(rest);
    if (cmd == "move")    return BmsCli::move(rest);
    if (cmd == "delete")  return BmsCli::del(rest);
    if (cmd == "add")     return BmsCli::add(rest);
    if (cmd == "item-add") return BmsCli::itemAdd(rest);
    if (cmd == "item-rm")  return BmsCli::itemRm(rest);

    fprintf(stderr, "未知命令: %s\n\n", qPrintable(cmd));
    printUsage();
    return 1;
}
