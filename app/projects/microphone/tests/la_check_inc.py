#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""TX-LA 递增码流校验：解析逻辑分析仪导出的 Async Serial 解码 CSV。

与固件 serial_max_baud_test.c 的 TX-LA 模式配套：板子自发 data[i]=(base+i)&0xFF
连续递增码流，本脚本检查解码字节流的 +1 连续性——零断裂即板端 TX 位级无误码。

断裂归因（看断裂点的相邻字节时间差）：
    时间差 >> 字节周期  -> 丢字节（解码空洞/线路丢码）
    时间差 ≈ 字节周期   -> 位级误码（值错，无时间空洞）

用法：
    python la_check_inc.py <decoded.csv> [字节周期上限倍数=5]

CSV 格式与 la_validate_frames.py 兼容（Automation API 与 MCP 两种导出）。
"""
import csv
import sys


def iso_to_sec(s):
    t, _, _tz = s.partition('+')
    hh, mm, ss = t.split('T')[1].split(':')
    frac = 0.0
    if '.' in ss:
        ss, f = ss.split('.')
        frac = float('0.' + f)
    return int(hh) * 3600 + int(mm) * 60 + int(ss) + frac


def parse_time(s):
    return float(s) if not s.startswith('20') else iso_to_sec(s)


def parse_byte(s):
    s = s.strip().strip('"')
    if s.startswith(('0x', '0X')):
        return int(s, 16)
    if s.startswith(('0b', '0B')):
        return int(s, 2)
    if s.lstrip('-').isdigit():
        return int(s) & 0xFF
    return ord(s[0]) if s else 0


def main(csv_path, gap_factor=5.0):
    rows = []
    with open(csv_path, newline='', encoding='utf-8') as f:
        for r in csv.DictReader(f):
            if r['type'] == 'data':
                rows.append((parse_time(r['start_time']), parse_byte(r['data'])))

    n = len(rows)
    print("data bytes: %d" % n)
    if n < 16:
        print("数据不足，无法校验")
        return 1
    span = rows[-1][0] - rows[0][0]
    print("span      : %.6f s (平均 %.0f B/s)" % (span, (n - 1) / span if span > 0 else 0))

    gaps = sorted(rows[i + 1][0] - rows[i][0] for i in range(n - 1))
    med = gaps[len(gaps) // 2]
    print("byte gap  : min=%.2fus median=%.2fus max=%.2fus" %
          (gaps[0] * 1e6, med * 1e6, gaps[-1] * 1e6))
    if med > 0:
        print("implied   : 连续字节率 ~%.0f B/s（HUART 背靠背块时 ≈ 波特率/10；UART2 为结构吞吐）"
              % (1.0 / med))

    breaks = []
    big_gap_thresh = med * gap_factor
    for i in range(n - 1):
        if rows[i + 1][1] != ((rows[i][1] + 1) & 0xFF):
            dt = rows[i + 1][0] - rows[i][0]
            kind = "丢字节/空洞" if dt > big_gap_thresh else "位级误码"
            breaks.append((i, rows[i][1], rows[i + 1][1], dt, kind))

    print("\nbreaks    : %d" % len(breaks))
    for b in breaks[:10]:
        print("  [%d] 0x%02X -> 0x%02X  dt=%.2fus  %s" % b)

    verdict = "PASS：解码流 +1 连续，零断裂，板端 TX 位级无误码" if not breaks else \
              "FAIL：存在 %d 处断裂" % len(breaks)
    print("\n" + verdict)
    return 0 if not breaks else 1


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    factor = float(sys.argv[2]) if len(sys.argv) > 2 else 5.0
    sys.exit(main(sys.argv[1], factor))
