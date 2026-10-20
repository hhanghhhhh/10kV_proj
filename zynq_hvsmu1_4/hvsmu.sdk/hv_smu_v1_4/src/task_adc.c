/*
 * task_adc.c
 *
 *  Created on: 2025年7月2日
 *      Author: m
 */

#include "COMMON.h"


ST_ADC  st_ADCdata;

void InitTaskAdc(void)
{
    ST_ADC* p = &st_ADCdata;
    memset(&st_ADCdata, 0 ,sizeof(st_ADCdata));
    p->sample_num = 1000;
    p->interval_cnt = 500;   //20us
    p->cur_k = 1;
    p->vol_k = 1;
}

void TriggerAdcH(void)
{
	st_topl.cmd.bit.samp_start = 1;
}

void TriggerAdcL(void)
{
	st_topl.cmd.bit.samp_start = 0;
}

Uint16 ReadDataOk(void)
{
	return st_frompl.state.bit.dataok;
}

void SendSampPara(Uint16 num, Uint16 interval)
{
    st_topl.samp.bit.num = num;
    st_topl.samp.bit.interval = interval;
}

void TriggerAdcSamp(void)
{
	st_ADCdata.trigger = 1;
	st_ADCdata.dataOK = 0;
}

Uint16 AdcSampOk(void)
{
	return st_ADCdata.dataOK;
}


void VolMean1ms(void)
{
    static float32 total = 0;
    static Uint16 cnt_time = 0;

    if(cnt_time < Tms_CNT(1))
    {
        total += st_ADCdata.vol;
        cnt_time++;
    }
    else
    {
    	st_ADCdata.vol_1ms = total / cnt_time;
        total = 0;
        cnt_time = 0;
    }
}

void CurMean1ms(void)
{
    static float32 total = 0;
    static Uint16 cnt_time = 0;

    if(cnt_time < Tms_CNT(1))
    {
        total += st_ADCdata.cur;
        cnt_time++;
    }
    else
    {
    	st_ADCdata.cur_1ms = total / cnt_time;
        total = 0;
        cnt_time = 0;
    }
}



//如：2us一个点，采样率 1/2 = 0.5M, 25M时钟，一个点的cnt = 25*2 = 50
void CalSampandInterval(float32 nplc, float32 samp_us, Uint16* sampnum, Uint16* intercnt)
{
    Uint16 i = 1;
    Uint32 tmp_nplc_us = 0;
    Uint32 tmp_sample_num = 0;

    tmp_nplc_us = nplc * 20000;
    for(i = 1; i < 100; i++)
    {
        tmp_sample_num = tmp_nplc_us / (i * samp_us);
        if(tmp_sample_num <= 60000)
        {
            *intercnt = 25 * samp_us * i;
            *sampnum = tmp_sample_num;
            break;
        }
    }
}

void ADCCoefChoose(void)
{
    ST_ADC* p = &st_ADCdata;

    //选择系数 //先乘个理论系数，再乘上边的额外补偿系数
    p->cur_gain = CUR_IN_ADC_V * st_range_cur_use.full_range / st_range_cur_use.adc_full;
    p->vol_gain = VOL_IN_ADC_V * st_range_vol_use.full_range / st_range_vol_use.adc_full;
    if(mgmd_stSCIRx.coef.enable.bit.iadc == 1)
    {
        p->cur_k = st_eerom_para_COEF.f32i_adc_p_k[st_range_cur_use.range_index];
        p->cur_b = st_eerom_para_COEF.f32i_adc_p_b[st_range_cur_use.range_index];
    }
    else
    {
        p->cur_k = 1.0;
        p->cur_b = 0;
    }

    if(mgmd_stSCIRx.coef.enable.bit.vadc == 1)
    {
        p->vol_k = st_eerom_para_COEF.f32v_adc_p_k[st_range_vol_use.range_index];
        p->vol_b = st_eerom_para_COEF.f32v_adc_p_b[st_range_vol_use.range_index];
    }
    else
    {
        p->vol_k = 1.0;
        p->vol_b = 0;
    }
}



//p->trigger 置1一次，采样一次
void TaskAdcSample(void)
{
    ST_ADC* p = &st_ADCdata;
    static Uint16 state = 0;
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};

    //选择系数
    ADCCoefChoose();

    //采样点计算 根据nPLC来计算，最大6W
    CalSampandInterval(mgmd_stSCIRx.nplc.f32, SAMP_US, &p->sample_num, &p->interval_cnt);
    SendSampPara(p->sample_num, p->interval_cnt);

    //实时采样值
    p->cur_AD = st_frompl.cur_ad;
    p->vol_AD = st_frompl.vol_ad;
    p->cur = (float32)p->cur_AD * p->cur_gain * p->cur_k + p->cur_b;
    p->vol = (float32)p->vol_AD * p->vol_gain * p->vol_k + p->vol_b;

    mgmd_stSCIRx.vsamp.f32 = p->vol;
    mgmd_stSCIRx.isamp.f32 = p->cur;

    //计算电压1ms平均值
    VolMean1ms();
    CurMean1ms();

    switch(state)
    {
	case S_FSM_DELAY:
		FSM_DELAY_FUNC(st_delay);
		break;
	case 0:
	    if(p->trigger == 1)
	    {
	        // 触发采样
	    	TriggerAdcH();
	    	// 清除变量
	        p->trigger = 0;
	        p->dataOK = 0;
	        // 状态跳转
	        FSM_DELAY_NEXT(st_delay, 1, 1);
	    }
		break;
	case 1:
        if(ReadDataOk() == 1)
        {
            //读取AD平均值
            p->curAD_avg = st_frompl.cur_avg_ad;
            p->volAD_avg = st_frompl.vol_avg_ad;

            //计算真实值
            p->cur_avg = (float32)p->curAD_avg * p->cur_gain * p->cur_k + p->cur_b;
            p->vol_avg = (float32)p->volAD_avg * p->vol_gain * p->vol_k + p->vol_b;
            p->dataOK = 1;

            //记录采样结果
            mgmd_stSCIRx.adc_vol_avg.f32 = p->vol_avg;
            mgmd_stSCIRx.adc_cur_avg.f32 = p->cur_avg;
            mgmd_stSCIRx.adc_vol_full.f32 = st_range_vol_use.full_range;
            mgmd_stSCIRx.adc_cur_full.f32 = st_range_cur_use.full_range;
            mgmd_stSCIRx.dac_vol_ad = st_VDACpara.AD;
            mgmd_stSCIRx.dac_cur_ad = st_IDACpara.AD;
            TriggerAdcL();
            state = 0;
        }
		break;
	default: state = 0;
    }

//
//    //等待dataok
//    if(wait_dataok == 1)
//    {
//        if(ReadDataOk() == 1)
//        {
//            //读取AD平均值
//            p->curAD_avg = st_frompl.cur_avg_ad;
//            p->volAD_avg = st_frompl.vol_avg_ad;
//
//            //计算真实值
//            p->cur_avg = (float32)p->curAD_avg * p->cur_gain * p->cur_k + p->cur_b;
//            p->vol_avg = (float32)p->volAD_avg * p->vol_gain * p->vol_k + p->vol_b;
//
//            p->dataOK = 1;
//
//            mgmd_stSCIRx.adc_vol_avg.f32 = p->vol_avg;
//            mgmd_stSCIRx.adc_cur_avg.f32 = p->cur_avg;
//
//            //将adcTrigger拉低
//            wait_dataok = 0;
//            TriggerAdcL();
//        }
//    }
//
//    if(p->trigger == 1)
//    {
//        //触发采样
//    	TriggerAdcH();
//        wait_dataok = 1;
//        wait_dataok_cnt = 0;
//        p->trigger = 0;
//        p->dataOK = 0;
//    }
}






