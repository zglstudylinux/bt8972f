#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""TX-LA 逐档采集驱动：盯 COM9 的 [TXLA] arm 提示，在静默窗口内启动 LA 采集，
解码导出后调用 la_check_inc 校验递增连续性。单进程完成 等待->采集->导出->校验。

用法：
    python tx_la_capture.py --bauds 2000000,3000000 [--channel 1] [--debug-com COM9]

依赖同目录 logic2_mcp_client.py（JSON-RPC over HTTP 127.0.0.1:10530）与 la_check_inc.py。
"""
import argparse
import json
import os
import re
import sys
import tempfile
import time

import serial

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from logic2_mcp_client import tool_call          # noqa: E402
import la_check_inc                              # noqa: E402

ARM_RE_TMPL = r"\[TXLA\]\[Baud (\d+)\] arm in"
# 传输安全策略：单次采集 <=~58M 样点（原代 Logic USB 上限 ~60M）。
# 板子 arm 后 1.5s 才开始 3s TX：延迟 1.8s 启动采集（TX 已开始 0.3s），抓 2.4s 纯 TX。
START_DELAY_S = 1.8
DUR_S = 2.4


def sample_rate_for(baud):
    # 传输安全（原代 Logic 单次 <=~58M 样点）与解码质量折中；设备只认固定档：
    # 2M->8M, 3M->12M, 4M->12M, 6M->16M(2.67样点/位), 8M/9.5M->24M(短窗口)
    if baud <= 2000000:
        return 8000000
    if baud <= 4000000:
        return 12000000
    if baud <= 6000000:
        return 16000000
    return 24000000


def run_one(ser, baud, channel, out_csv, retries=5):
    """等待目标档 arm 提示后完成一次采集+校验（USB ReadTimeout 自动重试）。"""
    for attempt in range(retries + 1):
        verdict = _capture_once(ser, baud, channel, out_csv)
        if not verdict.startswith("wait_capture 失败") or attempt == retries:
            return verdict if attempt == retries else verdict
        print("  采集传输超时，%.1fs 后重试（第 %d/%d 次）..." %
              (1.5, attempt + 1, retries))
        time.sleep(1.5)
    return verdict


def _capture_once(ser, baud, channel, out_csv):
    """等待目标档 arm 提示后完成一次采集+校验，返回判定字符串。"""
    arm_re = re.compile(ARM_RE_TMPL)
    buf = b""
    print("[%s] 等待 arm 提示（板子循环梯子中，错过等下一圈）..." % baud)
    deadline = time.time() + 60
    while time.time() < deadline:
        chunk = ser.read(512)
        if chunk:
            buf += chunk
            text = buf.decode("utf-8", "replace")
            matches = list(arm_re.finditer(text))
            if matches and int(matches[-1].group(1)) == baud:
                break
            if matches:
                buf = b""        # 其他档位的 arm，清掉避免重复处理
        else:
            time.sleep(0.005)
    else:
        return "超时未等到 arm 提示"

    # 静默窗口 1.5s，等 TX 开始后再启动采集（压缩数据量过 USB 上限）
    time.sleep(START_DELAY_S)
    t0 = time.time()
    res = tool_call("start_capture", {
        "logicDeviceConfiguration": {
            "logicChannels": {"digitalChannels": [channel]},
            "digitalSampleRate": sample_rate_for(baud),
        },
        "captureConfiguration": {"timedCaptureMode": {"durationSeconds": DUR_S}},
    })
    if res["isError"]:
        return "start_capture 失败: %s" % json.dumps(res["data"])[:200]
    cap = res["data"]
    cap_id = cap.get("captureId") if isinstance(cap, dict) else None
    if cap_id is None:
        m = re.search(r'"captureId"\s*:\s*(\d+)', json.dumps(cap))
        cap_id = int(m.group(1)) if m else None
    if cap_id is None:
        return "未解析到 captureId: %s" % json.dumps(cap)[:200]
    print("  采集已启动 (延迟 %.2fs, captureId=%s, %dMS/s)" %
          (time.time() - t0, cap_id, sample_rate_for(baud) // 1000000))

    res = tool_call("wait_capture", {"captureId": cap_id})
    if res["isError"]:
        return "wait_capture 失败: %s" % json.dumps(res["data"])[:200]

    res = tool_call("add_analyzer", {
        "captureId": cap_id,
        "analyzerName": "Async Serial",
        "analyzerLabel": "uart_tx_%d" % baud,
        "settings": {
            "Input Channel": {"numberValue": channel},
            "Bit Rate (Bits/s)": {"numberValue": baud},
        },
    })
    if res["isError"]:
        return "add_analyzer 失败: %s" % json.dumps(res["data"])[:200]
    ana = json.dumps(res["data"])
    m = re.search(r'"analyzerId"\s*:\s*(\d+)', ana)
    if not m:
        return "未解析到 analyzerId: %s" % ana[:200]
    ana_id = int(m.group(1))

    res = tool_call("export_data_table_csv", {
        "captureId": cap_id,
        "filepath": out_csv.replace("\\", "/"),
        "analyzers": [{"analyzerId": ana_id, "radixType": 3}],
        "iso8601Timestamp": False,
    })
    if res["isError"] or not os.path.exists(out_csv):
        return "导出 CSV 失败: %s" % json.dumps(res["data"])[:200]

    print("  解码 CSV: %s" % out_csv)
    print("  ---- 递增连续性校验 ----")
    rc = la_check_inc.main(out_csv)
    return "PASS" if rc == 0 else "FAIL"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bauds", required=True, help="逗号分隔的波特率列表（按梯子顺序采集）")
    ap.add_argument("--channel", type=int, default=1, help="LA 数字通道，默认 1 (D1)")
    ap.add_argument("--debug-com", default="COM9")
    args = ap.parse_args()

    bauds = [int(x) for x in args.bauds.split(",")]
    ser = serial.Serial(args.debug_com, 1500000, timeout=0.05)
    results = []
    try:
        for baud in bauds:
            out_csv = os.path.join(tempfile.gettempdir(), "txla_%d.csv" % baud)
            if os.path.exists(out_csv):
                os.remove(out_csv)
            print("=" * 68)
            verdict = run_one(ser, baud, args.channel, out_csv)
            print("  >>> [%d] %s" % (baud, verdict))
            results.append((baud, verdict))
            time.sleep(3)   # 给 Logic 设备恢复时间，连续采集易 USB ReadTimeout
    finally:
        ser.close()

    print("\n" + "=" * 68)
    print("汇总：")
    for baud, v in results:
        print("  %-9d %s" % (baud, v))


if __name__ == "__main__":
    main()
