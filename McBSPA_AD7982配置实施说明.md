# McBSP-A + AD7982 采样链路配置实施说明

## 1. 文档目的

本文档用于指导 `MCU_2833x_APP` Demo 工程完成 AD7982 的基础采样链路配置。

当前阶段只实现以下内容：

- GPIO21、GPIO22、GPIO23 的 McBSP-A 引脚配置。
- McBSP-A 以 SPI 主机方式读取 AD7982。
- ePWM 产生固定 100 kHz 发起节拍。
- DMA 自动将 McBSP-A 的一个 32-bit 接收字搬到调用方持有的模块上下文成员。
- 提供必要的最小中断和诊断计数，保证链路可以连续运行。

当前阶段不实现：

- CAN 触发后的区间统计。
- 18-bit 有符号数转换与校准。
- 累加、平均值计算和 Modbus 寄存器映射。
- McBSP-B 数字电位器控制。

本工程仅为实例 Demo。现有代码与本方案冲突时，允许直接删除、替换或停用冲突逻辑，以 `架构.md` 和本文档记录的讨论结论为准，不要求保留 Demo 的原有行为。

## 2. 已确定的设计结论

### 2.1 引脚分配

| 引脚 | 复用功能 | 方向 | 连接信号 |
| --- | --- | --- | --- |
| GPIO21 | MDRA | 输入 | AD7982 SDO |
| GPIO22 | MCLKXA | 输出 | AD7982 SCK |
| GPIO23 | MFSXA | 输出 | AD7982 CNV |

GPIO23 必须复用为 `MFSXA`，不采用普通 GPIO 软件拉低、拉高方案。

一次 McBSP 32-bit 帧开始时，MFSXA 自动拉低；32-bit 帧结束时，MFSXA 自动拉高。这样 CNV 低电平宽度完全由硬件帧长度决定，不受 ePWM ISR 或 DMA ISR 响应抖动影响。

GPIO20/MDXA 不需要连接到 AD7982。McBSP-A 仍通过写 `DXR2/DXR1` 启动一帧传输，但不需要把 MDXA 复用输出到 GPIO20。

### 2.2 AD7982 时钟模式

根据 AD7982 接口时序：CNV 下降后 MSB 立即输出；后续数据位在 SCK 下降沿更新；数据可在上升沿或下降沿采集。当前 SCK 约 9.375 MHz，不需要采用依赖保持时间的下降沿高速采样方案，因此选择上升沿采样。

McBSP-A 使用以下组合：

```c
McbspaRegs.SPCR1.bit.CLKSTP = 3;  /* 11b，SPI clock-stop，带半周期延迟 */
McbspaRegs.PCR.bit.CLKXP    = 0;  /* SCK 空闲为低 */
McbspaRegs.PCR.bit.CLKRP    = 1;  /* SCK 上升沿采集接收数据 */

McbspaRegs.RCR2.bit.RDATDLY = 1;
McbspaRegs.XCR2.bit.XDATDLY = 1;
```

预期时序为：

1. MFSXA/CNV 下降。
2. AD7982 在 SDO 上输出 MSB。
3. 经过半个 SCK 周期后，第一个 SCK 上升沿采集 MSB。
4. SCK 下降沿推出下一位。
5. 后续 SCK 上升沿依次采集数据。
6. 第 18 个有效采样沿完成 18-bit 数据接收。
7. McBSP 继续产生时钟以填满 32-bit 帧。
8. 32-bit 帧结束，MFSXA/CNV 自动升高并启动下一次转换。

若下板时出现固定一位偏移，应优先检查 `RDATDLY/XDATDLY`、32-bit 字长和 DXR 写入顺序，不应首先改成下降沿采样。

### 2.3 数据链路

```text
ePWM1 100 kHz 中断
        |
        v
写 McBSP-A DXR2/DXR1 dummy word
        |
        v
MFSXA 自动拉低，MCLKXA 输出 32 个 SCK
        |
        v
McBSP-A 完成 32-bit 接收，MFSXA 自动拉高
        |
        v
MREVTA 触发 DMA CH2
        |
        v
DRR2 + DRR1 -> context->raw_adc_value
```

ePWM 不直接作为接收 DMA 的触发源。接收 DMA 的直接触发源必须为 McBSP-A Receive Event，即 `MREVTA`，`PERINTSEL=15`。

F28335 的 ePWM 不能直接启动 McBSP-A 发送，因此使用一个极短的 ePWM1 ISR 写 `DXR2/DXR1`，以发起每个 32-bit 帧。

### 2.4 编码规范与上下文约束

本模块遵循 `C语言代码编写规范.md`：使用 `Ad7982_` 模块前缀和 `Ad7982_Context_t`，状态由应用层持有并通过Context Pointer传入；ISR只保留一层薄包装，不增加隐藏全局状态、间接回调或多余转发层。

## 3. 工程文件规划

建议新增：

```text
MCU_2833x_APP/APP/drv_ad7982.h
MCU_2833x_APP/APP/drv_ad7982.c
```

AD7982 外部采样链路不要继续放入现有 `drv_Adc.c`。现有 `drv_Adc.c` 表示 F28335 片内 ADC，拆开后可以避免 DMA 通道、变量和函数含义混淆。

建议在 `drv_ad7982.h` 中声明：

```c
typedef struct
{
    volatile Uint32 raw_adc_value;
    volatile Uint32 frame_count;
    volatile Uint32 launch_overrun_count;
    volatile Uint16 sample_valid;
} Ad7982_Context_t;

void Ad7982_Init(Ad7982_Context_t *context);
void Ad7982_StartFrame(Ad7982_Context_t *context);
void Ad7982_OnDmaComplete(Ad7982_Context_t *context);
```

当前阶段对外提供的采样结果为 `context->raw_adc_value`，它保存McBSP-A的原始32-bit接收位流。模块不再导出独立全局变量。

应用层可以把该context作为更大应用上下文的一个扁平成员，例如：

```c
typedef struct
{
    Ad7982_Context_t ad7982;
    /* 其他应用模块上下文 */
} App_Context_t;
```

`App_Context_t`必须位于生命周期覆盖整个采样运行期的静态存储区。该对象由应用层所有；AD7982模块只能通过传入的 `Ad7982_Context_t *` 访问自己的成员。

中断是异步入口，C28x PIE不会替软件传入context参数，因此应用层必须有一个长期有效的根context供ISR包装函数取得地址。这是应用层的context存储，不允许演变成AD7982驱动内部的隐藏单例。除该根context对象外，不再定义 `Raw_ADC_Value`、计数器、标志位等分散全局变量。

建议集中定义配置宏，避免在初始化代码中散布时钟和DMA魔数：

```c
#define AD7982_EPWM_TBPRD              (1499U)
#define AD7982_MCBSP_CLKGDV            (3U)
#define AD7982_DMA_PERINTSEL_MREVTA    (15U)
#define AD7982_DMA_BURST_SIZE          (1U)
#define AD7982_DMA_TRANSFER_SIZE       (0U)
```

## 4. 具体修改步骤

### 4.1 清理 Demo 冲突项

实施前先处理以下冲突：

1. 停用 SCI-B 的 GPIO22/GPIO23 配置。可以不调用 `Init_Scib()`，也可以直接删除 Demo 中对应初始化；GPIO22/GPIO23 以后专用于 AD7982。
2. 现有片内ADC及DMA CH1必须保留，继续保留 `InitAdc()`、`Init_ADC_DMA()`、`GetAdc()` 和 `g_AdcRawBuf`。AD7982固定使用DMA CH2，不得占用或重配CH1。
3. 不允许在 AD7982 DMA 配置之后再执行 `DmaRegs.DMACTRL.bit.HARDRESET=1`。DMA 全局硬复位只能在所有通道配置之前执行一次。
4. 删除或停用与本链路无关的 XINTF、DSO validation 等临时验证逻辑，避免 Timer0 高优先级 ISR 过长。
5. 按 `架构.md` 的规则禁止中断嵌套。删除现有 ISR 内用于重新开启全局中断的 `EINT`，ISR 入口不需要再次执行 `DINT`。

片内ADC DMA CH1先完成全局DMA硬复位和CH1配置，随后再配置AD7982 DMA CH2。AD7982初始化不得修改CH1寄存器，也不得再次执行DMA全局硬复位。

### 4.2 新增扁平模块上下文

在 `drv_ad7982.h` 中定义：

```c
typedef struct
{
    volatile Uint32 raw_adc_value;
    volatile Uint32 frame_count;
    volatile Uint32 launch_overrun_count;
    volatile Uint16 sample_valid;
} Ad7982_Context_t;
```

`Ad7982_Init()`接收非空context指针并显式初始化所有成员：

```c
context->raw_adc_value = 0U;
context->frame_count = 0U;
context->launch_overrun_count = 0U;
context->sample_valid = 0U;
```

`raw_adc_value`应保持32-bit对齐。不要将它拆成两个无关联的 `Uint16` 成员。不要在驱动内部保存context副本或context全局指针。

### 4.3 GPIO21/22/23 配置

新增专用函数，例如：

```c
static void Ad7982_InitGpio(void);
```

不要直接调用公共函数 `InitMcbspaGpio()`，因为该函数还会配置 GPIO20，并可能包含本项目不需要的输入脚设置。

配置原则：

```c
EALLOW;

/* GPIO21 = MDRA */
GpioCtrlRegs.GPAMUX2.bit.GPIO21  = 2;
GpioCtrlRegs.GPAPUD.bit.GPIO21   = 0;
GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 3;

/* GPIO22 = MCLKXA */
GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 2;
GpioCtrlRegs.GPAPUD.bit.GPIO22  = 0;

/* GPIO23 = MFSXA/CNV */
GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 2;
GpioCtrlRegs.GPAPUD.bit.GPIO23  = 0;

EDIS;
```

GPIO22和GPIO23是外设输出，不需要把它们配置成普通GPIO输出；方向由McBSP外设控制。

### 4.4 McBSP-A 配置

新增专用函数，例如：

```c
static void Ad7982_InitMcbspa(void);
```

初始化期间保持发送器、接收器、采样率发生器和帧发生器复位：

```c
McbspaRegs.SPCR2.all = 0;
McbspaRegs.SPCR1.all = 0;
```

主要配置：

```c
/* SPI 主机，CLKX/FSX 均由 McBSP-A 输出 */
McbspaRegs.PCR.bit.CLKXM = 1;
McbspaRegs.PCR.bit.FSXM  = 1;
McbspaRegs.PCR.bit.FSXP  = 1;  /* MFSXA/CNV 低有效 */

/* 内部 LSPCLK 驱动采样率发生器 */
McbspaRegs.PCR.bit.SCLKME  = 0;
McbspaRegs.SRGR2.bit.CLKSM = 1;

/* AD7982：SCK 空闲低，上升沿采集 */
McbspaRegs.SPCR1.bit.CLKSTP = 3;
McbspaRegs.PCR.bit.CLKXP    = 0;
McbspaRegs.PCR.bit.CLKRP    = 1;

/* 单相、每帧一个 32-bit word */
McbspaRegs.XCR2.bit.XPHASE  = 0;
McbspaRegs.RCR2.bit.RPHASE  = 0;
McbspaRegs.XCR1.bit.XFRLEN1 = 0;
McbspaRegs.RCR1.bit.RFRLEN1 = 0;
McbspaRegs.XCR1.bit.XWDLEN1 = 5;
McbspaRegs.RCR1.bit.RWDLEN1 = 5;

McbspaRegs.XCR2.bit.XDATDLY = 1;
McbspaRegs.RCR2.bit.RDATDLY = 1;

/* 写 DXR 后产生一次帧，不使用自由运行周期帧 */
McbspaRegs.SRGR2.bit.FSGM = 0;

/* LSPCLK=37.5 MHz，SCK=37.5/(3+1)=9.375 MHz */
McbspaRegs.SRGR1.bit.CLKGDV = AD7982_MCBSP_CLKGDV;
```

复位释放顺序：

1. 配置寄存器完成后等待至少两个 SRG 输入时钟。
2. `GRST=1`，启动采样率发生器。
3. 等待至少两个 CLKG 周期。
4. `XRST=1`、`RRST=1`。
5. `FRST=1`，释放帧同步发生器。
6. 清除可能残留的接收/同步状态。

可以复用工程现有的 `delay_loop()` 和 `clkg_delay_loop()`，但应核对其中常量是否与 `CLKGDV=3` 一致；更推荐在AD7982驱动中用明确的短延时实现，避免依赖公共McBSP Demo参数。

### 4.5 DMA CH2 配置

新增专用函数，例如：

```c
static void Ad7982_InitDmaCh2(Ad7982_Context_t *context);
```

本函数不得执行DMA全局硬复位，只配置CH2。

模式配置：

```c
DmaRegs.CH2.MODE.all = 0;
DmaRegs.CH2.MODE.bit.PERINTSEL = AD7982_DMA_PERINTSEL_MREVTA;
DmaRegs.CH2.MODE.bit.PERINTE = 1;
DmaRegs.CH2.MODE.bit.CHINTMODE = 1;  /* transfer结束时中断 */
DmaRegs.CH2.MODE.bit.ONESHOT = 0;
DmaRegs.CH2.MODE.bit.CONTINUOUS = 1;
DmaRegs.CH2.MODE.bit.DATASIZE = 0;   /* 16-bit DMA word */
DmaRegs.CH2.MODE.bit.CHINTE = 1;
```

一次ADC样点由两个16-bit DMA word组成：

```c
DmaRegs.CH2.BURST_SIZE.bit.BURSTSIZE = AD7982_DMA_BURST_SIZE;
DmaRegs.CH2.TRANSFER_SIZE = AD7982_DMA_TRANSFER_SIZE;
```

源地址从DRR2开始，按地址递增依次读取DRR2、DRR1：

```c
DmaRegs.CH2.SRC_BEG_ADDR_SHADOW = (Uint32)&McbspaRegs.DRR2.all;
DmaRegs.CH2.SRC_ADDR_SHADOW     = (Uint32)&McbspaRegs.DRR2.all;
DmaRegs.CH2.SRC_BURST_STEP      = 1;
```

C28x采用16-bit word地址。`context->raw_adc_value`的低地址是低16位，高地址是高16位。为了形成期望拼接顺序，DMA目的地址应从高16位开始，第二拍地址减1：

```c
Uint16 *raw_word_ptr = (Uint16 *)&context->raw_adc_value;

DmaRegs.CH2.DST_BEG_ADDR_SHADOW = (Uint32)&raw_word_ptr[1];
DmaRegs.CH2.DST_ADDR_SHADOW     = (Uint32)&raw_word_ptr[1];
DmaRegs.CH2.DST_BURST_STEP      = -1;
```

其余step/wrap寄存器显式清零，避免Demo残值：

```c
DmaRegs.CH2.SRC_TRANSFER_STEP = 0;
DmaRegs.CH2.DST_TRANSFER_STEP = 0;
DmaRegs.CH2.SRC_WRAP_SIZE = 0xFFFF;
DmaRegs.CH2.DST_WRAP_SIZE = 0xFFFF;
DmaRegs.CH2.SRC_WRAP_STEP = 0;
DmaRegs.CH2.DST_WRAP_STEP = 0;
```

启动前清外设事件、同步和错误标志，然后：

```c
DmaRegs.CH2.CONTROL.bit.RUN = 1;
```

DMA完成后的目标结果必须为：

```text
context->raw_adc_value[31:16] = DRR2
context->raw_adc_value[15:0]  = DRR1
```

### 4.6 ePWM1 100 kHz配置

新增专用函数，例如：

```c
static void Ad7982_InitEPwm1(void);
```

按 `SYSCLK=150 MHz`、ePWM分频均为1、向上计数计算：

```text
TBCLK = 150 MHz
TBPRD = 150 MHz / 100 kHz - 1 = 1499
```

建议配置：

```c
EPwm1Regs.TBCTL.bit.CTRMODE   = TB_COUNT_UP;
EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
EPwm1Regs.TBCTL.bit.CLKDIV    = TB_DIV1;
EPwm1Regs.TBPRD = AD7982_EPWM_TBPRD;
EPwm1Regs.TBCTR = 0;

EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
EPwm1Regs.ETSEL.bit.INTEN  = 1;
EPwm1Regs.ETPS.bit.INTPRD  = ET_1ST;
EPwm1Regs.ETCLR.bit.INT    = 1;
```

配置ePWM时建议暂时清 `TBCLKSYNC`，所有AD7982相关外设和PIE配置完成后再统一置位，避免初始化中途产生首个中断。

ePWM1不需要输出到GPIO0/GPIO1，也不需要配置AQ波形；它只使用计数器和中断事件。

### 4.7 ePWM1快速路径与应用层ISR包装

AD7982模块提供带context参数的快速处理函数：

```c
void Ad7982_StartFrame(Ad7982_Context_t *context)
{
    if (McbspaRegs.SPCR2.bit.XRDY != 0)
    {
        McbspaRegs.DXR2.all = 0;
        McbspaRegs.DXR1.all = 0; /* 写低16位后启动32-bit帧 */
    }
    else
    {
        context->launch_overrun_count++;
    }
}
```

PIE向量指向应用层中断包装函数。包装函数只传递context、清中断标志并ACK：

```c
interrupt void Interrupt_Ad7982EPwm1Isr(void)
{
    Ad7982_StartFrame(&app_context.ad7982);

    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}
```

`Interrupt_Ad7982EPwm1Isr`属于中断适配模块，因此使用 `Interrupt_` 模块前缀；`app_context`由应用层所有，不定义在AD7982驱动中。

ISR中禁止：

- 等待XRDY的while循环。
- 软件延时。
- 读取DRR。
- 18-bit解析、除法、Modbus处理。
- `EINT`开启嵌套。

如果确认硬件始终能在10 us内完成，也仍建议保留XRDY单次判断和异常计数，不应阻塞等待。`Ad7982_StartFrame()`应被编译器内联；若未内联，需通过反汇编或实测确认函数调用开销可以接受。

### 4.8 DMA CH2快速路径与应用层ISR包装

AD7982模块提供带context参数的完成处理函数：

```c
void Ad7982_OnDmaComplete(Ad7982_Context_t *context)
{
    context->frame_count++;

    if (context->frame_count >= 2U)
    {
        context->sample_valid = 1U;
    }
}
```

应用层DMA中断包装函数：

```c
interrupt void Interrupt_Ad7982DmaCh2Isr(void)
{
    Ad7982_OnDmaComplete(&app_context.ad7982);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;
}
```

如DMA控制器要求软件清通道事件或错误标志，应在ACK前按芯片手册补充清除，但不要重新启动整个DMA模块。

当前阶段不在ISR中重新复制 `context->raw_adc_value`，因为DMA已经直接写入该context成员。

后续实现18-bit解析和平均值时，再把符号扩展、实时值更新、累加及完成锁存加入此ISR。

### 4.9 PIE向量和中断使能

在 `PieInit()` 中增加：

```c
EALLOW;
PieVectTable.EPWM1_INT = &Interrupt_Ad7982EPwm1Isr;
PieVectTable.DINTCH2   = &Interrupt_Ad7982DmaCh2Isr;
EDIS;

PieCtrlRegs.PIEIER3.bit.INTx1 = 1; /* ePWM1 */
PieCtrlRegs.PIEIER7.bit.INTx2 = 1; /* DMA CH2 */

IER |= M_INT3;
IER |= M_INT7;
```

保留需要的Timer0和CAN中断时，也必须遵守禁止嵌套规则。所有ISR都不应主动执行 `EINT`。

### 4.10 主函数初始化顺序

推荐顺序：

```text
InitSysCtrl
  -> DINT
  -> InitPieCtrl / InitPieVectTable
  -> InitGpio
  -> InitAdc（保留片内ADC）
  -> Init_ADC_DMA（执行一次DMA全局硬复位并配置CH1）
  -> 其他仍保留的外设初始化
  -> Ad7982_Init(&app_context.ad7982)
       -> 初始化context成员
       -> AD7982 GPIO初始化
       -> AD7982 McBSP-A初始化
       -> AD7982 DMA CH2初始化
       -> AD7982 ePWM1初始化（暂不运行TBCLK）
  -> 配置PIE向量和使能位
  -> 清ePWM、DMA、PIE残留标志
  -> 确认DMA CH1仍为片内ADC配置
  -> 启动DMA CH2
  -> TBCLKSYNC=1
  -> EINT / ERTM
```

`Ad7982_Init()`封装context、GPIO、McBSP、DMA CH2和ePWM配置，但PIE向量建议仍在统一的 `PieInit()` 中设置。调用完成后禁止再次执行 `Init_ADC_DMA()`，否则其中的DMA全局硬复位会同时清除CH2。

不要在链路开始运行后再次调用 `InitGpio()`、`InitMcbspaGpio()`、`InitMcbspa()`或DMA硬复位函数，以免覆盖专用配置。

## 5. 首样点处理

MFSXA空闲状态应为高。第一次ePWM事件会让MFSXA下降并读取数据，帧结束后的MFSXA上升沿才启动下一次转换，因此第一次接收的数据可能不是有效转换结果。

当前Demo阶段采用简单方式：丢弃第一次DMA完成结果。

首样点状态直接通过 `frame_count` 判断，不增加单独的丢弃标志。第一次DMA完成后 `frame_count=1U`，保持 `sample_valid=0U`；第二次DMA完成后置 `sample_valid=1U`。调用方只有在 `sample_valid!=0U` 时才能使用 `raw_adc_value`。

后续若需要上电后第一次读取也有效，可在GPIO复用为MFSXA之前使用普通GPIO手动制造一次CNV上升沿并等待转换完成；当前阶段没有必要增加这段切换逻辑。

## 6. 原始数据格式

本阶段只确认DMA拼接，不立即做18-bit符号转换。

按当前架构假设，有效数据位为：

```text
context->raw_adc_value[31:14]
```

后续转换公式：

```c
Uint32 raw18 = (context->raw_adc_value >> 14) & 0x3FFFFUL;
int32 adc_value = (int32)raw18;

if ((raw18 & 0x20000UL) != 0)
{
    adc_value -= 0x40000L;
}
```

位对齐必须通过固定输入电压和逻辑分析仪实测确认。在确认之前，不要把 `[31:14]` 假设扩散到Modbus和校准模块。

## 7. 编译与下板验证顺序

### 7.1 静态检查

1. 工程中只有AD7982驱动配置GPIO21/22/23。
2. 没有SCI-B初始化覆盖GPIO22/23。
3. DMA全局硬复位只在 `Init_ADC_DMA()` 中发生一次，且位于AD7982 DMA CH2配置之前。
4. 片内ADC DMA CH1配置和运行保持有效；AD7982初始化没有修改任何CH1寄存器。
5. CH2触发源为15，CH1/其他通道没有覆盖CH2。
6. McBSP-A字长为32 bit，而不是现有Demo的16 bit。
7. AD7982驱动中没有模块全局状态或静态context指针。
8. 所有导出函数和类型使用 `Ad7982_` 前缀，变量和宏符合本文编码规范。
9. 所有新增ISR均没有等待、延时、除法和 `EINT`。

### 7.2 示波器/逻辑分析仪检查

按顺序确认：

1. GPIO23/MFSXA空闲为高。
2. MFSXA下降沿频率为100 kHz，周期约10 us。
3. 每个低电平窗口内正好有32个GPIO22时钟。
4. SCK空闲为低。
5. MFSXA下降到第一个SCK上升沿约为半个SCK周期。
6. SCK频率约9.375 MHz，单帧时长约3.41 us。
7. MFSXA在32-bit帧结束时自动升高。
8. MFSXA高电平转换时间约为6.59 us。
9. GPIO21数据在SCK下降沿附近变化，在上升沿前后稳定。

### 7.3 DMA检查

1. `context->frame_count`稳定按约100000次/秒增加。
2. `context->launch_overrun_count`保持为0。
3. `context->sample_valid`在第二个DMA样点完成后置1。
4. `context->raw_adc_value`随模拟输入变化，不恒定为0或全1。
5. 固定输入条件下检查DRR2/DRR1和raw成员的高低16位顺序。
6. 同时确认片内ADC DMA CH1缓冲仍正常更新。
7. 检查DMA溢出、同步错误和McBSP接收同步错误标志。

### 7.4 时序与CPU占用检查

ePWM ISR每秒执行100000次，DMA ISR每秒执行100000次。必须测量两个ISR总耗时和Timer0阻塞时间。

`SYSCLK=150 MHz`时每个10 us周期约1500个CPU cycle。当前阶段按以下原则控制执行时间：

- ePWM快速路径只做一次XRDY读取、一个分支、两次DXR写入；正常路径不修改context计数器。
- DMA快速路径只做计数和首样点状态更新，不复制32-bit原始值。
- context指针在ISR入口只求值一次，快速函数应内联，禁止重复层级调用。
- 100 kHz路径不做逐次空指针检查；context合法性只在 `Ad7982_Init()` 时检查一次。
- 不使用64-bit运算、浮点、除法、库函数、动态内存或通用回调。
- Timer0继续驱动片内ADC时，必须实测其最长执行时间，删除无关的XINTF/DSO验证耗时逻辑。

建议用GPIO翻转或CPU Timer测量：ePWM ISR和当前DMA ISR各自尽量控制在1 us以内，两者加上Timer0最坏阻塞后仍必须显著小于10 us。后续把18-bit转换和平均值累加加入DMA ISR后，应重新测量并检查编译器生成的汇编。

## 8. 当前阶段完成判据

满足以下条件即可认为“McBSP-A ADC配置部分”完成：

- GPIO21/22/23复用正确，无SCI-B覆盖。
- ePWM1稳定以100 kHz发起McBSP帧。
- MFSXA/CNV自动产生正确的低读数窗口和高转换窗口。
- 每帧有32个约9.375 MHz的SCK。
- McBSP-A在SCK上升沿接收数据。
- DMA CH2由MREVTA触发，每帧搬运DRR2和DRR1。
- `context->raw_adc_value`连续更新且高低字顺序正确。
- 首样点被丢弃或明确标记无效。
- 片内ADC DMA CH1保持正常运行，AD7982 DMA CH2未覆盖其配置。
- AD7982模块没有用全局变量保存状态，所有处理函数通过 `Ad7982_Context_t *` 访问状态。
- 无McBSP同步错误、DMA溢出和ePWM启动overrun。
- 未加入CAN统计、平均值或Modbus业务逻辑。

完成上述验证后，再进入18-bit符号扩展、实时值、CAN触发统计和平均值计算阶段。
