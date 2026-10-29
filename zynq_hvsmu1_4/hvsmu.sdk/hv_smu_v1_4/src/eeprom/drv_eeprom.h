/*
 * drv_eeprom.h
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#ifndef SRC_DRV_EEPROM_H_
#define SRC_DRV_EEPROM_H_

#include "TYPEDEF.h"


#define  IEEETOF(x)     (*(float32*)&(x))
#define  FTOIEEE(x)     (*(Uint32*)&(x))


//#define  EEPROM_INIT_FUNC(name) void InitTaskEeprom_##name(void)
//#define  EEPROM_RUN_FUNC(name) void TaskEeprom_##name(void)


#define  EEPROM_ST_STORED_READ(name) 	st_eerom_para_##name##_read
#define  EEPROM_ST_STORED_WRITE(name) 	st_eerom_para_##name##_write
#define  EEPROM_ST_PARA(name) 			st_eerom_para_##name

//0x50--1024   0x54--512
#define EEPROM_ADDR                (0x54)

//总长 1024k bit  即128k 个8bit的数据。 目前都是8bit为单位存储的。地址最大是0x20000
//sizeof(Uint16) = 1
//EEROM_PARA_LEN = 118 = 0x76
#define EEROM_COEF_START           0x0000
//len = 50
#define EEROM_PARA_IP_START        0x0500
#define EEROM_CONFIG_START         0x0580

//写eeprom时的控制命令
#define EEROM_W_COEF1       1
#define EEROM_W_COEF2       2
#define EEROM_W_CONFIG1     3
#define EEROM_W_CONFIG2     4
#define EEROM_W_IP1       100
#define EEROM_W_IP2       101



#define EEROM_HEAD    (0x4848)
#define EEROM_TAIL    (0x6868)

#define EEROM_DATAOK             (6666)
#define EEROM_ERR_HEAD           (1001)
#define EEROM_ERR_TAIL           (1002)
#define EEROM_ERR_CRC            (1003)
#define EEROM_DATAERR_OVERRANG   (1004)


Uint16 CalCrc(Uint8* data, Uint16 num_nocrc);
Uint16 CheckReadCrc(Uint8* pRead, Uint16 len_nocrc);
void U16ToU8(Uint16 s_u16, Uint8 *d_u8);
void U32ToU8(Uint32 s_u32, Uint8 *d_u8);
Uint16 U8ToU16(Uint8 *s_u8);
Uint32 U8ToU32(Uint8 *s_u8);

void PS_IIC0_Init_100k(void);
void PS_IIC0_SeqWrite(uint16_t start_addr, uint8_t *pbuf, uint32_t len);
void PS_IIC0_SeqRead(uint16_t start_addr, uint8_t *pbuf, uint32_t len);




#endif /* SRC_DRV_EEPROM_H_ */
