#ifndef DRV_CAN_CONTROLLER_H
#define DRV_CAN_CONTROLLER_H

#include "TYPEDEF.h"

#define CAN_CONTROLLER_NODE_ID (3U) // 低压源表 1，表 2，高压源表 3
#define CAN_CONTROLLER_DEFAULT_SAMPLE_ID (2U)
#define CAN_CONTROLLER_REPLY_ID (0x0200U | CAN_CONTROLLER_NODE_ID)
#define CAN_CONTROLLER_REPLY_ID_MASK (0x070FU)
#define CAN_CONTROLLER_TX_TIMEOUT_MS (10U)
#define CAN_CONTROLLER_REPLY_TIMEOUT_MS (1000U)
#define CAN_CONTROLLER_STATUS_SUCCESS (0U)
#define CAN_CONTROLLER_STATUS_TIMEOUT (2U)

typedef struct
{
    Uint16 next_seq;
    volatile Uint16 request_seq;
    volatile Uint16 request_dst;
    volatile Uint16 request_pending;
    volatile Uint16 tx_done;
    volatile Uint16 tx_status;
    volatile Uint16 reply_received;
    volatile Uint16 reply_status;
} CanController_Context_t;

extern CanController_Context_t g_can_controller;

/* 初始化新协议状态，不重复初始化PS CAN。 */
void CanController_Init(void);
/* 定时任务提交单次请求，实际发送由主循环执行。 */
void CanController_RequestStart(Uint16 sample_node_id);
/* 主循环中限时发送标准帧。 */
void CanController_Process(void);
/* CAN接收中断传入标准数据帧，核对类型、节点及seq。 */
void CanController_HandleReply(Uint16 can_id, Uint16 seq, Uint16 status);

#endif
