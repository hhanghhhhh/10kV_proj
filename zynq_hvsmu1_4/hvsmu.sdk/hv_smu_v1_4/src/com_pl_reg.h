/*
 * read_pl_reg.h
 *
 *  Created on: 2025Äê6ÔÂ29ÈÕ
 *      Author: m
 */

#ifndef SRC_COM_PL_REG_H_
#define SRC_COM_PL_REG_H_

#include "TYPEDEF.h"

#define PLREGADDR 0x43C00000
#define ReadPlReg(addr) Xil_In32(PLREGADDR + (addr << 2))
#define WritePlReg(addr, data)                    \
    do                                            \
    {                                             \
        Xil_Out32(PLREGADDR + (addr << 2), data); \
    } while (0)

typedef struct
{
    union
    {
        Uint32 u32;
        struct
        {
            Uint32 samp_start : 1;
            Uint32 vdac_slowen : 1;
            Uint32 vdac_slow_a0 : 1;
            Uint32 vdac_slow_a1 : 1;
            Uint32 idac_amps : 1;
            Uint32 idac_slowen : 1;
            Uint32 idac_slow_a0 : 1;
            Uint32 idac_slow_a1 : 1;
            Uint32 vma0 : 1;
            Uint32 vma1 : 1;
            Uint32 vma_en : 1;
            Uint32 va0 : 1;
            Uint32 va1 : 1;
            Uint32 va_en : 1;
            Uint32 imeas_gain10 : 1;
            Uint32 imeas_gain1 : 1;
            Uint32 ifb_gain3 : 1;
            Uint32 ifb_gain1 : 1;
            Uint32 cmode_n : 1;
            Uint32 cmode_bw1_n : 1;
            Uint32 cmode_bw2_n : 1;
            Uint32 cmult23_n : 1;
            Uint32 range_10nA_100nA : 1;
            Uint32 range_1uA_10uA : 1;
            Uint32 range_100uA_1mA : 1;
            Uint32 range_5mA_50mA : 1;
            Uint32 lo_protect : 1;
            Uint32 outoff : 1;
            Uint32 pramp_p : 1;
            Uint32 relay : 1;
            Uint32 wave_filter_en : 1;
            Uint32 reserve : 1;
        } bit;
    } cmd;
    union
    {
        Uint32 u32;
        struct
        {
            Uint16 vol;
            Uint16 cur;
        } bit;
    } dac;
    union
    {
        Uint32 u32;
        struct
        {
            Uint16 num;
            Uint16 interval;
        } bit;
    } samp;
    union
    {
        Uint32 u32;
        struct
        {

            Uint32 vma2 : 1; /* Slow-link word 4, bit 0 */
            Uint32 va2 : 1;  /* Slow-link word 4, bit 1 */
            Uint32 reserve : 30;
        } bit;
    } cmd2;
    union
    {
        Uint32 u32;
        struct
        {
            Uint32 reserve : 32;
        } bit;
    } para;
    union
    {
        Uint32 u32;
        struct
        {
            Uint32 reserve : 32;
        } bit;
    } para2;
    union
    {
        Uint32 u32;
        struct
        {
            Uint32 reserve : 32;
        } bit;
    } pl_cmd;

    union
    {
        Uint32 u32;
        struct
        {
            Uint32 reserve : 32;
        } bit;
    } pl_cmd2;

    union
    {
        Uint32 u32;
        struct
        {
            Uint32 update : 1;
            Uint32 data : 16;
            Uint32 en : 1;
            Uint32 range_sw_cur : 2;
            Uint32 range_sw_vol : 2;
            Uint32 reserve : 10;
        } bit;
    } test_ddr3;

    union
    {
        Uint32 u32;
        struct
        {
            Uint32 num : 17;
            Uint32 div : 8;
            Uint32 reserve : 7;
        } bit;
    } wave_para;

} ST_TOPL;

extern ST_TOPL st_topl;

typedef struct
{
    int32 vol_avg_ad;
    int32 cur_avg_ad;
    int32 vol_ad;
    int32 cur_ad;
    union
    {
        Uint32 u32;
        struct
        {
            Uint32 dataok : 1;
            Uint32 anlu_version : 8;
            Uint32 anlu_run10 : 4;
            Uint32 over_temp : 1;
            Uint32 resv : 19;
        } bit;
    } state;

    Uint32 slowrx_err_cnt;

    union
    {
        Uint32 u32;
        struct
        {
            Uint32 reserve : 32;
        } bit;
    } pl_data;
    union
    {
        Uint32 u32;
        struct
        {
            Uint32 wave_ok : 1;
            Uint32 reserve : 31;
        } bit;
    } pl_data2;
    union
    {
        Uint32 u32;
        struct
        {
            Uint32 wr_cnt : 16;
            Uint32 rd_cnt : 16;
        } bit;
    } test_cnt;

    Uint32 test_awaddr;

} ST_FROMPL;

extern ST_FROMPL st_frompl;

void InitTaskCom(void);
void TaskComToPl(void);
void TaskComFromPl(void);

#endif /* SRC_COM_PL_REG_H_ */
