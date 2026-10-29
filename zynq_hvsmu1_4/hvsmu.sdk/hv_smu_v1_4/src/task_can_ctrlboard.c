#include <task_can_ctrlboard.h>
#include <task_uph_common.h>
#include "COMMON.h"

METER_DATA MeterData;
Uint16 CmdFormCtrlBoard = 0, TrigFromCtrlBoard = 0;
void TransMeterStatus(METER_DATA* pMeterData);
void ReadMeterData(METER_DATA* pMeterData);
void MeterAnswerCtrlBoard(void);


void InitECanbUPHMailbox(void)
{

}


Uint32 recv_ctrl_cnt = 0;
//由中断函数调用
void RecvCmdFormCtrlBoard(void)
{
    recv_ctrl_cnt++;

	switch(can_msg_recv.id)
	{
	case 0x1FFC0021:
		CmdFormCtrlBoard = can_msg_recv.data[0];
		break;
	case 0X1FFD0022:
		TrigFromCtrlBoard = can_msg_recv.data[0];
		break;
	default:
		break;
	}

    MeterAnswerCtrlBoard();
}


//根据接收到来自主控板的命令执行不同的操作
void MeterAnswerCtrlBoard(void)
{
    METER_DATA* pMeterData = &MeterData;
	ReadMeterData(pMeterData);				//读取源表数据
	if(CmdFormCtrlBoard != 0)
	{
		switch(CmdFormCtrlBoard)
		{
		case 0x11:							//检查工序命令
			TransMeterStatus(pMeterData);	//向主控板发送源表数据
			break;
		case 0x22:							//开始测试命令
			WriteMeterStart();				//向源表写开始测试指令
			break;
		case 0x33:							//停止测试命令
			WriteMeterStop();				//向源表写停止测试指令
			break;
		default:
			break;
		}
		CmdFormCtrlBoard = 0;
	}

	if(TrigFromCtrlBoard == 0x55)
	{
		WriteMeterTrig();					//接收到trigger信号，则向源表写trgger指令
		TrigFromCtrlBoard = 0;
	}
}

//向主控板发送源表数据，主循环运行
void SendMsgToCtrlBoard(void)
{
    METER_DATA* pMeterData = &MeterData;
    Uint8 Byte1Temp = 0, Byte0Temp = 0;

    if(pMeterData->MeterSendEn == 0)
        return;
    pMeterData->MeterSendEn = 0;

    Byte0Temp = pMeterData->MeterItemNum & 0x00ff;                  //BYTE0存放源表待测工序剩余数量
    Byte1Temp = (pMeterData->MeterStatus & 0x000f)<<4;              //BYTE1高4位存放源表状态
    Byte1Temp = Byte1Temp | (pMeterData->MeterTrigDone & 0x000f);   //BYTE1低4位存放源表trigger完成信号
    can_msg_send.data[0] = Byte0Temp;
    can_msg_send.data[1] = Byte1Temp;
    can_msg_send.len = 2;
    can_msg_send.id = CAN_ID_TO_CTRL;
    CANSendMsg();
}

void TransMeterStatus(METER_DATA* pMeterData)
{
    pMeterData->MeterSendEn = 1;
}

//源表向主控板发送结果，仅执行一次
void SendRstOnce(Uint16 rst)
{
    ReadMeterData(&MeterData);          //读取源表数据
    MeterData.MeterTrigDone = rst;
    TransMeterStatus(&MeterData);       //源表trigger完成，则向控制板发送完成信号和待测工序剩余数量
}

void ReadMeterData(METER_DATA* pMeterData)
{
	pMeterData->MeterStatus = ReadMeterStatus();		//读源表状态
	pMeterData->MeterItemNum = ReadMeterItem();			//读源表待测工序剩余数量
	//结果由主控板主动发送
	pMeterData->MeterTrigDone = RST_ASK;	//读源表是否trigger完成标志
}
