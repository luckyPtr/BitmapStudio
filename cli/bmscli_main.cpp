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
        "Usage: bms-cli <command> [arguments]\n"
        "\n"
        "  init    <new_project.bms> [--screen WxH] [--scan ZH|ZL|HL|LH] [--bit MSB|LSB]\n"
        "          [--format C|bin] [--outdir dir] [--note text]    Create a new project\n"
        "  check   <project.bms> [--json]                  Validate project (dangling refs/bad data/auto-renames)\n"
        "                                                  Exit codes: 0 clean, 1 issues, 2 invalid project\n"
        "  info    <project.bms> [--json]                  Show tree structure and export settings\n"
        "  render  <project.bms> <path> [-o output.png] [-s scale] [--ascii]\n"
        "                                                  Render image/composite to PNG (nearest-neighbor upscale)\n"
        "  export  <project.bms> [-o dir] [--json]         Export bitmaps per project settings (--json lists artifacts with sha256)\n"
        "  rename  <project.bms> <path> <new name>         Rename node (composite refs cascade automatically)\n"
        "  move    <project.bms> <path> <target folder|/>  Move node (refs cascade automatically)\n"
        "  delete  <project.bms> <path>                    Delete node (reports affected composites first)\n"
        "  add     <project.bms> <parent|/> <name> [--kind img|folder|group|composite]\n"
        "          [--png image.png] [--size WxH] [--note text]    Add a node\n"
        "  item-add <project.bms> <composite path> <image path> <x> <y>    Add a composite member\n"
        "  item-rm  <project.bms> <composite path> <index>                 Remove a composite member\n"
        "\n"
        "Paths are '/'-separated, '/' means root. Low-risk edits (reorder/notes/export settings) are best made directly in the JSON file, then validated with check.\n",
        stdout);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    SetConsoleOutputCP(65001);      // 控制台UTF-8输出（工程内节点名/备注可能含非ASCII字符）
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

    fprintf(stderr, "Unknown command: %s\n\n", qPrintable(cmd));
    printUsage();
    return 1;
}
