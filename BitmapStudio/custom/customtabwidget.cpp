#include "customtabwidget.h"
#include "gui/formpixeleditor.h"
#include "gui/formcomimgeditor.h"
#include <QTabBar>
#include <QContextMenuEvent>
#include "core/imgconvertor.h"
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
        removeAll();
    });

    connect(actCloseOthers, &QAction::triggered, this, [=](){
        removeOtherTabs(index);
    });

    menu.exec(mapToGlobal(event->pos()));
}

CustomTabWidget::CustomTabWidget(QWidget *parent) : QTabWidget(parent)
{
    this->setMouseTracking(true);
    connect(this, SIGNAL(updateSize(QSize)), this->parent()->parent(), SLOT(on_UpdateStatusBarSize(QSize)));
    connect(this, &CustomTabWidget::currentChanged, this, [=](){
        CustomTab *tab = static_cast<CustomTab *>(currentWidget());
        if(tab != nullptr)
            emit updateSize(tab->getSize());
    });
}

int CustomTabWidget::openTab(TreeItem *treeItem)
{
    if(treeItem->getType() == RawData::TypeImgFile)
    {
        return addImgTab(treeItem);
    }
    else if(treeItem->getType() == RawData::TypeComImgFile)
    {
        return addComImgTab(treeItem);
    }
    return -1;
}

void CustomTabWidget::closeTab(TreeItem *treeItem)
{
    int id = treeItem->getID();
    QString project = treeItem->getRawData()->getProject();

    // 如果已经存在，不新增，直接选中相应的tabWidget
    for(int i = 0; i < count(); i++)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->widget(i));
        if(widget->getProject() == project && widget->getId() == id)
        {
            removeTab(i);
            break;
        }
    }
}




int CustomTabWidget::addImgTab(TreeItem *treeItem)
{
    int id = treeItem->getID();
    QString project = treeItem->getRawData()->getProject();
    BmFile bf = treeItem->getRawData()->getBmFile(id);
    ImgConvertor ic(treeItem->getRawData()->getDataMap().values().toVector(), treeItem->getRawData()->getSettings());

    // 如果已经存在，不新增，直接选中相应的tabWidget
    for(int i = 0; i < count(); i++)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->widget(i));
        if(widget->getProject() == project && widget->getId() == id)
        {
            (static_cast<FormPixelEditor *>(widget))->on_LoadImage(bf.image);
            setTabText(i, bf.name);
            setCurrentIndex(i);
            return i;
        }
    }

    FormPixelEditor *window = new FormPixelEditor(this);
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
    ImgConvertor ic(treeItem->getRawData()->getDataMap().values().toVector(), treeItem->getRawData()->getSettings());

    // 如果已存在直接切换到相应的界面
    for(int i = 0; i < count(); i++)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->widget(i));
        if(widget->getProject() == project && widget->getId() == id)
        {
            (static_cast<FormComImgEditor *>(widget))->on_LoadComImg(bf.comImg, treeItem->getRawData());
            setTabText(i, bf.name);
            setCurrentIndex(i);
            return i;
        }
    }


    FormComImgEditor *window = new FormComImgEditor(this);
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
    qDebug() << "delete" << index;
    QWidget *removeWidget = widget(index);
    QTabWidget::removeTab(index);
    delete removeWidget;
}

void CustomTabWidget::removeAll()
{
    while(widget(0) != nullptr)
    {
        removeTab(0);
    }
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

void CustomTabWidget::setSaveStatus(int index, bool unsaved)
{

}

void CustomTabWidget::on_Changed(QString project, int id, bool unsaved)
{
    for(int i = 0; i < count(); i++)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->widget(i));
        if(widget->getProject() == project && widget->getId() == id)
        {
            if(unsaved)
            {
                if(!widget->isChanged())
                {
                    //this->tabBar()->setTabTextColor(i, Qt::red);
                    this->tabBar()->setTabText(i, this->tabBar()->tabText(i) + "*");
                }
            }
            else
            {
                if(widget->isChanged())
                {
                    this->tabBar()->setTabText(i, this->tabBar()->tabText(i).remove('*'));
                }
            }
            widget->setChanged(unsaved);
            break;
        }
    }
}

void CustomTabWidget::on_ActSave_Triggered()
{
    if(this->count() > 0)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->currentWidget());
        widget->save();
    }
}

void CustomTabWidget::on_ActSaveAll_Triggered()
{
    for(int i = 0; i < count(); i++)
    {
        CustomTab *widget = static_cast<CustomTab *>(this->widget(i));
        widget->save();
    }
}


