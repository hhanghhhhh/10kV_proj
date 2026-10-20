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
    st_protect.fire.rangesw = 0;
}

void DisableProt_Fire(void)
{
    st_protect.fire.enable = 0;
    st_protect.fire.rangesw = 0;
}

// 档位切换时，打火保护更换为检测小于 50V
void FireRangeSw(void)
{
    st_protect.fire.rangesw = 1;
}


void ProtectFunc(void)
{
    ST_PROTECT* pProt = &st_protect;
    ST_ADC*  pADC = &st_ADCdata;
    ST_SCI_RX_PAR* pPC = &mgmd_stSCIRx;
    float32 drop_ratio = 0;

    static Uint16 delay_cnt = 0;

    if(protectflag == 0 || pPC->cfg.disable_all_prot == 1)
    {
        //清零计数变量
        delay_cnt = 0;
        pProt->fire.vol_max = 0;
        pProt->scp.cnt = 0;
        return;
    }

    //保护晚两个周期再开
    if(delay_cnt < 2)
    {
        delay_cnt++;
        return;
    }

    //ovp限压保护，防止输出电压高于设定值太多
    if(pPC->plusewidth.f32 > 0)
    {
        pProt->ovp.th = pPC->cfgvalue[EEROM_CONFIG_ovp_th];
        if(ABS(pADC->vol) > ABS(pPC->v_set.f32) + pProt->ovp.th)
        {
            pPC->dataok = ERR_PROT_OVP;
            pPC->isamp_avg.f32 = st_ADCdata.cur;
            pPC->vsamp_avg.f32 = st_ADCdata.vol;
            pPC->power_ctrl = POWER_OFF;
            return;
        }
    }

    //打火保护，保护输出过程中电压突然跌落的情况,drop太小会频繁保护，太大就不会保护
    //电压大于100V后开始保护
    if(pPC->plusewidth.f32 > 0 && pProt->fire.enable == 1)
    {
        if(pProt->fire.vol_max < ABS(pADC->vol))
            pProt->fire.vol_max = ABS(pADC->vol);

        if(pProt->fire.vol_max > pPC->cfgvalue[EEROM_CONFIG_fire_th])
        {
            drop_ratio = pProt->fire.vol_max * 0.01 * pPC->cfgvalue[EEROM_CONFIG_fire_drop_ratio_100];
            pProt->fire.drop = (drop_ratio < pPC->cfgvalue[EEROM_CONFIG_fire_drop_value]) ? pPC->cfgvalue[EEROM_CONFIG_fire_drop_value] : drop_ratio;  //取两者中较大的
            if(pProt->fire.drop < 10)
                pProt->fire.drop = 10;

            if(st_protect.fire.rangesw == 1)
            	pProt->fire.prot_th = 50;	// 切档后按照 50V 的阈值
            else
            	pProt->fire.prot_th = pProt->fire.vol_max - pProt->fire.drop;

            if((pADC->vol) < pProt->fire.prot_th)
            {
                pPC->dataok = ERR_PROT_FIRE;
                pPC->isamp_avg.f32 = st_ADCdata.cur;
                pPC->vsamp_avg.f32 = st_ADCdata.vol;
                pPC->power_ctrl = POWER_OFF;
                return;
            }
        }
    }
    else
    {
        // 清零
        pProt->fire.vol_max = 0;
    }


    //短路保护  , th = 0时关闭该保护，仅在正常模式有短路保护
    if(pPC->test_mode == MODE_VBR)
    {
        pProt->scp.th = pPC->cfgvalue[EEROM_CONFIG_scp_th];
        pProt->scp.ms = pPC->cfgvalue[EEROM_CONFIG_vbr_shorttime_0p1ms] * 0.1;
    }
    else
    {
        pProt->scp.th = pPC->cfgvalue[EEROM_CONFIG_scp_th];
        pProt->scp.ms = pPC->cfgvalue[EEROM_CONFIG_scp_time_0p1ms] * 0.1;
    }

    if(pPC->plusewidth.f32 > 0 && pProt->scp.th < ABS(pPC->v_set.f32))
    {
        if((ABS(pADC->vol) < pProt->scp.th) && (pProt->scp.th > 0))
        {
            pProt->scp.cnt++;
            if(pProt->scp.cnt > Tms_CNT(pProt->scp.ms))
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













