# AI 操作软件的三条链路：程序化 API、MCP、GUI 自动化

> 以 2026-09-09 用 AI 驱动 Saleae Logic 2 采集 UART2 TX 波形（见 `docs/peripheral/uart2_tx_bringup.md` 的逻辑分析仪验证一节）为真实案例，讲清三层机制的原理。
>
> 证据等级标注：【实测】= 本次会话直接观察到的；【背景】= 通用技术背景知识。

## 0. 一句话总结

| 问题 | 答案 |
|---|---|
| skill 靠什么"直接操作软件"？ | skill 本身没有操作能力，它是"说明书+脚本"。真正干活的是**软件官方开放的程序化接口**（Logic 2 的 Automation API：本地 gRPC 服务，端口 10430），AI 用 Bash 调 Python 脚本走这条通道 |
| 走的是 MCP 吗？ | skill 那条链路不是：它是 `Bash → python → gRPC → 本地端口`。但 Logic 2 同时还内置了**官方 MCP server**（10530 端口，EXPERIMENTAL），已实测跑通全流程，见第 8 节 |
| MCP 是什么？ | Model Context Protocol，AI 应用与外部工具/数据源之间的开放接入协议（JSON-RPC 2.0，stdio/HTTP 传输）。本环境里的 `computer-use`、`web_reader` 等工具就是 MCP server 提供的 |
| 没有 skill 时怎么操作 GUI？ | 是"模仿鼠标点击"，但分两层：先读**辅助功能树**（Windows UIA，语义级，可程序化按下控件），不行再用**截图+坐标+合成鼠标事件**（像素级，需前台）。本次两条都用了，最终因界面自绘改走 API |

## 1. 案例回顾：同一个目标，三条链路都出现了

目标：把 Logic 2 抓到的 UART 波形解码结果拿出来分析。

| 尝试 | 链路 | 结果 |
|---|---|---|
| ① 在 Logic 2 界面上找"导出"按钮 | MCP computer-use（辅助树 + 截图） | 找到了数据面板三个图标（跳转/表格视图/终端视图），但没有导出入口，点按无效【实测】 |
| ② 放大截图辨认图标 | MCP computer-use（zoom 视觉兜底） | 确认三个图标都不是"导出"【实测】 |
| ③ 改用 logic-analyzer skill | Bash → python → gRPC → Logic 2 API | 10 秒采集 + 3M UART 解码 + CSV 导出一次成功【实测】 |

结论：**同一个软件有两扇门——给人的 GUI 和给程序的 API**。AI 两种都能走，但 API 门更稳、更快、可重复。

## 2. Skill 的原理：它只是"知识包"，不是"执行器"

【背景】skill 是一种过程性知识的打包格式，典型结构（本仓库用过的 logic-analyzer skill 即如此）：

```
skills/logic-analyzer/
├── SKILL.md            # 触发条件（description）+ 操作手册（步骤/参数/失败分流）
├── references/usage.md # 更详细的参数参考
└── scripts/la_tool.py  # 可重复调用的执行脚本
```

工作机制：

1. **按需加载**：AI 的上下文里平时只有各 skill 的一句话描述（"当需要……时使用"）。任务匹配时，AI 才去读 SKILL.md 全文——相当于把一本操作手册临时装进脑子。
2. **执行仍走普通工具**：SKILL.md 让 AI"读 usage.md → 跑 `python scripts/la_tool.py --detect`"，AI 执行这些动作用的还是最基本的 Bash 工具。skill 没有任何特权能力。
3. **所以 skill 能"直接操作软件"的真正原因是**：目标软件自己开了程序化入口。skill 的价值是把"端口是多少、参数怎么填、设置键名和界面文字必须一致、失败了怎么分流"这些**踩过坑的经验**固化下来，让每次调用都可重复。

类比：skill 是菜谱，Bash 是灶台，真正把菜炒熟的是食材本身的化学性质（软件开放接口）。

## 3. 真正的执行通道：Logic 2 Automation API

【背景+实测】Logic 2 桌面软件内置一个**自动化服务**（官方功能，可用 `Logic.exe --automation` 启动或在设置里开启），启动后在本机监听 `127.0.0.1:10430`。它对外暴露的是 **gRPC** 接口（HTTP/2 + protobuf），官方提供 Python SDK（pip 包 `logic2-automation`，`import saleae.automation`）。

本次完整调用链【实测】：

```
AI 决策"采 10 秒、CH1、3M UART 解码"
   │  Bash 工具
   ▼
python la_tool.py --capture --channels 1 --samplerate 16M --duration 10 \
       --analyzer "uart:rx=1,baud=3000000"
   │  官方 SDK（saleae.automation）
   ▼
gRPC 调用 → 127.0.0.1:10430（Logic 2 进程 pid=35820）
   │  Logic 2 内部完成：配置采样率/通道 → 驱动 USB 硬件 → 采样 10s
   ▼
cap.add_analyzer("Async Serial", settings={Input Channel:1, Bit Rate:3000000})
cap.export_data_table(filepath=...csv)      ← 导出解码表
   ▼
CSV 落盘 → AI 再用 python 做帧格式/CRC 校验（这一步就是纯本地数据分析了）
```

关键认知点：

- **这不是模拟点击**。`start_capture` / `add_analyzer` / `export_data_table` 是语义级函数调用，Logic 2 的 GUI 界面和这个 gRPC 服务是**同一个进程里的两套入口**，共享同一份设备与数据。
- 证据【实测】：`la_tool.py --detect` 通过该端口返回 `AppInfo(app_version='2.4.46', app_pid=35820)`，与桌面上正在运行的 Logic.exe 进程号一致。
- 【背景】几乎所有专业软件都有类似入口，只是形态不同：命令行参数（`sox`、`ffmpeg`）、CLI/REPL（`python -c`）、本地服务（本例 gRPC、很多仪器的 SCPI/TCP）、COM/DCOM（Windows 老牌软件）、REST API（Web 应用）。**AI 提效的第一步永远是盘点目标软件有没有这类入口。**

## 4. MCP 是什么

【背景】MCP（Model Context Protocol）是 2024 年发布的开放协议，解决一个标准化问题：**AI 应用如何以统一方式接入任意外部工具和数据源**。

架构与职责：

```
┌─────────────────────────────────────────────┐
│ Host（AI 应用：ZCode / Claude Desktop / IDE）│
│  ├─ AI 模型（决定什么时候调用哪个工具）        │
│  └─ MCP Client（每个 server 一条连接）        │
└──────────┬──────────────────┬───────────────┘
     JSON-RPC│(stdio)    JSON-RPC│(Streamable HTTP)
        ┌────┴─────┐        ┌────┴──────┐
        │ MCP Server│        │ MCP Server │
        │ computer- │        │ 某内部系统  │
        │ use(键鼠/ │        │ (缺陷单/CI) │
        │ 截图/读屏) │        └───────────┘
        └──────────┘
```

- **消息格式**：JSON-RPC 2.0；本地 server 用 stdio（AI 应用把 server 作为子进程拉起，stdin/stdout 通信），远程 server 用 HTTP。
- **三类能力原语**：
  - **tools**：模型可主动调用的函数（带 JSON Schema 参数声明）——如 `mcp__computer-use__left_click`；
  - **resources**：可读的数据源（文件、数据库行）；
  - **prompts**：预置的提示词模板（用户以 `/命令` 触发）。
- **解决的问题**：没有 MCP 时，M 个 AI 应用 × N 个工具需要 M×N 套定制集成；有了 MCP，工具方只需实现一次 server，任何支持协议的 AI 应用即插即用，变成 M+N。

MCP 与本案例两层的关系：

| 概念 | 层次 | 一句话 |
|---|---|---|
| Automation API | 某软件的私有接口 | Logic 2 自己开的 gRPC 门 |
| MCP | 工具接入协议 | 把"某个能力"（键鼠控制、串口、数据库）封装成 AI 可发现的标准工具 |
| skill | 过程性知识 | 把"这套接口/工具怎么用好"写成手册+脚本 |

三者可组合：完全可以把 `la_tool.py` 包一层 MCP server（提供 `la_capture` 工具），那样 AI 连命令行参数都不用记，直接看到结构化的工具声明。skill 与 MCP 的取舍：**skill 更轻**（一组 md+脚本即可，改起来快），**MCP 更规范**（有类型化接口、可被发现、跨项目复用），高频核心工具适合 MCP，长尾流程适合 skill。

## 5. GUI 自动化的原理：是的，就是"合成鼠标键盘"，但分两个层次

【背景+实测】没有 API 时，AI 操作 GUI 依赖操作系统提供的两类底层机制。

### 5.1 层次一：辅助功能树（语义级，首选）

- **是什么**：Windows 的 UI Automation（UIA）、macOS 的 Accessibility（AX）API。这套机制本来是给视障人士的读屏器用的：应用必须把控件暴露成一棵树——每个节点有名称、类型（按钮/文本框）、屏幕边界、支持的动作（按下/展开/设值）。
- **AI 怎么用**：`get_app_state` 读树 → 拿到结构化元素列表（如 `[104] button "Async Serial"` 及其边界坐标）→ 对元素发 `AXPress/Invoke` 动作。
- **本质**：这是**程序化点击**，不是模拟鼠标——控件自己执行默认动作，不需要窗口在前台、不占用真实光标、不受遮挡影响。
- 【实测】本次读 Logic 2 拿到 318 个元素（菜单、通道名、数据气泡文本 `0xED` 等），并成功按下了数据面板的一个按钮（尽管后来证明点错了目标）。

### 5.2 层次二：截图 + 坐标 + 合成输入（像素级，兜底）

- **为什么需要**：现代软件大量使用**自绘界面**——Logic 2 的波形画布是 OpenGL 渲染的整块图像，画布上的数据气泡、自绘图标根本不进辅助功能树；游戏则整窗自绘。
- **怎么做**：
  1. 截屏（`screenshot`/`zoom` 局部放大）；
  2. 视觉模型看图，估算目标在屏幕上的像素坐标；
  3. 用**合成的鼠标/键盘事件**去点（Windows 上是 `SendInput` 注入的硬件级输入事件，与真实键鼠在系统层面等价）；
  4. 再截图验证效果，形成闭环。
- **限制**：需要窗口在前台（输入注入作用于真实焦点）；依赖分辨率/DPI 缩放，界面一变就可能点偏；速度慢、每步都要视觉确认。
- 【实测】本次 `zoom` 放大数据面板图标区，看清三个图标是"跳转/表格视图/终端视图"（从而排除导出功能在它们里面），以及波形上 `0xED` 气泡确实无法用元素方式定位——都是典型的自绘区域。

### 5.3 为什么这次 GUI 链路没走通

不是 GUI 自动化"不会点"，而是：Logic 2 的分析器数据导出没有做成显眼的常驻控件（右键菜单/快捷键路径在自绘区域里），纯靠辅助树定位效率低。**这正是工程上"有 API 就绝不走 GUI"的原因**——GUI 是给人设计的，布局常变、入口隐蔽；API 是给程序设计的，契约稳定。

## 6. 两条链路对比

| 维度 | 程序化 API（本例 skill→gRPC） | GUI 自动化（辅助树+像素） |
|---|---|---|
| 前提 | 软件必须开放接口 | 任何有界面的软件都行 |
| 语义层级 | 函数调用（`add_analyzer`） | 控件按压 / 坐标点击 |
| 稳定性 | 接口契约稳定，跨版本基本兼容 | 布局一改就失效 |
| 速度 | 秒级、可批量 | 每步需观察-决策-验证循环 |
| 前后台 | 完全后台 | 辅助树可后台；像素点击需前台 |
| 失败模式 | 端口没开、参数键名不匹配（可探测、报错明确） | 点错位置、焦点被抢、弹窗拦截 |
| 适用场景 | 高频、可重复的工程流程 | 只有 GUI 的遗留软件、一次性操作 |

## 7. 给原厂 AI 提效的落地映射

1. **盘点工具软件的自动化入口**（选型第一问）：有没有命令行参数 / SDK / 本地服务端口 / 可脚本化导出格式。Logic 2 是正面例子（`--automation` + gRPC + 官方 Python SDK）；没有入口的软件可以在外面包 CLI（很多仪器厂商提供 DLL，可封成 Python 模块）。
2. **高频流程沉淀为 skill**：把"端口、参数、设置键名与界面文字一致、失败分流"这类踩坑经验固化（logic-analyzer skill 的 `ANALYZER_SPECS` 注释"必须与界面文字完全一致"就是典型的坑）。skill 结构模板：`SKILL.md`（触发+手册）+ `references/`（参数参考）+ `scripts/`（幂等脚本）。
3. **核心工具升级为 MCP server**：串口、烧录器、逻辑分析仪这类每天用的能力，包成 MCP 工具后 AI 可以直接"看到"结构化接口并跨项目复用。
4. **GUI 自动化只作兜底**：留给无 API 的遗留软件和一次性场景，且优先用辅助功能树、避免裸像素点击。

## 8. 实测补充：Saleae 官方 MCP Server（Logic 2.4.46）

【实测】2026-09-09，Logic 2 官网 downloads 页新增 "AI-powered workflows with MCP" 板块，官方文档 [docs.saleae.com/mcp](https://docs.saleae.com/mcp)。Logic 2 内置 MCP server（标注 **EXPERIMENTAL**），与第 3 节的 gRPC Automation API 并存：

- **开启**：Settings → Automation → 打开 MCP Server 开关（无独立命令行参数）；
- **端点**：`http://127.0.0.1:10530`，MCP 标准的 Streamable HTTP 传输（JSON-RPC 2.0，无状态、无会话头）；
- **进程**：与 gRPC 的 10430 端口同属一个 Logic 后台进程——同一个引擎开了两扇标准不同的门；
- **接入**：任何 MCP 客户端一行即用，如 `claude mcp add --transport http logic2 http://127.0.0.1:10530`。

### 实测握手与工具清单

`initialize` 返回 `serverInfo: {name: "saleae-logic2", version: "2.4.46"}, capabilities: {tools}`。`tools/list` 共 **15 个工具**：

`get_devices`、`start_capture`、`stop_capture`、`wait_capture`、`load_capture`、`save_capture`、`close_capture`、`add_analyzer`、`remove_analyzer`、`add_high_level_analyzer`、`remove_high_level_analyzer`、`export_raw_data_csv`、`export_raw_data_binary`、`export_data_table_csv`、`legacy_export_analyzer`

覆盖 GUI 的核心工作流：设备 → 采集 → 协议解码 → 数据导出。

### 实测端到端结果

用原始 JSON-RPC 裸调（不经任何 MCP 客户端注册）跑完整流程：CH1（PE7）、16 MS/s、定时 5 s、Async Serial 3M 解码、导出 CSV，再用 `projects/microphone/tests/la_validate_frames.py` 校验：**50/50 帧 CRC 全对、序号零断档、帧周期均值 100.008 ms、帧内字节间隔中位数 31.750 µs**——与第 3 节 gRPC 链路采集的统计逐位一致（固件时序确定性）。

### 三个实测坑（2.4.46）

1. `add_analyzer` 的 settings 值必须包对象：`{"Input Channel":{"numberValue":1}}`，裸数字报 `should be object`；
2. `export_data_table_csv` 的 `radixType` 无文档，实测 **1=二进制、2=有符号十进制、3=hex**，缺省为 ASCII 字符（有损，勿用于校验）；
3. `iso8601Timestamp` 缺省 true；false 时 `start_time` 为相对秒。

最小客户端与一键采集脚本已沉淀到 `projects/microphone/tests/logic2_mcp_client.py`（零依赖标准库）。

### 与第 6 节对比表的关系

官方 MCP server 出现后，第 6 节的"程序化 API"一行拆成两档：**私有 API**（gRPC，功能最全、要读 SDK 文档）与**标准 MCP**（协议统一、AI 客户端即插即用、工具带 schema 自描述）。对 AI 提效的选型含义：软件提供 MCP server 时优先接 MCP（接入成本最低）；只有 gRPC/CLI 时用 skill 封装；都没有才退到 GUI 自动化。

## 9. 附：本次真实调用记录

```bash
# 环境探测（包、端口、版本、设备）
python skills/logic-analyzer/scripts/la_tool.py --detect
# ✅ logic2-automation 已安装；127.0.0.1:10430 可连接；
#    AppInfo(app_version='2.4.46', app_pid=35820)；设备 A7D1BB81883C0092

# 采集：CH1(PE7) 16MHz 采样 10 秒，挂 3M 波特率 Async Serial 解码器
LA_CAPTURE_DIR=<dir> python la_tool.py --capture --channels 1 \
    --samplerate 16M --duration 10 --analyzer "uart:rx=1,baud=3000000"
# 📄 输出: <dir>\uart_20260909_155457.csv（7283 行解码字节）
```

产物与后续：CSV 用独立 Python 脚本按帧格式（`55 AA 5A A5`+序号+payload+CRC16）校验，101/101 帧全通过——结果记录在 `docs/peripheral/uart2_tx_bringup.md`；校验脚本已沉淀为 `projects/microphone/tests/la_validate_frames.py`。

链路二（官方 MCP server，10530，2026-09-09 补测）：

```bash
# 列出 15 个工具
python projects/microphone/tests/logic2_mcp_client.py list

# 5 秒采集（CH1、16 MS/s）→ 解码 → 导出（详见脚本 docstring）
python projects/microphone/tests/logic2_mcp_client.py call start_capture '{"logicDeviceConfiguration":{"logicChannels":{"digitalChannels":[1]},"digitalSampleRate":16000000},"captureConfiguration":{"timedCaptureMode":{"durationSeconds":5}}}'
# 结果：50/50 帧全通过，与 gRPC 链路统计一致
```
