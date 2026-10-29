/*
 * task_wave_upload.h
 *
 *  Created on: 2025年9月23日
 *      Author: m
 */

#ifndef SRC_TASK_WAVE_UPLOAD_H_
#define SRC_TASK_WAVE_UPLOAD_H_

#include "TYPEDEF.h"

// num 为 16 倍数，保证实际数量为一次突发传输的量 16 * 64 bit
// 一次存 2 个 32 bit 的电压、电流，num = 16 时满足一次总数量
// 目前 div_cnt = 1 时，2us 一个点， = x, 2x us一个点
// 总时间 = div_cnt * num us
typedef struct
{

	Uint16  ms;
	Uint32  num;	// 必须是 32 倍数
	Uint16  div_cnt;// 至少是 1
	Uint16	ok;
}ST_WAVE;

extern ST_WAVE st_wave;


void InitPLddr3(void);
void StartWaveRecord(Uint16 wave_ms);
void StopWaveRecord(void);
void WaveRangeSwitchCur(void);
void WaveRangeSwitchVol(void);
void WaveRangeSaveCur(Uint16 range_index);
void WaveRangeSaveVol(Uint16 range_index);
void WaveUploadTask(void);

#endif /* SRC_TASK_WAVE_UPLOAD_H_ */
