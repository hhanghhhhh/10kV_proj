/*
 * task_eeprom_ip.h
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#ifndef SRC_TASK_EEPROM_IP_H_
#define SRC_TASK_EEPROM_IP_H_

#include "TYPEDEF.h"


#define EEROM_PARA_IP_LEN_NOCRC    (sizeof(ST_EEROM_PARA_IP_STORED) - 2)
#define EEROM_PARA_IP_LEN           sizeof(ST_EEROM_PARA_IP_STORED)


//head  tail  crc的位置是固定的，否则计算crc的函数会计算错偏移量的
typedef struct
{
    Uint8 head[2];

    Uint8 ip[4];

    Uint8 tail[2];
    Uint8 crc[2];
}ST_EEROM_PARA_IP_STORED;


typedef struct
{
    Uint16 eerom_ok;
    Uint16 ip[4];
}ST_EEROM_PARA_IP;

// 后缀名需要和 .c 中定义的 NAME 一样！！！
extern ST_EEROM_PARA_IP st_eerom_para_IP;

// 函数声明
void InitTaskEeprom_IP(void);
void TaskEeprom_IP(void);





#endif /* SRC_TASK_EEPROM_IP_H_ */
