from __future__ import annotations

import html
import os
import posixpath
import zipfile
from pathlib import Path

from PIL import Image


EMU_PER_IN = 914400
SLIDE_W = 12192000
SLIDE_H = 6858000

BASE = Path(__file__).resolve().parent
PIC = BASE / "picture"
OUT = BASE / "csapp_ch11_network_programming_report.pptx"

NS = (
    'xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" '
    'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
    'xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main"'
)

BG = "F7F8FB"
INK = "172033"
MUTED = "52627A"
BLUE = "2D5F9A"
GREEN = "2F7D4C"
ORANGE = "A36C16"
PURPLE = "6D4CA5"
RED = "B0493F"
LIGHT_BLUE = "EAF2FF"
LIGHT_GREEN = "EAF7EF"
LIGHT_ORANGE = "FFF3DC"
LIGHT_PURPLE = "F1EAFD"
LIGHT_RED = "FFECEB"
WHITE = "FFFFFF"


def emu(inches: float) -> int:
    return int(inches * EMU_PER_IN)


def esc(text: str) -> str:
    return html.escape(str(text), quote=False)


def group_start() -> str:
    return (
        "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/>"
        "<p:nvPr/></p:nvGrpSpPr><p:grpSpPr>"
        "<a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/>"
        "<a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm>"
        "</p:grpSpPr>"
    )


def solid_fill(color: str) -> str:
    return f"<a:solidFill><a:srgbClr val=\"{color}\"/></a:solidFill>"


def no_line() -> str:
    return "<a:ln><a:noFill/></a:ln>"


def line(color: str = MUTED, width: int = 16000) -> str:
    return f"<a:ln w=\"{width}\">{solid_fill(color)}</a:ln>"


def rect(
    shape_id: int,
    x: int,
    y: int,
    w: int,
    h: int,
    fill: str,
    outline: str | None = None,
    radius: str = "roundRect",
) -> str:
    ln = line(outline, 14000) if outline else no_line()
    return (
        f"<p:sp><p:nvSpPr><p:cNvPr id=\"{shape_id}\" name=\"shape {shape_id}\"/>"
        "<p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>"
        f"<a:xfrm><a:off x=\"{x}\" y=\"{y}\"/><a:ext cx=\"{w}\" cy=\"{h}\"/></a:xfrm>"
        f"<a:prstGeom prst=\"{radius}\"><a:avLst/></a:prstGeom>"
        f"{solid_fill(fill)}{ln}</p:spPr></p:sp>"
    )


def run(text: str, size: int, color: str, bold: bool = False, mono: bool = False) -> str:
    b = ' b="1"' if bold else ""
    face = "Cascadia Mono" if mono else "Microsoft YaHei"
    return (
        f"<a:r><a:rPr lang=\"zh-CN\" sz=\"{size}\"{b}>"
        f"{solid_fill(color)}<a:latin typeface=\"{face}\"/><a:ea typeface=\"{face}\"/>"
        f"</a:rPr><a:t>{esc(text)}</a:t></a:r>"
    )


def paragraph(
    text: str,
    size: int,
    color: str = INK,
    bold: bool = False,
    bullet: bool = False,
    mono: bool = False,
    level: int = 0,
) -> str:
    if bullet:
        mar_l = 360000 + level * 280000
        indent = -210000
        ppr = f"<a:pPr marL=\"{mar_l}\" indent=\"{indent}\"><a:buChar char=\"•\"/></a:pPr>"
    else:
        ppr = "<a:pPr><a:buNone/></a:pPr>"
    return f"<a:p>{ppr}{run(text, size, color, bold, mono)}</a:p>"


def text_box(
    shape_id: int,
    x: int,
    y: int,
    w: int,
    h: int,
    lines: list[str] | str,
    size: int = 2200,
    color: str = INK,
    bold: bool = False,
    bullet: bool = False,
    mono: bool = False,
    margin: int = 90000,
) -> str:
    if isinstance(lines, str):
        lines = lines.splitlines() or [""]
    paras = "".join(paragraph(item, size, color, bold, bullet, mono) for item in lines)
    return (
        f"<p:sp><p:nvSpPr><p:cNvPr id=\"{shape_id}\" name=\"text {shape_id}\"/>"
        "<p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>"
        f"<a:xfrm><a:off x=\"{x}\" y=\"{y}\"/><a:ext cx=\"{w}\" cy=\"{h}\"/></a:xfrm>"
        "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:noFill/>"
        f"{no_line()}</p:spPr><p:txBody>"
        f"<a:bodyPr wrap=\"square\" lIns=\"{margin}\" tIns=\"{margin}\" rIns=\"{margin}\" bIns=\"{margin}\"/>"
        f"<a:lstStyle/>{paras}</p:txBody></p:sp>"
    )


def title(shape_id: int, text: str, subtitle: str | None = None) -> str:
    parts = [
        text_box(shape_id, emu(0.55), emu(0.28), emu(12.2), emu(0.55), text, 3000, INK, True, margin=0)
    ]
    if subtitle:
        parts.append(
            text_box(shape_id + 1, emu(0.58), emu(0.85), emu(11.8), emu(0.36), subtitle, 1500, MUTED, margin=0)
        )
    return "".join(parts)


def bullet_list(shape_id: int, x: float, y: float, w: float, h: float, items: list[str], size: int = 2050) -> str:
    return text_box(shape_id, emu(x), emu(y), emu(w), emu(h), items, size=size, bullet=True)


class Slide:
    def __init__(self, title_text: str = "", subtitle: str | None = None):
        self.items: list[str] = []
        self.rels: list[tuple[str, str, str]] = []
        self.next_id = 2
        self.media: list[tuple[Path, str]] = []
        self.add(rect(self.id(), 0, 0, SLIDE_W, SLIDE_H, BG, radius="rect"))
        if title_text:
            self.add(title(self.id(), title_text, subtitle))
            self.next_id += 1 if subtitle else 0

    def id(self) -> int:
        value = self.next_id
        self.next_id += 1
        return value

    def add(self, xml: str) -> None:
        self.items.append(xml)

    def add_image(self, path: Path, x: float, y: float, w: float, h: float) -> None:
        ext = path.suffix.lower().lstrip(".")
        rid = f"rId{len(self.rels) + 1}"
        media_name = f"image_{len(self.media) + 1}_{path.name}"
        self.media.append((path, media_name))
        self.rels.append((rid, "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image", f"../media/{media_name}"))
        self.items.append(pic_xml(self.id(), rid, emu(x), emu(y), emu(w), emu(h)))

    def add_image_fit(self, path: Path, x: float, y: float, w: float, h: float) -> None:
        if path.suffix.lower() == ".svg":
            self.add_image(path, x, y, w, h)
            return
        with Image.open(path) as img:
            iw, ih = img.size
        target_w = emu(w)
        target_h = emu(h)
        scale = min(target_w / iw, target_h / ih)
        final_w = iw * scale
        final_h = ih * scale
        final_x = emu(x) + int((target_w - final_w) / 2)
        final_y = emu(y) + int((target_h - final_h) / 2)
        self.add_image_raw(path, final_x, final_y, int(final_w), int(final_h))

    def add_image_raw(self, path: Path, x: int, y: int, w: int, h: int) -> None:
        rid = f"rId{len(self.rels) + 1}"
        media_name = f"image_{len(self.media) + 1}_{path.name}"
        self.media.append((path, media_name))
        self.rels.append((rid, "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image", f"../media/{media_name}"))
        self.items.append(pic_xml(self.id(), rid, x, y, w, h))

    def xml(self) -> str:
        return (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            f"<p:sld {NS}><p:cSld><p:spTree>{group_start()}"
            + "".join(self.items)
            + "</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sld>"
        )

    def rels_xml(self) -> str:
        rows = [
            f'<Relationship Id="{rid}" Type="{typ}" Target="{target}"/>'
            for rid, typ, target in self.rels
        ]
        return (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
            + "".join(rows)
            + "</Relationships>"
        )


def pic_xml(shape_id: int, rid: str, x: int, y: int, w: int, h: int) -> str:
    return (
        f"<p:pic><p:nvPicPr><p:cNvPr id=\"{shape_id}\" name=\"Picture {shape_id}\"/>"
        "<p:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></p:cNvPicPr><p:nvPr/></p:nvPicPr>"
        "<p:blipFill>"
        f"<a:blip r:embed=\"{rid}\"/>"
        "<a:stretch><a:fillRect/></a:stretch></p:blipFill>"
        f"<p:spPr><a:xfrm><a:off x=\"{x}\" y=\"{y}\"/><a:ext cx=\"{w}\" cy=\"{h}\"/></a:xfrm>"
        "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr></p:pic>"
    )


def card(slide: Slide, x: float, y: float, w: float, h: float, heading: str, body: list[str], fill: str, stroke: str) -> None:
    slide.add(rect(slide.id(), emu(x), emu(y), emu(w), emu(h), fill, stroke))
    slide.add(text_box(slide.id(), emu(x + 0.18), emu(y + 0.16), emu(w - 0.36), emu(0.38), heading, 1800, INK, True, margin=0))
    slide.add(bullet_list(slide.id(), x + 0.2, y + 0.62, w - 0.35, h - 0.75, body, 1450))


def build_slides() -> list[Slide]:
    slides: list[Slide] = []

    s = Slide()
    s.add(rect(s.id(), emu(0.65), emu(0.7), emu(12.0), emu(5.9), WHITE, None))
    s.add(text_box(s.id(), emu(0.95), emu(1.2), emu(7.4), emu(1.0), "CSAPP 第 11 章：网络编程", 3600, INK, True, margin=0))
    s.add(text_box(s.id(), emu(0.98), emu(2.15), emu(7.4), emu(0.7), "从客户端-服务器模型到套接字接口", 2100, MUTED, margin=0))
    s.add(bullet_list(s.id(), 1.0, 3.15, 6.2, 1.65, ["研究生课程汇报", "依据 chapter_11/note/web.md，并参考原书第 11 章已覆盖部分", "重点：抽象、协议、系统调用之间的对应关系"], 1750))
    s.add_image_fit(PIC / "image-8.png", 7.45, 1.55, 4.65, 3.45)
    s.add(text_box(s.id(), emu(7.65), emu(5.25), emu(4.2), emu(0.45), "Socket interface 是本章落到程序员视角的核心", 1500, MUTED, margin=0))
    slides.append(s)

    s = Slide("汇报路线", "把网络编程理解为：进程通信问题在网络系统中的分层实现")
    steps = [
        ("1. 应用抽象", "客户端-服务器模型：服务、资源、事务"),
        ("2. 通信介质", "网络作为主机的一种 I/O 设备"),
        ("3. 互联机制", "LAN / WAN 通过路由器组成 internet"),
        ("4. 全球实现", "IP 地址、域名、TCP/IP 与连接"),
        ("5. 编程接口", "socket、connect、bind、listen、accept"),
    ]
    y = 1.55
    colors = [LIGHT_GREEN, LIGHT_BLUE, LIGHT_ORANGE, LIGHT_PURPLE, LIGHT_RED]
    strokes = [GREEN, BLUE, ORANGE, PURPLE, RED]
    for i, ((head, body), fill, stroke) in enumerate(zip(steps, colors, strokes)):
        x = 0.85 + i * 2.45
        s.add(rect(s.id(), emu(x), emu(y), emu(2.05), emu(2.8), fill, stroke))
        s.add(text_box(s.id(), emu(x + 0.16), emu(y + 0.22), emu(1.72), emu(0.45), head, 1750, INK, True, margin=0))
        s.add(text_box(s.id(), emu(x + 0.16), emu(y + 0.9), emu(1.72), emu(1.15), body, 1450, INK, margin=0))
    s.add(text_box(s.id(), emu(1.1), emu(5.4), emu(11.0), emu(0.72), "主线：应用进程并不直接关心底层链路差异，而是通过统一协议和套接字抽象完成可靠通信。", 1900, INK, True, margin=0))
    slides.append(s)

    s = Slide("客户端-服务器编程模型", "网络应用的基本组织方式")
    card(s, 0.8, 1.55, 3.55, 3.85, "服务器进程", ["管理某种资源", "被动等待请求", "解释请求并产生响应"], LIGHT_BLUE, BLUE)
    card(s, 8.98, 1.55, 3.55, 3.85, "客户端进程", ["主动发起服务请求", "等待服务器响应", "一次交互构成一个事务"], LIGHT_GREEN, GREEN)
    s.add(rect(s.id(), emu(4.95), emu(1.95), emu(3.4), emu(2.95), WHITE, MUTED))
    s.add(text_box(s.id(), emu(5.2), emu(2.25), emu(2.9), emu(0.5), "事务 Transaction", 1900, INK, True, margin=0))
    s.add(bullet_list(s.id(), 5.1, 2.95, 3.05, 1.6, ["请求：client → server", "处理：server 操作资源", "响应：server → client"], 1500))
    s.add(text_box(s.id(), emu(1.1), emu(6.02), emu(11.0), emu(0.48), "注意：客户端和服务器是进程，不是机器。一台主机可同时运行多个客户端和服务器。", 1650, RED, True, margin=0))
    slides.append(s)

    s = Slide("网络：主机视角下的 I/O 设备", "CSAPP 的切入点是系统编程，而不是从纯网络工程开始")
    s.add(bullet_list(s.id(), 0.85, 1.48, 5.3, 3.1, [
        "网络对主机来说是一种数据源和数据接收方",
        "适配器接入 I/O 总线，提供到网络的物理接口",
        "以太网帧包含源地址、目的地址、长度和有效载荷",
        "所有适配器都能看到帧，但只有目的主机会实际读取"
    ], 1750))
    s.add_image_fit(PIC / "image-1.png", 6.45, 1.3, 5.6, 3.9)
    s.add(rect(s.id(), emu(0.9), emu(5.45), emu(11.5), emu(0.85), LIGHT_ORANGE, ORANGE))
    s.add(text_box(s.id(), emu(1.15), emu(5.62), emu(11.0), emu(0.42), "汇报抓手：把帧、包、字节流看作不同层次的数据封装，后面 socket 读写的是抽象后的字节流。", 1650, INK, margin=0))
    slides.append(s)

    s = Slide("LAN、WAN、internet 与 Internet", "先区分通用概念，再理解全球 IP 因特网")
    s.add_image_fit(PIC / "network-concepts.svg", 0.75, 1.28, 11.85, 4.95)
    s.add(text_box(s.id(), emu(0.9), emu(6.25), emu(11.4), emu(0.36), "小写 internet 是互联网络的通用概念；大写 Internet/全球 IP 因特网是基于 TCP/IP 的具体实现。", 1500, RED, True, margin=0))
    slides.append(s)

    s = Slide("从以太网段到桥接以太网", "局域网内部的扩展：从一个段到多个段")
    s.add(text_box(s.id(), emu(0.85), emu(1.35), emu(5.25), emu(0.38), "以太网段", 1800, GREEN, True, margin=0))
    s.add_image_fit(PIC / "image-1.png", 0.85, 1.75, 5.4, 3.55)
    s.add(text_box(s.id(), emu(6.55), emu(1.35), emu(5.25), emu(0.38), "桥接以太网", 1800, BLUE, True, margin=0))
    s.add_image_fit(PIC / "image-2.png", 6.55, 1.75, 5.4, 3.55)
    s.add(bullet_list(s.id(), 1.0, 5.62, 11.0, 0.78, [
        "网桥把多个以太网段连接成更大的局域网；路由器进一步把不同 LAN/WAN 连接成互联网络。"
    ], 1650))
    slides.append(s)

    s = Slide("路由器与互联网络的关键思想", "不同链路技术的差异被统一协议隐藏")
    s.add_image_fit(PIC / "image.png", 0.85, 1.35, 5.45, 3.65)
    s.add_image_fit(PIC / "image-4.png", 6.65, 1.35, 5.35, 3.65)
    s.add(bullet_list(s.id(), 0.95, 5.25, 5.65, 1.0, [
        "路由器为其连接的每个网络配置适配器/端口",
        "互联网络可以由不同、不兼容的 LAN 和 WAN 组成"
    ], 1450))
    s.add(bullet_list(s.id(), 6.8, 5.25, 5.1, 1.0, [
        "命名机制：每台主机至少有一个互联网络地址",
        "传送机制：把数据封装成统一格式的包"
    ], 1450))
    slides.append(s)

    s = Slide("全球 IP 因特网：程序员视角", "TCP/IP 将全球主机集合抽象为可寻址、可连接的系统")
    s.add_image_fit(PIC / "image-5.png", 0.8, 1.35, 5.45, 3.75)
    s.add(rect(s.id(), emu(6.65), emu(1.35), emu(5.55), emu(3.75), WHITE, BLUE))
    s.add(bullet_list(s.id(), 6.85, 1.65, 5.1, 2.25, [
        "IP：提供基本命名方法和递送机制",
        "TCP：构建在 IP 之上，提供可靠全双工连接",
        "socket + Unix I/O：应用程序使用的通信接口"
    ], 1700))
    s.add(text_box(s.id(), emu(6.95), emu(4.35), emu(4.9), emu(0.4), "IP 地址 → 域名 → 进程间连接", 1800, RED, True, margin=0))
    s.add(text_box(s.id(), emu(1.0), emu(5.8), emu(11.4), emu(0.5), "研究生汇报时可强调：CSAPP 关注的是系统调用如何把网络协议族暴露给 C 程序。", 1600, MUTED, margin=0))
    slides.append(s)

    s = Slide("IP 地址与网络字节序", "跨主机通信必须处理表示差异")
    s.add(rect(s.id(), emu(0.9), emu(1.35), emu(5.8), emu(4.75), WHITE, GREEN))
    s.add(text_box(s.id(), emu(1.15), emu(1.65), emu(5.25), emu(0.5), "核心结构与转换函数", 1800, INK, True, margin=0))
    code = [
        "struct in_addr {",
        "    uint32_t s_addr;",
        "};",
        "",
        "uint32_t htonl(uint32_t hostlong);",
        "uint16_t htons(uint16_t hostshort);",
        "uint32_t ntohl(uint32_t netlong);",
        "uint16_t ntohs(uint16_t netshort);",
    ]
    s.add(text_box(s.id(), emu(1.15), emu(2.25), emu(5.35), emu(2.7), code, 1280, INK, mono=True, margin=80000))
    s.add(rect(s.id(), emu(7.1), emu(1.35), emu(5.25), emu(4.75), LIGHT_ORANGE, ORANGE))
    s.add(bullet_list(s.id(), 7.3, 1.75, 4.85, 2.1, [
        "TCP/IP 规定网络字节序为大端法",
        "IP 地址结构中存放的地址总是网络字节序",
        "主机字节序可能不同，因此需要 hton/ntoh 系列函数"
    ], 1700))
    s.add(text_box(s.id(), emu(7.35), emu(4.8), emu(4.8), emu(0.48), "host ↔ network", 2200, RED, True, margin=0))
    slides.append(s)

    s = Slide("点分十进制与地址转换", "让二进制地址在人类可读表示和网络表示之间转换")
    s.add(rect(s.id(), emu(0.8), emu(1.42), emu(5.35), emu(3.7), WHITE, BLUE))
    rows = ["128.2.194.242", "=", "0x80 0x02 0xC2 0xF2", "=", "0x8002C2F2"]
    s.add(text_box(s.id(), emu(1.1), emu(1.85), emu(4.8), emu(2.4), rows, 2100, INK, True, margin=0))
    s.add(rect(s.id(), emu(6.65), emu(1.42), emu(5.65), emu(3.7), LIGHT_BLUE, BLUE))
    s.add(text_box(s.id(), emu(6.95), emu(1.78), emu(5.05), emu(0.45), "应用程序 API", 1800, INK, True, margin=0))
    s.add(bullet_list(s.id(), 6.9, 2.35, 5.0, 1.45, [
        "inet_pton：文本表示 → 二进制网络字节序",
        "inet_ntop：二进制网络字节序 → 文本表示"
    ], 1600))
    s.add(text_box(s.id(), emu(1.0), emu(5.72), emu(11.3), emu(0.45), "汇报建议：这里可以现场解释 p= presentation，n=network，帮助听众记忆函数名。", 1550, MUTED, margin=0))
    slides.append(s)

    s = Slide("域名与 DNS", "名字系统把人类可读标识符映射到 IP 地址")
    s.add_image_fit(PIC / "image-6.png", 0.85, 1.3, 6.25, 4.25)
    s.add(rect(s.id(), emu(7.45), emu(1.5), emu(4.65), emu(3.85), WHITE, PURPLE))
    s.add(bullet_list(s.id(), 7.65, 1.85, 4.25, 2.0, [
        "客户端和服务器实际通信使用 IP 地址",
        "域名提供更友好的标识符",
        "DNS 是维护域名到 IP 映射的分布式数据库",
        "localhost 固定映射到回送地址 127.0.0.1"
    ], 1600))
    s.add(text_box(s.id(), emu(0.95), emu(5.95), emu(11.3), emu(0.38), "从程序员视角：域名解析是建立连接前把“名字”转换成“可路由地址”的步骤。", 1550, RED, True, margin=0))
    slides.append(s)

    s = Slide("因特网连接与套接字对", "连接把两个进程端点组织成可靠的全双工字节流")
    s.add_image_fit(PIC / "image-7.png", 0.85, 1.32, 6.2, 4.0)
    s.add(rect(s.id(), emu(7.35), emu(1.45), emu(4.95), emu(3.75), LIGHT_GREEN, GREEN))
    s.add(bullet_list(s.id(), 7.55, 1.8, 4.55, 1.9, [
        "连接是点对点的：连接一对进程",
        "连接是全双工的：数据可双向同时流动",
        "socket 是连接的端点"
    ], 1600))
    s.add(text_box(s.id(), emu(7.65), emu(4.35), emu(4.1), emu(0.38), "(cliaddr:cliport, servaddr:servport)", 1350, RED, True, mono=True, margin=0))
    s.add(text_box(s.id(), emu(1.0), emu(5.95), emu(11.1), emu(0.4), "临时端口由内核分配；服务器端口通常是服务约定的知名端口，例如 Web 服务的 80。", 1550, MUTED, margin=0))
    slides.append(s)

    s = Slide("套接字接口：从抽象到系统调用", "客户端和服务器通过不同的调用序列建立连接")
    s.add_image_fit(PIC / "image-8.png", 0.8, 1.3, 6.2, 4.8)
    s.add(rect(s.id(), emu(7.35), emu(1.35), emu(4.95), emu(4.75), WHITE, BLUE))
    s.add(text_box(s.id(), emu(7.65), emu(1.7), emu(4.4), emu(0.42), "客户端路径", 1700, GREEN, True, margin=0))
    s.add(bullet_list(s.id(), 7.62, 2.15, 4.2, 0.9, ["socket：创建主动套接字", "connect：连接服务器"], 1500))
    s.add(text_box(s.id(), emu(7.65), emu(3.25), emu(4.4), emu(0.42), "服务器路径", 1700, ORANGE, True, margin=0))
    s.add(bullet_list(s.id(), 7.62, 3.7, 4.2, 1.15, ["socket：创建描述符", "bind：绑定服务器地址", "listen：转为监听套接字", "accept：返回已连接描述符"], 1500))
    slides.append(s)

    s = Slide("套接字地址结构与端点语义", "Linux 中 socket 既是通信端点，也是一个描述符")
    code = [
        "struct sockaddr_in {",
        "    sa_family_t sin_family;  // AF_INET",
        "    in_port_t sin_port;      // 16-bit port",
        "    struct in_addr sin_addr; // 32-bit IP",
        "    char sin_zero[8];",
        "};",
        "",
        "struct sockaddr {",
        "    sa_family_t sa_family;",
        "    char sa_data[14];",
        "};",
    ]
    s.add(rect(s.id(), emu(0.85), emu(1.35), emu(5.95), emu(4.85), WHITE, GREEN))
    s.add(text_box(s.id(), emu(1.1), emu(1.65), emu(5.4), emu(3.7), code, 1220, INK, mono=True, margin=80000))
    s.add(rect(s.id(), emu(7.25), emu(1.35), emu(5.0), emu(4.85), LIGHT_RED, RED))
    s.add(bullet_list(s.id(), 7.45, 1.75, 4.6, 2.55, [
        "sockaddr_in 是 IPv4 套接字地址结构",
        "sockaddr 是通用地址结构，供 socket API 统一接收",
        "IP 地址和端口号总是按网络字节序存放"
    ], 1600))
    s.add(text_box(s.id(), emu(7.55), emu(5.0), emu(4.2), emu(0.42), "类型专用结构 → 通用接口参数", 1600, INK, True, margin=0))
    slides.append(s)

    s = Slide("listen / accept：两个描述符的区别", "很多初学者容易把监听描述符和已连接描述符混在一起")
    s.add_image_fit(PIC / "image-9.png", 0.85, 1.28, 5.85, 4.75)
    card(s, 7.15, 1.48, 4.95, 1.65, "监听描述符 listenfd", ["服务器整个生命周期通常只创建一次", "用于接收客户端连接请求"], LIGHT_BLUE, BLUE)
    card(s, 7.15, 3.55, 4.95, 1.75, "已连接描述符 connfd", ["accept 每接收一个连接创建一次", "只在服务某个客户端期间存在"], LIGHT_GREEN, GREEN)
    s.add(text_box(s.id(), emu(7.35), emu(5.85), emu(4.65), emu(0.38), "accept 的返回值是之后读写该客户端的端点。", 1500, RED, True, margin=0))
    slides.append(s)

    s = Slide("总结：本章已覆盖部分的三层理解", "面向汇报答辩的收束")
    card(s, 0.85, 1.45, 3.65, 3.5, "概念层", ["客户端-服务器模型", "事务、资源、进程角色", "网络是 I/O 设备"], LIGHT_GREEN, GREEN)
    card(s, 4.85, 1.45, 3.65, 3.5, "协议层", ["LAN/WAN/internet", "IP 命名与递送", "TCP 可靠全双工连接"], LIGHT_BLUE, BLUE)
    card(s, 8.85, 1.45, 3.65, 3.5, "接口层", ["套接字地址结构", "客户端 connect", "服务器 bind/listen/accept"], LIGHT_ORANGE, ORANGE)
    s.add(rect(s.id(), emu(1.1), emu(5.55), emu(11.1), emu(0.82), WHITE, RED))
    s.add(text_box(s.id(), emu(1.35), emu(5.75), emu(10.6), emu(0.36), "一句话主旨：socket 把全球 IP 因特网上的进程通信抽象成可用 Unix I/O 操作的连接端点。", 1650, RED, True, margin=0))
    slides.append(s)

    return slides


def content_types(slide_count: int, image_exts: set[str]) -> str:
    defaults = [
        '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>',
        '<Default Extension="xml" ContentType="application/xml"/>',
    ]
    if "png" in image_exts:
        defaults.append('<Default Extension="png" ContentType="image/png"/>')
    if "svg" in image_exts:
        defaults.append('<Default Extension="svg" ContentType="image/svg+xml"/>')
    overrides = [
        '<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>',
        '<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>',
        '<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>',
        '<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>',
    ]
    for i in range(1, slide_count + 1):
        overrides.append(f'<Override PartName="/ppt/slides/slide{i}.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>')
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
        + "".join(defaults)
        + "".join(overrides)
        + "</Types>"
    )


def root_rels() -> str:
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="ppt/presentation.xml"/>'
        "</Relationships>"
    )


def presentation_xml(slide_count: int) -> str:
    slide_ids = "".join(
        f'<p:sldId id="{255 + i}" r:id="rId{i + 1}"/>' for i in range(1, slide_count + 1)
    )
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f"<p:presentation {NS}>"
        '<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId1"/></p:sldMasterIdLst>'
        f"<p:sldIdLst>{slide_ids}</p:sldIdLst>"
        f'<p:sldSz cx="{SLIDE_W}" cy="{SLIDE_H}" type="wide"/>'
        '<p:notesSz cx="6858000" cy="9144000"/>'
        "</p:presentation>"
    )


def presentation_rels(slide_count: int) -> str:
    rels = ['<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>']
    for i in range(1, slide_count + 1):
        rels.append(f'<Relationship Id="rId{i + 1}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" Target="slides/slide{i}.xml"/>')
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        + "".join(rels)
        + "</Relationships>"
    )


def slide_master_xml() -> str:
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f"<p:sldMaster {NS}><p:cSld><p:bg><p:bgPr>{solid_fill(BG)}</p:bgPr></p:bg>"
        f"<p:spTree>{group_start()}</p:spTree></p:cSld>"
        '<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/>'
        '<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>'
        "<p:txStyles><p:titleStyle/><p:bodyStyle/><p:otherStyle/></p:txStyles></p:sldMaster>"
    )


def slide_master_rels() -> str:
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>'
        '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="../theme/theme1.xml"/>'
        "</Relationships>"
    )


def slide_layout_xml() -> str:
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f"<p:sldLayout {NS} type=\"blank\" preserve=\"1\"><p:cSld name=\"Blank\"><p:spTree>{group_start()}</p:spTree></p:cSld>"
        "<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>"
    )


def slide_layout_rels() -> str:
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="../slideMasters/slideMaster1.xml"/>'
        "</Relationships>"
    )


def theme_xml() -> str:
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="CSAPP Report">'
        '<a:themeElements><a:clrScheme name="CSAPP">'
        '<a:dk1><a:srgbClr val="172033"/></a:dk1><a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>'
        '<a:dk2><a:srgbClr val="52627A"/></a:dk2><a:lt2><a:srgbClr val="F7F8FB"/></a:lt2>'
        '<a:accent1><a:srgbClr val="2D5F9A"/></a:accent1><a:accent2><a:srgbClr val="2F7D4C"/></a:accent2>'
        '<a:accent3><a:srgbClr val="A36C16"/></a:accent3><a:accent4><a:srgbClr val="6D4CA5"/></a:accent4>'
        '<a:accent5><a:srgbClr val="B0493F"/></a:accent5><a:accent6><a:srgbClr val="52627A"/></a:accent6>'
        '<a:hlink><a:srgbClr val="2D5F9A"/></a:hlink><a:folHlink><a:srgbClr val="6D4CA5"/></a:folHlink>'
        '</a:clrScheme><a:fontScheme name="Microsoft YaHei"><a:majorFont><a:latin typeface="Microsoft YaHei"/><a:ea typeface="Microsoft YaHei"/></a:majorFont>'
        '<a:minorFont><a:latin typeface="Microsoft YaHei"/><a:ea typeface="Microsoft YaHei"/></a:minorFont></a:fontScheme>'
        '<a:fmtScheme name="simple"><a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>'
        '<a:lnStyleLst><a:ln w="16000"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>'
        '<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>'
        '<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst></a:fmtScheme>'
        '</a:themeElements></a:theme>'
    )


def main() -> None:
    slides = build_slides()
    image_exts: set[str] = set()
    media_files: dict[str, Path] = {}
    for slide in slides:
        for path, media_name in slide.media:
            image_exts.add(path.suffix.lower().lstrip("."))
            media_files[media_name] = path

    with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", content_types(len(slides), image_exts))
        z.writestr("_rels/.rels", root_rels())
        z.writestr("ppt/presentation.xml", presentation_xml(len(slides)))
        z.writestr("ppt/_rels/presentation.xml.rels", presentation_rels(len(slides)))
        z.writestr("ppt/slideMasters/slideMaster1.xml", slide_master_xml())
        z.writestr("ppt/slideMasters/_rels/slideMaster1.xml.rels", slide_master_rels())
        z.writestr("ppt/slideLayouts/slideLayout1.xml", slide_layout_xml())
        z.writestr("ppt/slideLayouts/_rels/slideLayout1.xml.rels", slide_layout_rels())
        z.writestr("ppt/theme/theme1.xml", theme_xml())
        for i, slide in enumerate(slides, 1):
            z.writestr(f"ppt/slides/slide{i}.xml", slide.xml())
            z.writestr(f"ppt/slides/_rels/slide{i}.xml.rels", slide.rels_xml())
        for media_name, path in media_files.items():
            z.write(path, posixpath.join("ppt/media", media_name))

    print(f"Generated {OUT}")


if __name__ == "__main__":
    main()
