#ifndef _METER_CAN_PROTOCOL_H_
#define _METER_CAN_PROTOCOL_H_


#include "TYPEDEF.h"

#define CAN_ID_TO_CTRL				0x10010031

typedef struct
{
    Uint16 MeterStatus;
    Uint16 MeterItemNum;
    Uint16 MeterTrigDone;
    Uint16 MeterSendEn;
}METER_DATA;
extern METER_DATA MeterData;

extern void InitECanbUPHMailbox(void);
extern void RecvCmdFormCtrlBoard(void);
extern void SendMsgToCtrlBoard(void);
#endif
