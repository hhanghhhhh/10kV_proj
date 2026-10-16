/***************************************************************************
*	NAME:			ECAT_Main.c
*	DESCRIPTION:	ECAT controller main function
*	VERSION:		1.0
*	AUTHOR:			LuXi
*	ACTION:			build
*	DATE:			2013-3-21
***************************************************************************/
#include "drv_GlobalVar.h"
#include "drv_Adc.h"
#include "Interrupt.h"
#include "drv_fpga.h"
#include <string.h>
#include "drv_Eeprom.h"
#include "drv_MgModbus.h"
#include "W5500/drv_Spi.h"
#include "W5500/device.h"
#include "W5500/udp.h"

#if DSP
	#include "DSP2833x_Device.h"     // Header file Include File
	#include "DSP2833x_Examples.h"   // Examples Include File
    #include "DSP2833x_Xintf.h"
    #include "DSP2833x_Sci.h"
#endif
#include "task_Transfer.h"
#include "task_ADC.h"
#include "task_Range.h"
#include "task_DAC.h"
#include "Version.h"
#include "task_eeprom_download.h"
#include "task_wave.h"

/*======================================================================*/
#include <math.h>

Uint16          slMain100ms;
Uint16          slMain10ms;
Uint16          slMain1ms;
Uint16          slMain1s;
extern float    MonParaArr[333];
extern void HH_test_main();
/***********************************************************************
* Function Name : MainTimeBase
* Arguments     : 
* Returns       :
* Description   :
* Updated ID    : 
***********************************************************************/
void MainTimeBase(void)
{
	if(slMain10ms != slINT10ms)
	{
		slMain10ms = slINT10ms;
	}

    if(slMain1ms != slINT1ms)
    {
        slMain1ms = slINT1ms;
    }

    if(slMain100ms != slINT100ms)
    {
        slMain100ms = slINT100ms;
    }

}



/***********************************************************************
* Function Name : main
* Arguments     : 
* Returns       :
* Description   : main function
* Updated ID    : 
***********************************************************************/
Uint32 hh_main_cnt = 0;
Uint16 hh_test_linled = 0;
void main(void)
{
    Uint16 index = 0;
    /*======================= MEM COPY FLASH TO RAM =========================*/
    //  MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
    //  InitFlash();
    /*=======================================================================*/

        InitSysCtrl();
        InitGpio();
        /*================= PIE configuration ====================================*/
        DINT;
        InitPieCtrl();
        IER = 0x0000;
        IFR = 0x0000;
        InitPieVectTable();
        EALLOW;
        PieVectTable.TINT0 = &INT6;
        PieVectTable.ECAN0INTA = &Interrupt_CanControllerRxIsr;
        EDIS;
        PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
        PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
        PieCtrlRegs.PIEIER9.bit.INTx5 = 0U;
        IER |= M_INT1;
        IER |= M_INT9;    //CANaINT0
        EINT;
        ERTM;
        /*================= PIE configuration ====================================*/
        InitSpiaGpio();
        InitSpi();
        InitMcbspGpio();
        InitMcbsp();
        /* Target board uses CAN-A on GPIO30/31; enable ISR after init. */
        g_can_controller_init_status = CanController_Init(&g_can_controller);
        if (g_can_controller_init_status == CAN_CONTROLLER_STATUS_SUCCESS)
        {
            PieCtrlRegs.PIEIER9.bit.INTx5 = 1U;
        }
        InitI2CGpio();
        InitI2C();
//        InitAdc();
        mgmd_vInit();
        InitCpuTimers();
        //InitXintf();
        DELAY_US(0.4);

        //USER Init
        glb_VarInit();
        InitTaskWave();
        Init_ADCdata(&st_ADCdata);
        Init_RangeData();
        Init_DACData();
        Init_ComFPGA();
        Init_Version(&st_version);
        InitEeromPara(&st_eerom_para);
		InitEeromPara11(&st_eerom_para11);
		InitEeromPara_Downloads();
        //init W5500必须放在InitEeromPara11后边1
        Init_W5500();


        //start timer
        ConfigCpuTimer(&CpuTimer0, 150, SYS_PERIOD);//100us
        StartCpuTimer0();

	    while (1)
	    {
	        MainTimeBase();

	        for(index = 0; index < SOCKET_NUM_USE; index++)
	        {
	            //第一个参数为socket号，直接用0 1 2...完后排列  最多8个
	            do_tcp_server(index, index);   //网口模块
	        }

            if(mgmd_stSCIRx.jump_cmd == JUMP_TO_BOOT)
            {
                mgmd_stSCIRx.jump_cmd = 0;
                // 将下载标志写入eeprom
                WriteEeromPara_Downloads(11);
                load_boot();
            }
			
		    //socket7用来做udp广播
		    do_udp(7);
		    CheckW5500Status();

		    Run_Version(&st_version);

		    RecvAndWriteEerom(&st_eerom_para);
            RecvAndWriteEerom11(&st_eerom_para11);
		    HH_test_main();

		    hh_main_cnt++;

	    }
}

/********************************************************************
*			END, do not code behind this line!!                            *
****************************************************************************/


