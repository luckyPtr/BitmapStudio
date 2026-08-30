#ifndef BMSCLI_COMMANDS_H
#define BMSCLI_COMMANDS_H

#include <QStringList>

// bms CLI 子命令实现。所有命令的args已去掉命令名本身，返回进程退出码：
// 0成功 / 1失败(或check发现问题) / 2工程文件无效
namespace BmsCli
{
    int init(const QStringList &args);
    int check(const QStringList &args);
    int info(const QStringList &args);
    int render(const QStringList &args);
    int compose(const QStringList &args);   // 渲染工程中不存在的组合图（JSON文件或字符串描述），不修改工程文件
    int exportCmd(const QStringList &args);
    int rename(const QStringList &args);
    int move(const QStringList &args);
    int del(const QStringList &args);
    int add(const QStringList &args);
    int itemAdd(const QStringList &args);
    int itemRm(const QStringList &args);
}

#endif // BMSCLI_COMMANDS_H
