#include "drv_can_sample.h"
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define CAN_SAMPLE_TYPE_START_AVG       (0x01U)
#define CAN_SAMPLE_TYPE_AVG_DONE        (0x02U)
#define CAN_SAMPLE_RX_LOCAL_MAILBOX     (16U)
#define CAN_SAMPLE_RX_BROADCAST_MAILBOX (17U)
#define CAN_SAMPLE_TX_MAILBOX           (0U)
#define CAN_SAMPLE_RX_LOCAL_MASK        (1UL << CAN_SAMPLE_RX_LOCAL_MAILBOX)
#define CAN_SAMPLE_RX_BROADCAST_MASK    (1UL << CAN_SAMPLE_RX_BROADCAST_MAILBOX)
#define CAN_SAMPLE_TX_MASK              (1UL << CAN_SAMPLE_TX_MAILBOX)
#define CAN_SAMPLE_SOURCE_ID_MASK       (0x0F0UL << 18U)
#define CAN_SAMPLE_LAM_IDE_COMPARE      (0x80000000UL)
#define CAN_SAMPLE_CONFIG_TIMEOUT       (65535U)
#define CAN_SAMPLE_TX_TIMEOUT           (1000000UL)

static Uint16 CanSample_BuildId(Uint16 type, Uint16 src_id, Uint16 dst_id);
static void CanSample_InitGpio(void);
static Uint16 CanSample_InitController(void);
static void CanSample_InitMailboxes(void);
static Uint16 CanSample_ProcessMailbox(CanSample_Context_t *context,
                                       volatile struct MBOX *mailbox,
                                       Uint32 mailbox_mask);

/* 初始化模块状态及采样板CAN通信资源。 */
Uint16 CanSample_Init(CanSample_Context_t *context)
{
    context->request_src = 0U;
    context->request_seq = 0U;
    context->request_valid = 0U;
    context->done_request_src = 0U;
    context->done_request_seq = 0U;
    context->done_pending = 0U;
    context->start_count = 0UL;
    context->invalid_count = 0UL;
    context->done_overrun_count = 0UL;

    CanSample_InitGpio();
    if (CanSample_InitController() != CAN_SAMPLE_STATUS_SUCCESS)
    {
        return CAN_SAMPLE_STATUS_TIMEOUT;
    }

    CanSample_InitMailboxes();
    return CAN_SAMPLE_STATUS_SUCCESS;
}

/* 按MIV0处理本次CAN-A中断指向的接收邮箱。 */
Uint16 CanSample_HandleRxInterrupt(CanSample_Context_t *context)
{
    union CANGIF0_REG interrupt_flags;
    Uint32 pending_mailboxes;

    interrupt_flags.all = ECanaRegs.CANGIF0.all;
    if (interrupt_flags.bit.GMIF0 == 0U)
    {
        return 0U;
    }

    pending_mailboxes = ECanaRegs.CANRMP.all;
    if ((interrupt_flags.bit.MIV0 == CAN_SAMPLE_RX_LOCAL_MAILBOX) &&
        ((pending_mailboxes & CAN_SAMPLE_RX_LOCAL_MASK) != 0UL))
    {
        return CanSample_ProcessMailbox(
            context,
            &ECanaMboxes.MBOX16,
            CAN_SAMPLE_RX_LOCAL_MASK);
    }

    if ((interrupt_flags.bit.MIV0 == CAN_SAMPLE_RX_BROADCAST_MAILBOX) &&
        ((pending_mailboxes & CAN_SAMPLE_RX_BROADCAST_MASK) != 0UL))
    {
        return CanSample_ProcessMailbox(
            context,
            &ECanaMboxes.MBOX17,
            CAN_SAMPLE_RX_BROADCAST_MASK);
    }

    return 0U;
}

/* 将刚完成测量对应的请求信息保存到独立完成快照。 */
void CanSample_LatchCompletedRequest(CanSample_Context_t *context)
{
    if (context->request_valid == 0U)
    {
        return;
    }

    if (context->done_pending == 0U)
    {
        context->done_request_src = context->request_src;
        context->done_request_seq = context->request_seq;
        context->done_pending = 1U;
    }
    else
    {
        /* 单槽快照优先保护尚未发送的上一轮完成通知。 */
        context->done_overrun_count++;
    }

    context->request_valid = 0U;
}

/* 阻塞发送AVG_DONE，等待邮箱空闲和发送完成均带超时。 */
Uint16 CanSample_SendCompleted(CanSample_Context_t *context, Uint16 status)
{
    struct ECAN_REGS can_shadow;
    Uint32 timeout_count;
    Uint16 can_id;

    if (context->done_pending == 0U)
    {
        return CAN_SAMPLE_STATUS_NO_REQUEST;
    }

    timeout_count = CAN_SAMPLE_TX_TIMEOUT;
    while (((ECanaRegs.CANTRS.all & CAN_SAMPLE_TX_MASK) != 0UL) &&
           (timeout_count > 0UL))
    {
        timeout_count--;
    }
    if (timeout_count == 0UL)
    {
        return CAN_SAMPLE_STATUS_TIMEOUT;
    }

    can_id = CanSample_BuildId(CAN_SAMPLE_TYPE_AVG_DONE,
                               CAN_SAMPLE_NODE_ID,
                               context->done_request_src);

    /* 修改MSGID前暂时关闭TX邮箱，其他RX邮箱保持使能。 */
    can_shadow.CANME.all = ECanaRegs.CANME.all;
    can_shadow.CANME.all &= ~CAN_SAMPLE_TX_MASK;
    ECanaRegs.CANME.all = can_shadow.CANME.all;

    ECanaMboxes.MBOX0.MSGID.all = 0UL;
    ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = can_id;
    ECanaMboxes.MBOX0.MSGCTRL.all = 0UL;
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 2U;
    ECanaMboxes.MBOX0.MDL.all = 0UL;
    ECanaMboxes.MBOX0.MDH.all = 0UL;
    ECanaMboxes.MBOX0.MDL.byte.BYTE0 = context->done_request_seq & 0x00FFU;
    ECanaMboxes.MBOX0.MDL.byte.BYTE1 = status & 0x00FFU;

    can_shadow.CANME.all |= CAN_SAMPLE_TX_MASK;
    ECanaRegs.CANME.all = can_shadow.CANME.all;
    ECanaRegs.CANTA.all = CAN_SAMPLE_TX_MASK;
    ECanaRegs.CANTRS.all = CAN_SAMPLE_TX_MASK;

    timeout_count = CAN_SAMPLE_TX_TIMEOUT;
    while (((ECanaRegs.CANTA.all & CAN_SAMPLE_TX_MASK) == 0UL) &&
           (timeout_count > 0UL))
    {
        timeout_count--;
    }
    if (timeout_count == 0UL)
    {
        return CAN_SAMPLE_STATUS_TIMEOUT;
    }

    /* CANTA写1清零，发送确认后释放完成快照。 */
    ECanaRegs.CANTA.all = CAN_SAMPLE_TX_MASK;
    context->done_pending = 0U;
    return CAN_SAMPLE_STATUS_SUCCESS;
}

/* 按Type、SrcID和DstID生成11-bit标准帧ID。 */
static Uint16 CanSample_BuildId(Uint16 type, Uint16 src_id, Uint16 dst_id)
{
    return (Uint16)(((type & 0x0007U) << 8U) |
                    ((src_id & 0x000FU) << 4U) |
                    (dst_id & 0x000FU));
}

/* 将GPIO30/31配置为CANRXA和CANTXA。 */
static void CanSample_InitGpio(void)
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
static Uint16 CanSample_InitController(void)
{
    struct ECAN_REGS can_shadow;
    Uint16 timeout_count = CAN_SAMPLE_CONFIG_TIMEOUT;

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
        return CAN_SAMPLE_STATUS_TIMEOUT;
    }

    /* 150MHz SYSCLK下配置1Mbps：15TQ，采样点约80%。 */
    can_shadow.CANBTC.all = 0UL;
    can_shadow.CANBTC.bit.BRPREG = 4U;
    can_shadow.CANBTC.bit.TSEG1REG = 10U;
    can_shadow.CANBTC.bit.TSEG2REG = 2U;
    can_shadow.CANBTC.bit.SAM = 1U;
    ECanaRegs.CANBTC.all = can_shadow.CANBTC.all;

    can_shadow.CANMC.all = ECanaRegs.CANMC.all;
    can_shadow.CANMC.bit.CCR = 0U;
    ECanaRegs.CANMC.all = can_shadow.CANMC.all;
    timeout_count = CAN_SAMPLE_CONFIG_TIMEOUT;
    while ((ECanaRegs.CANES.bit.CCE != 0U) && (timeout_count > 0U))
    {
        timeout_count--;
    }
    EDIS;

    if (timeout_count == 0U)
    {
        return CAN_SAMPLE_STATUS_TIMEOUT;
    }
    return CAN_SAMPLE_STATUS_SUCCESS;
}

/* 配置AVG_DONE发送邮箱及START_AVG单播、广播接收邮箱。 */
static void CanSample_InitMailboxes(void)
{
    struct ECAN_REGS can_shadow;
    Uint16 local_id = CanSample_BuildId(CAN_SAMPLE_TYPE_START_AVG,
                                        0U,
                                        CAN_SAMPLE_NODE_ID);
    Uint16 broadcast_id = CanSample_BuildId(CAN_SAMPLE_TYPE_START_AVG,
                                            0U,
                                            CAN_SAMPLE_BROADCAST_ID);

    EALLOW;
    ECanaRegs.CANME.all = 0UL;

    ECanaMboxes.MBOX0.MSGCTRL.all = 0UL;
    ECanaMboxes.MBOX16.MSGCTRL.all = 0UL;
    ECanaMboxes.MBOX17.MSGCTRL.all = 0UL;

    ECanaMboxes.MBOX16.MSGID.all = 0UL;
    ECanaMboxes.MBOX16.MSGID.bit.STDMSGID = local_id;
    ECanaMboxes.MBOX16.MSGID.bit.AME = 1U;
    ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 1U;

    ECanaMboxes.MBOX17.MSGID.all = 0UL;
    ECanaMboxes.MBOX17.MSGID.bit.STDMSGID = broadcast_id;
    ECanaMboxes.MBOX17.MSGID.bit.AME = 1U;
    ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 1U;

    /* 仅忽略ID中的SrcID，Type和DstID必须匹配。 */
    ECanaLAMRegs.LAM16.all = CAN_SAMPLE_SOURCE_ID_MASK |
                            CAN_SAMPLE_LAM_IDE_COMPARE;
    ECanaLAMRegs.LAM17.all = CAN_SAMPLE_SOURCE_ID_MASK |
                            CAN_SAMPLE_LAM_IDE_COMPARE;

    can_shadow.CANMD.all = 0UL;
    can_shadow.CANMD.all = CAN_SAMPLE_RX_LOCAL_MASK |
                           CAN_SAMPLE_RX_BROADCAST_MASK;
    ECanaRegs.CANMD.all = can_shadow.CANMD.all;

    ECanaRegs.CANTA.all = 0xFFFFFFFFUL;
    ECanaRegs.CANRMP.all = 0xFFFFFFFFUL;
    ECanaRegs.CANGIF0.all = 0xFFFFFFFFUL;

    can_shadow.CANMIL.all = 0UL;
    ECanaRegs.CANMIL.all = can_shadow.CANMIL.all;
    can_shadow.CANMIM.all = CAN_SAMPLE_RX_LOCAL_MASK |
                            CAN_SAMPLE_RX_BROADCAST_MASK;
    ECanaRegs.CANMIM.all = can_shadow.CANMIM.all;
    can_shadow.CANGIM.all = 0UL;
    can_shadow.CANGIM.bit.I0EN = 1U;
    ECanaRegs.CANGIM.all = can_shadow.CANGIM.all;

    can_shadow.CANME.all = CAN_SAMPLE_TX_MASK |
                           CAN_SAMPLE_RX_LOCAL_MASK |
                           CAN_SAMPLE_RX_BROADCAST_MASK;
    ECanaRegs.CANME.all = can_shadow.CANME.all;
    EDIS;
}

/* 读取并校验一个START_AVG邮箱，保存有效请求信息。 */
static Uint16 CanSample_ProcessMailbox(CanSample_Context_t *context,
                                       volatile struct MBOX *mailbox,
                                       Uint32 mailbox_mask)
{
    union CANMSGID_REG message_id;
    union CANMSGCTRL_REG message_control;
    union CANMDL_REG data_low;
    Uint16 can_id;
    Uint16 type;
    Uint16 src_id;
    Uint16 dst_id;

    message_id.all = mailbox->MSGID.all;
    message_control.all = mailbox->MSGCTRL.all;
    data_low.all = mailbox->MDL.all;
    /* RMPn写1清零，同时解除该邮箱对GMIF0的中断请求。 */
    ECanaRegs.CANRMP.all = mailbox_mask;

    can_id = message_id.bit.STDMSGID;
    type = (can_id >> 8U) & 0x0007U;
    src_id = (can_id >> 4U) & 0x000FU;
    dst_id = can_id & 0x000FU;

    if ((message_id.bit.IDE != 0U) ||
        (message_control.bit.RTR != 0U) ||
        (message_control.bit.DLC != 1U) ||
        (type != CAN_SAMPLE_TYPE_START_AVG) ||
        (src_id == CAN_SAMPLE_BROADCAST_ID) ||
        ((dst_id != CAN_SAMPLE_NODE_ID) &&
         (dst_id != CAN_SAMPLE_BROADCAST_ID)))
    {
        context->invalid_count++;
        return 0U;
    }

    /* 新的有效请求直接覆盖尚未完成的旧测量请求。 */
    context->request_src = src_id;
    context->request_seq = data_low.byte.BYTE0 & 0x00FFU;
    context->request_valid = 1U;
    context->start_count++;
    return 1U;
}
