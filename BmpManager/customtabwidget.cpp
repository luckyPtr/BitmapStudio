#include "customtabwidget.h"
#include "formpixeleditor.h"
#include <QTabBar>
#include <QContextMenuEvent>
#include "imgconvertor.h"


void CustomTabWidget::initMenu()
{
    menu = new QMenu(this);


    \
    QTabBar *tb = tabBar();

    tb->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void CustomTabWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QTabBar* bar = tabBar();

    int index = bar->tabAt(bar->mapFrom(this, event->pos()));

    if(index < 0)
        return;

    QMenu menu(this);
    QAction *actCloseThis = new QAction(tr("关闭"));
    QAction *actCloseAll = new QAction(tr("关闭全部"));
    QAction *actCloseOthers = new QAction(tr("关闭其他"));

    menu.addAction(actCloseThis);
    menu.addAction(actCloseAll);
    menu.addAction(actCloseOthers);

    connect(actCloseThis, &QAction::triggered, this, [=](){
        removeTab(index);
    });

    connect(actCloseAll, &QAction::triggered, this, [=](){
        removeAllTabs();
    });

    connect(actCloseOthers, &QAction::triggered, this, [=](){
        removeOtherTabs(index);
    });


    menu.exec(mapToGlobal(event->pos()));
}




int CustomTabWidget::addImgTab(TreeItem *treeItem)
{
    // 如果已经存在，不新增，直接选中相应的tabWidget
    for(int i = 0; i < tabWidgetItems.size(); i++)
    {
        if(treeItem == tabWidgetItems.at(i)->treeItem)
        {
            this->setCurrentIndex(i);
            return i;
        }
    }

    TabWidgetItem *tabItem = new TabWidgetItem;
    int id = treeItem->getID();
    BmFile bf = treeItem->getRawData()->getBmFile(id);
    ImgConvertor ic(treeItem->getRawData()->getDataMap().values().toVector());

    tabItem->title = bf.name;
    tabItem->treeItem = treeItem;
    tabItem->window = new FormPixelEditor;
    FormPixelEditor *window = static_cast<FormPixelEditor *>(tabItem->window);
    window->on_LoadImage(bf.image);
    int index = this->addTab(window, tabItem->title);
    tabWidgetItems << tabItem;

    QTabBar *tb = this->tabBar();
    tb->setTabToolTip(index, ic.getFullName(bf).replace("_", "\\"));
    tb->setTabIcon(index, QIcon(":/Image/TreeIco/ImageFile.svg"));

    return index;
}

void CustomTabWidget::removeTab(int index)
{
    for(int i = 0; i < tabWidgetItems.size(); i++)
    {
        if(index == this->indexOf(tabWidgetItems[i]->window))
        {
            QTabWidget::removeTab(index);
            tabWidgetItems.removeAt(i);
            break;
        }
    }
}

void CustomTabWidget::removeAllTabs()
{
    tabWidgetItems.clear();
    QTabWidget::clear();
}

void CustomTabWidget::removeOtherTabs(int index)
{
    // 移除左边的tabs
    for(int i = 0; i < index; i++)
    {
        removeTab(0);
    }
    // 移除右边的tabs
    while(count() > 1)
    {
        removeTab(1);
    }
}
