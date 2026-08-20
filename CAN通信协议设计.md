# CAN 触发与完成通知协议设计

## 1. 设计定位

本项目当前 CAN 仅承担**平均值采样触发**和**采样完成通知**两类实时控制消息，不承担采样参数配置、平均值结果传输或其他大块业务数据。

- 参数配置、结果读取等业务继续由 ModbusTCP 负责。
- CAN 使用 **11-bit 标准帧**。
- 协议目标以简单、直观、易调试为主，同时支持多个外部板卡触发本设备，并满足“谁触发，完成后回复谁”。

## 2. CAN ID 规划

11-bit CAN ID 按以下方式划分：

```text
bit10........bit0
┌───────┬────────┬────────┐
│ Type  │ Src ID │ Dst ID │
│ 3 bit │ 4 bit  │ 4 bit  │
└───────┴────────┴────────┘
```

计算方式：

```c
CAN_ID = (Type << 8) | (SrcID << 4) | DstID;
```

字段定义：

| 字段 | 位宽 | 说明 |
| --- | --- | --- |
| Type | 3 bit | 报文类型 |
| SrcID | 4 bit | 发送节点 ID |
| DstID | 4 bit | 目标节点 ID |

NodeID 约定：

- `0x0 ~ 0xE`：普通节点地址。
- `0xF`：广播目标地址，仅用于 `DstID`。
- 普通节点发送报文时 `SrcID` 不使用 `0xF`。

当前只定义两种 Type：

| Type | 名称 | 含义 |
| --- | --- | --- |
| `0x1` | `START_AVG` | 开始一次平均值采样 |
| `0x2` | `AVG_DONE` | 平均值采样完成 |

其余 Type 预留，当前不提前扩展。

## 3. Data 定义

### 3.1 START_AVG

```text
DLC = 1
Byte0 = Seq
```

- `Seq` 为 8-bit 请求序号，由触发方生成。
- 本设备收到触发后保存本次请求的 `SrcID` 和 `Seq`，供完成回复使用。

### 3.2 AVG_DONE

```text
DLC = 2
Byte0 = Seq
Byte1 = Status
```

- `Seq`：原样返回本次触发报文中的请求序号。
- `Status = 0x00`：本次平均值采样正常完成。
- 其他状态值暂时预留，只有后续出现明确需求时再定义。

## 4. 收发规则

### 4.1 单播触发

当收到满足以下条件的 `START_AVG` 时，本设备接受该请求：

- `DstID == 本机 NodeID`；或
- `DstID == 0xF`，即广播触发。

本设备记录：

```text
request_src = SrcID
request_seq = Seq
```

平均值采样正常完成后发送 `AVG_DONE`：

```text
Type  = AVG_DONE
SrcID = 本机 NodeID
DstID = request_src
Seq   = request_seq
```

因此无论有多少外部板卡可以触发本设备，都统一遵循：**谁触发，采样完成后回复谁。**

### 4.2 广播触发

广播触发时：

```text
DstID = 0xF
```

所有需要响应该广播的采样板均可开始本轮采样，但采样完成后各自向原始 `SrcID` 发送单播 `AVG_DONE`，不发送广播完成报文。

### 4.3 新触发覆盖旧测量

保持现有平均值状态机规则：

- 空闲状态收到 `START_AVG`：开始新一轮统计。
- 正在统计时收到新的有效 `START_AVG`：立即放弃当前未完成统计，按新请求重新开始。
- 新请求到来时同时覆盖当前保存的 `request_src` 和 `request_seq`。
- 被覆盖的旧请求不额外发送取消或失败报文。
- 最终只对实际完成的那一轮请求发送 `AVG_DONE`。

该规则保持协议简单，触发方通过 `Seq` 判断完成通知对应哪一次请求。

## 5. 示例

假设：

```text
外部板卡 A：NodeID = 2
外部板卡 B：NodeID = 3
本采样板：  NodeID = 5
```

A 触发本采样板：

```text
Type = 1, Src = 2, Dst = 5
CAN ID = 0x125
DLC = 1
Byte0 = Seq
```

本采样板完成后回复 A：

```text
Type = 2, Src = 5, Dst = 2
CAN ID = 0x252
DLC = 2
Byte0 = 原 Seq
Byte1 = 0x00
```

如果由 B 触发：

```text
START：0x135
DONE ：0x253
```

如果 A 广播触发所有采样板：

```text
Type = 1, Src = 2, Dst = 0xF
CAN ID = 0x12F
```

各采样板完成后分别单播回复 A。

## 6. 架构边界

当前 CAN 协议只固定以下内容：

- 11-bit 标准帧。
- `Type + SrcID + DstID` 的 ID 编码方式。
- `START_AVG / AVG_DONE` 两种消息。
- `Seq / Status` 的 Data 定义。
- 谁触发就回复谁。
- 新触发覆盖未完成的旧测量。

CAN 控制器寄存器配置、邮箱分配、中断标志清除、收发驱动函数等属于具体实现细节，不在本文档中展开。