/*
 * task_dac.c
 *
 *  Created on: 2025年7月2日
 *      Author: m
 */
#include "COMMON.h"

ST_DACPARA st_VDACpara, st_IDACpara;


void InitTaskDac(void)
{
    memset(&st_VDACpara, 0 ,sizeof(ST_DACPARA));
    memset(&st_IDACpara, 0 ,sizeof(ST_DACPARA));
}


static void GetVolCoef(float32* k, float32* b, Uint16 range_index)
{
    if(mgmd_stSCIRx.coef.enable.bit.vdac == 1)
    {
        *k = st_eerom_para_COEF.f32v_dac_p_k[range_index];
        *b = st_eerom_para_COEF.f32v_dac_p_b[range_index];
    }
    else
    {
        *k = 1.0;
        *b = 0;
    }
}


static void GetCurCoef(float32* k, float32* b, Uint16 range_index)
{
    if(mgmd_stSCIRx.coef.enable.bit.idac == 1)
    {
        *k = st_eerom_para_COEF.f32i_dac_p_k[range_index];
        *b = st_eerom_para_COEF.f32i_dac_p_b[range_index];
    }
    else
    {
        *k = 1;
        *b = 0;
    }
}


//设置对应的dac值, vol_set为正值
void SetVolRef(float32 vol_set)
{
    ST_DACPARA* vol = &st_VDACpara;
    ST_RANGE_PARA* pRange = &st_range_vol_use;

    if(vol_set > pRange->full_range)    vol->set = pRange->full_range;
    else if(vol_set < 0)                vol->set = 0;
    else								vol->set = vol_set;

    GetVolCoef(&vol->k, &vol->b, pRange->range_index);
    vol->AD = (vol->set * vol->k + vol->b) / pRange->full_range * pRange->dac_full / 4.096 * 65535;  // dac 5V <-> 65535

    // 四舍五入，且限幅在最大 65535
    vol->AD += 0.5;
    if(vol->AD > 65535)
    	vol->AD = 65535;
    st_topl.dac.bit.vol = (Uint16)vol->AD;
}

void SetCurRef(float32 cur_set)
{
    ST_DACPARA* cur = &st_IDACpara;
    ST_RANGE_PARA* pRange = &st_range_cur_use;

    if(cur_set > pRange->full_range)    cur->set = pRange->full_range;
    else if(cur_set < 0)                cur->set = 0;
    else								cur->set = cur_set;
//    cur->set = cur_set;

    GetCurCoef(&cur->k, &cur->b, pRange->range_index);
    cur->AD = (cur->set * cur->k + cur->b) / pRange->full_range * pRange->dac_full / 4.096 * 65535; //dac 5V <-> 65535

    // 四舍五入，且限幅在最大 65535
    cur->AD += 0.5;
    if(cur->AD > 65535)
    	cur->AD = 65535;
    st_topl.dac.bit.cur = (Uint16)cur->AD;
}






