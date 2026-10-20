#ifndef ACZ702_LIB_COMMON_H_
#define ACZ702_LIB_COMMON_H_


//系统头文件
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>

//Xilinx头文件
#include "xil_types.h"
#include "xil_cache.h"
#include "sleep.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xscutimer.h"
#include "xqspips.h"
#include "netif/xadapter.h"
#include "lwipopts.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "lwip/inet.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "xil_mmu.h"
#include "xil_cache.h"

//ACZ702头文件
#include "ISR.h"
#include "SCU_GIC.h"
#include "SCU_TIMER.h"
#include "PS_GPIO.h"
#include "PS_LWIP.h"
#include "PS_IIC.h"
#include "PS_QSPI.h"
#include "TYPEDEF.h"
#include "PS_CAN.h"

//用户头文件
#include "com_pl_reg.h"
#include "user_gpio.h"
#include "drv_ModbusData.h"
#include "drv_MgModbus.h"
#include "drv_eeprom.h"
#include "task_adc.h"
#include "task_dac.h"
#include "version.h"
#include "task_protect.h"
#include "task_range.h"
#include "task_onoff.h"
#include "task_eeprom_coef.h"
#include "task_eeprom_config.h"
#include "task_eeprom_ip.h"
#include "task_wave_upload.h"
#include <task_can_ctrlboard.h>
#include <task_can_meter.h>

//用户宏定义
#define	CPU_CLK_HZ	XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ	//CPU时钟频率(单位Hz)
#define INPUT		1
#define OUTPUT		0
#define	REG8		8
#define	REG16		16
#define TIMER_PERIOD  (50)     //us
#define Tms_CNT(t)  ((t) * (1000L / TIMER_PERIOD))
#define ABS(x)		(((x) > 0) ? (x) : (-(x)))
#define MAX(x,y)	(((x) > (y)) ? (x) : (y))
#define MIN(x,y)    (((x) > (y)) ? (y) : (x))

//用户声明引用
extern Uint32 timer_cnt_max;
extern Uint32 task_run_cnt;


// 1. 函数内定义
//  static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
// 2. 使用：
// delay 并且状态跳转
//		  FSM_DELAY_NEXT(st_delay, x_ms, S_STATE);
// 3. 增加delay状态
//    case S_FSM_DELAY:
//        FSM_DELAY_FUNC(st_delay);
//        break;
//状态机
////FSM_DELAY_NEXT(st_delay, x_ms, S_STATE);
//static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
//static Uint16 state = 0;
//switch(state)
//{
//case S_FSM_DELAY:
//	FSM_DELAY_FUNC(st_delay);
//	break;
//case 0:
//	break;
//case 1:
//	break;
//default: state = 0;
//}

#define	S_FSM_DELAY		999

typedef struct
{
    Uint16 firstflag;
    Uint32 now_time;
    float32 delay_ms;
    Uint16 next_state;
}ST_FSM_DELAY;


#define 	FSM_DELAY_NEXT(v, delayms, nextstate)	    	\
do{                                                         \
	v.firstflag = 0;                            			\
	v.delay_ms = delayms;                            		\
	v.next_state = nextstate;                        		\
	state = S_FSM_DELAY;                         			\
}while(0)


#define     FSM_DELAY_FUNC(v)                               \
do{                                                         \
    if(v.firstflag == 0)                                    \
    {                                                       \
        v.firstflag = 1;                                    \
        v.now_time = task_run_cnt;                          \
    }                                                       \
    if((task_run_cnt - v.now_time) >= Tms_CNT(v.delay_ms))  \
    {                                                       \
        v.firstflag = 0;                                    \
        state = v.next_state;                               \
    }                                                       \
}while(0)




#endif /* ACZ702_LIB_COMMON_H_ */
