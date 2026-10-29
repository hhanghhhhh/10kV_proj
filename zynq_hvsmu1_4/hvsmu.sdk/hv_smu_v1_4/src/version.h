/*
 * version.h
 *
 *  Created on: 2025Äê7ÔÂ8ÈÕ
 *      Author: m
 */

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_



#include "TYPEDEF.h"

#define     DEVICE_HIGHCURRENT2_0    1
#define     DEVICE_HIGHVOL1_3        2
#define     DEVICE_LOWVOL            3
#define     DEVICE_HIGHVOL2_0        4
#define     DEVICE_HIGHVOL2_1        5
#define     DEVICE_HIGHVOL1_4        6


#define		SELF_DEVICE				DEVICE_HIGHVOL1_4



typedef struct
{
    Uint16 zynq_version;
    Uint16 anlu_version;
    Uint16 anlu_version_std;
    Uint16 anlu_runflag;
    Uint16 device_num;
    Uint16 initok;
    union
	{
    	Uint32 u32;
    	struct
		{
    		Uint16 l;
    		Uint16 h;
		}u16;
	}utc_time;
}ST_VERSION;

extern ST_VERSION st_version;






extern void InitVersion(void);
extern void RunVersion(void);





#endif /* SRC_VERSION_H_ */
