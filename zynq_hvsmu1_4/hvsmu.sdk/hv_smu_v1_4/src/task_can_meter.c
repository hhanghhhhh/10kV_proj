#include <task_can_meter.h>
#include <task_uph_common.h>
#include "COMMON.h"

ST_SOURCE_TO_METER st_source_to_meter;
void MeterAnswerCtrlBoard(void);
void WaitMeterDataOk(void);

void InitECanbMailbox_HvSource(void)
{

}


ST_METER_TO_SOURCE st_meter_to_source[4];
Uint16 msg_from_meter_flag[4] = {0, 0, 0, 0};
//中断中调用
//接收表的信息 byte0 代表要发给的源的地址， byte1 代表表的采样结果
void RecvMsgFromMeter(void)
{
	switch(can_msg_recv.id)
	{
	case 0x115C0024:
		msg_from_meter_flag[0] = 1;
		st_meter_to_source[0].byte0.u8 = can_msg_recv.data[0];
		st_meter_to_source[0].byte1.u8 = can_msg_recv.data[1];
		break;
	default:
		break;
	}

    WaitMeterDataOk();
}

Uint16 send_to_meter_flag = 0;
Uint16 send_to_meter_data = 0;
Uint16 revc_meter_flag = 0;
Uint16 revc_meter_rst = 0;
void TriggerMeter(void)
{
    send_to_meter_flag = 1;
    send_to_meter_data = SOURCE_TO_METER_TRIGGER;
    revc_meter_flag = 0;
    revc_meter_rst = 0;
}

void SetMeterPara(void)
{
    send_to_meter_flag = 1;
    send_to_meter_data = SOURCE_TO_METER_SETPARA;
}


//向表发送开始采样信号，byte0 存放自身源序号，byte1 存放类型
//main函数运行
void SendMsgToMeter(void)
{
    ST_SOURCE_TO_METER *p_to_meter = &st_source_to_meter;

    if(send_to_meter_flag == 0)
    {
        return;
    }

    send_to_meter_flag = 0;

    memset(p_to_meter, 0, sizeof(ST_SOURCE_TO_METER));

    //发送数据格式
    p_to_meter->byte0.bit.source_id = DEVICE_ID;
    p_to_meter->byte0.bit.meter1_en = 1;
    p_to_meter->byte0.bit.meter2_en = 0;
    p_to_meter->byte0.bit.meter3_en = 0;
    p_to_meter->byte0.bit.meter4_en = 0;
    p_to_meter->byte1.bit.data = send_to_meter_data;

    can_msg_send.data[0] = p_to_meter->byte0.u8;
    can_msg_send.data[1] = p_to_meter->byte1.u8;
    can_msg_send.len = 2;
    can_msg_send.id = CAN_ID_TO_METER;
    CANSendMsg();
}

void WaitMeterDataOk(void)
{
    //目前仅判断第一个接收邮箱
    if(msg_from_meter_flag[0] == 1)
    {
        msg_from_meter_flag[0] = 0;

        if(st_meter_to_source[0].byte0.bit.source_id == DEVICE_ID)
        {
            if(st_meter_to_source[0].byte1.bit.rst == 0x22)
            {
                // set para 回复的报文
                // 暂时不判断是否回复报文
            }
            else
            {
                // sample 回复的报文
                revc_meter_flag = 1;
                revc_meter_rst = st_meter_to_source[0].byte1.bit.rst;
            }
        }
    }
}
