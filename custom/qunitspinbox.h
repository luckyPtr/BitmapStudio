#ifndef QUNITSPINBOX_H
#define QUNITSPINBOX_H

#include <QWidget>
#include <QSpinBox>
#include <QLabel>

// https://blog.csdn.net/qq_40861091/article/details/118389118?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-118389118-blog-119606313.235%5Ev36%5Epc_relevant_default_base3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-118389118-blog-119606313.235%5Ev36%5Epc_relevant_default_base3&utm_relevant_index=2

class QUnitSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit QUnitSpinBox(QWidget *parent = nullptr);
    ~QUnitSpinBox();

    void setUnitText(const QString &unit);
private:
    QString unitText;   // 单位文本
    QLabel *label;      // 显示单位文本的label
signals:

};

#endif // QUNITSPINBOX_H
