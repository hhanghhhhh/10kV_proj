/**
 *****************************************************************************
 * 					存放用户中断处理函数，方便统一处理
 *****************************************************************************
 *
 * @File   : ISR.c
 * @By     : Sun
 * @Version: V0.5
 * @Date   : 2022 / 06 / 01
 * @Shop	: https://xiaomeige.taobao.com/
 *
 *****************************************************************************
 **/

#include "ISR.h"
#include "COMMON.h"
#include "task_onoff.h"

void MultiModeFSM(void);
Uint32 task_run_time = 0;
Uint32 max_task_run_time = 0;

/**
 *****************************************************
 * @brief	私有定时器中断处理程序
 * @tag	本函数用来处理私有定时器中断，在内部加入用户程序即可
 *****************************************************
 **/
void ScuTimer_IRQ_Handler(void *CallBackRef)
{
    /* ↓↓↓用户处理↓↓↓ */

    TaskComFromPl();
    TaskAdcRealtime();

    InitDeviceState();
    RangeLimit();

    // OverTemp();
    MainPowerCtrl();
    if (mgmd_stSCIRx.test_mode == MODE_CURTEST)
        PowerOnCurTest();
    else if (mgmd_stSCIRx.test_mode == MODE_VBR)
        PowerOnVbr();
    else if (mgmd_stSCIRx.test_mode == MODE_UPH)
        PowerOnUph_nometer();
    else if (mgmd_stSCIRx.test_mode == MODE_CAN)
        PowerOnCan();
    else
        PowerOnNormal();

    ProtectFunc();
    PowerOff();
    AutoZeroFunc();
    TaskAdcSample();
    TaskComToPl();
    WaveUploadTask();

    // UPH
    MultiModeFSM();

    task_run_cnt++;
    task_run_time = timer_cnt_max - XScuTimer_GetCounterValue(&ScuTimer);
    if (task_run_time > max_task_run_time)
    {
        max_task_run_time = task_run_time;
    }
    /* ↑↑↑结束处理↑↑↑ */
    XScuTimer_ClearInterruptStatus(&ScuTimer);
}
