#ifndef DIALOGNEWIMGFILE_H
#define DIALOGNEWIMGFILE_H

#include <QDialog>
#include <QVector>

namespace Ui {
class DialogNewImgFile;
}

class DialogNewImgFile : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewImgFile(QWidget *parent = nullptr, QSize defaultSize = QSize(16, 16));
    ~DialogNewImgFile();
    quint16 width();
    quint16 height();
    QSize size();
    QString imgFileName();
    QString brief();
    void setExistFile(QVector<QString> files);
    void setSize(QSize size);

private slots:
    void on_lineEditImgFileName_textChanged(const QString &arg1);

private:
    Ui::DialogNewImgFile *ui;
    QVector<QString> existFile; // 已经存在的文件，用于文件重名检查
    bool checkImgFileName();    // 检查名称输入是否合法，是否重复/符合C语言变量命名规则
};

#endif // DIALOGNEWIMGFILE_H
