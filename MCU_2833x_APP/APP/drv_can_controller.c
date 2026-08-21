#include "drv_can_controller.h"
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define CAN_CONTROLLER_TYPE_START_AVG   (0x01U)
#define CAN_CONTROLLER_TYPE_AVG_DONE    (0x02U)
#define CAN_CONTROLLER_TX_MAILBOX       (0U)
#define CAN_CONTROLLER_RX_MAILBOX       (16U)
#define CAN_CONTROLLER_TX_MASK          (1UL << CAN_CONTROLLER_TX_MAILBOX)
#define CAN_CONTROLLER_RX_MASK          (1UL << CAN_CONTROLLER_RX_MAILBOX)
#define CAN_CONTROLLER_SOURCE_ID_MASK   (0x0F0UL << 18U)
#define CAN_CONTROLLER_LAM_IDE_COMPARE  (0x80000000UL)
#define CAN_CONTROLLER_CONFIG_TIMEOUT   (65535U)
#define CAN_CONTROLLER_TX_TIMEOUT       (1000000UL)

static Uint16 CanController_BuildId(Uint16 type, Uint16 src_id, Uint16 dst_id);
static void CanController_InitGpio(void);
static Uint16 CanController_InitCan(void);
static void CanController_InitMailboxes(void);
static Uint16 CanController_SendStart(CanController_Context_t *context,
                                      Uint16 dst_id);

/* 初始化模块状态及触发板CAN通信资源。 */
Uint16 CanController_Init(CanController_Context_t *context)
{
    context->next_seq = 0U;
    context->last_request_seq = 0U;
    context->last_request_dst = 0U;
    context->reply_received = 0U;
    context->reply_status = 0U;

    CanController_InitGpio();
    if (CanController_InitCan() != CAN_CONTROLLER_STATUS_SUCCESS)
    {
        return CAN_CONTROLLER_STATUS_TIMEOUT;
    }

    CanController_InitMailboxes();
    return CAN_CONTROLLER_STATUS_SUCCESS;
}

/* 向一个合法普通节点提交单播START_AVG。 */
Uint16 CanController_StartUnicast(CanController_Context_t *context,
                                  Uint16 sample_node_id)
{
    if (sample_node_id >= CAN_CONTROLLER_BROADCAST_ID)
    {
        return CAN_CONTROLLER_STATUS_INVALID_ID;
    }

    return CanController_SendStart(context, sample_node_id);
}

/* 确认中断源为MBOX16后，校验并保存本次AVG_DONE。 */
Uint16 CanController_HandleRxInterrupt(CanController_Context_t *context)
{
    union CANMSGCTRL_REG message_control;
    union CANMDL_REG data_low;
    Uint16 can_id;
    Uint16 src_id;

    if ((ECanaRegs.CANRMP.all & CAN_CONTROLLER_RX_MASK) == 0UL)
    {
        return 0U;
    }

    can_id = ECanaMboxes.MBOX16.MSGID.bit.STDMSGID;
    message_control.all = ECanaMboxes.MBOX16.MSGCTRL.all;
    data_low.all = ECanaMboxes.MBOX16.MDL.all;
    /* RMP16写1清零，同时解除MBOX16对GMIF0的中断请求。 */
    ECanaRegs.CANRMP.all = CAN_CONTROLLER_RX_MASK;

    src_id = (can_id >> 4U) & 0x000FU;

    if ((message_control.bit.RTR != 0U) ||
        (src_id != context->last_request_dst) ||
        ((data_low.byte.BYTE0 & 0x00FFU) != context->last_request_seq))
    {
        return 0U;
    }

    context->reply_status = data_low.byte.BYTE1 & 0x00FFU;
    context->reply_received = 1U;

    return 1U;
}

/* 供APP轮询本次请求是否收到AVG_DONE。 */
Uint16 CanController_GetReply(CanController_Context_t *context,
                              Uint16 *reply_status)
{
    if (context->reply_received == 0U)
    {
        return 0U;
    }

    *reply_status = context->reply_status;
    return 1U;
}

/* 按Type、SrcID和DstID生成11-bit标准帧ID。 */
static Uint16 CanController_BuildId(Uint16 type, Uint16 src_id, Uint16 dst_id)
{
    return (Uint16)(((type & 0x0007U) << 8U) |
                    ((src_id & 0x000FU) << 4U) |
                    (dst_id & 0x000FU));
}

/* 将GPIO30/31配置为CANRXA和CANTXA。 */
static void CanController_InitGpio(void)
{
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0U;
    GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0U;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3U;
    GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1U; /* CANRXA */
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1U; /* CANTXA */
    EDIS;
}

/* 配置eCAN-A为1 Mbps并完成配置模式切换。 */
static Uint16 CanController_InitCan(void)
{
    struct ECAN_REGS can_shadow;
    Uint16 timeout_count = CAN_CONTROLLER_CONFIG_TIMEOUT;

    EALLOW;
    can_shadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
    can_shadow.CANTIOC.bit.TXFUNC = 1U;
    ECanaRegs.CANTIOC.all = can_shadow.CANTIOC.all;
    can_shadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
    can_shadow.CANRIOC.bit.RXFUNC = 1U;
    ECanaRegs.CANRIOC.all = can_shadow.CANRIOC.all;

    ECanaRegs.CANME.all = 0UL;
    ECanaRegs.CANMIM.all = 0UL;
    ECanaRegs.CANGIM.all = 0UL;

    can_shadow.CANMC.all = ECanaRegs.CANMC.all;
    can_shadow.CANMC.bit.SCB = 1U;
    can_shadow.CANMC.bit.CCR = 1U;
    ECanaRegs.CANMC.all = can_shadow.CANMC.all;

    while ((ECanaRegs.CANES.bit.CCE == 0U) && (timeout_count > 0U))
    {
        timeout_count--;
    }
    if (timeout_count == 0U)
    {
        EDIS;
        return CAN_CONTROLLER_STATUS_TIMEOUT;
    }

    /* 与采样板保持一致：150MHz SYSCLK、1Mbps、15TQ。 */
    can_shadow.CANBTC.all = 0UL;
    can_shadow.CANBTC.bit.BRPREG = 4U;
    can_shadow.CANBTC.bit.TSEG1REG = 10U;
    can_shadow.CANBTC.bit.TSEG2REG = 2U;
    can_shadow.CANBTC.bit.SAM = 1U;
    ECanaRegs.CANBTC.all = can_shadow.CANBTC.all;

    can_shadow.CANMC.all = ECanaRegs.CANMC.all;
    can_shadow.CANMC.bit.CCR = 0U;
    ECanaRegs.CANMC.all = can_shadow.CANMC.all;
    timeout_count = CAN_CONTROLLER_CONFIG_TIMEOUT;
    while ((ECanaRegs.CANES.bit.CCE != 0U) && (timeout_count > 0U))
    {
        timeout_count--;
    }
    EDIS;

    if (timeout_count == 0U)
    {
        return CAN_CONTROLLER_STATUS_TIMEOUT;
    }
    return CAN_CONTROLLER_STATUS_SUCCESS;
}

/* 配置START_AVG发送邮箱和AVG_DONE接收邮箱。 */
static void CanController_InitMailboxes(void)
{
    struct ECAN_REGS can_shadow;
    Uint16 receive_id = CanController_BuildId(CAN_CONTROLLER_TYPE_AVG_DONE,
                                              0U,
                                              CAN_CONTROLLER_NODE_ID);

    EALLOW;
    ECanaRegs.CANME.all = 0UL;
    ECanaMboxes.MBOX0.MSGCTRL.all = 0UL;
    ECanaMboxes.MBOX16.MSGCTRL.all = 0UL;

    ECanaMboxes.MBOX16.MSGID.all = 0UL;
    ECanaMboxes.MBOX16.MSGID.bit.STDMSGID = receive_id;
    ECanaMboxes.MBOX16.MSGID.bit.AME = 1U;
    ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 2U;
    ECanaLAMRegs.LAM16.all = CAN_CONTROLLER_SOURCE_ID_MASK |
                            CAN_CONTROLLER_LAM_IDE_COMPARE;

    can_shadow.CANMD.all = CAN_CONTROLLER_RX_MASK;
    ECanaRegs.CANMD.all = can_shadow.CANMD.all;
    ECanaRegs.CANTA.all = 0xFFFFFFFFUL;
    ECanaRegs.CANRMP.all = 0xFFFFFFFFUL;
    ECanaRegs.CANGIF0.all = 0xFFFFFFFFUL;

    can_shadow.CANMIL.all = 0UL;
    ECanaRegs.CANMIL.all = can_shadow.CANMIL.all;
    can_shadow.CANMIM.all = CAN_CONTROLLER_RX_MASK;
    ECanaRegs.CANMIM.all = can_shadow.CANMIM.all;
    can_shadow.CANGIM.all = 0UL;
    can_shadow.CANGIM.bit.I0EN = 1U;
    ECanaRegs.CANGIM.all = can_shadow.CANGIM.all;

    can_shadow.CANME.all = CAN_CONTROLLER_TX_MASK |
                           CAN_CONTROLLER_RX_MASK;
    ECanaRegs.CANME.all = can_shadow.CANME.all;
    EDIS;
}

/* 阻塞发送START_AVG，等待邮箱空闲和发送完成均带超时。 */
static Uint16 CanController_SendStart(CanController_Context_t *context,
                                      Uint16 dst_id)
{
    struct ECAN_REGS can_shadow;
    Uint32 timeout_count;
    Uint16 can_id;
    Uint16 request_seq;

    timeout_count = CAN_CONTROLLER_TX_TIMEOUT;
    while (((ECanaRegs.CANTRS.all & CAN_CONTROLLER_TX_MASK) != 0UL) &&
           (timeout_count > 0UL))
    {
        timeout_count--;
    }
    if (timeout_count == 0UL)
    {
        return CAN_CONTROLLER_STATUS_TIMEOUT;
    }

    request_seq = context->next_seq & 0x00FFU;
    can_id = CanController_BuildId(CAN_CONTROLLER_TYPE_START_AVG,
                                   CAN_CONTROLLER_NODE_ID,
                                   dst_id);

    can_shadow.CANME.all = ECanaRegs.CANME.all;
    can_shadow.CANME.all &= ~CAN_CONTROLLER_TX_MASK;
    ECanaRegs.CANME.all = can_shadow.CANME.all;

    ECanaMboxes.MBOX0.MSGID.all = 0UL;
    ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = can_id;
    ECanaMboxes.MBOX0.MSGCTRL.all = 0UL;
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 1U;
    ECanaMboxes.MBOX0.MDL.all = 0UL;
    ECanaMboxes.MBOX0.MDH.all = 0UL;
    ECanaMboxes.MBOX0.MDL.byte.BYTE0 = request_seq;

    can_shadow.CANME.all |= CAN_CONTROLLER_TX_MASK;
    ECanaRegs.CANME.all = can_shadow.CANME.all;
    ECanaRegs.CANTA.all = CAN_CONTROLLER_TX_MASK;
    ECanaRegs.CANTRS.all = CAN_CONTROLLER_TX_MASK;

    context->last_request_seq = request_seq;
    context->last_request_dst = dst_id;
    context->reply_received = 0U;
    context->reply_status = 0U;
    context->next_seq = (request_seq + 1U) & 0x00FFU;

    timeout_count = CAN_CONTROLLER_TX_TIMEOUT;
    while (((ECanaRegs.CANTA.all & CAN_CONTROLLER_TX_MASK) == 0UL) &&
           (timeout_count > 0UL))
    {
        timeout_count--;
    }
    if (timeout_count == 0UL)
    {
        return CAN_CONTROLLER_STATUS_TIMEOUT;
    }

    ECanaRegs.CANTA.all = CAN_CONTROLLER_TX_MASK;
    return CAN_CONTROLLER_STATUS_SUCCESS;
}
