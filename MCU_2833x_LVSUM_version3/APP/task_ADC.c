#include "drv_GlobalVar.h"

#if DSP
/*========================= Head file in DSP============================*/
#include "DSP2833x_Device.h"   // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h" // DSP2833x Examples Include File
/*======================================================================*/
#endif
#include <math.h>
#include <string.h>
#include "drv_fpga.h"
#include "drv_MgModbus.h"
#include "drv_Eeprom.h"
#include "task_ADC.h"
#include "task_Transfer.h"
#include "test_hh.h"
#include "task_Range.h"
#include "task_ComFPGA.h"
#include "task_wave.h"

ST_ADC st_ADCdata;

// float32 ADC_Vol_P_ratio_buf[RANGE_VOL_NUM] = {ADC_VOL_P_RATIO0, ADC_VOL_P_RATIO1, ADC_VOL_P_RATIO2, ADC_VOL_P_RATIO3, ADC_VOL_P_RATIO4};
// float32 ADC_Vol_P_offset_buf[RANGE_VOL_NUM] = {ADC_VOL_P_OFFSET0, ADC_VOL_P_OFFSET1, ADC_VOL_P_OFFSET2, ADC_VOL_P_OFFSET3, ADC_VOL_P_OFFSET4};
// float32 ADC_Vol_N_ratio_buf[RANGE_VOL_NUM] = {ADC_VOL_N_RATIO0, ADC_VOL_N_RATIO1, ADC_VOL_N_RATIO2, ADC_VOL_N_RATIO3, ADC_VOL_N_RATIO4};
// float32 ADC_Vol_N_offset_buf[RANGE_VOL_NUM] = {ADC_VOL_N_OFFSET0, ADC_VOL_N_OFFSET1, ADC_VOL_N_OFFSET2, ADC_VOL_N_OFFSET3, ADC_VOL_N_OFFSET4};
//
// float32 ADC_Cur_P_ratio_buf[RANGE_CUR_NUM] = {ADC_CUR_P_RATIO0, ADC_CUR_P_RATIO1, ADC_CUR_P_RATIO2, ADC_CUR_P_RATIO3, ADC_CUR_P_RATIO4,
//                                        ADC_CUR_P_RATIO5, ADC_CUR_P_RATIO6, ADC_CUR_P_RATIO7, ADC_CUR_P_RATIO8, ADC_CUR_P_RATIO9};
// float32 ADC_Cur_P_offset_buf[RANGE_CUR_NUM] = {ADC_CUR_P_OFFSET0, ADC_CUR_P_OFFSET1, ADC_CUR_P_OFFSET2, ADC_CUR_P_OFFSET3, ADC_CUR_P_OFFSET4,
//                                         ADC_CUR_P_OFFSET5, ADC_CUR_P_OFFSET6, ADC_CUR_P_OFFSET7, ADC_CUR_P_OFFSET8, ADC_CUR_P_OFFSET9};
// float32 ADC_Cur_N_ratio_buf[RANGE_CUR_NUM] = {ADC_CUR_N_RATIO0, ADC_CUR_N_RATIO1, ADC_CUR_N_RATIO2, ADC_CUR_N_RATIO3, ADC_CUR_N_RATIO4,
//                                        ADC_CUR_N_RATIO5, ADC_CUR_N_RATIO6, ADC_CUR_N_RATIO7, ADC_CUR_N_RATIO8, ADC_CUR_N_RATIO9};
// float32 ADC_Cur_N_offset_buf[RANGE_CUR_NUM] = {ADC_CUR_N_OFFSET0, ADC_CUR_N_OFFSET1, ADC_CUR_N_OFFSET2, ADC_CUR_N_OFFSET3, ADC_CUR_N_OFFSET4,
//                                            ADC_CUR_N_OFFSET5, ADC_CUR_N_OFFSET6, ADC_CUR_N_OFFSET7, ADC_CUR_N_OFFSET8, ADC_CUR_N_OFFSET9};

void Init_ADCdata(ST_ADC *p)
{
    memset(p, 0, sizeof(ST_ADC));
    p->sample_num = 1000;  // 1000个点 20ms
    p->interval_cnt = 500; // 25M / 500 = 50K FREQ 20us
}

void CalSampandInterval(float32 freq_M, Uint16 cntpersamp, Uint16 *sampnum, Uint16 *intercnt)
{
    Uint16 i = 1;
    Uint32 tmp_nplc_us = 0;
    Uint32 tmp_sample_num = 0;

    tmp_nplc_us = mgmd_stSCIRx.f32nPLC * 20000;

    for (i = 1; i < 100; i++)
    {
        tmp_sample_num = tmp_nplc_us * freq_M / i;
        if (tmp_sample_num <= 10000)
        {
            *intercnt = cntpersamp * i;
            *sampnum = tmp_sample_num;
            break;
        }
    }
}

float32 ADCReadAvg(Uint16 tmp_dataH, Uint16 dataM, Uint16 dataL, Uint16 sample_num)
{
    Uint16 tmp_sign = 0, dataH = 0;
    float32 data_avg = 0;
    int32 tmp_dataallDiv64 = 0;

    // 读取总和
    dataH = tmp_dataH & 0x003F; // 最高位仅有6位有效

    // 计算平均值
    // 先判断下dataH和dataM的最高位是否全1（负）或者全0（正），若是则可以不要dataH
    // 这里一定注意dataM的最高位也要一起判断，否则丢掉dataH后数据就错了
    tmp_sign = (dataH << 1) | (dataM >> 15);
    if ((tmp_sign == 0) || (tmp_sign == 0x7F))
    {
        data_avg = (float32)(int32)(((Uint32)dataM << 16) | (dataL)) / sample_num;
    }
    else // 若不是，说明数据很大，可以先除个64，将其转化为32位，强转int32来判断正负
    {
        tmp_dataallDiv64 = (int32)(((Uint32)dataH << 26) | ((Uint32)dataM << 10) | ((Uint32)dataL >> 6));
        data_avg = (float32)tmp_dataallDiv64 / sample_num * 64;
    }

    return data_avg;
}

float32 wave_vol_k = 0, wave_vol_b = 0, wave_cur_k = 0, wave_cur_b = 0;
Uint16 wave_coef_record = 0;
void ADCCoeff(ST_ADC *p)
{
    float32 hh_Volfullscal = 0, hh_Curfullscal = 0;

    hh_Volfullscal = (st_volrangeset.range.bit.dac01 == 0) ? 2.5 : 0.25; // 1/0.4  和 1/4   满量程是4V对应本量程最大值
    hh_Curfullscal = (st_currangeset.range.bit.dac01 == 0) ? 2.5 : 0.25;

    // 采样系数选择
    p->vol_gain = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32v_adc_p_k[mgmd_stSCIRx.SetRangeVol_index] : st_eerom_para.f32v_adc_n_k[mgmd_stSCIRx.SetRangeVol_index];   // PO = 0 正
    p->vol_offset = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32v_adc_p_b[mgmd_stSCIRx.SetRangeVol_index] : st_eerom_para.f32v_adc_n_b[mgmd_stSCIRx.SetRangeVol_index]; // PO = 0 正

    if (mgmd_stSCIRx.SetRangeCur_index == RANGE_CUR_200uA_INDEX)
    {
        p->cur_gain = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_adc_p_k[RANGE_CUR_100uA_INDEX] : st_eerom_para.f32i_adc_n_k[RANGE_CUR_100uA_INDEX];         // PO = 0 正
        p->cur_offset = ((mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_adc_p_b[RANGE_CUR_100uA_INDEX] : st_eerom_para.f32i_adc_n_b[RANGE_CUR_100uA_INDEX]) * 2; // PO = 0 正
    }
    else if (mgmd_stSCIRx.SetRangeCur_index == RANGE_CUR_10A_INDEX)
    {
        p->cur_gain = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_adc_p_k[RANGE_CUR_2A_INDEX] : st_eerom_para.f32i_adc_n_k[RANGE_CUR_2A_INDEX];         // PO = 0 正
        p->cur_offset = ((mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_adc_p_b[RANGE_CUR_2A_INDEX] : st_eerom_para.f32i_adc_n_b[RANGE_CUR_2A_INDEX]) * 5; // PO = 0 正
    }
    else
    {
        p->cur_gain = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_adc_p_k[mgmd_stSCIRx.SetRangeCur_index] : st_eerom_para.f32i_adc_n_k[mgmd_stSCIRx.SetRangeCur_index];   // PO = 0 正
        p->cur_offset = (mgmd_stSCIRx.CMD_POLARITY == 0) ? st_eerom_para.f32i_adc_p_b[mgmd_stSCIRx.SetRangeCur_index] : st_eerom_para.f32i_adc_n_b[mgmd_stSCIRx.SetRangeCur_index]; // PO = 0 正
    }

    if (mgmd_stSCIRx.adjust_adc_v == 1)
    {
        p->vol_gain = 1;
        p->vol_offset = 0;
    }
    if (mgmd_stSCIRx.adjust_adc_i == 1)
    {
        p->cur_gain = 1;
        p->cur_offset = 0;
    }

    mgmd_stSCIRx.f32DataCurRealT = (-mgmd_stSCIRx.f32DataCurRealT_5v * hh_Curfullscal * mgmd_stSCIRx.f32SetRangeCur) * p->cur_gain + p->cur_offset;
    mgmd_stSCIRx.f32DataVolRealT = (-mgmd_stSCIRx.f32DataVolRealT_5v * hh_Volfullscal * mgmd_stSCIRx.f32SetRangeVol) * p->vol_gain + p->vol_offset;

    wave_cur_k = -hh_Curfullscal / 8388608 * 5 * mgmd_stSCIRx.f32SetRangeCur * p->cur_gain;
    wave_cur_b = p->cur_offset;
    wave_vol_k = -hh_Volfullscal / 8388608 * 5 * mgmd_stSCIRx.f32SetRangeVol * p->vol_gain;
    wave_vol_b = p->vol_offset;
}

void SampCalculate(ST_ADC *p)
{
    float32 hh_Volfullscal = 0, hh_Curfullscal = 0;

    hh_Volfullscal = (st_volrangeset.range.bit.dac01 == 0) ? 2.5 : 0.25; // 1/0.4  和 1/4   满量程是4V对应本量程最大值
    hh_Curfullscal = (st_currangeset.range.bit.dac01 == 0) ? 2.5 : 0.25;

    // ADC与实际值的符号是相反的，因此这边加了个负号
    mgmd_stSCIRx.f32DataCur = (-st_ADCdata.data_cur_V * hh_Curfullscal * mgmd_stSCIRx.f32SetRangeCur) * p->cur_gain + p->cur_offset;
    mgmd_stSCIRx.f32DataVol = (-st_ADCdata.data_vol_V * hh_Volfullscal * mgmd_stSCIRx.f32SetRangeVol) * p->vol_gain + p->vol_offset;

    // test-- 1mA  100uA挡位   po = 0 正的时候 如果测出来负电流，则做处理。
    if (mgmd_stSCIRx.testflag == 0)
    {
        if ((mgmd_stSCIRx.CMD_POLARITY == 0) && (mgmd_stSCIRx.f32DataCur < 0) || (mgmd_stSCIRx.CMD_POLARITY == 1) && (mgmd_stSCIRx.f32DataCur > 0))
        {
            if (mgmd_stSCIRx.SetRangeCur_index == RANGE_CUR_1mA_INDEX)
                mgmd_stSCIRx.f32DataCur = -mgmd_stSCIRx.f32DataCur / 10;
            else if (mgmd_stSCIRx.SetRangeCur_index == RANGE_CUR_100uA_INDEX)
                mgmd_stSCIRx.f32DataCur = -mgmd_stSCIRx.f32DataCur / 10;
        }
    }
}

Uint32 hh_test_time_cnt[3] = {0, 0, 0};
// int32 tmp_dataallDiv64 = 0;
// p->trigger 置1一次，采样一次
void ADCSample(ST_ADC *p)
{
    static Uint16 state = 0;
    static Uint16 firstflag = 0;
    static Uint32 nowtime = 0;

    // 设置nPLC
    //    p->sample_num = mgmd_stSCIRx.f32nPLC*1000;   //1000个点是1个nPLC
    CalSampandInterval(CUR_2US_1, CUR_INTERVAL_2US, &p->sample_num, &p->interval_cnt);
    //    p->sample_num = 10000;
    //    p->interval_cnt = 50;
    // 判断是否开始采样
    if (mgmd_stSCIRx.ADC_trigger == 1)
    {
        mgmd_stSCIRx.ADC_trigger = 0;
        p->trigger = 1;
        p->dataOK = 0;
    }
    mgmd_stSCIRx.DataOKFlag = p->dataOK;

    // 采样系数选择
    ADCCoeff(p);

    // 记录波形存储的系数
    if (wave_coef_record == 1)
    {
        wave_coef_record = 0;
        WaveCurCoef(wave_cur_k, wave_cur_b);
        WaveVolCoef(wave_vol_k, wave_vol_b);
    }

    // 采样状态机
    switch (state)
    {
    case 0:
        // trigger = 1时等待2ms
        if (p->trigger == 1)
        {
            p->trigger = 0;
            p->dataOK = 0;
            state = 2;
            hh_test_time_cnt[0] = task_run_cnt;
        }

        break;

        //    case 1:   //延时
        //        if(firstflag == 0)
        //        {
        //            firstflag = 1;
        //            nowtime = task_run_cnt;
        //        }
        //        if((task_run_cnt - nowtime) > 1)   //暂时不用延时了，延时在前边做的
        //        {
        //            firstflag = 0;
        //            state = 2;
        //        }
        //        break;

    case 2:
        // 开始触发采样
        ADCTRIGGER_H()
        p->nowtime = task_run_cnt;

        state = 3;
        break;

    case 3: // 延时
        // 等待FPGA响应，至少得等一个clk，因为读在最后。要等到下一次读才会改变状态
        if (firstflag == 0)
        {
            firstflag = 1;
            nowtime = task_run_cnt;
        }
        if ((task_run_cnt - nowtime) > 3)
        {
            firstflag = 0;
            state = 4;
        }
        break;

    case 4: // 等待读数据
        // 等待dataok
        if (st_from_fpga.fpga_info.bit.adcok == 1)
        {
            p->data_cur_avg = ADCReadAvg(st_from_fpga.mdata1.bit.cur_H, st_from_fpga.mdata1.bit.cur_M, st_from_fpga.mdata2.bit.cur_L, p->sample_num);
            p->data_cur_V = p->data_cur_avg / 8388608 * 5; //--test

            //-----------------------------------2------------------------------//
            p->data_vol_avg = ADCReadAvg(st_from_fpga.mdata2.bit.vol_H, st_from_fpga.mdata3.bit.vol_M, st_from_fpga.mdata3.bit.vol_L, p->sample_num);
            p->data_vol_V = p->data_vol_avg / 8388608 * 5; //--test

            SampCalculate(p);

            p->dataOK = 1;
            // 将adcTrigger拉低

            ADCTRIGGER_L()

            p->sampletime = task_run_cnt - p->nowtime;

            state = 0;
            hh_test_time_cnt[1] = task_run_cnt - hh_test_time_cnt[0];
            hh_test_time_cnt[2]++;
        }

        break;

    default:
        state = 0;
    }
}
