#ifndef EXPORTRUNNER_H
#define EXPORTRUNNER_H

#include <QString>
#include <QStringList>
#include "rawdata.h"

// 导出调度（GUI的Run动作与CLI的export命令共用）
namespace ExportRunner
{
    // outdirOverride非空时取代工程设置里的输出目录（相对/绝对均可）
    // generated（可选）收集实际产出的文件绝对路径
    // 返回是否全部生成成功
    bool run(const RawData &rd, const QString &outdirOverride = QString(), QStringList *generated = nullptr);
}

#endif // EXPORTRUNNER_H
