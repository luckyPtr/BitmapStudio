#include "customtabwidget.h"
#include "formpixeleditor.h"
#include <QTabBar>


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
    tabItem->title = bf.name;
    tabItem->treeItem = treeItem;
    tabItem->window = new FormPixelEditor;
    FormPixelEditor *window = static_cast<FormPixelEditor *>(tabItem->window);
    window->on_LoadImage(bf.image);
    int index = this->addTab(window, tabItem->title);
    tabWidgetItems << tabItem;

    QTabBar *tb = this->tabBar();
    tb->setTabToolTip(index, tabItem->title);
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
