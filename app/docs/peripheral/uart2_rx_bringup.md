# UART2 RX 修复与板测指南（BT8970 无线麦 SDK）

> 目标：在保留 UART0/PB3 1.5 Mbps 调试口的前提下，为 UART2 建立可验证的 PC→板接收通道。
>
> 当前映射：TX=PE7（TX2-G1），RX=PB1（RX2-G2），波特率由 `UART2_COM_BAUD` 配置。

## 1. 结论与证据等级

### 原厂资料明确支持

原厂文档 `docs/bt897x无线麦SDK.pdf` 第 48–50 页确认：

- 芯片提供 UART0、UART1、UART2 和 HUART，用户可使用 UART1、UART2、HUART。
- UART2 可以参考 SDK 的 `bsp_uart.c` 修改。
- PE7 是 TX2-G1，PB1 是 RX2-G2，属于合法 UART2 复用组合。
- UART 必须打开对应时钟门控，波特率应按所选输入时钟计算。

### 从原厂 UART1 源码迁移的工作假设

`bsp/bsp_uart.c` 的 UART1 ISR 使用以下接收顺序：

1. 检查 `UART1CON & BIT(9)`；
2. 读取一次 `UART1DATA`；
3. 写 `UART1CPND = BIT(9)` 清接收挂起。

UART2 没有公开的 `uart2_register_isr()` API，也没有公开寄存器位说明。因此当前实现先按相同语义在主循环轮询：

```c
if (UART2CON & BIT(9)) {
    data = UART2DATA;
    // data 入软件队列
    UART2CPND = BIT(9);
}
```

这是当前最有原厂源码依据的方案，但仍需实板验证 UART2 的 BIT9 行为。

### 原厂资料没有支持的旧结论

旧版本曾把以下现象写成硬件定论，但原厂 PDF 没有给出相应依据：

- `UART2DATA` 空读必定返回 `0xFF`；
- DATA 是无法区分重复字节的保持寄存器；
- BIT9 不能用于逐字节接收；
- 每轮必须先写 `UART2CPND = 0xFF00`；
- 连续相同字节、`0xFF` 或突发流是硬件固有不可接收。

旧代码本身会主动丢弃 `0xFF` 和与上个字节相同的数据，并在判断事件前清除全部状态，因此旧测试无法证明上述硬件结论。

## 2. 当前实现

相关文件：

- 驱动：`bsp/bsp_uart2_com.c`
- 接口：`bsp/bsp_uart2_com.h`
- 配置：`projects/microphone/config.h`
- 主循环入口：`functions/func.c`
- 初始化入口：`system/system.c`

主要行为：

- 每次主循环只在 `UART2CON BIT(9)` 置位时读取一次 DATA，然后清 BIT9。
- 不按数据值去重，`0x00`、`0xFF` 和连续重复字节均作为合法数据入队。
- 128 字节软件环形缓冲在满时丢弃新字节，并累加 `rx_overflow_count`，不覆盖未读数据。
- 正常模式下 `bsp_uart2_com_process()` 不消费队列；业务通过 `bsp_uart2_com_get()` 取数。
- `UART2_COM_RX_TEST_EN=1` 时启用二进制原样回显，便于主机逐字节比对。回显采用非阻塞状态机，每次主循环最多提交一个待发字节，UART2 不输出启动文本、十六进制文本或心跳。
- 诊断信息只通过 UART0 输出，包括接收数、BIT9 命中数、软件溢出数、回显发送数及关键寄存器快照。

UART2 当前选择 24 MHz XOSC，BAUD 分频据此计算，不依赖系统主频恰好也是 24 MHz。

## 3. 接线与配置

| USB-UART | 开发板 |
|---|---|
| TX | PB1 / UART2 RX |
| RX | PE7 / UART2 TX |
| GND | GND |

配置位于 `projects/microphone/config.h`：

```c
#define UART2_COM_EN          1
#define UART2_COM_BAUD        115200
#define UART2_COM_RX_TEST_EN  1
```

当前驱动沿用原厂 UART1 初始化中的 `BIT(4)`，按该源码注释使用 **8N2**（8 数据位、无校验、2 停止位）；`test-uart2.ps1` 已使用相同格式。BIT4 的 UART2 精确语义仍应由原厂寄存器资料确认。

UART0/PB3 仍为 SDK 调试输出口，波特率 1.5 Mbps。UART0 和 UART2 应分别连接，避免把 UART0 日志混入 UART2 二进制测试数据。

## 4. 构建与主机测试

构建：

```powershell
cd projects/microphone
powershell -ExecutionPolicy Bypass -NoProfile -File .\build.ps1 -Rebuild
```

查看串口：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -File .\test-uart2.ps1 -ListPorts
```

先跑低速特殊字节测试：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -File .\test-uart2.ps1 `
  -Port COM7 -BaudRate 115200 -Case slow-special
```

再跑完整测试：

```powershell
powershell -ExecutionPolicy Bypass -NoProfile -File .\test-uart2.ps1 `
  -Port COM7 -BaudRate 115200 -Case all
```

脚本覆盖：

- 单字节 `00`、`FF`；
- 连续相同字节；
- `00 FF 00 FF`、`55 55 AA AA`；
- `00..FF` 全字节序列，10 ms 与 1 ms 字节间隔；
- 128、129、255、256、257 字节无间隔突发。

每个用例会报告发送长度、接收长度、首个不一致位置、期望值和实际值。

## 5. 分阶段判定

### 慢速特殊字节全部通过

说明 BIT9 至少可以作为轮询接收事件使用，且旧版本对重复字节与 `0xFF` 的丢弃来自软件算法，不是已证实的硬件限制。

### 慢速单字节也完全收不到

通过 UART0 保存以下信息：

- `UART2CON`
- `UART2CPND`
- `UART2BAUD`
- `FUNCMCON2`
- `rx_pending_count`

若线路和电平已确认，但 BIT9 始终不出现，需要向原厂确认 UART2 RX 完成位、清挂起方法及是否存在未公开的初始化位。不得退回“读取值变化即新字节”的算法。

### 慢速通过、连续流丢包

这表明主循环轮询延迟超过 UART2 的接收保持能力。下一步应向原厂确认：

- UART2 是否接入 `IRQ_UART_VECTOR`；
- UART0/1/2 是否由库内部共享分发；
- UART2 RX 中断使能位；
- 是否存在未公开的 UART2 ISR 注册接口；
- UART2DATA 是否有 FIFO，以及 FIFO 深度。

在确认共享中断分发方式前，不应直接用 `sys_irq_init()` 覆盖 UART 向量。若 SDK 不开放可靠的 UART2 RX 中断，持续无损流应改用原厂明确支持回调/FIFO 的 HUART。

### `rx_overflow_count` 增加

说明硬件收数已进入软件，但业务或测试回显消费不够快。需要提高消费频率、缩短阻塞区，或扩大缓冲；不能把它误判成 GPIO/复用故障。

## 6. 资源与低功耗限制

- `uart2_key_mode()` 会重新占用 UART2/VUSB 资源；进入相关升级/按键模式后，PE7/PB1 的 UART2 COM 配置不能假定仍然有效。
- I2C、SPI、SD、TMR3 等模块可能复用 PB1/PE7，部分 SDK 代码会整值改写 `FUNCMCON2`。打开这些功能前必须重新检查引脚冲突。
- 睡眠代码可能暂时关闭 PB1/PE7 的数字功能。若外部主机需要随时发送，应建立单独唤醒协议或禁止相关睡眠路径。

## 7. 实板测试记录

### 2026-09-08：BIT9 轮询版本，阻塞回显

环境：

- UART0：COM9，1.5 Mbps，用于诊断日志；
- UART2：COM17，115200 bps、8N2，用于二进制回显；
- 测试脚本：`test-uart2.ps1 -Case all -TimeoutMs 5000`。

结果：

- `00`、`FF`、连续四个 `61`、`00 FF 00 FF`、`55 55 AA AA`：全部逐字节通过；
- `00..FF`、10 ms 字节间隔：256/256 通过；
- `00..FF`、脚本 1 ms 间隔：256/256 通过；
- 无间隔 128/129/255/256/257 字节：分别收到 86/90/176/177/178 字节，失败；
- UART0 诊断中 `rx_byte_count == rx_pending_count`，`rx_overflow_count == 0`，`tx_timeout_count == 0`。

这组结果证明：

1. `UART2CON BIT(9)` 可以作为 UART2 轮询接收事件；
2. 重复字节和 `0xFF` 可以正常接收，旧结论是旧软件过滤造成；
3. 软件环形缓冲没有溢出；
4. 第一版测试回显在收到字节后阻塞等待 TX 完成，显著拉长了 RX 轮询间隔，是无间隔突发丢包的直接软件瓶颈。

随后把测试回显改为非阻塞状态机并重新烧录复测。特殊字节、10 ms 和 1 ms 用例仍全部通过；无间隔 128/129/255/256/257 字节分别收到 93/93/180/184/184 字节。UART0 同时显示 `rx_byte_count == rx_pending_count == tx_byte_count` 且 `rx_overflow_count == 0`。与阻塞版相比仅小幅改善，说明主要瓶颈已不是回显等待，而是主循环轮询无法在每个 115200 bps 字符到达时及时读取单字节接收寄存器。

进一步反汇编 `libplatform.a` 后确认：`uart1_register_isr()` 会先置 `UART1CON BIT(2)`，再把回调直接注册到 IRQ14；SDK 没有 UART2 注册 API，也没有二级 UART 分发表。当前工程 UART1 功能宏为开启状态，因此不能直接用仅处理 UART2 的 ISR 覆盖 IRQ14。

实验性共享 IRQ14 版本也已完成实板测试：启动稳定，`UART2CON=0x011100f5`，每个收到的字节均使 `irq`、`pending` 和 `rx` 同步递增，证明 `UART2CON BIT(2)` 确实能使 UART2 通过 IRQ14 进入回调。但无间隔 128/129/255/256/257 字节仅收到 70/70/137/140/136 字节，比非阻塞轮询更差；`overflow=0` 且 `rx == pending == irq == tx`。这表明 UART2 的接收数据寄存器不具备足以吸收连续流的 FIFO，或者同优先级/关中断区导致 IRQ 响应超过字符周期。IRQ 本身无法恢复已经被覆盖的字节。

因此默认将 `UART2_COM_RX_IRQ_TEST_EN` 设回 0，保留已验证更好的轮询实现。实验代码仅用于记录和后续原厂确认，不能作为当前产品方案。

## 8. 当前验收状态

- 源码已通过完整命令行构建。
- BIT9 轮询、特殊字节、重复字节及脚本 1 ms 间隔数据已通过实板验证。
- UART2 IRQ14 和 `CON BIT(2)` 已由实板证明可触发，但不改善无间隔连续流，并可能覆盖 UART1 的 IRQ14 注册，默认禁用。
- 普通 UART2 在当前 SDK/主程序负载下不能保证 115200 bps 无间隔连续流；可靠使用边界应限制为字节间隔不小于 1 ms。需要连续无损流时，应改用 HUART 或取得原厂 UART2 FIFO/中断资料后重新设计。
