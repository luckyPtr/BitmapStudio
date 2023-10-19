#ifndef CUSTOMTABWIDGET_H
#define CUSTOMTABWIDGET_H

#include <QTabWidget>
#include <QObject>
#include <QWidget>
#include <QMenu>
#include "treeitem.h"
#include <QPoint>

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
        }
    };

    QVector<TabWidgetItem *> tabWidgetItems;
    QMenu *menu;
    void initMenu();
protected:
    void contextMenuEvent(QContextMenuEvent *event);
public:
    CustomTabWidget(QWidget *parent = nullptr);

    int addImgTab(TreeItem *treeItem);
    int addComImgTab(TreeItem *treeItem);
//    void removeTab(int index);
    void removeAllTabs();
    void removeOtherTabs(int index);
signals:
    void updateSize(QSize);
};

#endif // CUSTOMTABWIDGET_H
