#include "drv_digipot.h"
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define DIGIPOT_MCBSP_CLKGDV (36U)
#define DIGIPOT_READY_TIMEOUT (65535U)
#define DIGIPOT_FRAME_WAIT_US (10.0F)
#define DIGIPOT_DEVICE_TPL0501 (0U)
#define DIGIPOT_DEVICE_AD5290 (1U)

/* 两个CS的对应关系只在此处定义，硬件调整时交换这两个编号即可。 */
#define DIGIPOT_TPL0501_CS_HIGH() (GpioDataRegs.GPASET.bit.GPIO27 = 1U)
#define DIGIPOT_TPL0501_CS_LOW() (GpioDataRegs.GPACLEAR.bit.GPIO27 = 1U)
#define DIGIPOT_AD5290_CS_HIGH() (GpioDataRegs.GPASET.bit.GPIO28 = 1U)
#define DIGIPOT_AD5290_CS_LOW() (GpioDataRegs.GPACLEAR.bit.GPIO28 = 1U)

static void Digipot_InitGpio(void);
static void Digipot_SetChipSelect(Uint16 device, Uint16 selected);
static Uint16 Digipot_WriteByte(Uint16 device, Uint16 value);

void Digipot_Init(void)
{
    Digipot_InitGpio();

    /* McBSP-B固定为SPI Mode 0、MSB first、单帧8-bit发送。 */
    McbspbRegs.SPCR2.all = 0x0000U;
    McbspbRegs.SPCR1.all = 0x0000U;
    McbspbRegs.RCR2.all = 0x0000U;
    McbspbRegs.RCR1.all = 0x0000U;
    McbspbRegs.XCR2.all = 0x0000U;
    McbspbRegs.XCR1.all = 0x0000U;
    McbspbRegs.SRGR2.all = 0x0000U;
    McbspbRegs.SRGR1.all = 0x0000U;
    McbspbRegs.MCR2.all = 0x0000U;
    McbspbRegs.MCR1.all = 0x0000U;
    McbspbRegs.PCR.all = 0x0000U;

    McbspbRegs.PCR.bit.CLKXM = 1U;
    McbspbRegs.PCR.bit.FSXM = 1U;
    McbspbRegs.PCR.bit.FSXP = 1U;
    McbspbRegs.PCR.bit.CLKXP = 0U;
    McbspbRegs.PCR.bit.CLKRP = 1U;
    McbspbRegs.PCR.bit.SCLKME = 0U;
    McbspbRegs.SRGR2.bit.CLKSM = 1U;
    McbspbRegs.SRGR2.bit.FSGM = 0U;
    McbspbRegs.SPCR1.bit.CLKSTP = 3U;

    McbspbRegs.XCR2.bit.XPHASE = 0U;
    McbspbRegs.XCR2.bit.XDATDLY = 1U;
    McbspbRegs.XCR1.bit.XFRLEN1 = 0U;
    McbspbRegs.XCR1.bit.XWDLEN1 = 0U;
    McbspbRegs.RCR2.bit.RPHASE = 0U;
    McbspbRegs.RCR2.bit.RDATDLY = 1U;
    McbspbRegs.RCR1.bit.RFRLEN1 = 0U;
    McbspbRegs.RCR1.bit.RWDLEN1 = 0U;

    /* LSPCLK=37.5MHz时，SCLK=37.5MHz/(36+1)，约为1.01MHz。 */
    McbspbRegs.SRGR1.bit.CLKGDV = DIGIPOT_MCBSP_CLKGDV;

    DELAY_US(1.0F);
    McbspbRegs.SPCR2.bit.GRST = 1U;
    DELAY_US(1.0F);
    McbspbRegs.SPCR2.bit.XRST = 1U;
    McbspbRegs.SPCR1.bit.RRST = 1U;
    McbspbRegs.SPCR2.bit.FRST = 1U;
}

Uint16 Digipot_WriteTpl0501(Uint16 value)
{
    return Digipot_WriteByte(DIGIPOT_DEVICE_TPL0501, value);
}

Uint16 Digipot_WriteAd5290(Uint16 value)
{
    return Digipot_WriteByte(DIGIPOT_DEVICE_AD5290, value);
}

static void Digipot_InitGpio(void)
{
    /* 先锁存CS高电平，再切换为输出，避免初始化期间误选中器件。 */
    DIGIPOT_TPL0501_CS_HIGH();
    DIGIPOT_AD5290_CS_HIGH();

    EALLOW;
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 3U; /* MDXB */
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 3U; /* MCLKXB */
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0U;
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0U;
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1U;
    GpioCtrlRegs.GPADIR.bit.GPIO28 = 1U;
    GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0U;
    GpioCtrlRegs.GPAPUD.bit.GPIO26 = 0U;
    GpioCtrlRegs.GPAPUD.bit.GPIO27 = 0U;
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0U;
    EDIS;
}

static void Digipot_SetChipSelect(Uint16 device, Uint16 selected)
{
    if (device == DIGIPOT_DEVICE_TPL0501)
    {
        if (selected != 0U)
        {
            DIGIPOT_TPL0501_CS_LOW();
        }
        else
        {
            DIGIPOT_TPL0501_CS_HIGH();
        }
    }
    else
    {
        if (selected != 0U)
        {
            DIGIPOT_AD5290_CS_LOW();
        }
        else
        {
            DIGIPOT_AD5290_CS_HIGH();
        }
    }
}

static Uint16 Digipot_WriteByte(Uint16 device, Uint16 value)
{
    Uint16 timeout_count = DIGIPOT_READY_TIMEOUT;

    while ((McbspbRegs.SPCR2.bit.XRDY == 0U) && (timeout_count > 0U))
    {
        timeout_count--;
    }

    if (timeout_count == 0U)
    {
        return DIGIPOT_STATUS_TIMEOUT;
    }

    Digipot_SetChipSelect(device, 1U);
    McbspbRegs.DXR1.all = value & 0x00FFU;

    /* 1MHz下8-bit帧约8us；先越过寄存器状态更新窗口，再确认移位完成。 */
    DELAY_US(DIGIPOT_FRAME_WAIT_US);
    timeout_count = DIGIPOT_READY_TIMEOUT;
    while ((McbspbRegs.SPCR2.bit.XEMPTY == 0U) && (timeout_count > 0U))
    {
        timeout_count--;
    }

    Digipot_SetChipSelect(device, 0U);

    if (timeout_count == 0U)
    {
        return DIGIPOT_STATUS_TIMEOUT;
    }

    return DIGIPOT_STATUS_SUCCESS;
}
