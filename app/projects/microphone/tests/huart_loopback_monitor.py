#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""监听本工程 HUART 帧式回环的 COM9 日志。"""

import argparse
import serial
import time


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="COM9")
    ap.add_argument("--baud", type=int, default=1500000)
    ap.add_argument("--timeout", type=int, default=300)
    args = ap.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=0.2)
    print("监听 %s@%d，最长 %ds..." % (args.port, args.baud, args.timeout), flush=True)
    end = time.time() + args.timeout
    buf = b""
    while time.time() < end:
        data = ser.read(4096)
        if not data:
            continue
        buf += data
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            text = line.decode("utf-8", "replace").rstrip("\r")
            print(text, flush=True)
            if "=== HUART Frame Loopback Done ===" in text:
                ser.close()
                return
    ser.close()
    print("!! 超时，未收到 Done", flush=True)


if __name__ == "__main__":
    main()
