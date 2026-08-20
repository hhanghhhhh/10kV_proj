#ifndef DRV_CAN_SAMPLE_H
#define DRV_CAN_SAMPLE_H

#include "TypeDefine.h"

#define CAN_SAMPLE_NODE_ID             (2U)
#define CAN_SAMPLE_BROADCAST_ID        (0x0FU)

#define CAN_SAMPLE_STATUS_SUCCESS      (0U)
#define CAN_SAMPLE_STATUS_BUSY         (1U)
#define CAN_SAMPLE_STATUS_TIMEOUT      (2U)
#define CAN_SAMPLE_STATUS_NO_REQUEST   (3U)

#define CAN_SAMPLE_DONE_OK             (0x00U)

typedef struct
{
    volatile Uint16 request_src;
    volatile Uint16 request_seq;
    volatile Uint16 request_valid;

    volatile Uint16 done_request_src;
    volatile Uint16 done_request_seq;
    volatile Uint16 done_pending;

    volatile Uint32 start_count;
    volatile Uint32 invalid_count;
    volatile Uint32 done_overrun_count;
} CanSample_Context_t;

/* 初始化采样板eCAN-A、GPIO、邮箱及模块Context。 */
Uint16 CanSample_Init(CanSample_Context_t *context);

/* 在CAN-A接收ISR中解析START_AVG，返回是否收到有效触发。 */
Uint16 CanSample_HandleRxInterrupt(CanSample_Context_t *context);

/* 在本轮统计完成时锁存回复目标和Seq，防止被新触发覆盖。 */
void CanSample_LatchCompletedRequest(CanSample_Context_t *context);

/* 在结果发布后阻塞发送AVG_DONE，成功或超时后返回。 */
Uint16 CanSample_SendCompleted(CanSample_Context_t *context, Uint16 status);

#endif
