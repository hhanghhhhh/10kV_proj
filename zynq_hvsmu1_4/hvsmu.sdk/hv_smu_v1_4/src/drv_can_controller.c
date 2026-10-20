#include "COMMON.h"
#include "drv_can_controller.h"

CanController_Context_t g_can_controller;

/* 每次上电从seq=0开始，独立于原meter扩展帧协议。 */
void CanController_Init(void)
{
    memset(&g_can_controller, 0, sizeof(g_can_controller));
}

/* 先准备本次请求，pending最后置位，交给main发送。 */
void CanController_RequestStart(Uint16 sample_node_id)
{
    g_can_controller.request_seq = g_can_controller.next_seq;
    g_can_controller.next_seq = (g_can_controller.next_seq + 1U) & 0x00FFU;
    g_can_controller.request_dst = sample_node_id;
    g_can_controller.tx_done = 0U;
    g_can_controller.tx_status = CAN_CONTROLLER_STATUS_SUCCESS;
    g_can_controller.reply_received = 0U;
    g_can_controller.reply_status = 0U;
    g_can_controller.request_pending = 1U;
}

/* 原协议继续使用FIFO，新协议使用独立的高优先级发送缓冲区。 */
void CanController_Process(void)
{
    Uint16 can_id;
    Uint16 status;

    if (g_can_controller.request_pending == 0U)
    {
        return;
    }
    g_can_controller.request_pending = 0U;
    if ((mgmd_stSCIRx.test_mode != MODE_CAN) ||
        (mgmd_stSCIRx.power_ctrl == POWER_OFF))
    {
        return;
    }

    can_id = 0x0100U | (CAN_CONTROLLER_NODE_ID << 4U) |
             g_can_controller.request_dst;
    status = PS_CAN_SendStandard(can_id, g_can_controller.request_seq);
    g_can_controller.tx_status = status;
    g_can_controller.tx_done = 1U;
}

/* 新协议回复只认本次指定节点及seq。 */
void CanController_HandleReply(Uint16 can_id, Uint16 seq, Uint16 status)
{
    if (((can_id & CAN_CONTROLLER_REPLY_ID_MASK) != CAN_CONTROLLER_REPLY_ID) ||
        (((can_id >> 4U) & 0x000FU) != g_can_controller.request_dst) ||
        (seq != g_can_controller.request_seq))
    {
        return;
    }
    g_can_controller.reply_status = status;
    g_can_controller.reply_received = 1U;
}
