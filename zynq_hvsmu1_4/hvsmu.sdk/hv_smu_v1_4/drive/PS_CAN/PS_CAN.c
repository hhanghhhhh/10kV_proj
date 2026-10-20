
/***************************** Include Files *********************************/
#include "xcanps.h"
#include "COMMON.h"
#include "drv_can_controller.h"
#include "xtime_l.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

static void Config(XCanPs *InstancePtr);
static void SendHandler(void *CallBackRef);
static void RecvHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 ErrorMask);
static void EventHandler(void *CallBackRef, u32 Mask);
static void PS_CAN1_Intr_Init(XCanPs *CanInstancePtr,u16 CanIntrId);

/************************** Variable Definitions *****************************/
XCanPs CanInstance;    /* Instance of the Can driver */
static u32 TxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];
static u32 RxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];
CAN_MSG	can_msg_send, can_msg_recv;

/* 把 29-bit ID 转为 PS CAN 的 ID 寄存器格式 */
static u32 MakeIdReg_29bit(u32 id29)
{
    u32 id1 = (id29 >> 18) & 0x7FF;     // 高 11 bit
    u32 id2 = id29 & 0x3FFFF;          // 低 18 bit

    /* IDE = 1（扩展帧），SRR=1，RTR=0 */
    return XCanPs_CreateIdValue(id1, 1, 1, id2, 0);
}
/* 把 PS CAN 的 ID 寄存器格式  转为 29-bit ID  */
static u32 Make29bit_IdReg(u32 idreg)
{
	u32 can_id = 0;

    // 判断是否扩展帧（IDE 位）
    if (idreg & XCANPS_IDR_IDE_MASK)
    {
        /* 扩展帧（29-bit ID）    */
    	can_id = ((idreg & XCANPS_IDR_ID1_MASK) >> XCANPS_IDR_ID1_SHIFT);
    	can_id = (can_id << 18) | ((idreg & XCANPS_IDR_ID2_MASK) >> XCANPS_IDR_ID2_SHIFT);
    }
    else
    {
        /* 标准帧（11-bit ID）*/
    	can_id = (idreg & XCANPS_IDR_ID1_MASK) >> XCANPS_IDR_ID1_SHIFT;
    }

    return can_id;
}
/* 把 29-bit mask 转为寄存器格式（IDE 也进行比较） */
static u32 MakeMaskReg_29bit(u32 mask29)
{
    u32 id1 = (mask29 >> 18) & 0x7FF;
    u32 id2 = mask29 & 0x3FFFF;

    u32 reg = 0;

    reg |= (id1 << XCANPS_IDR_ID1_SHIFT) & XCANPS_IDR_ID1_MASK;
    reg |= XCANPS_IDR_IDE_MASK;   /* 让硬件比较 IDE=1 */
    reg |= (id2 << XCANPS_IDR_ID2_SHIFT) & XCANPS_IDR_ID2_MASK;

    return reg;
}

void PS_CAN1_Init(void)
{
	XCanPs *CanInstPtr = &CanInstance;
	XCanPs_Config *ConfigPtr;

	ConfigPtr = XCanPs_LookupConfig(CAN_DEVICE_ID);
	XCanPs_CfgInitialize(CanInstPtr, ConfigPtr,	ConfigPtr->BaseAddr);

	/* Configure CAN device. */
	Config(CanInstPtr);

	/* Connect to the interrupt controller.	*/
	PS_CAN1_Intr_Init(CanInstPtr, CAN_INTR_VEC_ID);

	// exit config mode
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_NORMAL);
	while(XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_NORMAL);
}

static void PS_CAN1_Intr_Init(XCanPs *CanInstancePtr,u16 CanIntrId)
{
	Set_ScuGic_Link(CanIntrId, 0xA5, HIGH_Level_Sensitive,
			(Xil_InterruptHandler)XCanPs_IntrHandler, (void *)CanInstancePtr);

	/* Set interrupt handlers. */
	XCanPs_SetHandler(CanInstancePtr, XCANPS_HANDLER_SEND,
			(void *)SendHandler, (void *)CanInstancePtr);
	XCanPs_SetHandler(CanInstancePtr, XCANPS_HANDLER_RECV,
			(void *)RecvHandler, (void *)CanInstancePtr);
	XCanPs_SetHandler(CanInstancePtr, XCANPS_HANDLER_ERROR,
			(void *)ErrorHandler, (void *)CanInstancePtr);
	XCanPs_SetHandler(CanInstancePtr, XCANPS_HANDLER_EVENT,
			(void *)EventHandler, (void *)CanInstancePtr);

	/* Enable all interrupts in CAN device.	*/
	XCanPs_IntrEnable(CanInstancePtr, XCANPS_IXR_ALL);
}

void CANSendMsg(void)
{
	u8 *FramePtr;
	int Index;
	XCanPs *CanInstPtr = &CanInstance;

    TxFrame[0] = MakeIdReg_29bit(can_msg_send.id);
    TxFrame[1] = (u32)XCanPs_CreateDlcValue((u32)can_msg_send.len);
	FramePtr = (u8 *)(&TxFrame[2]);
	for (Index = 0; Index < can_msg_send.len; Index++) {
		*FramePtr++ = can_msg_send.data[Index];
	}

	/* wait until the TX FIFO is not full. */
	while (XCanPs_IsTxFifoFull(CanInstPtr) == TRUE);

	XCanPs_Send(CanInstPtr, TxFrame);
}

/* 新协议只在main发送；高优先级缓冲区与旧协议的TxFrame/FIFO分离。 */
Uint16 PS_CAN_SendStandard(Uint16 can_id, Uint16 seq)
{
    u32 frame[XCANPS_MAX_FRAME_SIZE_IN_WORDS] = {0U};
    XTime start;
    XTime now;
    XTime timeout_ticks =
        ((XTime)COUNTS_PER_SECOND * CAN_CONTROLLER_TX_TIMEOUT_MS) / 1000U;

    XTime_GetTime(&start);
    while (XCanPs_IsHighPriorityBufFull(&CanInstance) == TRUE)
    {
        XTime_GetTime(&now);
        if ((now - start) >= timeout_ticks)
        {
            return CAN_CONTROLLER_STATUS_TIMEOUT;
        }
    }

    frame[0] = XCanPs_CreateIdValue(can_id, 0U, 0U, 0U, 0U);
    frame[1] = XCanPs_CreateDlcValue(1U);
    ((u8 *)&frame[2])[0] = (u8)seq;
    if (XCanPs_SendHighPriority(&CanInstance, frame) != XST_SUCCESS)
    {
        return CAN_CONTROLLER_STATUS_TIMEOUT;
    }
    XTime_GetTime(&start);
    while (XCanPs_IsHighPriorityBufFull(&CanInstance) == TRUE)
    {
        XTime_GetTime(&now);
        if ((now - start) >= timeout_ticks)
        {
            return CAN_CONTROLLER_STATUS_TIMEOUT;
        }
    }
    return CAN_CONTROLLER_STATUS_SUCCESS;
}

static void Config(XCanPs *InstancePtr)
{
    u32 afr_reg  = MakeIdReg_29bit(AFR_29);
    u32 mask_reg = MakeMaskReg_29bit(AFMR_29);

    u32 standard_id = XCanPs_CreateIdValue(
        CAN_CONTROLLER_REPLY_ID, 0U, 0U, 0U, 0U);
    u32 standard_mask =
        ((u32)CAN_CONTROLLER_REPLY_ID_MASK << XCANPS_IDR_ID1_SHIFT) |
        XCANPS_IDR_IDE_MASK;

	/* Enter Configuration Mode	*/
	XCanPs_EnterMode(InstancePtr, XCANPS_MODE_CONFIG);
	while(XCanPs_GetMode(InstancePtr) != XCANPS_MODE_CONFIG);

	/* 波特率 1M */
	XCanPs_SetBaudRatePrescaler(InstancePtr, TEST_BRPR_BAUD_PRESCALAR);
	XCanPs_SetBitTiming(InstancePtr, TEST_BTR_SYNCJUMPWIDTH,
					TEST_BTR_SECOND_TIMESEGMENT,
					TEST_BTR_FIRST_TIMESEGMENT);

    /* 写入 AFR 和 MASK */
	XCanPs_AcceptFilterSet(InstancePtr, XCANPS_AFR_UAF1_MASK, mask_reg, afr_reg);

    /* 启用滤波 */
    /* 过滤器1保留旧扩展帧，过滤器2接收新标准帧AVG_DONE。 */
    XCanPs_AcceptFilterSet(InstancePtr, XCANPS_AFR_UAF2_MASK,
                         standard_mask, standard_id);
	XCanPs_AcceptFilterEnable(InstancePtr,
        XCANPS_AFR_UAF1_MASK | XCANPS_AFR_UAF2_MASK);
}


static void RecvHandler(void *CallBackRef)
{
	XCanPs *CanPtr = (XCanPs *)CallBackRef;
	int Index;
	u8 *FramePtr;

    if (XCanPs_Recv(CanPtr, RxFrame) != XST_SUCCESS)
    {
        return;
    }
    /* 标准帧直接交给新协议；旧扩展帧继续走原解析函数。 */
    if ((RxFrame[0] & XCANPS_IDR_IDE_MASK) == 0U)
    {
        u32 dlc = (RxFrame[1] & XCANPS_DLCR_DLC_MASK) >>
                  XCANPS_DLCR_DLC_SHIFT;
        if (((RxFrame[0] & (XCANPS_IDR_SRR_MASK | XCANPS_IDR_RTR_MASK)) == 0U) &&
            (dlc >= 2U))
        {
            FramePtr = (u8 *)(&RxFrame[2]);
            CanController_HandleReply((Uint16)Make29bit_IdReg(RxFrame[0]),
                                      FramePtr[0], FramePtr[1]);
        }
        return;
    }

    can_msg_recv.id = Make29bit_IdReg(RxFrame[0]);
    can_msg_recv.len = (RxFrame[1] & XCANPS_DLCR_DLC_MASK) >> XCANPS_DLCR_DLC_SHIFT;
	FramePtr = (u8 *)(&RxFrame[2]);
	for (Index = 0; Index < 8; Index++)
	{
		can_msg_recv.data[Index] = FramePtr[Index];
	}

	RecvCmdFormCtrlBoard();
	RecvMsgFromMeter();
}

static void SendHandler(void *CallBackRef)
{
}

static void ErrorHandler(void *CallBackRef, u32 ErrorMask)
{
	if(ErrorMask & XCANPS_ESR_ACKER_MASK) {
	}
	if(ErrorMask & XCANPS_ESR_BERR_MASK) {
	}
	if(ErrorMask & XCANPS_ESR_STER_MASK) {
	}
	if(ErrorMask & XCANPS_ESR_FMER_MASK) {
	}
	if(ErrorMask & XCANPS_ESR_CRCER_MASK) {
	}
}

/*****************************************************************************/
/**
*
* Callback function (called from interrupt handler) to handle the following
* interrupts:
*   - XCANPS_IXR_BSOFF_MASK:	Bus Off Interrupt
*   - XCANPS_IXR_RXOFLW_MASK:	RX FIFO Overflow Interrupt
*   - XCANPS_IXR_RXUFLW_MASK:	RX FIFO Underflow Interrupt
*   - XCANPS_IXR_TXBFLL_MASK:	TX High Priority Buffer Full Interrupt
*   - XCANPS_IXR_TXFLL_MASK:	TX FIFO Full Interrupt
*   - XCANPS_IXR_WKUP_MASK:	Wake up Interrupt
*   - XCANPS_IXR_SLP_MASK:	Sleep Interrupt
*   - XCANPS_IXR_ARBLST_MASK:	Arbitration Lost Interrupt
*
*
* @param	CallBackRef is the callback reference passed from the
*		interrupt Handler, which in our case is a pointer to the
*		driver instance.
* @param	IntrMask is a bit mask indicating pending interrupts.
*		Its value equals 'OR'ing one or more of the XCANPS_IXR_*_MASK
*		value(s) mentioned above.
*
* @return	None.
*
* @note		This function is called by the driver within interrupt context.
* 		This function should be changed to meet specific application
*		needs.
*
******************************************************************************/
static void EventHandler(void *CallBackRef, u32 IntrMask)
{
	XCanPs *CanPtr = (XCanPs *)CallBackRef;

	if (IntrMask & XCANPS_IXR_BSOFF_MASK) {
		/*
		 * Entering Bus off status interrupt requires
		 * the CAN device be reset and reconfigured.
		 */
		XCanPs_Reset(CanPtr);
		Config(CanPtr);
		return;
	}

	if(IntrMask & XCANPS_IXR_RXOFLW_MASK) {
	}
	if(IntrMask & XCANPS_IXR_RXUFLW_MASK) {
	}
	if(IntrMask & XCANPS_IXR_TXBFLL_MASK) {
	}
	if(IntrMask & XCANPS_IXR_TXFLL_MASK) {
	}
	if (IntrMask & XCANPS_IXR_WKUP_MASK) {
	}
	if (IntrMask & XCANPS_IXR_SLP_MASK) {
	}
	if (IntrMask & XCANPS_IXR_ARBLST_MASK) {
	}
}


