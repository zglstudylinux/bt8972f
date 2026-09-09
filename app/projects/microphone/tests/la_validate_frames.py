#!/usr/bin/env python
"""UART2 TX 测试帧校验：解析逻辑分析仪导出的 Async Serial 解码 CSV。

帧格式与固件 `UART2_COM_TX_TEST_EN=1` 一致（72 字节）：
    55 AA 5A A5 | u16 LE 序号 | 64 字节 payload (seq+i)&0xFF | CRC-16/MODBUS LE

用法：
    python la_validate_frames.py <decoded.csv>

兼容两种导出来源：
    - Saleae Automation API（logic2-automation）export_data_table：
      start_time 为 ISO8601 时间戳，data 列形如 0x55；
    - Saleae Logic 2 官方 MCP server（127.0.0.1:10530）export_data_table_csv：
      start_time 为相对秒，data 列随 radixType 而定（3=0x55 / 2=十进制 / 1=0b / 默认 ASCII 字符）。

数据列自动识别（0x/0b/十进制/单字符）。校验项：帧同步、CRC16、payload、
序号连续性、帧周期、帧内字节间隔。全部通过返回退出码 0，否则 1。
"""

import csv
import sys

MAGIC = bytes([0x55, 0xAA, 0x5A, 0xA5])
FRAME_LEN = 72  # 4 magic + 2 seq + 64 payload + 2 crc16


def crc16_modbus(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc


def iso_to_sec(s):
    """'2026-09-09T07:54:46.795747600+00:00' -> float seconds（仅用于同列内的相对运算）。"""
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
    return ord(s[0]) if s else 0  # ASCII 导出（有损，不建议用于校验）


def main(csv_path):
    rows, types = [], {}
    with open(csv_path, newline='', encoding='utf-8') as f:
        for r in csv.DictReader(f):
            types[r['type']] = types.get(r['type'], 0) + 1
            if r['type'] == 'data':
                rows.append((parse_time(r['start_time']), parse_byte(r['data'])))

    print(f"rows by type : {types}")
    print(f"data bytes   : {len(rows)}")
    if len(rows) < 2:
        print("数据不足，无法校验")
        return 1
    print(f"span of data : {rows[-1][0] - rows[0][0]:.6f} s")

    stream = [b for _, b in rows]
    n = len(stream)

    frames = []  # (idx, seq, crc_ok, payload_ok, crc_calc, crc_recv)
    i = 0
    while i <= n - FRAME_LEN:
        if stream[i:i + 4] == list(MAGIC):
            fr = stream[i:i + FRAME_LEN]
            seq = fr[4] | (fr[5] << 8)
            payload_ok = all(fr[6 + k] == ((seq + k) & 0xFF) for k in range(64))
            crc_recv = fr[70] | (fr[71] << 8)
            frames.append((i, seq, crc16_modbus(fr[:70]) == crc_recv, payload_ok,
                           crc16_modbus(fr[:70]), crc_recv))
            i += FRAME_LEN
        else:
            i += 1

    print(f"\nframes found : {len(frames)}")
    bad = [f for f in frames if not (f[2] and f[3])]
    print(f"frames passed: {len(frames) - len(bad)} (CRC+payload)")
    print(f"frames failed: {len(bad)}")
    for f in bad[:10]:
        print(f"  FAIL stream[{f[0]}] seq={f[1]} crc_ok={f[2]} payload_ok={f[3]} "
              f"calc=0x{f[4]:04X} recv=0x{f[5]:04X}")

    gaps = [(a[1], b[1], (b[1] - a[1]) & 0xFFFF) for a, b in zip(frames, frames[1:])
            if (b[1] - a[1]) & 0xFFFF != 1]
    print(f"seq discontinuities: {len(gaps)}")
    for g in gaps[:10]:
        print(f"  seq {g[0]} -> {g[1]} (delta {g[2]})")

    periods = [(rows[b[0]][0] - rows[a[0]][0]) * 1000.0 for a, b in zip(frames, frames[1:])]
    if periods:
        print(f"\nframe period (ms): min={min(periods):.3f} max={max(periods):.3f} "
              f"avg={sum(periods) / len(periods):.3f}")

    gi = []
    for f in frames:
        base = f[0]
        for k in range(FRAME_LEN - 1):
            gi.append((rows[base + k + 1][0] - rows[base + k][0]) * 1e6)
    if gi:
        gi.sort()
        print(f"inter-byte gap (us): min={gi[0]:.3f} median={gi[len(gi) // 2]:.3f} "
              f"p95={gi[int(len(gi) * 0.95)]:.3f} max={gi[-1]:.3f}")

    covered = [False] * n
    for f in frames:
        for k in range(FRAME_LEN):
            covered[f[0] + k] = True
    orphan = [j for j in range(n) if not covered[j]]
    print(f"\nbytes outside frames: {len(orphan)}")
    for s in orphan[:16]:
        j = 0
        print(f"  stream[{s}] = {stream[s]:02X}", end='')
        j += 1
        if j % 8 == 0:
            print()
    if orphan:
        print()

    return 0 if not bad and not gaps else 1


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1]))
