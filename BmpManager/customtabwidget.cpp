#include "customtabwidget.h"
#include "formpixeleditor.h"
#include "formcomimgeditor.h"
#include <QTabBar>
#include <QContextMenuEvent>
#include "imgconvertor.h"
#include "custom/customtab.h"


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
    int id = treeItem->getID();
    QString project = treeItem->getRawData()->getProject();
    BmFile bf = treeItem->getRawData()->getBmFile(id);
    ImgConvertor ic(treeItem->getRawData()->getDataMap().values().toVector());

    // 如果已经存在，不新增，直接选中相应的tabWidget
    for(int i = 0; i < count(); i++)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->widget(i));
        if(widget->getProject() == project && widget->getId() == id)
        {
            this->setCurrentIndex(i);
            return i;
        }
    }


    FormPixelEditor *window = new FormPixelEditor;
    window->setProjct(project);
    window->setId(id);
    window->on_LoadImage(bf.image);
    int index = this->addTab(window, bf.name);

    QTabBar *tb = this->tabBar();
    tb->setTabToolTip(index, ic.getFullName(bf).replace("_", "\\"));
    tb->setTabIcon(index, QIcon(":/Image/TreeIco/ImageFile.svg"));

    setCurrentIndex(index);

    return index;
}

int CustomTabWidget::addComImgTab(TreeItem *treeItem)
{
    int id = treeItem->getID();
    QString project = treeItem->getRawData()->getProject();
    BmFile bf = treeItem->getRawData()->getBmFile(id);
    ImgConvertor ic(treeItem->getRawData()->getDataMap().values().toVector());

    // 如果已存在直接切换到相应的界面
    for(int i = 0; i < count(); i++)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->widget(i));
        if(widget->getProject() == project && widget->getId() == id)
        {
            this->setCurrentIndex(i);
            return i;
        }
    }

    FormComImgEditor *window = new FormComImgEditor;
    window->setId(id);
    window->setProjct(project);
    window->on_LoadComImg(bf.comImg, treeItem->getRawData());
    int index = this->addTab(window, bf.name);

    QTabBar *tb = this->tabBar();
    tb->setTabToolTip(index, ic.getFullName(bf).replace("_", "\\"));
    tb->setTabIcon(index, QIcon(":/Image/TreeIco/ComImgFile.svg"));

    setCurrentIndex(index);

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
