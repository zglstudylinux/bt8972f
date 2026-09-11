#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
serial_max_baud_test.py - 普通串口(UART2)/高速串口(HUART) 最大波特率统一测试编排脚本

与固件 modules/test/serial_max_baud_test.c 配套，四个测试项方法完全一致：
  每档波特率：PC 发递增码流 -> 板端校验并打印 [rx] 统计 -> 板回传 -> PC 比对（TX 方向）
脚本监听调试口(COM9,1.5M)的板端提示自动换档换波特率，全程无需人工干预。

用法示例：
  python serial_max_baud_test.py --debug-com COM9 --data-com COM17 --periph uart2  --adapter ch340
  python serial_max_baud_test.py --debug-com COM9 --data-com COM19 --periph huart --adapter cp210x
  python serial_max_baud_test.py ... --bauds 2000000,3000000   # 只关注部分档位(其余档自动打点跳过)

节流 profile（与固件结构约束对齐，详见 docs/peripheral/serial_max_baud_test_plan.md）：
  huart: 512B 块 + 10ms 间隔（规避 HUART 库非环形模式连续块缺陷，块长须=固件 DMA 块长）
  uart2: 64B 块 + 8ms 间隔（适配 UART2 128B 环形缓冲 + 主循环轮询排水速率）
"""

import argparse
import re
import sys
import time
import os
import threading

import serial
from serial.tools import list_ports

DEBUG_BAUD = 1500000

PROMPT_RE = re.compile(r"\[Baud (\d+)\] waiting PC data\.\.\.")
RX_RE = re.compile(r"\[rx\] got=(\d+) err=(\d+) first_err=(-?\d+) drop=(\d+)")
ECHO_RE = re.compile(r"\[echo\] (\d+) bytes sent")
DONE_RE = re.compile(r"=== Serial Max Baud Test Done ===")

PROFILES = {
    "huart": {"chunk": 512, "delay_ms": 10},
    "uart2": {"chunk": 64, "delay_ms": 8},
}


def list_com_ports():
    ports = list(list_ports.comports())
    print("可用串口：")
    for p in ports:
        print("  %-8s %s" % (p.device, p.description))
    if not ports:
        print("  (无)")


class DebugReader:
    """后台线程读取调试口并按行分发（板端提示同步的唯一依据）"""

    def __init__(self, port, baud):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.lines = []
        self._buf = b""
        self._stop = False
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _run(self):
        while not self._stop:
            try:
                data = self.ser.read(256)
            except Exception:
                break
            if data:
                self._buf += data
                while b"\n" in self._buf:
                    line, self._buf = self._buf.split(b"\n", 1)
                    self.lines.append(line.decode("utf-8", "replace").rstrip("\r"))

    def next_line(self, timeout):
        """取一行，超时返回 None；取走即从队列移除"""
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self.lines:
                return self.lines.pop(0)
            time.sleep(0.01)
        return None

    def wait_re(self, regex, timeout):
        """等待匹配 regex 的行，返回 match；超时返回 None（不匹配的行打印出来）"""
        deadline = time.time() + timeout
        while time.time() < deadline:
            line = self.next_line(0.2)
            if line is None:
                continue
            m = regex.search(line)
            if m:
                return m
            if line.strip():
                print("    [板] " + line)
        return None

    def close(self):
        self._stop = True
        time.sleep(0.15)
        self.ser.close()


def send_paced(ser, data, chunk, delay_ms, log_every=200):
    """按 块大小+固定间隔 节流发送（Windows sleep 只会偏长不会偏短，方向安全）"""
    total = len(data)
    sent = 0
    n_chunks = (total + chunk - 1) // chunk
    idx = 0
    while sent < total:
        n = min(chunk, total - sent)
        ser.write(data[sent:sent + n])
        sent += n
        idx += 1
        if delay_ms:
            time.sleep(delay_ms / 1000.0)
        if idx % log_every == 0:
            print("      已发送 %d/%d 字节 (%d/%d 块)" % (sent, total, idx, n_chunks))
    print("      发送完成 %d 字节" % sent)


def read_echo(ser, expect_len, dbg, hard_timeout=60):
    """收回传数据：收满 expect_len 且线路空闲 300ms（或见到 [echo] 后空闲）即结束"""
    buf = bytearray()
    last_data = time.time()
    hard_deadline = time.time() + hard_timeout
    while time.time() < hard_deadline:
        n = ser.in_waiting
        if n:
            buf += ser.read(n)
            last_data = time.time()
            if len(buf) >= expect_len and (time.time() - last_data) >= 0.05 and ser.in_waiting == 0:
                # 已够数，再等一小段确认没有尾随字节
                time.sleep(0.2)
                if ser.in_waiting:
                    continue
                break
        else:
            if buf and (time.time() - last_data) >= 0.3:
                break
            if len(buf) >= expect_len and (time.time() - last_data) >= 0.3:
                break
            time.sleep(0.02)
    return bytes(buf)


def compare_echo(sent, echoed):
    """与发送文件逐字节比对，返回 (ok, match_len, first_diff, echo_len)"""
    echo_len = len(echoed)
    match_len = 0
    first_diff = -1
    for i in range(min(len(sent), echo_len)):
        if sent[i] == echoed[i]:
            match_len += 1
        else:
            if first_diff < 0:
                first_diff = i
            break
    ok = (first_diff < 0) and (echo_len == len(sent))
    return ok, match_len, first_diff, echo_len


def poke_advance(ser):
    """打 1 字节唤醒板端跳过当前档（板端收到数据->空闲超时->打印统计->切下一档）"""
    ser.write(b"\x00")
    time.sleep(0.1)


def main():
    ap = argparse.ArgumentParser(description="普通/高速串口最大波特率统一测试")
    ap.add_argument("--debug-com", default="COM9", help="调试口(UART0/PB3,1.5M)，默认 COM9")
    ap.add_argument("--data-com", required=True, help="数据口(适配器接 PE7/PB1)")
    ap.add_argument("--periph", choices=["uart2", "huart"], required=True)
    ap.add_argument("--adapter", choices=["ch340", "cp210x"], default="ch340", help="适配器型号(仅用于结果标注)")
    ap.add_argument("--file", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "huart_dual_inc.bin"),
                    help="递增码流文件(默认 tests/huart_dual_inc.bin, 50760B)")
    ap.add_argument("--chunk", type=int, default=None, help="覆盖 profile 块大小")
    ap.add_argument("--delay-ms", type=float, default=None, help="覆盖 profile 块间隔")
    ap.add_argument("--bauds", default=None, help="只测这些档位(逗号分隔)，其余档自动打点跳过")
    ap.add_argument("--start-baud", type=int, default=None,
                    help="板子已停在某一档等待数据时，直接从该档开始(跳过横幅/首条提示等待)")
    ap.add_argument("--out", default=None, help="结果 markdown 追加文件(默认 tests/serial_max_baud_results.md)")
    args = ap.parse_args()

    if args.data_com.lower() == "list":
        list_com_ports()
        return

    profile = dict(PROFILES[args.periph])
    if args.chunk:
        profile["chunk"] = args.chunk
    if args.delay_ms is not None:
        profile["delay_ms"] = args.delay_ms

    only_bauds = None
    if args.bauds:
        only_bauds = set(int(x) for x in args.bauds.split(","))

    data = open(args.file, "rb").read()
    expect = len(data)

    out_path = args.out or os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                        "serial_max_baud_results.md")

    print("=" * 72)
    print("外设=%s 适配器=%s 数据口=%s 调试口=%s@%d" %
          (args.periph.upper(), args.adapter, args.data_com, args.debug_com, DEBUG_BAUD))
    print("码流=%s (%d B)  节流=%dB/%gms" % (args.file, expect, profile["chunk"], profile["delay_ms"]))
    print("=" * 72)

    hdr = "| 波特率 | 适配器 | got | err | drop | 回传比对 | RX判定 | TX判定 |"
    sep = "| --- | --- | --- | --- | --- | --- | --- | --- |"
    print(hdr)

    with open(out_path, "a", encoding="utf-8") as f:
        f.write("\n## run: periph=%s adapter=%s data=%s chunk=%d delay=%gms %s\n%s\n%s\n" %
                (args.periph, args.adapter, args.data_com, profile["chunk"],
                 profile["delay_ms"], time.strftime("%Y-%m-%d %H:%M:%S"), hdr, sep))

    rows = []
    results = []   # (baud, (rx_pass, tx_pass))，用于结尾汇总

    def emit(row, baud, rx_pass, tx_pass):
        rows.append(row)
        results.append((baud, (rx_pass, tx_pass)))
        print(" ".join(row))
        with open(out_path, "a", encoding="utf-8") as f:
            f.write(" ".join(row) + "\n")

    dbg = DebugReader(args.debug_com, DEBUG_BAUD)
    try:
        dser = serial.Serial(args.data_com, 115200, timeout=0.05, write_timeout=2)
    except Exception as e:
        print("打开数据口失败: %s" % e)
        dbg.close()
        sys.exit(1)

    try:
        banner = None
        pending_baud = args.start_baud
        if pending_baud:
            print("跳过横幅/首条提示等待，直接从板子当前等待的档位 %d 开始" % pending_baud)
        else:
            # 等待起始横幅（板子复位后打印；错过也不影响，直接等 Baud 提示）
            print("等待板端横幅...（请按一下板子复位键，之后不要再动接线）")
            banner = dbg.wait_re(re.compile(r"=== Serial Max Baud Test \((\w+)\) Start ==="), 300)
            if banner:
                print("板端横幅: %s" % banner.group(0))
                dbg.lines.clear()   # 丢弃复位前的陈旧输出，保证从干净状态同步
            else:
                print("!! 90s 未等到横幅（板子可能不是本测试固件），仍继续尝试……")
            if banner and banner.group(1) != args.periph.upper():
                print("!! 固件外设(%s)与 --periph(%s) 不一致，请确认烧录的固件" % (banner.group(1), args.periph))

        while True:
            m = None
            done = False
            if pending_baud:
                baud = pending_baud
                pending_baud = None
            else:
                # 等待下一档提示或 Done 横幅（二选一，避免吞掉彼此）
                deadline = time.time() + 60
                while time.time() < deadline:
                    line = dbg.next_line(0.3)
                    if line is None:
                        continue
                    if DONE_RE.search(line):
                        done = True
                        break
                    mp = PROMPT_RE.search(line)
                    if mp:
                        m = mp
                        break
                    if line.strip():
                        print("    [板] " + line)
                if done:
                    break
                if not m:
                    if banner:
                        print("!! 板端已打印 Start 横幅但 60s 内无 [Baud] 提示：固件在首个外设初始化处挂起，")
                        print("!! 请再按一次复位重试；若复现，需要排查固件（注意启动日志是否有 LVD 低压复位）")
                    else:
                        print("!! 60s 内未收到板端 [Baud] 提示或 Done 横幅，退出。请检查调试口接线/是否已烧测试固件")
                    break
                baud = int(m.group(1))

            baud_ok = True
            try:
                dser.baudrate = baud
            except Exception as e:
                baud_ok = False
                print("!! 数据口不支持 %d: %s（打点跳过该档，该档记适配器不支持）" % (baud, e))

            if not baud_ok or (only_bauds is not None and baud not in only_bauds):
                if baud_ok:
                    print("[跳过] %d (不在 --bauds 列表)" % baud)
                poke_advance(dser)
                dbg.wait_re(RX_RE, 10)
                dbg.wait_re(ECHO_RE, 10)
                if not baud_ok:
                    emit(["| %d | %s | 适配器不支持该波特率，档位无效 | | | | - | - |" %
                          (baud, args.adapter)], baud, False, False)
                continue

            print("[Baud %d] 发送中 (chunk=%d delay=%gms)..." % (baud, profile["chunk"], profile["delay_ms"]))
            send_paced(dser, data, profile["chunk"], profile["delay_ms"])

            rxm = dbg.wait_re(RX_RE, 90)
            if not rxm:
                print("!! 未收到板端 [rx] 统计")
                break
            got, err, first_err, drop = (int(rxm.group(i)) for i in (1, 2, 3, 4))

            echoed = read_echo(dser, got if got <= expect else expect, dbg)
            dbg.wait_re(ECHO_RE, 10)
            ok, match_len, first_diff, echo_len = compare_echo(data[:got] if got <= expect else data, echoed)

            rx_pass = (got == expect and err == 0 and drop == 0)
            # 重建回传：TX 判定只看回传内容是否与板端声称长度(got)的码流一致，
            # got<expect 属于 RX 方向的损失，不应拖累 TX 判定
            tx_pass = ok and got > 0
            note = ""
            if first_diff >= 0:
                note = " (首差异@%d)" % first_diff
            elif echo_len != got:
                note = " (回传 %d/%d 字节)" % (echo_len, got)

            emit(["| %d | %s | %d | %d | %d | %s | %s | %s |" %
                  (baud, args.adapter, got, err, drop,
                   "OK" if ok else "DIFF" + note,
                   "PASS" if rx_pass else "FAIL",
                   "PASS" if tx_pass else "FAIL")],
                 baud, rx_pass, tx_pass)

        print(sep)
        print("完成。结果已追加到 %s" % out_path)
    finally:
        dser.close()
        dbg.close()

    if results:
        def max_ok(idx):
            vals = [b for b, flags in results if flags[idx]]
            return max(vals) if vals else None
        print("RX 最大无错档: %s" % (max_ok(0),))
        print("TX 最大无错档: %s" % (max_ok(1),))
    else:
        print("（无有效结果行）")


if __name__ == "__main__":
    main()
