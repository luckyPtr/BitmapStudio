#include "dialognewproject.h"
#include "ui_dialognewproject.h"
#include "global.h"
#include <QFileDialog>
#include <QDebug>

DialogNewProject::DialogNewProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewProject)
{
    ui->setupUi(this);

    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);        // 固定窗口
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);  // 取消Dialog的？

    btnGroup1 = new QButtonGroup(this);
    btnGroup1->addButton(ui->radioButton_LSB, 0);
    btnGroup1->addButton(ui->radioButton_MSB, 1);

    btnGroup2 = new QButtonGroup(this);
    btnGroup2->addButton(ui->radioButton_ZL, 0);
    btnGroup2->addButton(ui->radioButton_ZH, 1);
    btnGroup2->addButton(ui->radioButton_LH, 2);
    btnGroup2->addButton(ui->radioButton_HL, 3);

    connect(btnGroup1, SIGNAL(idToggled(int,bool)), this, SLOT(on_btnToggled(int,bool)));
    connect(btnGroup2, SIGNAL(idToggled(int,bool)), this, SLOT(on_btnToggled(int,bool)));

    // 使用配置文件中保存的默认尺寸
    ui->spinBoxwidth->setValue(Global::defaultProjectWidth);
    ui->spinBoxHeight->setValue(Global::defaultProjectHeight);

    // 使用配置文件中保存的默认取模方式
    if (Global::defaultProjectMode != -1) {
        // 根据 mode 值设置按钮状态
        int btnGroup1Id, btnGroup2Id;
        switch(Global::defaultProjectMode) {
            case ImgEncoderFactory::ZL_LSB:
                btnGroup1Id = 0; btnGroup2Id = 0; break;
            case ImgEncoderFactory::ZH_LSB:
                btnGroup1Id = 0; btnGroup2Id = 1; break;
            case ImgEncoderFactory::LH_LSB:
                btnGroup1Id = 0; btnGroup2Id = 2; break;
            case ImgEncoderFactory::HL_LSB:
                btnGroup1Id = 0; btnGroup2Id = 3; break;
            case ImgEncoderFactory::ZL_MSB:
                btnGroup1Id = 1; btnGroup2Id = 0; break;
            case ImgEncoderFactory::ZH_MSB:
                btnGroup1Id = 1; btnGroup2Id = 1; break;
            case ImgEncoderFactory::LH_MSB:
                btnGroup1Id = 1; btnGroup2Id = 2; break;
            case ImgEncoderFactory::HL_MSB:
                btnGroup1Id = 1; btnGroup2Id = 3; break;
            default:
                btnGroup1Id = -1; btnGroup2Id = -1; break;
        }
        if (btnGroup1Id != -1) {
            btnGroup1->button(btnGroup1Id)->setChecked(true);
            btnGroup2->button(btnGroup2Id)->setChecked(true);
        }
    }
}

DialogNewProject::~DialogNewProject()
{
    delete ui;
}

QString DialogNewProject::getNewProject()
{
    return QString("%1/%2.bms").arg(ui->lineEditProjectPath->text()).arg(ui->lineEditProjectName->text());
}

RawData::Settings DialogNewProject::getSettings()
{
    RawData::Settings settings;
    settings.size = QSize(ui->spinBoxwidth->value(), ui->spinBoxHeight->value());
    settings.mode = mode;

    // 保存尺寸和取模方式到配置文件，作为下次新建的默认值
    Global::saveDefaultProjectSize(ui->spinBoxwidth->value(), ui->spinBoxHeight->value());
    Global::saveDefaultProjectMode(mode);

    return settings;
}

void DialogNewProject::on_btnOK1_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void DialogNewProject::on_btnLastStep_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void DialogNewProject::on_btnToggled(int btn, bool checked)
{
    switch(btnGroup2->checkedId())
    {
    case 0:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::ZL_LSB : ImgEncoderFactory::ZL_MSB;
        break;
    case 1:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::ZH_LSB : ImgEncoderFactory::ZH_MSB;
        break;
    case 2:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::LH_LSB : ImgEncoderFactory::LH_MSB;
        break;
    case 3:
        mode = btnGroup1->checkedId() == 0 ? ImgEncoderFactory::HL_LSB : ImgEncoderFactory::HL_MSB;
        break;
    }
    ui->widget->setMode(mode);
    updateBtnStatus();
}


void DialogNewProject::on_btnBrowse_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("打开文件夹"));
    if(!path.isEmpty())
    {
        ui->lineEditProjectPath->setText(path);
    }
    updateBtnStatus();
}


void DialogNewProject::on_lineEditProjectName_textChanged(const QString &arg1)
{
    updateBtnStatus();
}

void DialogNewProject::updateBtnStatus()
{
    ui->btnOK1->setEnabled(!ui->lineEditProjectPath->text().isEmpty() && !ui->lineEditProjectName->text().isEmpty());
    ui->btnOK2->setEnabled(btnGroup1->checkedId() != -1 && btnGroup2->checkedId() != -1);
}

