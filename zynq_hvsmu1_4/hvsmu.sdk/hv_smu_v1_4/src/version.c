/*
 * version.c
 *
 *  Created on: 2025年7月8日
 *      Author: m
 */

#include "COMMON.h"
#include "build_info.h"


ST_VERSION st_version;


void InitVersion(void)
{
    ST_VERSION *p = &st_version;
    memset(&st_version, 0, sizeof(ST_VERSION));

    p->zynq_version =      2;   //
    p->anlu_version_std =  1;   //     //相匹配的FPGA版本
    p->device_num = SELF_DEVICE;
    p->utc_time.u32 = build_unix_time;
}


void RunVersion(void)
{
    ST_VERSION *p = &st_version;

    p->anlu_runflag = (st_frompl.state.bit.anlu_run10 == 10) ? 1 : 0;
    p->anlu_version = st_frompl.state.bit.anlu_version;
}

