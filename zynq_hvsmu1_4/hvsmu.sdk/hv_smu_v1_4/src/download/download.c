/*
 * download.c
 *
 *  Created on: 2025年8月20日
 *      Author: m
 */

#include "COMMON.h"

// 8Mb 的内存大小用于存储整个程序
#define PROGRAM_SIZE	8388608
Uint8 program_data[PROGRAM_SIZE];
Uint32 byte_num = 0;
Uint16 program_recv_ok = 0;

#define HEAD_FRAME	1
#define DATA_FRAME	2
#define TAIL_FRAME	3


void ResetZynq(void)
{
    Xil_Out32(0xF8000008, 0xDF0D);   // Unlock SLCR
    Xil_Out32(0xF8000200, 0x1);      // PS 软件复位
}


/*
 *  1. 发送数据
 *  数据从 0 地址开始，连续不间断，两个字节组成一个 Uint16 发送，高地址在高位
 *  每次发送 120 个 Uint16，最后一帧不够 120 则最后全部补 0xFFFF，凑够 120
 *  第一次发送，frame_type = 1，最后一次发送， frame_type = 3，其余数据， frame_type = 2
 *  frame_type 在数据后边发，同时作为接收的 flag 用。
 *
 *  2. 每次发送等待
 *  每次发送 120 个 Uint16 后，读取 frame_type 为 0 说明接收完毕
 *
 *  3. 全部发送完成校验
 *  全部发送完成后，读取 crc 以及 byte_num，作为校验，看接收是否正确
 *  其中， crc 为每个 Uint16 的累加和，byte_num 为总的字节数，一个 Uint16 为 2 个字节
 *
 *  4. 校验 ok 发送写入命令
 *  校验ok，发送 cmd = 1，开始写入 flash
 *
 *  5. 等待写入 ok
 *  读取 ok flag，为 1 说明写入成功，超时则失败，超时时间待定，需实测。
 *
 * */

// 每帧数据都是120个地址，末尾不够则补 0xFF
void RecvProgramData(void)
{
	ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
	Uint16 i = 0;
	Uint16 rst = 1;

	pPC->download.byte_num.u32 = byte_num;

	if(pPC->download.frame_type == HEAD_FRAME)
	{
		byte_num = 0;
		program_recv_ok = 0;
		pPC->download.crc = 0;
		pPC->download.ok = 0;
		if(byte_num + 240 < PROGRAM_SIZE)
		{
			for(i = 0; i < 120; i++)
			{
				program_data[byte_num++] = pPC->download.data[i] & 0x00FF;
				program_data[byte_num++] = (pPC->download.data[i] >> 8) & 0x00FF;
				pPC->download.crc += pPC->download.data[i];
			}
		}

		pPC->download.frame_type = 0;
	}
	else if(pPC->download.frame_type == DATA_FRAME)
	{
		program_recv_ok = 0;
		if(byte_num + 240 < PROGRAM_SIZE)
		{
			for(i = 0; i < 120; i++)
			{
				program_data[byte_num++] = pPC->download.data[i] & 0x00FF;
				program_data[byte_num++] = (pPC->download.data[i] >> 8) & 0x00FF;
				pPC->download.crc += pPC->download.data[i];
			}
		}
		pPC->download.frame_type = 0;
	}
	else if(pPC->download.frame_type == TAIL_FRAME)
	{
		if(byte_num + 240 < PROGRAM_SIZE)
		{
			for(i = 0; i < 120; i++)
			{
				program_data[byte_num++] = pPC->download.data[i] & 0x00FF;
				program_data[byte_num++] = (pPC->download.data[i] >> 8) & 0x00FF;
				pPC->download.crc += pPC->download.data[i];
			}
		}
		pPC->download.frame_type = 0;
		program_recv_ok = 1;
	}

	if(pPC->download.write_flash == 1 && program_recv_ok == 1)
	{
		pPC->download.write_flash = 0;
		rst = QspiUpdate(byte_num, program_data);
		if(rst == 0)
		{
			pPC->download.ok = 1;
		}
	}

	if(pPC->download.reset == 1 && pPC->download.ok == 1)
	{
		pPC->download.reset = 0;
		ResetZynq();
	}
}



