#ifndef CUSTOMTABWIDGET_H
#define CUSTOMTABWIDGET_H

#include <QTabWidget>
#include <QObject>
#include <QWidget>
#include "treeitem.h"

class CustomTabWidget : public QTabWidget
{
    Q_OBJECT
private:
    enum TabWidgetType
    {
        TypeImg,
        TypeComImg,
    };

    struct TabWidgetItem
    {
        TreeItem *treeItem;
        TabWidgetType type; // tabWidget窗口类型
        QWidget *window = nullptr;
        QString title;
        int id;
        ~TabWidgetItem()
        {
            if(window != nullptr)
                delete window;
            qDebug() << "deleted";
        }
    };

    QVector<TabWidgetItem *> tabWidgetItems;
public:
    CustomTabWidget(QWidget *parent = nullptr) : QTabWidget(parent)
    {

    }

    int addImgTab(TreeItem *treeItem);
    void removeTab(int index);
};

#endif // CUSTOMTABWIDGET_H
