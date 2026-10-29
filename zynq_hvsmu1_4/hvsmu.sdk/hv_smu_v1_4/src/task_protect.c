/*
 * task_protect.c
 *
 *  Created on: 2025年7月8日
 *      Author: m
 */

#include "COMMON.h"

Uint16 protectflag = 0;
ST_PROTECT st_protect;
void Init_Protect(void)
{
    memset(&st_protect, 0, sizeof(st_protect));
}

void EnableProt_All(void)
{
    protectflag = 1;
    EnableProt_Fire();
}

void DisableProt_All(void)
{
    protectflag = 0;
    DisableProt_Fire();
}

void EnableProt_Fire(void)
{
    st_protect.fire.enable = 1;
    st_protect.fire.prev_valid = 0;
}

void DisableProt_Fire(void)
{
    st_protect.fire.enable = 0;
    st_protect.fire.prev_valid = 0;
}

void ProtectFunc(void)
{
    ST_PROTECT *pProt = &st_protect;
    ST_ADC *pADC = &st_ADCdata;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
    float32 voltage_drop;

    static Uint16 delay_cnt = 0;

    if (protectflag == 0 || pPC->cfg.disable_all_prot == 1)
    {
        // 清零计数变量
        delay_cnt = 0;
        pProt->fire.prev_valid = 0;
        pProt->scp.cnt = 0;
        return;
    }

    // 保护晚两个周期再开
    if (delay_cnt < 2)
    {
        delay_cnt++;
        pProt->fire.prev_vol = pADC->vol;
        pProt->fire.prev_valid = pProt->fire.enable;
        return;
    }

    // ovp限压保护，防止输出电压高于设定值太多
    pProt->ovp.th = pPC->cfgvalue[EEROM_CONFIG_ovp_th];
    if (ABS(pADC->vol) > ABS(pPC->v_set.f32) + pProt->ovp.th)
    {
        pPC->dataok = ERR_PROT_OVP;
        pPC->isamp_avg.f32 = st_ADCdata.cur;
        pPC->vsamp_avg.f32 = st_ADCdata.vol;
        pPC->power_ctrl = POWER_OFF;
        return;
    }

    /* 比较相邻两个 50us 采样点的电压跌落，脉冲和持续输出均适用。
     * 不再使用历史峰值、跌落比例、启动电压门槛或切档专用阈值。
     */
    if (pProt->fire.enable == 1)
    {
        if (pProt->fire.prev_valid == 1)
        {
            voltage_drop = pProt->fire.prev_vol - pADC->vol;
            if (voltage_drop > (float32)pPC->fire_drop_50us)
            {
                pPC->dataok = ERR_PROT_FIRE;
                pPC->isamp_avg.f32 = pADC->cur;
                pPC->vsamp_avg.f32 = pADC->vol;
                pPC->power_ctrl = POWER_OFF;
                return;
            }
        }
        pProt->fire.prev_vol = pADC->vol;
        pProt->fire.prev_valid = 1;
    }
    else
    {
        pProt->fire.prev_valid = 0;
    }

    // 短路保护  , th = 0时关闭该保护，仅在正常模式有短路保护
    if (pPC->test_mode == MODE_VBR)
    {
        pProt->scp.th = pPC->cfgvalue[EEROM_CONFIG_scp_th];
        pProt->scp.ms = pPC->cfgvalue[EEROM_CONFIG_vbr_shorttime_0p1ms] * 0.1;
    }
    else
    {
        pProt->scp.th = pPC->cfgvalue[EEROM_CONFIG_scp_th];
        pProt->scp.ms = pPC->cfgvalue[EEROM_CONFIG_scp_time_0p1ms] * 0.1;
    }

    if (pProt->scp.th < ABS(pPC->v_set.f32))
    {
        if ((ABS(pADC->vol) < pProt->scp.th) && (pProt->scp.th > 0))
        {
            pProt->scp.cnt++;
            if (pProt->scp.cnt > Tms_CNT(pProt->scp.ms))
            {
                pPC->dataok = ERR_PROT_SCP;
                pPC->isamp_avg.f32 = st_ADCdata.cur;
                pPC->vsamp_avg.f32 = st_ADCdata.vol;
                pPC->power_ctrl = POWER_OFF;
                return;
            }
        }
        else
        {
            pProt->scp.cnt = 0;
        }
    }
}
