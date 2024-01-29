<div align=center>
  <img src="Docs/Images/bitmapstudio.png" alt="bitmapstudio" style="zoom:50%;" />
</div>
<h1 align="center">
  Bitmap Studio
</h1>
<p align="center">
  🚩单片机OLED位图管理、编辑和自动取模软件
</p>


![](Docs\Images\image-20240127000727936.png)



# 🚀 快速开始

## 获取软件

- 下载[release](https://github.com/luckyPtr/BitmapStudio/releases)

- 解压后运行 **Bitmap Studio.exe**

  （如果遇到缺少dll无法运行的情况，请安装Bitmap Studio.exe同文件夹下的VC++运行库vc_redist.x64.exe）

  或

- clone项目工程

  ```git
  git clone git@github.com:luckyPtr/BitmapStudio.git
  ```

- 使用Qt Creator打开工程并编译



## 新建工程

- 点击新建工程按键
- 选择工程名称和路径
- 设置项目屏幕的尺寸和取模方式

![新建工程](Docs/Images/new_project.gif)



## 导入图片

### 从图片文件导入

- 图片-右键-导入-图片
- 选择图片文件
- 设置图片名称
- 调整图片的取色阈值

![从图片导入](Docs/Images/import_from_images.gif)

### 从字模导入

- 图片-右键-导入-自模
- 输入字模（剪贴板有字模会自动导入）
- 选择取模方式和图片尺寸
- 输入图片名称

![从字模导入](Docs/Images/import_from_code.gif)

## 图片编辑

可以对图片进行简单的编辑

- 像素编辑
- 位置变换
- 尺寸调整
- 旋转和镜像
- 反色

![图片编辑](Docs/Images/edit.gif)

## 一键生成字模

- 点击**运行**一键生成导入图片的字模
- 生成.c/.h文件可直接在单片机工程中使用


![一键生成](Docs/Images/generate.gif)

