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
    explicit DialogNewImgFile(QWidget *parent = nullptr);
    ~DialogNewImgFile();
    quint16 width();
    quint16 height();
    QString imgFileName();
    void setExistFile(QVector<QString> files);
private slots:
    void on_lineEditImgFileName_textChanged(const QString &arg1);
    void on_inputUpdate();

private:
    Ui::DialogNewImgFile *ui;
    QVector<QString> existFile; // 已经存在的文件，用于文件重名检查
    bool checkImgFileName();    // 检查名称输入是否合法，是否重复/符合C语言变量命名规则
};

#endif // DIALOGNEWIMGFILE_H
