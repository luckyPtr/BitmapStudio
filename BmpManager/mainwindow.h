#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QStandardItem>
#include <projectmng.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actOpenProject_triggered();

    void on_treeViewProject_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    ProjectMng pm;
    void init();

};
#endif // MAINWINDOW_H