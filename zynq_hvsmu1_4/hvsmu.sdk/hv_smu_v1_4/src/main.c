/*
 * main.c
 *
 *  Created on: 2025年6月15日
 *      Author: m
 */

#include "COMMON.h"
#include "xtime_l.h"
#include "drv_can_controller.h"

Uint32 main_run_cnt = 0;

Uint16 hh_test_cmd = 0;
void task_led_main(void)
{
	static Uint32 nowtime = 0;
	static Uint8  state = 0;
	if(task_run_cnt - nowtime > Tms_CNT(500))
	{
		nowtime = task_run_cnt;
		state ^= 0x1;
		PS_GPIO_SetPort(IO_PL_LED1, state);
	}

	if(ReadPlReg(0) > 2500000)
		PS_GPIO_SetPort(IO_PL_LED2, 1);
	else
		PS_GPIO_SetPort(IO_PL_LED2, 0);
}

void RecvProgramData(void);
void Init_taskUPH(void);
void TestDDR3(void);

int main(void)
{
	// 初始化PS端MIO和EMIO
	PS_GPIO_Init();
	// gpio 引脚初始化
	UserGpioInit();
	// 初始化通用中断控制器
	ScuGic_Init();
	// can1 外设初始化
    CanController_Init();
	PS_CAN1_Init();
	// modbusTCP 协议初始化
	TaskModbusTCPInit();
	// IIC0 外设初始化，时钟频率为100k
	PS_IIC0_Init_100k();
	// QSPI 外设初始化
	PS_QSPI_Init();


	// init
	InitVersion();
	InitTaskCom();
	InitTaskAdc();
	InitTaskDac();
	InitTaskRange();
	InitTaskEeprom_COEF();
	InitTaskEeprom_IP();
	InitTaskEeprom_CONFIG();
	Init_taskUPH();
	// LWIP 协议初始化
	PS_LWIP_Init();

	// test
	InitPLddr3();

	// 初始化私有定时器, 开启定时器
	ScuTimer_Int_Init(TIMER_PERIOD);


	while(1)
	{
		main_run_cnt++;

		//tcp 协议
		RunTcpServerMain();

		RunVersion();
		TaskEeprom_COEF();
		TaskEeprom_IP();
		TaskEeprom_CONFIG();

		RecvProgramData();
		TestDDR3();
		// 500ms 翻转一次 led 灯状态
		task_led_main();

        CanController_Process();
		SendMsgToCtrlBoard();
		SendMsgToMeter();
	}
	return 0;
}
