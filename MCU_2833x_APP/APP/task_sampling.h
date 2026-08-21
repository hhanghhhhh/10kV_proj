#ifndef TASK_SAMPLING_H
#define TASK_SAMPLING_H

#include "TypeDefine.h"

#define SAMPLING_SOURCE_CAN       (0U)
#define SAMPLING_SOURCE_MODBUS    (1U)

/* 初始化采样业务状态和数字电位计控制通道。 */
void SamplingTask_Init(void);

/* 按指定触发来源重新开始一次平均采样。 */
void SamplingTask_StartAverage(Uint16 source);

/* 处理采样参数、数据发布及完成报文回复。 */
void SamplingTask_Run(void);

#endif
