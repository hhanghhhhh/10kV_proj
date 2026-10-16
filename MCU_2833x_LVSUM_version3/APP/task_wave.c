/*
 * task_wave.c
 *
 *  Created on: 2025年11月11日
 *      Author: m
 *
 *      后续提升速度的做法：1. 增大 GROUP_SIZE ，每次多读，此时要看中断时间是否够，以及修改 fpga 相关代码
 *                       2. 增大单次网口发送数据量，最大可改为 2048
 *
 *
 */

#include <string.h>
#include "task_wave.h"
#include "drv_fpga.h"
#include "drv_McBSP.h"
#include "drv_GlobalVar.h"

ST_WAVE st_wave;
Uint16 start_upload = 0;

void InitTaskWave(void)
{
    ST_WAVE *p = &st_wave;
    memset(&st_wave, 0, sizeof(ST_WAVE));
    p->to_fpga.bit.intervl_cnt = SAMP_INTERVAL; // 100k
    p->to_fpga.bit.delay_cnt = 3;               // en 使能多 1ms
    p->to_fpga.bit.samp_num = 1000;
    p->cur_k = 1;
    p->cur_b = 0;
}

void StartWaveRecord(Uint16 sel)
{
    ST_WAVE *p = &st_wave;

    p->cur_addr = ReadFPGA32(ADDR_START_ADDR);
    p->last_addr = p->cur_addr;
    p->to_fpga.bit.reset ^= 1; // 触发 FPGA 清零并开始存储
    p->to_fpga.bit.sel = sel;
    p->read_ok = 0;
    p->send_ok = 0;
    p->index_head = 0;
    p->index_tail = 0;
    p->en = 1;
    WriteFPGA32(ADDR_CTRL, p->to_fpga.u32); // 写控制字到 FPGA 控制寄存器
}

void EnableWave(void)
{
    ST_WAVE *p = &st_wave;

    p->to_fpga.bit.test_en = 1;
    WriteFPGA32(ADDR_CTRL, p->to_fpga.u32); // 写控制字到 FPGA 控制寄存器
}

void DisableWave(void)
{
    ST_WAVE *p = &st_wave;

    p->to_fpga.bit.test_en = 0;
    WriteFPGA32(ADDR_CTRL, p->to_fpga.u32); // 写控制字到 FPGA 控制寄存器
}

// 注意这里存储时间稍微小于 en 使能时间，否则最后的点还是上次的，没刷新。
void WaveTimeSet(Uint16 t_ms)
{
    ST_WAVE *p = &st_wave;
    Uint16 div = 0;

    p->t_ms = t_ms;
    if (p->div_cnt == 0)
        p->div_cnt = 1;
    if (p->t_ms > WAVE_TOTAL_MS)
        p->t_ms = WAVE_TOTAL_MS;
    if (p->t_ms == 0)
        p->t_ms = 1;

    div = ((Uint32)p->t_ms * SAMP_CNT_MS + FPGA_RAM_DEPTH - 1) / FPGA_RAM_DEPTH;
    if (div > 9)
        div = 9;
    p->div_cnt = div;
    p->to_fpga.bit.intervl_cnt = SAMP_INTERVAL * div;
    p->to_fpga.bit.samp_num = (p->t_ms * SAMP_CNT_MS / div) / GROUP_SIZE * GROUP_SIZE; // 向下取整
    p->to_fpga.bit.delay_cnt = 3;                                                      // en 使能多 1ms
}

// 计算 ringbuf 剩余可用空间，剩余一个，防止出现 head 与 tail 相等计算 len 出错
static inline Uint16 RingBufFreeCnt(ST_WAVE *p)
{
    return (p->index_head >= p->index_tail) ? (WAVE_BUF_NUM - (p->index_head - p->index_tail) - 1) : (p->index_tail - p->index_head - 1);
}

static inline void RingBufPush(ST_WAVE *p, float32 val)
{
    p->ringbuf[p->index_head] = val;
    p->index_head = (p->index_head + 1) % WAVE_BUF_NUM;
}

static inline float32 RingBufPop(ST_WAVE *p)
{
    float32 data = p->ringbuf[p->index_tail];
    p->index_tail = (p->index_tail + 1) % WAVE_BUF_NUM;
    return data;
}

void WaveCurCoef(float32 k, float32 b)
{
    st_wave.cur_k = k;
    st_wave.cur_b = b;
}
void WaveVolCoef(float32 k, float32 b)
{
    st_wave.vol_k = k;
    st_wave.vol_b = b;
}

Uint16 max_over_time = 0;
Uint16 test_addr = 0;
void TaskWaveUpload(Uint16 t_ms)
{
    ST_WAVE *p = &st_wave;
    Uint32 tmp_data = 0;
    float32 fdata = 0;
    Uint16 i;
    static Uint16 over_time = 0;

    if (start_upload == SEL_CUR + 1)
    {
        start_upload = 0;
        StartWaveRecord(SEL_CUR);
    }
    else if (start_upload == SEL_VOL + 1)
    {
        start_upload = 0;
        StartWaveRecord(SEL_VOL);
    }

    WaveTimeSet(t_ms);
    WriteFPGA32(ADDR_CTRL, p->to_fpga.u32); // 写控制字到 FPGA 控制寄存器

    if (p->read_ok == 1 && p->index_head == p->index_tail)
        p->send_ok = 1;

    if (p->en == 0)
        return;

    if (RingBufFreeCnt(p) < GROUP_SIZE)
        return;

    p->cur_addr = ReadFPGA32(ADDR_START_ADDR);

    if (p->last_addr != p->cur_addr)
    {
        p->last_addr = p->cur_addr;
        // 读取数据
        for (i = 0; i < GROUP_SIZE; i++)
        {
            tmp_data = ReadFPGA32(ADDR_DATA_BASE + i); // 读出 32 位

            // 转换为 float
            if (p->to_fpga.bit.sel == SEL_CUR)
                fdata = (float32)(int32)tmp_data * st_wave.cur_k + st_wave.cur_b;
            else
                fdata = (float32)(int32)tmp_data * st_wave.vol_k + st_wave.vol_b;

            // 存入 ringbuf
            RingBufPush(p, fdata);
        }
        // 翻转 FPGA 的 next 信号，请求下一个 10 点
        p->to_fpga.bit.next ^= 1;
        over_time = 0;

        // 读完停止
        if (p->cur_addr >= (p->to_fpga.bit.samp_num - GROUP_SIZE))
        {
            p->en = 0;
            p->read_ok = 1;
        }
    }
    else
    {
        over_time++;
        if (over_time > Tms_CNT(1))
        {
            p->to_fpga.bit.next ^= 1;
            over_time = 0;
        }
    }
}

typedef union
{
    float32 f32;
    struct
    {
        Uint32 LL : 8;
        Uint32 LH : 8;
        Uint32 HL : 8;
        Uint32 HH : 8;
    } u8;
} CONV_F32;

Uint8 socket_record = 0;
void WaveCmdParse(Uint8 *rxbuf, Uint8 socket, Uint8 *txbuf, Uint16 (*fsendp)(Uint8, const Uint8 *, Uint16))
{
    CONV_F32 tmp_data;
    Uint16 send_cnt = 0;

    if (!memcmp("wave_getcur", rxbuf, 11))
    {
        start_upload = SEL_CUR + 1;
        socket_record = socket;
    }
    else if (!memcmp("wave_getvol", rxbuf, 11))
    {
        start_upload = SEL_VOL + 1;
        socket_record = socket;
    }
    else if (!memcmp("wave_getfreq_ns", rxbuf, 15))
    {
        tmp_data.f32 = st_wave.to_fpga.bit.intervl_cnt * 20; // /50*1000  (50Mclk)
        txbuf[send_cnt++] = tmp_data.u8.LL;
        txbuf[send_cnt++] = tmp_data.u8.LH;
        txbuf[send_cnt++] = tmp_data.u8.HL;
        txbuf[send_cnt++] = tmp_data.u8.HH;
        fsendp(socket, txbuf, send_cnt);
    }
}

void WaveSendData(Uint8 *txbuf, Uint16 (*fsendp)(Uint8, const Uint8 *, Uint16))
{
    ST_WAVE *p = &st_wave;
    static Uint16 send_cnt = 0;
    CONV_F32 tmp_data;

    if (p->send_ok == 1)
        return;

    while (p->index_tail != p->index_head)
    {
        tmp_data.f32 = RingBufPop(p);
        txbuf[send_cnt++] = tmp_data.u8.LL;
        txbuf[send_cnt++] = tmp_data.u8.LH;
        txbuf[send_cnt++] = tmp_data.u8.HL;
        txbuf[send_cnt++] = tmp_data.u8.HH;
        if (send_cnt >= 1024)
        {
            fsendp(socket_record, txbuf, send_cnt);
            send_cnt = 0;
        }
    }

    // 剩余部分发出去
    if (send_cnt > 0)
    {
        fsendp(socket_record, txbuf, send_cnt);
        send_cnt = 0;
    }
}
