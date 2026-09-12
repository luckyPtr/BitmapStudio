#ifndef CUSTOMTABWIDGET_H
#define CUSTOMTABWIDGET_H

#include <QTabWidget>
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QHash>
#include "custom/treeitem.h"
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
    void checkActSaveStatus();
//    void openImgTab(TreeItem *treeItem);
//    void openComImgTab(TreeItem *treeItem);
protected:
    void contextMenuEvent(QContextMenuEvent *event);
public:
    CustomTabWidget(QWidget *parent = nullptr);


    int openTab(TreeItem *treeItem);
    void closeTab(TreeItem *treeItem);
    int addImgTab(TreeItem *treeItem);
    int addComImgTab(TreeItem *treeItem);
    void removeTab(int index);
    void removeAll();
    void removeOtherTabs(int index);
    void setSaveStatus(int index, bool unsaved);
    void remapIds(const QString &project, const QHash<quint16, quint16> &idMap); // 节点重排重编id后修正标签页持有的id
signals:
    void updateSize(QSize);
    void updateSaveStatus(bool, bool);  // save和saveAll的状态
public slots:
    void on_Changed(QString, int, bool);
    void on_ActSave_Triggered();
    void on_ActSaveAll_Triggered();
};

#endif // CUSTOMTABWIDGET_H
