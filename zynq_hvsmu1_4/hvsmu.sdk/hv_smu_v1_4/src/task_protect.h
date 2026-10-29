/*
 * task_protect.h
 *
 *  Created on: 2025年7月8日
 *      Author: m
 */

#ifndef SRC_TASK_PROTECT_H_
#define SRC_TASK_PROTECT_H_

#include "TYPEDEF.h"



typedef struct
{
    //过压保护
    struct
    {
        float32 th;
    }ovp;

    //短路保护
    struct
    {
    	Uint16 ms;
        Uint16 th;
        Uint32 cnt;
    }scp;

    //打火保护
    struct
    {
        Uint16  enable;
        Uint16  prev_valid;
        float32 prev_vol;
    }fire;
}ST_PROTECT;

extern ST_PROTECT st_protect;

void ProtectFunc(void);
void EnableProt_Fire(void);
void DisableProt_Fire(void);
void EnableProt_All(void);
void DisableProt_All(void);

#endif /* SRC_TASK_PROTECT_H_ */
