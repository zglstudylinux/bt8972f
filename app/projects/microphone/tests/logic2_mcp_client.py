#!/usr/bin/env python
"""Saleae Logic 2 官方 MCP server 最小客户端（零依赖，仅标准库）。

前置：Logic 2 → Settings/Automation → 打开 MCP Server（EXPERIMENTAL），
监听 http://127.0.0.1:10530（Streamable HTTP，JSON-RPC 2.0，无状态）。

用法：
    python logic2_mcp_client.py list                    # 列出全部工具
    python logic2_mcp_client.py call <tool> '<json>'    # 调用工具

端到端示例（CH1、16 MS/s、5 秒定时采集、3M UART 解码、导出 CSV）：
    python logic2_mcp_client.py call get_devices
    python logic2_mcp_client.py call start_capture '{"logicDeviceConfiguration":{"logicChannels":{"digitalChannels":[1]},"digitalSampleRate":16000000},"captureConfiguration":{"timedCaptureMode":{"durationSeconds":5}}}'
    python logic2_mcp_client.py call wait_capture '{"captureId":5}'
    python logic2_mcp_client.py call add_analyzer '{"captureId":5,"analyzerName":"Async Serial","analyzerLabel":"uart2_tx_3m","settings":{"Input Channel":{"numberValue":1},"Bit Rate (Bits/s)":{"numberValue":3000000}}}'
    python logic2_mcp_client.py call export_data_table_csv '{"captureId":5,"filepath":"C:/tmp/cap.csv","analyzers":[{"analyzerId":10016,"radixType":3}],"iso8601Timestamp":false}'

已实测的坑：
    1. add_analyzer 的 settings 值必须包对象：{"numberValue":3000000}，裸数字报
       "should be object"；
    2. export_data_table_csv 的 radixType 枚举无文档：1=二进制、2=有符号十进制、
       3=hex、缺省=ASCII 字符（有损，勿用于校验）；
    3. iso8601Timestamp 缺省为 true（ISO 时间戳）；false 时 start_time 为相对秒。
"""

import json
import sys
import urllib.request

URL = "http://127.0.0.1:10530/"


def rpc(method, params=None, _id=1, timeout=300):
    payload = {"jsonrpc": "2.0", "id": _id, "method": method, "params": params or {}}
    req = urllib.request.Request(
        URL, data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json",
                 "Accept": "application/json, text/event-stream"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        d = json.loads(r.read().decode())
    if "error" in d:
        raise RuntimeError(f"JSON-RPC error: {d['error']}")
    return d["result"]


def tool_call(name, args):
    res = rpc("tools/call", {"name": name, "arguments": args})
    texts = [c.get("text", "") for c in res.get("content", []) if c.get("type") == "text"]
    out = "\n".join(texts)
    try:
        out = json.loads(out)
    except Exception:
        pass
    return {"isError": res.get("isError", False), "data": out}


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == "list":
        for t in rpc("tools/list")["tools"]:
            print(f"- {t['name']}: {t.get('description', '')[:100]}")
        return 0
    if len(sys.argv) >= 4 and sys.argv[1] == "call":
        print(json.dumps(tool_call(sys.argv[2], json.loads(sys.argv[3])),
                         ensure_ascii=False, indent=1)[:3000])
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
