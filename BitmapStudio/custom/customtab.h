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
    bool iscChanged = false;
public:
    explicit CustomTab(QWidget *parent = nullptr);

    void setProjct(QString project);
    void setType(int type);
    void setId(int id);
    void setSize(QSize size);
    void setChanged(bool changed);

    QString getProject();
    int getType();
    int getId();
    QSize getSize();
    bool isChanged();
signals:
    void changed(QString, int, bool);
};

#endif // CUSTOMTAB_H
