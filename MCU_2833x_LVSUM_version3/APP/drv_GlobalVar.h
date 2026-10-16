


#ifndef __DRV_GLOBAL_VAR_H_
#define __DRV_GLOBAL_VAR_H_

/***************************************************************************
*			END, do not code behind this line!!                            *
****************************************************************************/
#define DSP                 (1)

#if DSP

#else
    #include    "Plecs_Constant.h"  // Only need this header file for PLECS sim
#endif
#include "TypeDefine.h"
#include "drv_can_controller.h"

/* CAN controller context shared by application code and receive ISR. */
extern CanController_Context_t g_can_controller;
extern Uint16 g_can_controller_init_status;
/***************************************************************************
*           END, do not code behind this line!!                            *
****************************************************************************/
/*=========================TIME BASE===============================*/
#define     SYS_PERIOD          (200)     //us
#define     TIMER_CNT_MAX       (30000)   //150M 50us时，TIMER计数值为150*50 = 7500
#define     SYS_FS              (5000)  //1000/SYS_PERIOD *1000
#define     SYS_Tb_ms           (5L)     //1000/SYS_PERIOD
#define     Tms_CNT(t)          ((t)*SYS_Tb_ms)

#define     NOW_TIMER_CNT       (CpuTimer0Regs.TIM.all)
/*=========================LIMIT VALUE===============================*/
    //reset FPGA 高电平时reset
    #define RESET_CMD_H             (GpioDataRegs.GPCSET.bit.GPIO80 = 1)
    #define RESET_CMD_L             (GpioDataRegs.GPCCLEAR.bit.GPIO80 = 1)
	
    //W5500
    #define W5500_RESET_H           (GpioDataRegs.GPBSET.bit.GPIO49 = 1)
    #define W5500_RESET_L           (GpioDataRegs.GPBCLEAR.bit.GPIO49 = 1)
    #define W5500_CSN_H             (GpioDataRegs.GPASET.bit.GPIO19 = 1)
    #define W5500_CSN_L             (GpioDataRegs.GPACLEAR.bit.GPIO19 = 1)

    //user
    #define EN235P_H                (GpioDataRegs.GPBSET.bit.GPIO38 = 1)
    #define EN235P_L                (GpioDataRegs.GPBCLEAR.bit.GPIO38 = 1)
    #define EN235N_H                (GpioDataRegs.GPBSET.bit.GPIO37 = 1)
    #define EN235N_L                (GpioDataRegs.GPBCLEAR.bit.GPIO37 = 1)

    #define TO_FPGA_TRIGGER_OUT_H   (GpioDataRegs.GPCSET.bit.GPIO81 = 1)
    #define TO_FPGA_TRIGGER_OUT_L   (GpioDataRegs.GPCCLEAR.bit.GPIO81 = 1)
    //read
    //trigger in
    #define FROM_FPGA_TRIGGER_IN    (GpioDataRegs.GPCDAT.bit.GPIO76)
    #define FROM_FPGA_RESERVE6      (GpioDataRegs.GPCDAT.bit.GPIO77)
    #define FROM_FPGA_RESERVE7      (GpioDataRegs.GPCDAT.bit.GPIO78)
    #define FROM_FPGA_RESERVE8      (GpioDataRegs.GPCDAT.bit.GPIO79)





#define S_FSM_DELAY     999

typedef struct
{
    Uint16 firstflag;
    Uint32 now_time;
    float32 delay_ms;
    Uint16 next_state;
}ST_FSM_DELAY;


#define     FSM_DELAY_NEXT(v, delayms, nextstate)           \
do{                                                         \
    v.firstflag = 0;                                        \
    v.delay_ms = delayms;                                   \
    v.next_state = nextstate;                               \
    state = S_FSM_DELAY;                                    \
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


/*=============== Main Time Base variable ()============================*/

/***********************************************************************
* Function Name : Global Variable ()
***********************************************************************/





extern Uint32    task_run_cnt;
extern Uint32    task_run_time;
extern Uint32    max_task_run_time;
/*=====================Fault=======================================*/

extern Uint16 err_flag;
extern Uint32 err_code;
extern Uint16 fpga_state;






extern              void glb_VarInit(void);


/***********************************************************************
* typedef of union and structure
***********************************************************************/


#endif
