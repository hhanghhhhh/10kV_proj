#ifndef DRV_AD7982_H
#define DRV_AD7982_H

#include "TypeDefine.h"

#define AD7982_TARGET_COUNT_20MS (2000UL)

typedef struct
{
    volatile Uint32 raw_adc_value;        /* McBSP原始32-bit接收位流 */
    volatile Uint32 frame_count;          /* DMA完成帧数 */
    volatile Uint32 launch_overrun_count; /* McBSP未就绪导致的漏启动次数 */
    volatile Uint16 start_error;          /* 启动前McBSP未进入就绪状态 */

    volatile int32 live_adc_value; /* 最新有效18-bit有符号样点 */
    volatile float32 sum_adc;      /* 当前统计窗口累加值 */
    volatile Uint32 count_adc;     /* 当前统计窗口样点数 */
    volatile Uint32 target_count;  /* 统计目标样点数 */
    volatile Uint16 avg_active;    /* 平均值统计使能 */

    volatile float32 done_sum;      /* 最新完成窗口的累加快照 */
    volatile Uint32 done_count;     /* 最新完成窗口的计数快照 */
    volatile Uint16 calc_done;      /* 主循环待处理标志 */
    volatile float32 final_average; /* 主循环计算的原始码平均值 */
} Ad7982_Context_t;

void Ad7982_Init(Ad7982_Context_t *context);
void Ad7982_Start(Ad7982_Context_t *context);
void Ad7982_StartFrame(Ad7982_Context_t *context);

/* CAN触发时放弃旧窗口并重新开始统计。 */
void Ad7982_StartAverage(Ad7982_Context_t *context);

/* 处理一个DMA样点并在达到目标数量时锁存结果。 */
void Ad7982_OnDmaComplete(Ad7982_Context_t *context);

#endif
