/*
 * task_wave_upload.c
 *
 *  Created on: 2025年9月23日
 *      Author: m
 */
#include "COMMON.h"

#define UNCACHED_BASE 0x06000000U /* 起始地址 */
#define SECTION_SIZE 0x00100000U  /* 1MB */
#define NUM_SECTIONS 1            /* 若要覆盖 >1MB, 改为需要的节数 */

ST_WAVE st_wave;
static Uint16 wave_trans_start = 0;
Uint16 wave_trans_err = 0;
void WaveParaLimit(void);
extern float32 rangeVol_buff[];
extern float32 rangeCur_buff[];
extern float32 vol_adc_full[];
extern float32 cur_adc_full[];

// 将对应内存设置为 non-cacheable 属性
void InitPLddr3(void)
{
    Uint32 addr = UNCACHED_BASE;
    Uint16 i;

    memset(&st_wave, 0, sizeof(ST_WAVE));
    st_wave.ms = 1;
    st_wave.num = 128;
    st_wave.div_cnt = 1;

    // 为每个 1MB section 设置 non-cacheable 属性
    for (i = 0; i < NUM_SECTIONS; ++i)
    {
        Xil_SetTlbAttributes((UINTPTR)addr, (u64)NORM_NONCACHE);
        addr += SECTION_SIZE;
    }

    // 清除该范围的 D-cache
    Xil_DCacheInvalidateRange((UINTPTR)UNCACHED_BASE, SECTION_SIZE * NUM_SECTIONS);
    asm volatile("dsb" ::: "memory");
}

void StartWaveRecord(Uint16 wave_ms)
{
    ST_WAVE *p = &st_wave;

    p->ms = wave_ms;
    p->ok = 0;
    wave_trans_start = 0;

    // 参数限制
    WaveParaLimit();

    // 参数传递给 pl 侧
    st_topl.test_ddr3.bit.en = 1;
    st_topl.test_ddr3.bit.range_sw_cur = 0;
    st_topl.test_ddr3.bit.range_sw_vol = 0;
    st_topl.wave_para.bit.num = p->num;
    st_topl.wave_para.bit.div = p->div_cnt;
}
void StopWaveRecord(void)
{
    st_topl.test_ddr3.bit.en = 0;
    wave_trans_start = 1;
}
void WaveRangeSwitchCur(void)
{
    st_topl.test_ddr3.bit.range_sw_cur++;
}
void WaveRangeSwitchVol(void)
{
    st_topl.test_ddr3.bit.range_sw_vol++;
}

static Uint16 range_vol[4];
static Uint16 range_cur[4];
void WaveRangeSaveVol(Uint16 range_index)
{
    range_vol[st_topl.test_ddr3.bit.range_sw_vol] = range_index;
}
void WaveRangeSaveCur(Uint16 range_index)
{
    range_cur[st_topl.test_ddr3.bit.range_sw_cur] = range_index;
}

static Uint32 GetNum(void)
{
    return (st_frompl.test_awaddr - UNCACHED_BASE) / 4;
}

#define WAVE_BUF_SIZE 100000
float32 wave_vol_buf[WAVE_BUF_SIZE];
float32 wave_cur_buf[WAVE_BUF_SIZE];
Uint32 wave_vol_index;
Uint32 wave_cur_index;

union
{
    Uint32 u32;
    struct
    {
        Uint32 data : 23;
        Uint32 sign : 1;
        Uint32 tag : 8;
    } bit;
} un_wave_data;

union
{
    int32 i32;
    struct
    {
        Uint32 data : 23;
        Uint32 sign : 1;
        Uint32 sign_ex : 8;
    } bit;
} tmp_data;

Uint32 hh_test_cnt_start = 0;
Uint32 hh_test_cnt_use = 0;
// 将采样值转化为真实值，乘不同的采样系数
void TransferWaveData(void)
{
    Uint32 i = 0;
    Uint32 len = 0;
    Uint32 *p = (Uint32 *)UNCACHED_BASE;

    if (wave_trans_start == 1)
    {
        wave_trans_start = 0; // 仅转化一次
        hh_test_cnt_start = task_run_cnt;

        len = GetNum();
        wave_vol_index = 0;
        wave_cur_index = 0;
        for (i = 0; i < len; i += 2)
        {
            if (wave_vol_index >= WAVE_BUF_SIZE || wave_cur_index >= WAVE_BUF_SIZE)
                break;

            // 电流
            un_wave_data.u32 = *(p + i);
            // 转化为32bit有符号数
            tmp_data.bit.data = un_wave_data.bit.data;
            tmp_data.bit.sign = un_wave_data.bit.sign;
            tmp_data.bit.sign_ex = (un_wave_data.bit.sign) ? 0xFF : 0x00;

            if (mgmd_stSCIRx.wave.cur_coef_en == 1)
            {
                if (un_wave_data.bit.tag == 0x5A)
                    wave_cur_buf[wave_cur_index++] = tmp_data.i32 * CUR_IN_ADC_V * rangeCur_buff[range_cur[0]] / cur_adc_full[range_cur[0]];
                else if (un_wave_data.bit.tag == 0x5B)
                    wave_cur_buf[wave_cur_index++] = tmp_data.i32 * CUR_IN_ADC_V * rangeCur_buff[range_cur[1]] / cur_adc_full[range_cur[1]];
                else
                    wave_trans_err++;
            }
            else
            {
                wave_cur_buf[wave_cur_index++] = tmp_data.i32 * CUR_IN_ADC_V;
            }

            // 电压
            un_wave_data.u32 = *(p + i + 1);
            // 转化为32bit有符号数
            tmp_data.bit.data = un_wave_data.bit.data;
            tmp_data.bit.sign = un_wave_data.bit.sign;
            tmp_data.bit.sign_ex = (un_wave_data.bit.sign) ? 0xFF : 0x00;

            if (mgmd_stSCIRx.wave.vol_coef_en == 1)
            {
                if (un_wave_data.bit.tag == 0xA5)
                    wave_vol_buf[wave_vol_index++] = tmp_data.i32 * VOL_IN_ADC_V * rangeVol_buff[range_vol[0]] / vol_adc_full[range_vol[0]];
                else
                    wave_trans_err++;
            }
            else
            {
                wave_vol_buf[wave_vol_index++] = tmp_data.i32 * VOL_IN_ADC_V;
            }
        }
        st_wave.ok = 1;
        hh_test_cnt_use = task_run_cnt - hh_test_cnt_start;
    }
}

Uint16 hh_test_en = 0;
Uint16 hh_test_data = 0;
Uint32 hh_test_buf[5];
Uint16 hh_test_ms = 1;
void TestDDR3(void)
{
    static Uint16 hh_test_cnt = 0;
    Uint16 i = 0;
    volatile Uint32 *ddr_ptr = (volatile Uint32 *)UNCACHED_BASE;

    // 转化为真实值 float 类型
    TransferWaveData();

    if (hh_test_en == 1)
    {
        //		hh_test_en = 0;
        st_topl.test_ddr3.bit.data = hh_test_data;
        usleep(1000);
        st_topl.test_ddr3.bit.update = 1;
        usleep(1000);
        st_topl.test_ddr3.bit.update = 0;
        hh_test_cnt++;
        if (hh_test_cnt == 64)
        {
            hh_test_cnt = 0;
            hh_test_en = 0;
        }
    }
    else if (hh_test_en == 3)
    {
        hh_test_en = 0;
        Xil_DCacheInvalidateRange((UINTPTR)ddr_ptr, 64 * sizeof(Uint32));
        for (i = 0; i < 5; i++)
        {
            hh_test_buf[i] = ddr_ptr[i];
        }
    }
    else if (hh_test_en == 2)
    {
        hh_test_en = 0;
        StartWaveRecord(hh_test_ms);
    }
}

#define WAVE_TOTAL_MS 1000 // 最大脉宽 1s
#define WAVE_MIN_DIV 5     // 最小采样间隔 5 * 2us = 10us

void WaveParaLimit(void)
{
    ST_WAVE *p = &st_wave;
    Uint32 tmp_num = 0;
    Uint16 i = 0;

    // div_cnt 限制
    if (p->div_cnt == 0)
        p->div_cnt = 1;
    // 脉宽限制
    if (p->ms > WAVE_TOTAL_MS)
        p->ms = WAVE_TOTAL_MS;
    if (p->ms == 0)
        p->ms = 1;

    // 脉宽满足，则无需重新计算 div 和 num
    tmp_num = (Uint32)p->ms * 1000U / (p->div_cnt * SAMP_US);
    if ((p->num < tmp_num + 16) && (p->num > tmp_num - 16) && (p->num != 0))
        return;

    // 寻找合适的 div_cnt
    for (i = WAVE_MIN_DIV; i < 100; i++)
    {
        tmp_num = (Uint32)p->ms * 1000U / (i * SAMP_US);
        if (tmp_num <= WAVE_BUF_SIZE && tmp_num >= 16)
        {
            // 保证 num 为 16 倍数，保证一次 axi 突发传输的数量
            p->num = (tmp_num + 15) & ~15;
            p->div_cnt = i;
            return;
        }
    }

    // 如果循环没找到合适值，使用默认值
    p->num = WAVE_BUF_SIZE;
    p->div_cnt = 1;
}

float32 div_ns = 0;
void WaveUploadTask(void)
{
    static Uint16 first_flag = 0;
    ST_WAVE *p = &st_wave;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
    static Uint32 en_cnt = 0;

    // 上位机主动触发录波
    if (pPC->wave.enable == 1 && pPC->wave.trigger == 1)
    {
        pPC->wave.trigger = 0;
        StartWaveRecord(pPC->wave.time_ms);
    }
    pPC->wave.ok = p->ok;
    st_topl.cmd.bit.wave_filter_en = pPC->wave.filter_en;

    if (p->ok == 1)
        div_ns = p->div_cnt * SAMP_US * 1000;
    else
        div_ns = -1; // 波形未 ok

    if (st_topl.test_ddr3.bit.en == 1)
        en_cnt++;
    else
        en_cnt = 0;

    if (en_cnt > Tms_CNT(10))
    {
        // 超时 1s 后关闭
        if (en_cnt > Tms_CNT(1000))
            StopWaveRecord();

        // 数量到达后关闭
        if (st_frompl.pl_data2.bit.wave_ok == 1)
        {
            if (first_flag == 0)
            {
                first_flag = 1;
                StopWaveRecord();
            }
        }
        else
            first_flag = 0;
    }
}

float32 send_state = 0;
u8_t test_send[1048576];
// 网口接收、发送
Uint32 WaveUploadHandle(Uint8 *recv_buf, Uint16 recv_len, Uint8 **p_send_buf)
{
    Uint32 send_len = 0;

    if (recv_len == 8 && !memcmp("testsend", recv_buf, 8))
    {
        send_len = 1048576;
        *p_send_buf = test_send;
        //		send_len = 0x400;
        //		*p_send_buf = (Uint8 *)UNCACHED_BASE;
    }
    else if (recv_len == 12 && !memcmp("wave_trigger", recv_buf, 12))
    {
        send_state = 1; // 回复数字 1
        send_len = 4;
        *p_send_buf = (Uint8 *)&send_state;
        StartWaveRecord(mgmd_stSCIRx.wave.time_ms);
    }
    else if (recv_len == 14 && !memcmp("wave_en_filter", recv_buf, 14))
    {
        send_state = 2; // 回复数字 2
        send_len = 4;
        *p_send_buf = (Uint8 *)&send_state;
        mgmd_stSCIRx.wave.filter_en = 1;
    }
    else if (recv_len == 15 && !memcmp("wave_getfreq_ns", recv_buf, 15))
    {
        send_len = 4;
        *p_send_buf = (Uint8 *)&div_ns; // -1 代表波形未 ok
    }
    else if (recv_len == 11 && !memcmp("wave_getcur", recv_buf, 11))
    {
        send_len = wave_cur_index << 2;
        *p_send_buf = (Uint8 *)wave_cur_buf;
    }
    else if (recv_len == 11 && !memcmp("wave_getvol", recv_buf, 11))
    {
        send_len = wave_vol_index << 2;
        *p_send_buf = (Uint8 *)wave_vol_buf;
    }

    return send_len;
}
