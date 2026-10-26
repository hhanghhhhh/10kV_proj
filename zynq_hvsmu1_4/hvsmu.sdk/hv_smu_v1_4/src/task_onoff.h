/*
 * task_onoff.h
 *
 *  Created on: 2025年7月2日
 *      Author: m
 */

#ifndef SRC_TASK_ONOFF_H_
#define SRC_TASK_ONOFF_H_

#include "TYPEDEF.h"

#define MODE_NORMAL 0
#define MODE_VBR 1
#define MODE_CGRG 2
#define MODE_CURTEST 3
#define MODE_UPH 4
#define MODE_CAN 5

#define POWER_IDEL 0
#define POWER_ON 1
#define POWER_OFF 2

#define SELFT_IDEL 0
#define SELFT_OUT 1
#define SELFT_CHECK 2
#define SELFT_OFF 3

#define MP_IDEL 0
#define MP_DISLLC 1
#define MP_DISPFC 2
#define MP_LLCPULSE1 3
#define MP_LLCPULSE2 4
#define MP_PFCEN 5
#define MP_LLCEN 6
#define MP_OK 7
#define MP_CHECK 8

#define PWON_IDEL 0
#define PWON_DELAY_RELAY 1
#define PWON_VOL_RANGESW 2
#define PWON_SET_REF 3
#define PWON_DELAY_SETREF 4
#define PWON_CUR_RANGESW 5
#define PWON_DELAY_PLUSEWIDTH 6
#define PWON_SAMP_TRIGGER 7
#define PWON_WAIT_DATA 8
#define PWON_PWOFF 9
#define PWON_UPDATE_TRIGGER 10
#define PWON_UPDATE_WAIT 11

#define PWOFF_IDEL 0
#define PWOFF_SET_REF 1
#define PWOFF_DELAY_DISCHARGE 2
#define PWOFF_RELAY_OFF 3
#define PWOFF_RANGESW 4
#define PWOFF_DISCHARGE 5

#define PWONVBR_IDEL 0
#define PWONVBR_DELAY_RELAY 1
#define PWONVBR_RANGESW 2
#define PWONVBR_SETCUR 3
#define PWONVBR_DELAY_SETCUR 4
#define PWONVBR_SETVOL 5
#define PWONVBR_DETECT_VBR 6
#define PWONVBR_DELAY_PLUSEWIDTH 7
#define PWONVBR_PWOFF 8

#define ERR_OK 1            // ok
#define ERR_IDSS_OVERTIME 2 // idss采样超时
#define ERR_VBR_OVERTIME 3  // Vbr判定超时，在规定时间内电压未达到平稳
#define ERR_VBR_SHORT 4     // Vbr时输出短路
#define ERR_PROT_OVP 5      // 过压保护，输出有过冲
#define ERR_PROT_FIRE 6     // 电压跌落，电流档位设置太小或者打火
#define ERR_PROT_SCP 7      // 短路保护，输出短路

void InitDeviceState(void);
void PowerOnNormal(void);
void PowerOnVbr(void);
void PowerOnCurTest(void);
void PowerOff(void);
void MainPowerCtrl(void);
void OverTemp(void);
void AutoZeroFunc(void);

#endif /* SRC_TASK_ONOFF_H_ */
