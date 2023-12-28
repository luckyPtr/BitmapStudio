#include "dialognotice.h"
#include "ui_dialognotice.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QTimer>

DialogNotice::DialogNotice(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNotice)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint);

    // 居中显示
    QWidget *widget = nullptr;
    //获取程序所有的widget
    QWidgetList widgetList = QApplication::allWidgets();
    //获取父窗口指针 此处获取objectName 可根据需求自行判断
    for(int i=0; i<widgetList.length(); i++){
        if(widgetList.at(i)->objectName() == "MainWindow"){
            widget = widgetList.at(i);
        }
    }
    if(widget){
        //获取父窗口geometry
        QRect rect = widget->geometry();
        //计算显示原点
        int x = rect.width()/2 - this->width() / 2;
        int y = rect.height()/2 - this->height() / 2;
        this->move(x, y);
    }

    QTimer* t = new QTimer(this);//加入对象树
    t->start(1000);
    connect(t, &QTimer::timeout, [=]{
        timer = startTimer(50);
    });



    backgroundColor = QColor("#dadada");
    backgroundColor.setAlphaF(0.8);
}

DialogNotice::~DialogNotice()
{
    qDebug() << "dlg delete";
    delete ui;
}

void DialogNotice::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setBrush(QBrush(backgroundColor)); //窗体背景色
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();          //rect为绘制大小
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 8, 8);  //15为圆角角度
    //也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 15, 15);
    //QPainterPath painterPath;
    //painterPath.addRoundedRect(rect, 15, 15);//15为圆角角度
    //painter.drawPath(painterPath);
    QWidget::paintEvent(event);
}

void DialogNotice::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == timer)
    {
        qreal alpha = backgroundColor.alphaF();

        if(alpha > 0.1)
        {
            alpha -= 0.04;
            backgroundColor.setAlphaF(alpha);
            repaint();
        }
        else
        {
            this->close();
        }
    }
}
