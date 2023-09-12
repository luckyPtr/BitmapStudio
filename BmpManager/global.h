#ifndef GLOBAL_H
#define GLOBAL_H




class Global
{
public:
    Global();

    // 像素编辑器相关
#define AUX_LINE_SCALE  true   // 拖动辅助线时是否显示辅助线刻度
    static int pixelSize;      // 像素编辑窗口一个像素的大小
    static int scaleWidth;     // 标尺宽度
    static int scaleOffset;    // 标尺上0点起始位置的偏移
    static int pixelColor_0;    // 像素编辑像素值为0的颜色
    static int pixelColor_1;
    static int gridColor;       // 像素编辑器网格颜色
    static int itemBoundColor;
    static int selectedItemBoundColor;

};

#endif // GLOBAL_H
