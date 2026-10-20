/*
 * task_dac.h
 *
 *  Created on: 2025Äê7ÔÂ2ÈÕ
 *      Author: m
 */

#ifndef SRC_TASK_DAC_H_
#define SRC_TASK_DAC_H_

#include "TYPEDEF.h"

typedef struct
{
	float32 AD;
    float32 set;
    float32 k;
    float32 b;
}ST_DACPARA;

extern ST_DACPARA st_VDACpara, st_IDACpara;



void InitTaskDac(void);
void SetVolRef(float32 vol_set);
void SetCurRef(float32 cur_set);




#endif /* SRC_TASK_DAC_H_ */
