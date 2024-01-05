#ifndef CUSTOMTAB_H
#define CUSTOMTAB_H

#include <QWidget>

class CustomTab : public QWidget
{
    Q_OBJECT

    QString project;
    int type;
    int id;
    QSize size;
public:
    explicit CustomTab(QWidget *parent = nullptr);

    void setProjct(QString project);
    void setType(int type);
    void setId(int id);
    void setSize(QSize size);

    QString getProject();
    int getType();
    int getId();
    QSize getSize();
signals:

};

#endif // CUSTOMTAB_H
