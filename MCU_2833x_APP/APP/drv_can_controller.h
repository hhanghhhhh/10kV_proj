#ifndef DRV_CAN_CONTROLLER_H
#define DRV_CAN_CONTROLLER_H

#include "TypeDefine.h"

#define CAN_CONTROLLER_NODE_ID              (1U)
#define CAN_CONTROLLER_DEFAULT_SAMPLE_ID    (2U)
#define CAN_CONTROLLER_BROADCAST_ID         (0x0FU)

#define CAN_CONTROLLER_STATUS_SUCCESS       (0U)
#define CAN_CONTROLLER_STATUS_TIMEOUT       (2U)
#define CAN_CONTROLLER_STATUS_INVALID_ID    (3U)

typedef struct
{
    Uint16 next_seq;
    volatile Uint16 last_request_seq;
    volatile Uint16 last_request_dst;
    volatile Uint16 reply_received;
    volatile Uint16 reply_status;
} CanController_Context_t;

/* 初始化触发板eCAN-A、GPIO、邮箱及模块Context。 */
Uint16 CanController_Init(CanController_Context_t *context);

/* 向指定采样板发送START_AVG，Seq由驱动自动递增。 */
Uint16 CanController_StartUnicast(CanController_Context_t *context,
                                  Uint16 sample_node_id);

/* 在CAN-A接收ISR中校验并保存当前目标节点的AVG_DONE。 */
Uint16 CanController_HandleRxInterrupt(CanController_Context_t *context);

/* 返回当前目标节点是否已回复，已回复时输出Status。 */
Uint16 CanController_GetReply(CanController_Context_t *context,
                              Uint16 *reply_status);

#endif
