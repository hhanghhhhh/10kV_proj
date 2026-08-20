#include "drv_GlobalVar.h"
#include "Main.h"
#include "drv_Adc.h"
#include "DSP2833x_Device.h"   // Header file Include File
#include "DSP2833x_Examples.h" // Examples Include File
#include "Version.h"
#include "drv_Fpga.h"

Uint32 task_run_cnt = 0;
Uint32 task_run_time = 0;
Uint32 max_task_run_time = 0;

interrupt void Interrupt_CanA0Isr(void)
{
    /* 有效START_AVG的请求信息由CAN模块保存，统计状态后续在此处接入。 */
    (void)CanSample_HandleRxInterrupt(&app_context.can_sample);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

interrupt void Interrupt_Ad7982EPwm1Isr(void)
{
    /* 仅启动McBSP帧，保持100 kHz ISR路径固定且短小。 */
    Ad7982_StartFrame(&app_context.ad7982);

    EPwm1Regs.ETCLR.bit.INT = 1U;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void Interrupt_Ad7982DmaCh2Isr(void)
{
    /* 原始数据已由DMA直接写入Context，此处只更新完成状态。 */
    Ad7982_OnDmaComplete(&app_context.ad7982);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;
}

// 定时器中断
interrupt void INT6(void)
{
    CpuTimer0Regs.TCR.bit.TIF = 1U;

    // 读 fpga 寄存器
    // FpgaISRReadUpdate();

    // dsp 自带 adc 采样
    GetAdc();

    // 写 fpga 寄存器
    // FpgaISRWriteUpdate();

    task_run_cnt++;
    task_run_time = TIMER_CNT_MAX - NOW_TIMER_CNT;
    if (task_run_time > max_task_run_time)
    {
        max_task_run_time = task_run_time;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
    return;
}

/***************************************************************************
 *			END, do not code behind this line!!                            *
 ****************************************************************************/
