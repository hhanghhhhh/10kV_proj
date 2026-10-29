/*
 * task_adc.h
 *
 *  Created on: 2025年7月2日
 *      Author: m
 */

#ifndef SRC_TASK_ADC_H_
#define SRC_TASK_ADC_H_

#include "TYPEDEF.h"

#define  SAMP_US   		(2)
// 1.0 / 8388608.0 * 4.096

#define  CUR_IN_ADC_V	(4.8828125e-7)
#define  VOL_IN_ADC_V	(4.8828125e-7)

typedef struct
{
    Uint16 trigger;
    Uint16 dataOK;

    float32 cur_avg;
    float32 vol_avg;
    int32   curAD_avg;
    int32   volAD_avg;

    float32 cur;
    float32 vol;
    int32   cur_AD;
    int32   vol_AD;

    float32 cur_1ms;
    float32 vol_1ms;

    float32 cur_gain;
    float32 vol_gain;
    float32 cur_k;
    float32 cur_b;
    float32 vol_k;
    float32 vol_b;

    //500k采样率 2us一个点
    Uint16 sample_num;   //采样点数
    Uint16 interval_cnt;   //采样间隔，25M时钟  25为1us， 50个cnt为500k 1个点 2us，以此为倍数
}ST_ADC;

extern ST_ADC  st_ADCdata;

void InitTaskAdc(void);
void TaskAdcRealtime(void);
void TaskAdcSample(void);
void TriggerAdcSamp(void);
Uint16 AdcSampOk(void);

#endif /* SRC_TASK_ADC_H_ */
