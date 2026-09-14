#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""跨自动复位采集 UART2 回环的完整一轮日志。"""

import argparse
import serial
import time


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="COM9")
    ap.add_argument("--baud", type=int, default=1500000)
    ap.add_argument("--timeout", type=int, default=150)
    args = ap.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=0.2)
    print("监听 %s@%d，跨自动复位等待完整 UART2 回环轮次..." %
          (args.port, args.baud), flush=True)
    end = time.time() + args.timeout
    buf = b""
    in_cycle = False
    while time.time() < end:
        data = ser.read(4096)
        if not data:
            continue
        buf += data
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            text = line.decode("utf-8", "replace").rstrip("\r")
            print(text, flush=True)
            if "=== Serial Max Baud Loopback (UART2) Start ===" in text:
                in_cycle = True
            elif in_cycle and "=== Serial Max Baud Loopback Done ===" in text:
                ser.close()
                return
    ser.close()
    print("!! 超时，未见完整轮次", flush=True)


if __name__ == "__main__":
    main()
