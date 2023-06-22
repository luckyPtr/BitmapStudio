#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel() override;

    TreeItem *root();

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    void clear();
    void removeRow(const QModelIndex &index);
    void beginReset() {beginResetModel();}
    void endReset() {endResetModel();}
    enum
    {
        RoleID = Qt::UserRole+1,
        RoleProjPath,
    };


    TreeItem *itemFromIndex(const QModelIndex &index) const;

private:
    QStringList _headers;
    TreeItem* _rootItem;
};
#endif // TREEMODEL_H
