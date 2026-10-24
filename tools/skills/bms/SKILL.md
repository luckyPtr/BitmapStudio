---
name: bms
description: >-
  操作 Bitmap Studio 单片机位图工程（.bms 文件）：查看/编辑 JSON 存档、用 bms-cli 渲染图片、
  添加/重命名/移动/删除图片与组合图、导出 C 字模数组或 bin、校验工程、转换旧版 SQLite 工程。
  当任务涉及 .bms 文件、Bitmap Studio、bms-cli、取模、字模、OLED/LCD 位图资源时使用本技能。
---

# Bitmap Studio 工程操作

## 安装（一次性）

把本目录（`bms/SKILL.md`）拷贝到你的固件工程的 `.agents/skills/` 下（随工程生效），
或 `~/.agents/skills/` 下（对你所有工程生效）。

## 工具与文件

- **bms-cli.exe**：随 Bitmap Studio 软件分发，与主程序同目录（解压/安装目录下）。
  建议加入 PATH；未加 PATH 时用完整路径调用，如 `"C:\Tools\BitmapStudio\bms-cli.exe"`
- **convert_legacy.py**：同样随软件分发（软件目录的 tools/ 下），转换旧版 SQLite 工程
- **工程文件**：单文件 JSON（`.bms`），顶层 `format:"bms"`；`images`/`composites` 两棵嵌套树
- **格式契约**（直改 JSON 前必读）：
  - 节点形状即类型：`children`=文件夹、`frames`=图片组、`png`=图片（base64，单行长字符串）、`items`=组合图
  - 组合图成员用**树路径**引用（如 `"icons/logo"`，从 images 树根起算）；`items` 数组顺序=绘制顺序（后者覆盖前者）
  - 同级**容器在前**；同树同级不重名、名称不含 `/`
  - **节点名仅允许字母/数字/下划线**（数字开头可以）：名称会拼接为导出的 C 数组名（`IMG_`/`CMP_` 前缀+路径段），中文/空格/连字符会导致生成的 C 无法编译。`add`/`rename` 拒绝非法名；`check` 对存量非法名输出警告（退出码 1）
  - 默认省略：`note` 空不写、`keywords` 全默认不写、组合图 `size` 省略=跟随 `screen`
  - 工程设置字段（`check` 对枚举非法值报警，直改时务必按下表取值）：

    | 字段 | 合法值 | 默认 | 说明 |
    |---|---|---|---|
    | `screen` | `[宽,高]` 正整数 | `[128,64]` | 屏幕像素尺寸 |
    | `export.scan` | `ZH`/`ZL`/`HL`/`LH` | `ZH` | 扫描方向：逐行/逐列/行列/列行 |
    | `export.bitOrder` | `MSB`/`LSB` | `LSB` | 位序 |
    | `export.output` | `C`/`bin` | `C` | C 数组或 bin 镜像 |
    | `export.outdir` | 目录字符串 | 省略 | 导出目录（相对工程文件；省略=工程所在目录） |
    | `export.keywords.const` | 类型/修饰符 | `const` | BMS_CONST 展开；51 平台 Keil 填 `code`、SDCC 填 `const __code` |
    | `export.keywords.pos`/`size` | C 类型 | `unsigned char` | `BmsImageItem_t` 的 x/y 与 width/height 字段类型（大屏超 255 像素改宽） |
    | `export.keywords.addr` | C 类型 | `unsigned int` | bin 模式图片地址字段类型（注意 C51 的 unsigned int 仅 16 位，bin 超 64KB 需改 `unsigned long`） |
    | `export.customTypedef` | `true` | 省略 | 不生成 `bms_typedef.h`；自备头须含 `BMS_CONST`/`BMS_END_OF_IMG`/`BmsImage_t`/`BmsImageItem_t` |

以下命令用 `bms-cli` 代称；路径均相对你的工程文件。

## 标准工作流

### 0. 创建新工程（无头初始化）
```bash
bms-cli init demo.bms --screen 128x64 --scan ZH --bit MSB [--format C|bin] [--outdir 目录] [--note 备注]
```
文件已存在会拒绝（防覆盖）。创建后即可直接 add 图片 / item-add 组装组合图，全程无需 GUI。

### 1. 理解工程结构
```bash
bms-cli info <工程.bms>            # 人读树形
bms-cli info <工程.bms> --json     # 机器可读（组合图成员已是路径）
```
也可以直接读 JSON 文件本身（跳过 png 长行即可），结构与语义完全自解释。

### 2. 查看图片内容（视觉验证）
```bash
bms-cli render <工程.bms> icons/logo -o /tmp/out.png -s 8   # 放大8倍近邻渲染到临时目录
bms-cli render <工程.bms> screens/menu -o /tmp/menu.png     # 组合图自动合成
bms-cli render <工程.bms> alpha --ascii                     # 无视觉能力时的文本视图
bms-cli render <工程.bms> icons/logo --invert -o /tmp/neg.png   # 反色预览（通道取反）
```
PNG 输出到临时目录（`-o /tmp/...`，即用即弃），随后用视觉能力读取该文件——
没有 base64 输出选项，文件路径就是视觉通道的标准入口。
组合图的合成语义（绘制顺序/跟随屏幕/越界裁剪）与 GUI 完全一致。

**优先用 render 而非脚本解码**：小图标需放大才可辨认（`-s 8` 近邻放大），组合图只有 render 是
与 GUI 一致的合成结果。仅当环境里没有 bms-cli（裸仓库/CI/只有 .bms 文件）时，才用 python 解码
**单张图片**兜底（读 png 字段 → base64 解码 → 存文件查看）；**组合图永远不要用脚本自行合成**——
层叠/跟随屏幕/裁剪语义复刻出错是静默的，宁可先取得 bms-cli。

要预览**工程里还不存在**的组合图（演示/测试假设布局），用 `compose` 传入描述 JSON（文件或字符串均可），
**不修改工程文件**：
```bash
bms-cli compose <工程.bms> '{"size":[128,64],"items":[{"image":"icons/logo","pos":[0,0]}]}' -o /tmp/preview.png
bms-cli compose <工程.bms> layout.json -o /tmp/preview.png      # 传 JSON 文件也可以
```
描述与组合图节点同构：`size` 省略=跟随屏幕、`items` 顺序=绘制顺序、成员必须是 images 树中的图片。
items 还支持绘图原语（画线/填充矩形/矩形反色/画点），便于演示边框、占位、高亮等假设效果
（`line` 用 `pos`+`end`，`fillrect`/`invertrect` 用 `pos`+`size`，`points` 用 `[[x,y],...]` 点数组；
前三者及点可用 `"color":"black"(默认)|"white"`）：
```bash
bms-cli compose <工程.bms> '{"size":[128,64],"items":[{"type":"fillrect","pos":[0,0],"size":[128,10]},{"image":"icons/logo","pos":[0,20]},{"type":"line","pos":[0,63],"end":[127,63]},{"type":"invertrect","pos":[0,0],"size":[12,12]}]}' -o /tmp/preview.png
```
预览确认后再用 `add` + `item-add` 真正落盘。

### 3. 低风险编辑：直改 JSON
重排（挪数组元素）、加文件夹/图片组节点——直接编辑 JSON，
然后**必须**校验：
```bash
bms-cli check <工程.bms> [--json]     # 退出码 0干净 / 1有警告 / 2文件无效；--json输出结构化结果
```
check 覆盖：悬空引用、坏图片数据、同级重名/非法名称的自动改名、节点放错树（如图片节点出现在组合图树中）、`export` 非法枚举值（scan/bitOrder/output，非法值会静默回退默认所以必须查）、`screen` 非法尺寸。

改 `note` 备注和 `export` 导出设置优先用专用命令（不用直改 JSON）：
```bash
bms-cli note <工程.bms> /测试图 "电池 图标"            # 设置备注（多词加引号，单词可省）
bms-cli note <工程.bms> /测试图 ""                    # 清空备注（传空即清空）
bms-cli export-set <工程.bms> --scan LH --bit MSB      # 逐项修改导出设置，只改提供的选项
bms-cli export-set <工程.bms> --format bin --outdir ./out --note 固件导出   # --note "" 清空工程备注
```

### 4. 路径敏感操作：走 CLI（引用自动级联）
重命名/移动/删除/添加会改变路径，手改 JSON 容易漏改组合图引用且**失败是静默的**（悬空引用加载时被丢弃）。
CLI 内部保证引用一致：
```bash
bms-cli rename <工程.bms> /icons icons2          # 所有引用自动跟随
bms-cli move   <工程.bms> /bell_icon /status_bar
bms-cli move   <工程.bms> /bell_icon /status_bar --before battery_icon   # 指定插入位置：battery_icon须是目标下已有子节点
bms-cli delete <工程.bms> /icons                 # 先报告哪些组合图将丢失成员
bms-cli add    <工程.bms> / battery_icon --png battery.png --note 电池图标
bms-cli add    <工程.bms> / boot_screen --kind composite     # 尺寸默认跟随屏幕
bms-cli add    <工程.bms> / boot_anim --kind group           # 图片组（动画帧）
```

> `rename`/`move` 会拒绝造成**同级重名**的操作（组合图按路径寻址，同级重名有歧义）；
> 目标下已有同名节点时需先 `rename` 其中一个。

**组合图成员的增删建议走 CLI**（写入时即校验路径存在，直改写错虽可被 check 发现但要靠自觉跑）：
```bash
bms-cli item-add <工程.bms> /boot_screen icons/logo 4 4    # 添加成员（图片必须真实存在）
bms-cli item-rm  <工程.bms> /boot_screen 0                 # 按序号移除（0起，顺序=绘制顺序）
```
**重排成员顺序（调层叠）、微调坐标**则直接编辑 items 数组（局部编辑，无级联）+ check 兜底即可。

### 5. 添加自绘图片的完整流
1. 用脚本生成 PNG（python：zlib+struct 即可，无需第三方库；黑白图，黑=点亮）
2. `bms-cli add <工程.bms> <父路径/> <名称> --png <文件>`
3. `bms-cli render ... --ascii` 或渲染 PNG 自检
4. `bms-cli check` 确认干净

### 6. 导出字模（等价于 GUI 的"运行"）
```bash
bms-cli export <工程.bms> [-o 输出目录] [--json]
```
产物：`bms_image.c/.h`（或 bin 模式的 `bms.bin`）、`bms_image_item.c/.h`、`bms_typedef.h`、统一头 `bms.h`。
取模方式/输出格式跟随工程 `export` 设置。stdout 打印产物清单。

产物形状（C 格式，固件 `#include "bms.h"` 一站引入全部符号）：
```c
extern BMS_CONST BmsImage_t     IMG_logo[];         /* 单图：取模后的字节流 */
extern BMS_CONST BmsImage_t     IMG_boot_anim[][N]; /* 图片组：N=每帧字节数，行=帧，IMG_boot_anim[i] 即第 i 帧 */
extern BMS_CONST BmsImageItem_t CMP_menu[];         /* 组合图：成员表，末项为哨兵 {BMS_END_OF_IMG} */

/* 固件端遍历组合图（遇哨兵结束，image 为空指针）：
   for (const BmsImageItem_t *it = CMP_menu; it->image; ++it)
       draw_bitmap(it->x, it->y, it->width, it->height, it->image);
*/
```
bin 模式：图片数据集中进 `bms.bin`（前 64 字节文件头：ASCII 魔数 `"BitmapStudio"` + 大端的版本/数据长度/SUM32/CRC32/CRC16/CRC8/XOR8，图片数据从 0x40 起）；
`bms_image.h` 用 `IMG_*` 宏给出各图起始地址，图片组另有 `IMG_*_OFFSET`（每帧字节数，第 i 帧位于 `IMG_x + i*IMG_x_OFFSET`）；组合图表仍以 C 数组给出（成员的 image 字段为 bin 内偏移值）。
`--json` 输出机器可读清单（每个产物的路径/字节数/sha256），CI 中可重新导出并比对 sha256 检测资产漂移。
建议把产物纳入固件构建。

### 7. 旧版 SQLite 工程（.bs / 早期 .bms）
```bash
python <软件目录>/tools/convert_legacy.py <旧文件> [-o 输出.bms]
```
按文件头识别，`.bs` 输出同名 `.bms`，SQLite 版 `.bms` 输出 `_converted.bms`（不覆盖原文件）。

## 关键规则与坑

- 直改 JSON 后**永远跑一次 `check`**：悬空引用/重名/坏图片数据只在加载时检测，check 把静默问题变显式
- 加载时会自动规范化：同级容器排到前面、非法名称改写（加 `_1` 后缀）——保存后 diff 里看到的这类变化是预期行为
- JSON 里**没有 id/pid/z**：那是程序的会话内存概念；持久身份就是路径
- 修改后交回用户前，建议 `info` 展示最终树形结构供确认
- GUI 与 CLI 操作同一文件，可混用；GUI 打开工程期间 CLI 的写操作会在 GUI 重新加载后生效

## 退出码

`0` 成功（check：干净）；`1` 失败（check：有警告）；`2` 工程文件无效
