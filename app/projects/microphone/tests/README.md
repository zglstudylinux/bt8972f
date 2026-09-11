# UART2/HUART 主机侧测试脚本

本目录统一存放串口板测所需的主机（PC）侧脚本：串口收发校验、逻辑分析仪解码数据校验、Logic 2 MCP 客户端。脚本同时服务 UART2 普通串口与 HUART 高速串口（帧格式一致，固件侧由 `config.h` 的 `UART2_COM_*` / `HUART_COM_*` 开关切换）。

## 脚本清单

| 脚本 | 用途 | 依赖 |
|---|---|---|
| `test-uart2-tx.ps1` | TX 板测校验：固件 `UART2_COM_TX_TEST_EN=1` 每 100ms 发 72 字节测试帧（`55 AA 5A A5`+序号+payload+CRC16），本脚本逐帧校验并统计丢帧 | PowerShell + .NET SerialPort |
| `test-uart2.ps1` | RX 板测校验：固件 `UART2_COM_RX_TEST_EN=1` 原样回显，本脚本发送特殊字节/全字节/突发等用例并逐字节比对 | PowerShell + .NET SerialPort |
| `huart_serial_echo.py` | HUART 双机回传板测：PC 发 `huart_dual_inc.bin`（50 760 字节递增码流）→ 固件 `HUART_BAUD_TEST_EN=1` 回传 → 逐字节比对打 PASS/FAIL | Python 3 + pyserial |
| `la_validate_frames.py` | 逻辑分析仪导出的 Async Serial 解码 CSV 逐帧校验（CRC16/序号/帧周期/字节间隔），兼容 Automation API 与官方 MCP 两种导出格式 | Python 3 标准库 |
| `logic2_mcp_client.py` | Saleae Logic 2 官方 MCP server（127.0.0.1:10530）最小客户端：列工具/调用工具，可完成采集→解码→导出全流程 | Python 3 标准库 |

## 常用命令

TX 板测（波特率随 `UART2_COM_BAUD` 修改后同步传参）：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -File .\tests\test-uart2-tx.ps1 `
  -Port COM17 -BaudRate 115200 -FrameCount 100
```

RX 板测（固件切 `UART2_COM_RX_TEST_EN=1` 后）：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -File .\tests\test-uart2.ps1 `
  -Port COM17 -BaudRate 115200 -Case all
```

逻辑分析仪端到端（Logic 2 开启 MCP Server 后）：

```bash
python tests/logic2_mcp_client.py call get_devices
python tests/logic2_mcp_client.py call start_capture '{"logicDeviceConfiguration":{"logicChannels":{"digitalChannels":[1]},"digitalSampleRate":16000000},"captureConfiguration":{"timedCaptureMode":{"durationSeconds":10}}}'
python tests/logic2_mcp_client.py call wait_capture '{"captureId":<id>}'
python tests/logic2_mcp_client.py call add_analyzer '{"captureId":<id>,"analyzerName":"Async Serial","settings":{"Input Channel":{"numberValue":1},"Bit Rate (Bits/s)":{"numberValue":3000000}}}'
python tests/logic2_mcp_client.py call export_data_table_csv '{"captureId":<id>,"filepath":"C:/tmp/cap.csv","analyzers":[{"analyzerId":<aid>,"radixType":3}],"iso8601Timestamp":false}'
python tests/la_validate_frames.py C:/tmp/cap.csv
```

替代路径：Saleae 官方 Python SDK（`pip install logic2-automation`，gRPC 端口 10430）可完成同样的采集导出，二者独立可用。

## 已知坑（Logic 2 MCP server 2.4.46）

1. `add_analyzer` 的 settings 值必须包对象：`{"Bit Rate (Bits/s)":{"numberValue":3000000}}`，裸数字报 `should be object`；
2. `export_data_table_csv` 的 `radixType`：1=二进制、2=有符号十进制、3=hex，缺省为 ASCII 字符（有损）；
3. `iso8601Timestamp` 缺省 true；false 时 `start_time` 为相对秒。`la_validate_frames.py` 两种都认。

## 测试数据判读参考

- TX 校验结果与历史实测记录见 `docs/peripheral/uart2_tx_bringup.md`（含 2026-09-09 逻辑分析仪独立验证）；
- RX 校验结果与丢包边界分析见 `docs/peripheral/uart2_rx_bringup.md`；
- **HUART 高速串口**（TX 波特率扫描至 8M、12M+ underrun 结论、RX 无间隔突发固件侧 100%）见 `docs/peripheral/huart_tx_bringup.md` 与 `docs/peripheral/huart_rx_bringup.md`，固件开关换成 `HUART_COM_EN`/`HUART_COM_BAUD`/`HUART_COM_TX_TEST_EN`/`HUART_COM_RX_TEST_EN`，测试命令不变。

### HUART 测试注意

- 使能 `HUART_BAUD_TEST_EN=1`（或 `HUART_COM_EN=1`）时必须关闭 `EQ_DBG_IN_UART` 和 `UART2_COM_EN`（单外设与共脚互斥，否则链接报错/映射冲突）；
- 逻辑分析仪判定 ≥8M 信号时必须与主机侧交叉验证——LA 探头电容+地环路在 8M 产生过观测伪影（详见 huart_tx_bringup.md 第 3.2 节）；
- 长时间采集（>7s @24MS/s）Logic 设备可能 USB ReadTimeout 导致采样流缺失、解码出现成簇坏帧，应缩短单次采集时长。

### HUART 双机回传压力测试（huart_baud_test + huart_serial_echo.py）

固件切 `HUART_BAUD_TEST_EN=1`（同时关 `EQ_DBG_IN_UART`/`UART2_COM_EN`），引脚 PE7/PB1 与 UART2 一致。板子上电后逐档等待 PC 数据，PC 按档位发递增码流，板子回传后脚本自动比对：

```bash
# 必须带 --chunk-size 512 --send-delay-ms 10（逐块节流），且只用递增码流——原因与实验矩阵见
# docs/peripheral/huart_dual_failure_analysis.md（连续灌流与恒值码流都会得到不可信结果）
python tests/huart_serial_echo.py COM17 115200 tests/huart_dual_inc.bin --chunk-size 512 --send-delay-ms 10
# 板子自动切下一档，依次：230400 / 460800 / 921600 / 1000000 / 1500000 / 2000000 / 3000000
```

进度看 UART0（COM9，1.5M）：`[Baud xxx] waiting PC data...` / `[rx] xxx bytes`。实测 115200~3M 八档：115200~1.5M 全 PASS（2026-09-10）；2M/3M 板端接收正确，仅 CH340 超规格/边界在回传方向有少量丢字节（2026-09-11，需规格内高速适配器做无适配器误差的验收）。
