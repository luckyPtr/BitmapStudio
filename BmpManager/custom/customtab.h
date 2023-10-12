#ifndef CUSTOMTAB_H
#define CUSTOMTAB_H

#include <QWidget>

class CustomTab : public QWidget
{
    Q_OBJECT

    QString project;
    int type;
    int id;
public:
    explicit CustomTab(QWidget *parent = nullptr);

    void setProjct(QString project);
    void setType(int type);
    void setId(int id);

    QString getProject();
    int getType();
    int getId();
signals:

};

#endif // CUSTOMTAB_H
