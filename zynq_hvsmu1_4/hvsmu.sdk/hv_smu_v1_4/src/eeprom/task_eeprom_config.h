/*
 * task_eeprom_config.h
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#ifndef SRC_TASK_EEPROM_CONFIG_H_
#define SRC_TASK_EEPROM_CONFIG_H_

#include "TYPEDEF.h"

#define EEROM_CONFIG_LEN_NOCRC      (sizeof(ST_EEROM_CONFIG_STORED) - 2)
#define EEROM_CONFIG_LEN             sizeof(ST_EEROM_CONFIG_STORED)


#define EEROM_CONFIG_NUM			40


#define EEROM_CONFIG_vbr_thx100			     		0
#define EEROM_CONFIG_ovp_th							1
#define EEROM_CONFIG_fire_th						2
#define EEROM_CONFIG_fire_drop_ratio_100			3
#define EEROM_CONFIG_fire_drop_value				4
#define EEROM_CONFIG_scp_th							5
#define EEROM_CONFIG_scp_time_0p1ms					6
#define EEROM_CONFIG_idss_overtime					7
#define EEROM_CONFIG_vbr_overtime					8
#define EEROM_CONFIG_vbr_shorttime_0p1ms			9
#define EEROM_CONFIG_vbr_fast_cnt					10
#define EEROM_CONFIG_vbr_slow_cnt					11




//head  tail  crc的位置是固定的，否则计算crc的函数会计算错偏移量的
typedef struct
{
    Uint8 head[2];

    Uint8 data[EEROM_CONFIG_NUM][2];

    Uint8 tail[2];
    Uint8 crc[2];
}ST_EEROM_CONFIG_STORED;

typedef struct
{
    Uint16 eerom_ok;

    //总共预留40个Uint16
    Uint16 data[EEROM_CONFIG_NUM];


}ST_EEROM_CONFIG;


// 后缀名需要和 .c 中定义的 NAME 一样！！！
extern ST_EEROM_CONFIG st_eerom_para_CONFIG;

// 函数声明
void InitTaskEeprom_CONFIG(void);
void TaskEeprom_CONFIG(void);



#endif /* SRC_TASK_EEPROM_COEF_H_ */
