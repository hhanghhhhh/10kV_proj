/*****************************************************************************************
*	NAME:			ECAT_INT.c
*	DESCRIPTION:	ECAT controller interrupt function
*	VERSION:		1.0
*	AUTHOR:			LuXi
*	ACTION:			build
*	DATE:			2013-3-21
******************************************************************************************/
#include "drv_GlobalVar.h"

#if DSP
/*========================= Head file in DSP============================*/
	#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
	#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
/*======================================================================*/
#endif
#include <math.h>
#include "drv_Adc.h"
#include "drv_fpga.h"
#include "drv_MgModbus.h"
#include "task_Transfer.h"
#include "task_ADC.h"
/***********************************************************************
* Variable Definition for instruction
***********************************************************************/

Uint16  slINTcnt;
Uint16  slINT1ms;
Uint16  slINT10ms;
Uint16  slINT100ms;
Uint16  slINT1s;


Uint32          task_run_cnt = 0;
Uint32          task_run_time = 0;
Uint32          max_task_run_time = 0;
/**********************************************************************
* Function Name : IntTimeBase
* Arguments     : 
* Returns       :
* Description   : Generate the basic time base, 100ms
* Updated ID    : 
***********************************************************************/
void IntTimeBase(void)
{
	slINTcnt++;

	if(slINTcnt == SYS_Tb_ms)
	{
		slINT1ms++;
		slINTcnt = 0;
	}

	if(slINT1ms == 10)
	{
		slINT10ms++;
		slINT1ms = 0;
	}

	if(slINT10ms == 10)
	{
		slINT100ms++;
		slINT10ms = 0;
	}
	if(slINT100ms == 10)
	{
		slINT1s++;
		slINT100ms = 0;
	}
}





/***********************************************************************
* Function Name :
* Arguments     : 
* Returns       :
* Description   : main INT-ISR
* Updated ID    : 
***********************************************************************/
extern void HH_test_INT(void);


/* CAN-A接收中断：保存采样板回复并应答PIE。 */
interrupt void Interrupt_CanControllerRxIsr(void)
{
    (void)CanController_HandleRxInterrupt(&g_can_controller);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}



interrupt void INT6(void) 				
{
	DINT;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
    CpuTimer0Regs.TCR.bit.TIF=1;
    CpuTimer0Regs.TCR.bit.TRB=1;

    //enable can interrupt
    IER |= M_INT9;
    EINT;

    task_run_cnt++;

    IntTimeBase();

	//GetAdc();


	HH_test_INT();

	ADCSample(&st_ADCdata);

	Run_ComFPGA();

	task_run_time = TIMER_CNT_MAX - NOW_TIMER_CNT;
	if(task_run_time > max_task_run_time)
	{
	    max_task_run_time = task_run_time;
	}

	return;	
}

/***************************************************************************
*			END, do not code behind this line!!                            *
****************************************************************************/

