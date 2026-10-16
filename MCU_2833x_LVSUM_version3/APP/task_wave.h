/*
 * task_wave.h
 *
 *  Created on: 2025年11月11日
 *      Author: m
 */

#ifndef APP_TASK_WAVE_H_
#define APP_TASK_WAVE_H_

#include "TypeDefine.h"


#define WAVE_BUF_NUM     500
#define GROUP_SIZE       10
#define FPGA_RAM_DEPTH   11000
#define WAVE_TOTAL_MS    1000

#define SAMP_CNT_MS      100    // 100k  1ms 100个点
#define SAMP_INTERVAL    500    // 100k  10us 一个点  50M clk

#define  ADDR_DATA_BASE     6
#define  ADDR_START_ADDR    16
#define  ADDR_CTRL          5

#define  SEL_CUR            0
#define  SEL_VOL            1


typedef struct
{
    Uint16 en;
    Uint16 read_ok;
    Uint16 send_ok;
    Uint16 t_ms;
    Uint16 div_cnt;
    float32 cur_k;
    float32 cur_b;
    float32 vol_k;
    float32 vol_b;
    Uint16 last_addr;
    Uint16 cur_addr;
    Uint16 index_head;
    Uint16 index_tail;
    float32 ringbuf[WAVE_BUF_NUM];

    union
    {
        Uint32 u32;
        struct
        {
            Uint32 reset:1;
            Uint32 next:1;
            // 采样率
            Uint32 intervl_cnt:11;  //50M  100k 为 500
            // 脉宽结束后再采多久
            Uint32 delay_cnt:3;    //50M  1 代表 524_288(1ms)，fpga 那边再放大
            Uint32 test_en:1;
            Uint32 samp_num:14;
            Uint32 sel:1;
        }bit;
    }to_fpga;
}ST_WAVE;


void InitTaskWave(void);
void TaskWaveUpload(Uint16 t_ms);
void WaveCmdParse(Uint8* rxbuf, Uint8 socket, Uint8* txbuf, Uint16 (*fsendp)(Uint8, const Uint8*, Uint16));
void WaveSendData(Uint8* txbuf, Uint16 (*fsendp)(Uint8, const Uint8*, Uint16));
void EnableWave(void);
void DisableWave(void);
void WaveVolCoef(float32 k, float32 b);
void WaveCurCoef(float32 k, float32 b);

#endif /* APP_TASK_WAVE_H_ */
