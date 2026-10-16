#include "drv_GlobalVar.h"
#include <string.h>
#include "task_DAC.h"
#include "test_hh.h"
#include "task_Range.h"
#include "drv_MgModbus.h"

ST_DACPARA st_VDACpara, st_IDACpara;

void Init_DACData()
{
    memset(&st_VDACpara, 0 ,sizeof(ST_DACPARA));
    memset(&st_IDACpara, 0 ,sizeof(ST_DACPARA));

}

void DACCalculate(ST_DACPARA* vol, ST_DACPARA* cur)
{
    if(mgmd_stSCIRx.adjust_dac_v == 1)
    {
        vol->gain = 1;
        vol->offset = 0;
    }

    if(mgmd_stSCIRx.adjust_dac_i == 1)
    {
        cur->gain = 1;
        cur->offset = 0;
    }

    vol->gain   = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32v_dac_p_k[vol->range_index] : st_eerom_para.f32v_dac_n_k[vol->range_index];
    vol->offset = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32v_dac_p_b[vol->range_index] : st_eerom_para.f32v_dac_n_b[vol->range_index];
    vol->AD = (vol->set * vol->gain + vol->offset) / rangeVol_buff[vol->range_index] * 52428;   // 52428 = 4 / 5 * 65535


    if(cur->range_index == RANGE_CUR_200uA_INDEX)
    {
        cur->gain   = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_dac_p_k[RANGE_CUR_100uA_INDEX] : st_eerom_para.f32i_dac_n_k[RANGE_CUR_100uA_INDEX];
        cur->offset = ((mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_dac_p_b[RANGE_CUR_100uA_INDEX] : st_eerom_para.f32i_dac_n_b[RANGE_CUR_100uA_INDEX]) * 2;
    }
    else if(cur->range_index == RANGE_CUR_10A_INDEX)
    {
        cur->gain   = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_dac_p_k[RANGE_CUR_2A_INDEX] : st_eerom_para.f32i_dac_n_k[RANGE_CUR_2A_INDEX];
        cur->offset = ((mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_dac_p_b[RANGE_CUR_2A_INDEX] : st_eerom_para.f32i_dac_n_b[RANGE_CUR_2A_INDEX]) * 5;
    }
    else
    {
        cur->gain   = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_dac_p_k[cur->range_index] : st_eerom_para.f32i_dac_n_k[cur->range_index];
        cur->offset = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_dac_p_b[cur->range_index] : st_eerom_para.f32i_dac_n_b[cur->range_index];
    }
    cur->AD = (cur->set * cur->gain + cur->offset) / rangeCur_buff[cur->range_index] * 52428;   // 52428 = 4 / 5 * 65535
}



