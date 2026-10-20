/*
 * task_range.c
 *
 *  Created on: 2025年7月2日
 *      Author: m
 */
#include "COMMON.h"

UN_VOLRANGESET un_volrangeset;
UN_CURRANGESET un_currangeset;
ST_RANGE_PARA st_range_vol_use, st_range_cur_use;

Uint16 rangeVolword[RANGE_VOL_NUM] = {VOLRANGE_400V, VOLRANGE_1000V, VOLRANGE_2000V, VOLRANGE_4000V, VOLRANGE_7000V, VOLRANGE_10000V};
Uint16 rangeCurword[RANGE_CUR_NUM] = {CURRANGE_1nA, CURRANGE_10nA, CURRANGE_100nA, CURRANGE_1uA, CURRANGE_10uA,
                                      CURRANGE_100uA, CURRANGE_1mA, CURRANGE_5mA, CURRANGE_50mA};

float32 rangeVol_buff[RANGE_VOL_NUM] = {400.0f, 1000.0f, 2000.0f, 4000.0f, 7000.0f, 10000.0f};
float32 rangeCur_buff[RANGE_CUR_NUM] = {1e-9f, 1e-8f, 1e-7f, 1e-6f, 1e-5f, 1e-4f,
                                        1e-3f, 5e-3f, 50e-3f};
float32 vol_adc_full[RANGE_VOL_NUM] = {4.0f, 4.0f, 4.0f, 4.0f, 3.92f, 4.0f};
float32 vol_dac_full[RANGE_VOL_NUM] = {3.75f, 3.75f, 3.9f, 3.75f, 3.85875f, 3.75f};
float32 cur_adc_full[RANGE_CUR_NUM] = {1.0, 3.333, 3.333, 3.92, 3.92, 3.66, 3.66, 3.125, 3.125};
float32 cur_dac_full[RANGE_CUR_NUM] = {1.0, 3.0, 3.3, 3.582, 3.92, 3.294, 3.66, 2.8125, 3.125};

void InitTaskRange(void)
{
    memset(&st_range_vol_use, 0, sizeof(ST_RANGE_PARA));
    memset(&st_range_cur_use, 0, sizeof(ST_RANGE_PARA));
    memset(&un_volrangeset, 0, sizeof(UN_VOLRANGESET));
    memset(&un_currangeset, 0, sizeof(UN_CURRANGESET));
}

void RangeLimit(void)
{
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
    ST_RANGE_PARA *pRange_vol = &st_range_vol_use;
    ST_RANGE_PARA *pRange_cur = &st_range_cur_use;

    float32 requested_vol = (pPC->autorangev == 0) ? pPC->v_range.f32 : pPC->v_set.f32;
    Uint16 vol_index;

    /* Select the smallest covering range; retain the existing 0.1 V tolerance. */
    for (vol_index = 0; vol_index < RANGE_VOL_NUM - 1; vol_index++)
    {
        if (requested_vol < rangeVol_buff[vol_index] + 0.1f)
            break;
    }
    pRange_vol->range_index_PC = vol_index;

    if (pPC->autorangei == 0)
    {
        //    	if(pPC->i_range.f32 < 1.01e-9)           pRange_cur->range_index_PC = RANGE_CUR_1nA_INDEX;
        //        else if(pPC->i_range.f32 < 10.01e-9)     pRange_cur->range_index_PC = RANGE_CUR_10nA_INDEX;
        if (pPC->i_range.f32 < 100.01e-9 && pPC->dis100nA == 0)
            pRange_cur->range_index_PC = RANGE_CUR_100nA_INDEX;
        else if (pPC->i_range.f32 < 1.01e-6)
            pRange_cur->range_index_PC = RANGE_CUR_1uA_INDEX;
        else if (pPC->i_range.f32 < 10.01e-6)
            pRange_cur->range_index_PC = RANGE_CUR_10uA_INDEX;
        else if (pPC->i_range.f32 < 100.01e-6)
            pRange_cur->range_index_PC = RANGE_CUR_100uA_INDEX;
        else if (pPC->i_range.f32 < 1.01e-3)
            pRange_cur->range_index_PC = RANGE_CUR_1mA_INDEX;
        else if (pPC->i_range.f32 < 5.01e-3)
            pRange_cur->range_index_PC = RANGE_CUR_5mA_INDEX;
        else
            pRange_cur->range_index_PC = RANGE_CUR_50mA_INDEX;
    }
    else
    {
        // 根据电流值选择档位
        //    	if(pPC->i_set.f32 < 1.1e-9)           pRange_cur->range_index_PC = RANGE_CUR_1nA_INDEX;
        //    	else if(pPC->i_set.f32 < 10.1e-9)     pRange_cur->range_index_PC = RANGE_CUR_10nA_INDEX;
        if (pPC->i_set.f32 < 100.1e-9 && pPC->dis100nA == 0)
            pRange_cur->range_index_PC = RANGE_CUR_100nA_INDEX;
        else if (pPC->i_set.f32 < 1.1e-6)
            pRange_cur->range_index_PC = RANGE_CUR_1uA_INDEX;
        else if (pPC->i_set.f32 < 10.1e-6)
            pRange_cur->range_index_PC = RANGE_CUR_10uA_INDEX;
        else if (pPC->i_set.f32 < 100.1e-6)
            pRange_cur->range_index_PC = RANGE_CUR_100uA_INDEX;
        else if (pPC->i_set.f32 < 1.1e-3)
            pRange_cur->range_index_PC = RANGE_CUR_1mA_INDEX;
        else if (pPC->i_set.f32 < 5.1e-3)
            pRange_cur->range_index_PC = RANGE_CUR_5mA_INDEX;
        else
            pRange_cur->range_index_PC = RANGE_CUR_50mA_INDEX;
    }

    CmdImplement();
}

void RangeVolSel(Uint16 range_index)
{
    UN_VOLRANGESET *p = &un_volrangeset;
    ST_RANGE_PARA *pRange = &st_range_vol_use;
    if (range_index >= RANGE_VOL_NUM)
        return;

    pRange->range_index = range_index;

    p->word = rangeVolword[range_index];
    pRange->full_range = rangeVol_buff[range_index];
    pRange->dac_full = vol_dac_full[range_index];
    pRange->adc_full = vol_adc_full[range_index];

    st_topl.cmd2.bit.vma2 = p->bit.vma2;
    st_topl.cmd2.bit.va2 = p->bit.va2;

    st_topl.cmd.bit.vma_en = p->bit.vmaen;
    st_topl.cmd.bit.vma1 = p->bit.vma1;
    st_topl.cmd.bit.vma0 = p->bit.vma0;
    st_topl.cmd.bit.va_en = p->bit.vaen;
    st_topl.cmd.bit.va1 = p->bit.va1;
    st_topl.cmd.bit.va0 = p->bit.va0;
}

void RangeCurSel(Uint16 range_index)
{
    UN_CURRANGESET *p = &un_currangeset;
    ST_RANGE_PARA *pRange = &st_range_cur_use;
    pRange->range_index = range_index;

    p->word = rangeCurword[range_index];
    pRange->full_range = rangeCur_buff[range_index];
    pRange->dac_full = cur_dac_full[range_index];
    pRange->adc_full = cur_adc_full[range_index];

    st_topl.cmd.bit.imeas_gain1 = p->bit.imeas_gain1;
    st_topl.cmd.bit.imeas_gain10 = p->bit.imeas_gain10;
    st_topl.cmd.bit.ifb_gain1 = p->bit.ifb_gain1;
    st_topl.cmd.bit.ifb_gain3 = p->bit.ifb_gain3;
    st_topl.cmd.bit.idac_amps = p->bit.idac_div3;
    st_topl.cmd.bit.range_10nA_100nA = p->bit.range_10n_100n;
    st_topl.cmd.bit.range_1uA_10uA = p->bit.range_1u_10u;
    st_topl.cmd.bit.range_100uA_1mA = p->bit.range_100u_1m;
    st_topl.cmd.bit.range_5mA_50mA = p->bit.range_5m_50m;
}

void CmdImplement(void)
{
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    if (pPC->highCmode == 1)
    {
        st_topl.cmd.bit.cmult23_n = 0;
    }
    else
    {
        st_topl.cmd.bit.cmult23_n = 1;
    }

    if (pPC->test_mode == MODE_VBR || pPC->test_mode == MODE_CURTEST)
    {
        //    	st_topl.cmd.bit.idac_slowen = 1;
        //    	st_topl.cmd.bit.idac_slow_a0 = 0;
        //    	st_topl.cmd.bit.idac_slow_a1 = 1;
        //    	st_topl.cmd.bit.vdac_slowen = 1;
        //    	st_topl.cmd.bit.vdac_slow_a0 = 0;
        //    	st_topl.cmd.bit.vdac_slow_a1 = 1;
    }
    else
    {
        //    	st_topl.cmd.bit.idac_slowen = 0;
        //    	st_topl.cmd.bit.idac_slow_a0 = 0;
        //    	st_topl.cmd.bit.idac_slow_a1 = 0;
        //    	st_topl.cmd.bit.vdac_slowen = 0;
        //    	st_topl.cmd.bit.vdac_slow_a0 = 0;
        //    	st_topl.cmd.bit.vdac_slow_a1 = 0;
    }

    st_topl.cmd.bit.idac_slowen = pPC->test_idac_en;
    st_topl.cmd.bit.idac_slow_a0 = pPC->test_idac_a0;
    st_topl.cmd.bit.idac_slow_a1 = pPC->test_idac_a1;
    st_topl.cmd.bit.vdac_slowen = pPC->test_vdac_en;
    st_topl.cmd.bit.vdac_slow_a0 = pPC->test_vdac_a0;
    st_topl.cmd.bit.vdac_slow_a1 = pPC->test_vdac_a1;
}
