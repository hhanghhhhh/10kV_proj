#ifndef DRV_CAN_CONTROLLER_H
#define DRV_CAN_CONTROLLER_H

#include "TypeDefine.h"

#define CAN_CONTROLLER_NODE_ID              (1U)
#define CAN_CONTROLLER_DEFAULT_SAMPLE_ID    (2U)
#define CAN_CONTROLLER_BROADCAST_ID         (0x0FU)
#define CAN_CONTROLLER_NODE_COUNT           (15U)

#define CAN_CONTROLLER_STATUS_SUCCESS       (0U)
#define CAN_CONTROLLER_STATUS_BUSY          (1U)
#define CAN_CONTROLLER_STATUS_TIMEOUT       (2U)
#define CAN_CONTROLLER_STATUS_INVALID_ID    (3U)
#define CAN_CONTROLLER_STATUS_NO_MESSAGE    (4U)

typedef struct
{
    Uint16 next_seq;
    Uint16 last_request_seq;
    Uint16 last_request_dst;

    volatile Uint16 completion_pending_mask;
    volatile Uint16 completion_seq[CAN_CONTROLLER_NODE_COUNT];
    volatile Uint16 completion_status[CAN_CONTROLLER_NODE_COUNT];
    volatile Uint32 completion_count;
    volatile Uint32 invalid_count;
    volatile Uint32 completion_overrun_count;
} CanController_Context_t;

/* 初始化触发板eCAN-A、GPIO、邮箱及模块Context。 */
Uint16 CanController_Init(CanController_Context_t *context);

/* 向指定采样板发送START_AVG，Seq由驱动自动递增。 */
Uint16 CanController_StartUnicast(CanController_Context_t *context,
                                  Uint16 sample_node_id);

/* 向所有采样板广播START_AVG，Seq由驱动自动递增。 */
Uint16 CanController_StartBroadcast(CanController_Context_t *context);

/* 在CAN-A接收ISR中解析并按源节点保存AVG_DONE。 */
Uint16 CanController_HandleRxInterrupt(CanController_Context_t *context);

/* 读取并清除指定采样板的一条待处理完成通知。 */
Uint16 CanController_TakeCompletion(CanController_Context_t *context,
                                    Uint16 sample_node_id,
                                    Uint16 *seq,
                                    Uint16 *status);

#endif
