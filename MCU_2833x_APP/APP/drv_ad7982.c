#include "drv_ad7982.h"
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "DSP2833x_EPwm_defines.h"

/*
 * AD7982采样链路：ePWM1以100 kHz发起McBSP-A 32-bit帧，
 * MREVTA触发DMA CH2将DRR2/DRR1直接写入模块Context。
 * DMA CH1保留给片内ADC，本模块不得修改。
 */
#define AD7982_EPWM_TBPRD              (1499U)
#define AD7982_MCBSP_CLKGDV            (3U)
#define AD7982_DMA_PERINTSEL_MREVTA    (15U)
#define AD7982_DMA_BURST_SIZE          (1U)
#define AD7982_DMA_TRANSFER_SIZE       (0U)
#define AD7982_MCBSP_READY_TIMEOUT     (65535U)

static void Ad7982_InitGpio(void);
static void Ad7982_InitMcbspa(void);
static void Ad7982_InitDmaCh2(Ad7982_Context_t *context);
static void Ad7982_InitEPwm1(void);

void Ad7982_Init(Ad7982_Context_t *context)
{
    if (context == 0)
    {
        return;
    }

    context->raw_adc_value = 0U;
    context->frame_count = 0U;
    context->launch_overrun_count = 0U;
    context->sample_valid = 0U;
    context->start_error = 0U;

    Ad7982_InitMcbspa();
    Ad7982_InitGpio();
    Ad7982_InitDmaCh2(context);
    Ad7982_InitEPwm1();
}

void Ad7982_Start(Ad7982_Context_t *context)
{
    Uint16 timeout_count = AD7982_MCBSP_READY_TIMEOUT;

    if (context == 0)
    {
        return;
    }

    /* 触发源保持停止，限时等待McBSP发送端真正可接收首帧。 */
    while ((McbspaRegs.SPCR2.bit.XRDY == 0U) && (timeout_count > 0U))
    {
        timeout_count--;
    }

    if (timeout_count == 0U)
    {
        context->start_error = 1U;
        return;
    }

    context->start_error = 0U;

    /* 先清历史状态并启动DMA，最后才开放ePWM触发源。 */
    EALLOW;
    DmaRegs.CH2.CONTROL.bit.PERINTCLR = 1U;
    DmaRegs.CH2.CONTROL.bit.SYNCCLR = 1U;
    DmaRegs.CH2.CONTROL.bit.ERRCLR = 1U;
    DmaRegs.CH2.CONTROL.bit.RUN = 1U;
    EDIS;

    EPwm1Regs.ETCLR.bit.INT = 1U;
    PieCtrlRegs.PIEIFR3.bit.INTx1 = 0U;
    PieCtrlRegs.PIEIFR7.bit.INTx2 = 0U;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3 | PIEACK_GROUP7;
    EPwm1Regs.ETSEL.bit.INTEN = 1U;

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1U;
    EDIS;
}

void Ad7982_StartFrame(Ad7982_Context_t *context)
{
    /* 100 kHz快速路径只启动一帧，不在此处解析或复制接收数据。 */
    if (McbspaRegs.SPCR2.bit.XRDY != 0U)
    {
        McbspaRegs.DXR2.all = 0U;
        McbspaRegs.DXR1.all = 0U;
    }
    else
    {
        context->launch_overrun_count++;
    }
}

void Ad7982_OnDmaComplete(Ad7982_Context_t *context)
{
    context->frame_count++;

    /* 首帧发生在第一个有效CNV上升沿之前，从第二帧开始标记有效。 */
    if (context->frame_count >= 2U)
    {
        context->sample_valid = 1U;
    }
}

static void Ad7982_InitGpio(void)
{
    EALLOW;

    /* GPIO21=MDRA/SDO，GPIO22=MCLKXA/SCK。 */
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 2U;
    GpioCtrlRegs.GPAPUD.bit.GPIO21 = 0U;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 3U;

    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 2U;
    GpioCtrlRegs.GPAPUD.bit.GPIO22 = 0U;

    /* GPIO23复用为MFSXA/CNV；复用前预置高电平，避免错误转换沿。 */
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0U;
    GpioDataRegs.GPASET.bit.GPIO23 = 1U;
    GpioCtrlRegs.GPADIR.bit.GPIO23 = 1U;
    GpioCtrlRegs.GPAPUD.bit.GPIO23 = 0U;
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 2U;

    EDIS;
}

static void Ad7982_InitMcbspa(void)
{
    /* 先保持发送、接收、采样率发生器和帧同步逻辑在复位状态。 */
    McbspaRegs.SPCR2.all = 0U;
    McbspaRegs.SPCR1.all = 0U;
    McbspaRegs.PCR.all = 0U;
    McbspaRegs.RCR2.all = 0U;
    McbspaRegs.RCR1.all = 0U;
    McbspaRegs.XCR2.all = 0U;
    McbspaRegs.XCR1.all = 0U;
    McbspaRegs.SRGR2.all = 0U;
    McbspaRegs.SRGR1.all = 0U;

    /* SPI主机模式：CLKX和FSX由McBSP-A输出，MFSXA低有效。 */
    McbspaRegs.PCR.bit.CLKXM = 1U;
    McbspaRegs.PCR.bit.FSXM = 1U;
    McbspaRegs.PCR.bit.FSXP = 1U;
    McbspaRegs.PCR.bit.SCLKME = 0U;

    /* LSPCLK=37.5 MHz，CLKGDV=3，对应SCK=9.375 MHz。 */
    McbspaRegs.SRGR2.bit.CLKSM = 1U;
    McbspaRegs.SRGR2.bit.FSGM = 0U;
    McbspaRegs.SRGR1.bit.CLKGDV = AD7982_MCBSP_CLKGDV;

    /* SCK空闲低，半周期延迟后输出时钟，在上升沿采集数据。 */
    McbspaRegs.SPCR1.bit.CLKSTP = 3U;
    McbspaRegs.PCR.bit.CLKXP = 0U;
    McbspaRegs.PCR.bit.CLKRP = 1U;

    /* 单相、每帧一个32-bit字，MFSXA在整个读取窗口保持低。 */
    McbspaRegs.XCR2.bit.XPHASE = 0U;
    McbspaRegs.RCR2.bit.RPHASE = 0U;
    McbspaRegs.XCR1.bit.XFRLEN1 = 0U;
    McbspaRegs.RCR1.bit.RFRLEN1 = 0U;
    McbspaRegs.XCR1.bit.XWDLEN1 = 5U;
    McbspaRegs.RCR1.bit.RWDLEN1 = 5U;
    McbspaRegs.XCR2.bit.XDATDLY = 1U;
    McbspaRegs.RCR2.bit.RDATDLY = 1U;

    /* 按McBSP要求依次释放采样率发生器、收发器和帧同步。 */
    DELAY_US(1L);
    McbspaRegs.SPCR2.bit.GRST = 1U;
    DELAY_US(1L);
    McbspaRegs.SPCR2.bit.XRST = 1U;
    McbspaRegs.SPCR1.bit.RRST = 1U;
    McbspaRegs.SPCR2.bit.FRST = 1U;
}

static void Ad7982_InitDmaCh2(Ad7982_Context_t *context)
{
    Uint16 *raw_word_ptr = (Uint16 *)&context->raw_adc_value;

    EALLOW;

    DmaRegs.CH2.CONTROL.bit.SOFTRESET = 1U;
    asm(" NOP");

    DmaRegs.CH2.MODE.all = 0U;
    DmaRegs.CH2.MODE.bit.PERINTSEL = AD7982_DMA_PERINTSEL_MREVTA;
    DmaRegs.CH2.MODE.bit.PERINTE = 1U;
    DmaRegs.CH2.MODE.bit.CHINTMODE = 1U;
    DmaRegs.CH2.MODE.bit.ONESHOT = 0U;
    DmaRegs.CH2.MODE.bit.CONTINUOUS = 1U;
    DmaRegs.CH2.MODE.bit.DATASIZE = 0U;
    DmaRegs.CH2.MODE.bit.CHINTE = 1U;

    /* 一次MREVTA搬运两个16-bit字：DRR2写高16位，DRR1写低16位。 */
    DmaRegs.CH2.BURST_SIZE.bit.BURSTSIZE = AD7982_DMA_BURST_SIZE;
    DmaRegs.CH2.SRC_BURST_STEP = 1;
    DmaRegs.CH2.DST_BURST_STEP = -1;

    DmaRegs.CH2.TRANSFER_SIZE = AD7982_DMA_TRANSFER_SIZE;
    DmaRegs.CH2.SRC_TRANSFER_STEP = 0;
    DmaRegs.CH2.DST_TRANSFER_STEP = 0;

    DmaRegs.CH2.SRC_WRAP_SIZE = 0xFFFFU;
    DmaRegs.CH2.SRC_WRAP_STEP = 0;
    DmaRegs.CH2.DST_WRAP_SIZE = 0xFFFFU;
    DmaRegs.CH2.DST_WRAP_STEP = 0;

    DmaRegs.CH2.SRC_BEG_ADDR_SHADOW = (Uint32)&McbspaRegs.DRR2.all;
    DmaRegs.CH2.SRC_ADDR_SHADOW = (Uint32)&McbspaRegs.DRR2.all;
    DmaRegs.CH2.DST_BEG_ADDR_SHADOW = (Uint32)&raw_word_ptr[1];
    DmaRegs.CH2.DST_ADDR_SHADOW = (Uint32)&raw_word_ptr[1];

    DmaRegs.CH2.CONTROL.bit.PERINTCLR = 1U;
    DmaRegs.CH2.CONTROL.bit.SYNCCLR = 1U;
    DmaRegs.CH2.CONTROL.bit.ERRCLR = 1U;
    EDIS;
}

static void Ad7982_InitEPwm1(void)
{
    /* TBCLK=150 MHz，向上计数至1499，产生100 kHz零点中断。 */
    EPwm1Regs.TBCTL.all = 0U;
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBPRD = AD7982_EPWM_TBPRD;
    EPwm1Regs.TBCTR = 0U;

    EPwm1Regs.ETSEL.all = 0U;
    EPwm1Regs.ETPS.all = 0U;
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
    EPwm1Regs.ETSEL.bit.INTEN = 0U;
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;
    EPwm1Regs.ETCLR.bit.INT = 1U;
}
