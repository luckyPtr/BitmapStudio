#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Bitmap Studio 旧版工程转换脚本：SQLite(.bs / 旧.bms) -> JSON(.bms)

自 v0.4.0 起，Bitmap Studio 工程文件改为单文件 JSON 格式，程序不再直接
打开旧版 SQLite 工程。本脚本把旧工程一次性转换为新格式。

用法：
    python convert_legacy.py <旧工程路径.bs|.bms> [-o 输出路径.bms]

规则：
    - 文件夹/图片/组合图按原有树结构转为嵌套 JSON，同级顺序复刻旧版显示顺序
      （文件夹在前，同类型按名称字母序），与用户在旧版中看到的排列一致
    - 组合图成员由数字 id 改为树路径引用，悬空引用会被丢弃并告警
    - 同树同级重名、含"/"的名称自动规范化（加后缀/替换字符）并告警
    - 组合图尺寸与屏幕一致时省略 size（跟随屏幕）
"""

import argparse
import base64
import json
import os
import sqlite3
import sys

# 与 ImgEncoderFactory 枚举对应：ZH_LSB=0 ... LH_MSB=7
SCANS = ["ZH", "ZL", "HL", "LH"]

DEFAULT_KEYWORDS = {
    "const": "const",
    "pos": "unsigned char",
    "size": "unsigned char",
    "addr": "unsigned int",
}

# 旧库 type 列：0=图片 1=组合图 2=图片文件夹 3=图片组文件夹 4=组合图文件夹
IMG_TYPES = (0, 2, 3)
COM_TYPES = (1, 4)


def class_of(row):
    return "img" if row["type"] in IMG_TYPES else "com"


class Converter:
    def __init__(self, db_path):
        self.conn = sqlite3.connect(db_path)
        self.conn.row_factory = sqlite3.Row
        self.rows = {r["id"]: dict(r) for r in self.conn.execute("SELECT * FROM tbl_img ORDER BY id")}
        self.path_of = {}       # 图片id -> 树路径
        self.screen = [128, 64]

    def warn(self, msg):
        print("[警告] " + msg, file=sys.stderr)

    def unique_name(self, name, pid, tree, row_id):
        """同树同级去重（排除自身）；替换路径分隔符。直接改写self.rows保证后续检查一致。"""
        name = (name or "untitled").replace("/", "_").replace("\\", "_") or "untitled"
        siblings = [r["name"] for r in self.rows.values()
                    if r["id"] != row_id and r["pid"] == pid and class_of(r) == tree]
        result, base, n = name, name, 1
        while result in siblings:
            result = "%s_%d" % (base, n)
            n += 1
        if result != name:
            self.warn("同级重名已自动改名: %s -> %s" % (name, result))
        return result

    def children_of(self, pid, tree):
        """指定父节点的同树子节点。复刻旧版程序的显示顺序，保证转换后排列
        与用户在旧版中看到的一致：类型降序（组合图文件夹>图片组>图片文件夹>组合图>图片），
        同类型按名称不区分大小写的字母序。"""
        kids = [r for r in self.rows.values() if r["pid"] == pid and class_of(r) == tree]
        kids.sort(key=lambda r: (-r["type"], r["name"].lower()))
        return kids

    def png_node(self, row, parent_path):
        node = {"name": row["name"]}
        if row["brief"]:
            node["note"] = row["brief"]
        node["png"] = base64.b64encode(bytes(row["data"] or b"")).decode("ascii")
        path = "%s/%s" % (parent_path, row["name"]) if parent_path else row["name"]
        self.path_of[row["id"]] = path
        return node

    def build_level(self, pid, tree, parent_path):
        out = []
        for row in self.children_of(pid, tree):
            # sqlite3.Row只读，名称规范化后写回dict
            name = self.unique_name(row["name"], pid, tree, row["id"])
            self.rows[row["id"]]["name"] = name
            path = "%s/%s" % (parent_path, name) if parent_path else name

            node = {"name": name}
            if row["brief"]:
                node["note"] = row["brief"]

            if row["type"] == 0:                    # 图片
                node["png"] = base64.b64encode(bytes(row["data"] or b"")).decode("ascii")
                self.path_of[row["id"]] = path
            elif row["type"] == 2:                  # 图片文件夹
                node["children"] = self.build_level(row["id"], "img", path)
            elif row["type"] == 3:                  # 图片组 -> frames
                frames = []
                for c in self.children_of(row["id"], "img"):
                    if c["type"] != 0:
                        self.warn("图片组 %s 内的非图片节点已跳过: %s" % (name, c["name"]))
                        continue
                    cname = self.unique_name(c["name"], row["id"], "img", c["id"])
                    self.rows[c["id"]]["name"] = cname
                    wrapped = dict(c)
                    wrapped["name"] = cname
                    frames.append(self.png_node(wrapped, path))
                node["frames"] = frames
            elif row["type"] == 4:                  # 组合图文件夹
                node["children"] = self.build_level(row["id"], "com", path)
            elif row["type"] == 1:                  # 组合图
                node.update(self.build_composite(row, name))
            out.append(node)
        return out

    def build_composite(self, row, name):
        result = {}
        try:
            ci = json.loads(row["data"]) if row["data"] else {}
        except (ValueError, TypeError):
            self.warn("组合图 %s 的数据损坏，按空组合图处理" % name)
            ci = {}
        size = [int(ci.get("width", 0)), int(ci.get("height", 0))]
        if size != self.screen:                     # 与屏幕一致时省略size，跟随屏幕
            result["size"] = size
        items = []
        for it in ci.get("images", []):
            ref_id = int(it.get("id", 0))
            ref_path = self.path_of.get(ref_id)
            if not ref_path:
                self.warn("组合图 %s 的悬空引用已丢弃: id=%d" % (name, ref_id))
                continue
            items.append({"image": ref_path,
                          "pos": [int(it.get("x", 0)), int(it.get("y", 0))]})
        result["items"] = items
        return result

    def convert(self):
        cur = self.conn.execute("SELECT * FROM tbl_settings")
        row = cur.fetchone()
        settings = dict(row) if row else {}
        self.screen = [int(settings.get("width") or 128), int(settings.get("height") or 64)]

        mode = int(settings.get("mode") or 0)
        if not 0 <= mode <= 7:
            mode = 0
        export = {
            "scan": SCANS[mode % 4],
            "bitOrder": "LSB" if mode < 4 else "MSB",
            "output": settings.get("format") or "C",
        }
        if settings.get("path"):
            export["outdir"] = settings["path"]
        keywords = {
            "const": settings.get("const") or DEFAULT_KEYWORDS["const"],
            "pos": settings.get("img_pos") or DEFAULT_KEYWORDS["pos"],
            "size": settings.get("img_size") or DEFAULT_KEYWORDS["size"],
            "addr": settings.get("img_addr") or DEFAULT_KEYWORDS["addr"],
        }
        if keywords != DEFAULT_KEYWORDS:
            export["keywords"] = keywords
        if int(settings.get("custom_typedef") or 0):
            export["customTypedef"] = True

        doc = {
            "format": "bms",
            "version": 1,
            "screen": self.screen,
            "export": export,
        }
        if settings.get("brief"):
            doc["note"] = settings["brief"]

        # 先构建图片树（生成路径索引），再构建组合图树（解析成员引用）
        doc["images"] = self.build_level(0, "img", "")
        doc["composites"] = self.build_level(0, "com", "")
        return doc


def main():
    parser = argparse.ArgumentParser(description="Bitmap Studio 旧版SQLite工程转JSON")
    parser.add_argument("input", help="旧版工程文件路径(.bs 或 SQLite版 .bms)")
    parser.add_argument("-o", "--output", help="输出路径（默认在原目录生成，不覆盖原文件）")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print("文件不存在: %s" % args.input, file=sys.stderr)
        sys.exit(1)
    with open(args.input, "rb") as f:
        if not f.read(16).startswith(b"SQLite format 3"):
            print("不是SQLite格式的旧版工程文件: %s" % args.input, file=sys.stderr)
            sys.exit(1)

    output = args.output
    if not output:
        stem, _ = os.path.splitext(args.input)
        if args.input.lower().endswith(".bms"):
            output = stem + "_converted.bms"     # 避免覆盖同名旧文件
        else:
            output = stem + ".bms"

    doc = Converter(args.input).convert()
    with open(output, "w", encoding="utf-8") as f:
        json.dump(doc, f, ensure_ascii=False, indent=4)
    print("转换完成: %s" % output)


if __name__ == "__main__":
    main()
