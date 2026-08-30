#include "dialogloading.h"
#include "ui_dialogloading.h"
#include <QTimer>
#include <QPainter>
#include <QDebug>

DialogLoading::DialogLoading(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLoading)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint);

    // 将默认的父窗口设为mainwindow
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

        QWidget::setParent(widget);

        //获取父窗口geometry
        QRect rect = widget->geometry();
        //计算显示原点

        int x = rect.width()/2 - this->width() / 2;
        int y = rect.height()/2 - this->height() / 2;
        this->move(x, y);
    }

    timer = startTimer(100);
}

DialogLoading::~DialogLoading()
{
    delete ui;
}



void DialogLoading::paintEvent(QPaintEvent *event)
{
    static const QColor colorTbl[] =
    {
        QColor(0x1E1E1E),
        QColor(0x3C3C3C),
        QColor(0x5A5A5A),
        QColor(0x787878),
        QColor(0x969696),
        QColor(0xB4B4B4),
        QColor(0xD4D4D4),
        QColor(0xF0F0F0),
    };

    static const QPoint offsetTbl[] =
    {
        QPoint(0, 0),
        QPoint(0, rectSize.height() + rectGap),
        QPoint(0, 2 * (rectSize.height() + rectGap)),
        QPoint(rectSize.width() + rectGap, 2 * (rectSize.height() + rectGap)),
        QPoint(2 * (rectSize.width() + rectGap), 2 * (rectSize.height() + rectGap)),
        QPoint(2 * (rectSize.width() + rectGap), rectSize.height() + rectGap),
        QPoint(2 * (rectSize.width() + rectGap), 0),
        QPoint(rectSize.width() + rectGap, 0),
    };

    QPoint startPos((this->width() - (3 * rectSize.width() + 2 * rectGap)) / 2, (this->height() - (3 * rectSize.height() + 2 * rectGap)) / 2);

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setBrush(QBrush(QColor("#F8A0A0A0"))); //窗体背景色
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();          //rect为绘制大小
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 8, 8);  //15为圆角角度



    QBrush brush;
    brush.setStyle(Qt::SolidPattern);

    for(int i = 0; i < 8; i++)
    {
        int index = (step - i + 8) % 8;
        brush.setColor(colorTbl[i]);
        QRect rect(startPos + offsetTbl[index], rectSize);
        painter.fillRect(rect, brush);
    }

    QWidget::paintEvent(event);
}

void DialogLoading::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == timer)
    {
        ++step %= 8;
        repaint();
    }
}
