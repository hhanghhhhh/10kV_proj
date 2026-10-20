#ifndef _TASK_CAN_WITH_METER_H_
#define _TASK_CAN_WITH_METER_H_

#include "TYPEDEF.h"

#define SOURCE_TO_METER_TRIGGER    0x11
#define SOURCE_TO_METER_SETPARA    0x22

#define METER_TO_SOURCE_RST_OK    0x22

#define CAN_ID_TO_METER				0x13490032

typedef struct
{
    union
    {
        Uint16 u8;
        struct
        {
            Uint16 source_id:4;
            Uint16 meter1_en:1;
            Uint16 meter2_en:1;
            Uint16 meter3_en:1;
            Uint16 meter4_en:1;
            Uint16 reserve:8;
        }bit;
    }byte0;
    union
    {
        Uint16 u8;
        struct
        {
            Uint16 data:8;
            Uint16 reserve:8;
        }bit;
    }byte1;
}ST_SOURCE_TO_METER;


typedef struct
{
    union
    {
        Uint16 u8;
        struct
        {
            Uint16 source_id:4;
            Uint16 resv:4;
            Uint16 reserve:8;
        }bit;
    }byte0;
    union
    {
        Uint16 u8;
        struct
        {
            Uint16 rst:8;
            Uint16 reserve:8;
        }bit;
    }byte1;
}ST_METER_TO_SOURCE;

//接口
//input
//output
extern void TriggerMeter(void);                 //发送trigger使能
extern void SetMeterPara(void);                 //发送set para 命令
extern Uint16 revc_meter_flag;                  //read, 采样结果ok
extern Uint16 revc_meter_rst;
extern void InitECanbMailbox_HvSource(void);    //初始化CAN邮箱
extern void SendMsgToMeter(void);               //发送CAN报文，main函数运行
extern void RecvMsgFromMeter(void);             //CAN接收中断中调用

#endif
