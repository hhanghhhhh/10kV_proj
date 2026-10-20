/*
 * drv_eeprom.c
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#include "COMMON.h"


// IIC0 外设初始化，时钟频率为100k
void PS_IIC0_Init_100k(void)
{
	PS_IIC_Init(&IIC0, XPAR_XIICPS_0_DEVICE_ID, 100000);
}

// 写 eeprom, eeprom 一页 128 byte，一次最多操作 128 byte(0x80), 且不能跨页操作！
// 为了简单起见，该接口要求起始地址必须是 0x80 的整数倍
void PS_IIC0_SeqWrite(uint16_t start_addr, uint8_t *pbuf, uint32_t len)
{
	uint16_t tmp_len = 0;
	uint32_t tmp_start = 0;
	uint8_t* tmp_pointer;

    tmp_len = len;
    tmp_start = start_addr;
    tmp_pointer = pbuf;

    while(tmp_len > 128)
    {
    	PS_IIC_SeqWrite_Reg(&IIC0, EEPROM_ADDR, REG16, tmp_start, tmp_pointer, 128);
        tmp_len -= 128;
        tmp_start += 128;
        tmp_pointer += 128;
        usleep(10000);
    }

    PS_IIC_SeqWrite_Reg(&IIC0, EEPROM_ADDR, REG16, tmp_start, tmp_pointer, tmp_len);
	usleep(10000);
}

void PS_IIC0_SeqRead(uint16_t start_addr, uint8_t *pbuf, uint32_t len)
{
	uint16_t tmp_len = 0;
	uint32_t tmp_start = 0;
	uint8_t* tmp_pointer;

    tmp_len = len;
    tmp_start = start_addr;
    tmp_pointer = pbuf;

    while(tmp_len > 128)
    {
    	PS_IIC_SeqRead_Reg(&IIC0, EEPROM_ADDR, REG16, tmp_start, tmp_pointer, 128);
        tmp_len -= 128;
        tmp_start += 128;
        tmp_pointer += 128;
        usleep(10000);
    }

	PS_IIC_SeqRead_Reg(&IIC0, EEPROM_ADDR, REG16, tmp_start, tmp_pointer, tmp_len);
	usleep(10000);
}



//num为字节数，每两个字节组成一个Uint16，求所有的累加和
Uint16 CalCrc(Uint8* data, Uint16 num_nocrc)
{
    Uint16 u16tmp = 0, i = 0;
    Uint16 crc = 0;

    if((num_nocrc & 0x01) == 1)    num_nocrc -= 1;

    for(i = 0; i < num_nocrc; i+=2)
    {
        u16tmp = U8ToU16(&data[i]);
        crc += u16tmp;
    }
    return crc;
}


Uint16 CheckReadCrc(Uint8* pRead, Uint16 len_nocrc)
{
    Uint16 u16tmp = 0, crctmp = 0;
    u16tmp = U8ToU16(&pRead[0]);
    if(u16tmp != EEROM_HEAD)
    {
        return EEROM_ERR_HEAD;
    }

    u16tmp = U8ToU16(&pRead[len_nocrc - 2]);
    if(u16tmp != EEROM_TAIL)
    {
        return EEROM_ERR_TAIL;
    }

    u16tmp = U8ToU16(&pRead[len_nocrc]);
    crctmp = CalCrc(pRead, len_nocrc);
    if(u16tmp != crctmp)
    {
        return EEROM_ERR_CRC;
    }

    return EEROM_DATAOK;
}



void U16ToU8(Uint16 s_u16, Uint8 *d_u8)
{
	d_u8[0] = s_u16 & 0x00FF;
	d_u8[1] = (s_u16 >> 8) & 0x00FF;
}


void U32ToU8(Uint32 s_u32, Uint8 *d_u8)
{
	d_u8[0] = s_u32 & 0x00FF;
	d_u8[1] = (s_u32 >> 8) & 0x00FF;
	d_u8[2] = (s_u32 >> 16) & 0x00FF;
	d_u8[3] = (s_u32 >> 24) & 0x00FF;;
}


Uint16 U8ToU16(Uint8 *s_u8)
{
    Uint16 des = 0;
    des = ((Uint16)s_u8[1] << 8) + s_u8[0];
    return des;
}

Uint32 U8ToU32(Uint8 *s_u8)
{
    Uint32 des;
    des = ((Uint32)s_u8[3] << 24) + ((Uint32)s_u8[2] << 16) + ((Uint32)s_u8[1] << 8) + s_u8[0];
    return des;
}












