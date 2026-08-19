#ifndef DRV_AD7982_H
#define DRV_AD7982_H

#include "TypeDefine.h"

typedef struct
{
    volatile Uint32 raw_adc_value;        /* McBSP原始32-bit接收位流 */
    volatile Uint32 frame_count;          /* DMA完成帧数 */
    volatile Uint32 launch_overrun_count; /* McBSP未就绪导致的漏启动次数 */
    volatile Uint16 sample_valid;         /* 丢弃首帧后置位 */
    volatile Uint16 start_error;          /* 启动前McBSP未进入就绪状态 */
} Ad7982_Context_t;

void Ad7982_Init(Ad7982_Context_t *context);
void Ad7982_Start(Ad7982_Context_t *context);
void Ad7982_StartFrame(Ad7982_Context_t *context);
void Ad7982_OnDmaComplete(Ad7982_Context_t *context);

#endif
