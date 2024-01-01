#ifndef DIALOGIMPORTIMG_H
#define DIALOGIMPORTIMG_H

#include <QDialog>
#include <QImage>

namespace Ui {
class DialogImportImg;
}

class DialogImportImg : public QDialog
{
    Q_OBJECT
private:
    QImage rawImg;      // 原始图片
    QImage monoImg;     // 二值化后的图片
    quint8 grayscale = 128;

public:
    explicit DialogImportImg(QImage &img, QWidget *parent = nullptr);
    ~DialogImportImg();

    void setRawImg(QImage &img) {rawImg = img;}
    QImage getMonoImg() {return monoImg;}
    void setImgName(QString name);
    void setBrief(QString breif);
    QString getImgName();
    QString getBrief();
    void imgTransform();
    void displayImg();
private slots:
    void on_sliderGrayscale_valueChanged(int value);

    void on_lineEditName_textChanged(const QString &arg1);

private:
    Ui::DialogImportImg *ui;


};

#endif // DIALOGIMPORTIMG_H
