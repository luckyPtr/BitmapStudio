#include "customtab.h"

CustomTab::CustomTab(QWidget *parent)
    : QWidget{parent}
{

}

void CustomTab::setProjct(QString project)
{
    this->project = project;
}

void CustomTab::setType(int type)
{
    this->type = type;
}

void CustomTab::setId(int id)
{
    this->id = id;
}

void CustomTab::setSize(QSize size)
{
    this->size = size;
}

void CustomTab::setChanged(bool changed)
{
    this->iscChanged = changed;
}

QString CustomTab::getProject()
{
    return project;
}

int CustomTab::getType()
{
    return type;
}

int CustomTab::getId()
{
    return id;
}

QSize CustomTab::getSize()
{
    return size;
}

bool CustomTab::isChanged()
{
    return iscChanged;
}
