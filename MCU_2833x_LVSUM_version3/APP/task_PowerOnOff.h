#ifndef _POWERONOFF_H_
#define _POWERONOFF_H_

/* POWER_CTRL: 0 idle, 1 local ON, 2 OFF, 3 local range-switch ON. */
#define POWER_CTRL_ON_CAN (4U)

// Vth 双源法测试
#define VTH_IDEL 0
#define VTH_PRE_PARA 1
#define VTH_PRE_OK 2
#define VTH_WAIT_STEP 3
#define VTH_STEP_DELAY 4
#define VTH_STEP_CHECK 5
#define VTH_STEP_OK 6

#define VTH_RST_OK 1
#define VTH_RST_OVER_MIN 2
#define VTH_RST_OVER_MAX 3
#define VTH_RST_OVER_TIME 4

// 上电状态机状态
#define PWON_IDEL 0
#define PWON_DELAY 1
#define PWON_SAMPLE 2
#define PWON_WAITOK 3
#define PWON_OFF 4
#define PWON_ONOK 5
#define PWON_200DELAY 6
#define PWON_START 7
#define PWON_SETDAC 8
#define PWON_DELAY_CHARGE 9
#define PWON_SW_RANGE 10
#define PWON_RE_SETDAC 11
#define PWON_DELAY_SW 12
#define PWON_RE_SW_RANGE 13
#define PWON_RE_RE_SETDAC 14
#define PWON_DELAY_OFF 15
#define PWON_RANGESW 16
#define PWON_AFTER_SAMP_DLY 17

// 下电状态机状态
#define PWOFF_IDEL 0
#define PWOFF_DELAY 1
#define PWOFF_OK 2
#define PWOFF_RANGESW 3

extern Uint16 poweronflag;

extern void PowerON(void);
extern void PowerOff(void);
extern void InitFpga();
extern void PowerON_RangeSW(void);
extern void PowerON_Can(void);
extern void PowerOn_Vth_DoubleSmu(void);
void PowerOn_Vth_8pre(void);

#endif
