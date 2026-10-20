#ifndef __PS_CAN_H__
#define __PS_CAN_H__



/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define CAN_DEVICE_ID		XPAR_XCANPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define CAN_INTR_VEC_ID		XPAR_INTC_0_CANPS_0_VEC_ID
#else
#define INTC		XScuGic
#define CAN_DEVICE_ID		XPAR_XCANPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define CAN_INTR_VEC_ID		XPAR_XCANPS_1_INTR
#endif
/* Maximum CAN frame length in word */
#define XCANPS_MAX_FRAME_SIZE_IN_WORDS (XCANPS_MAX_FRAME_SIZE / sizeof(u32))



/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CAN baud rate equals 40Kbps, assuming that the
 * the CAN clock is 24MHz. The user needs to modify these values based on
 * the desired baud rate and the CAN clock frequency. For more information
 * see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
 */

/*
 * Timing parameters to be set in the Bit Timing Register (BTR).
 * These values are for a 100 Kbps baudrate assuming the CAN input clock
 * frequency is 100 MHz.
 * CAN input clock frequency: 100MHz
 * baudrate: 1MHz
 *
 * 同步窗口
 * SJW <= TSEG2 (TEST_BTR_SYNCJUMPWIDTH = 3)
 *
 * 采样点一般是 80%
 * can SP(sampling point) 80%
 * SP = (1 + TSEG1 + 1) / (1 + TSEG1 + 1 + TSEG2 + 1)
 * TSEG1 = 6
 * TSEG2 = 1
 * SP = 80%
 * 每个 can bit 分为 10TQ:(1 + TSEG1 + 1 + TSEG2 + 1)
 * 以上几个参数不用动即可，不同 can 波特率，改时钟分频即可，控制 TQ 的大小
 * TQ = (BRPR + 1) / CAN_Clock
 * BRPR = 9  -- 1M    BRPR = 19 -- 500k
 * BRPR = 39 -- 250k  BRPR = 99 -- 100k
 */

#define TEST_BTR_SYNCJUMPWIDTH		0
#define TEST_BTR_SECOND_TIMESEGMENT	1
#define TEST_BTR_FIRST_TIMESEGMENT	6
#define TEST_BRPR_BAUD_PRESCALAR	9


/* 29-bit AFR 和 MASK */
#define AFR_29     0x10000020U   /* 1 0 0 0 0 0 2 0 */
#define AFMR_29    0xF000FFF0U   /* F 0 0 0 F F F 0 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


typedef struct
{
	u32 id;
	u16 len;
	u8 data[8];
}CAN_MSG;
extern CAN_MSG	can_msg_send, can_msg_recv;

extern void PS_CAN1_Init(void);
extern void CANSendMsg(void);
/* main专用：发送一个字节的标准帧，限时等待发送缓冲区释放。 */
Uint16 PS_CAN_SendStandard(Uint16 can_id, Uint16 seq);


#endif
