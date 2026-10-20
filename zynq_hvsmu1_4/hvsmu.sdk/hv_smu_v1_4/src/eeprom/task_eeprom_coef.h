/*
 * task_eeprom_coef.h
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#ifndef SRC_TASK_EEPROM_COEF_H_
#define SRC_TASK_EEPROM_COEF_H_

#include "TYPEDEF.h"

#define EEROM_V_RANGE_LAYOUT_ID 0x36565231UL /* Six voltage ranges, revision 1 */

#define EEROM_COEF_LEN_NOCRC (sizeof(ST_EEROM_COEF_STORED) - 2)
#define EEROM_COEF_LEN sizeof(ST_EEROM_COEF_STORED)

// 这里改变后，需要注意modbus那边地址的数组也要改变。否则会越界
// 电压系数，两线、四线是两套不同的系数
// 电流系数，预留两组
#define EEROM_V_ADCP_NUM (8)
#define EEROM_V_ADCN_NUM (8)
#define EEROM_V_DACP_NUM (8)
#define EEROM_V_DACN_NUM (8)
#define EEROM_I_ADCP_NUM (12)
#define EEROM_I_ADCN_NUM (12)
#define EEROM_I_DACP_NUM (12)
#define EEROM_I_DACN_NUM (12)

#define EEROM_PARA_V_NUM (EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM + EEROM_V_DACP_NUM + \
                          EEROM_V_DACN_NUM)
#define EEROM_PARA_I_NUM (EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM + \
                          EEROM_I_DACP_NUM + EEROM_I_DACN_NUM)

#define EEROM_PARA_NUM (EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM + EEROM_V_DACP_NUM + \
                        EEROM_V_DACN_NUM + EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM + \
                        EEROM_I_DACP_NUM + EEROM_I_DACN_NUM)

// head  tail  crc的位置是固定的，否则计算crc的函数会计算错偏移量的
typedef struct
{
    Uint8 head[2];

    Uint8 v_adc_p_k_byte[EEROM_V_ADCP_NUM][4];
    Uint8 v_adc_n_k_byte[EEROM_V_ADCN_NUM][4];
    Uint8 v_dac_p_k_byte[EEROM_V_DACP_NUM][4];
    Uint8 v_dac_n_k_byte[EEROM_V_DACN_NUM][4];

    Uint8 i_adc_p_k_byte[EEROM_I_ADCP_NUM][4];
    Uint8 i_adc_n_k_byte[EEROM_I_ADCN_NUM][4];
    Uint8 i_dac_p_k_byte[EEROM_I_DACP_NUM][4];
    Uint8 i_dac_n_k_byte[EEROM_I_DACN_NUM][4];

    Uint8 v_adc_p_b_byte[EEROM_V_ADCP_NUM][4];
    Uint8 v_adc_n_b_byte[EEROM_V_ADCN_NUM][4];
    Uint8 v_dac_p_b_byte[EEROM_V_DACP_NUM][4];
    Uint8 v_dac_n_b_byte[EEROM_V_DACN_NUM][4];

    Uint8 i_adc_p_b_byte[EEROM_I_ADCP_NUM][4];
    Uint8 i_adc_n_b_byte[EEROM_I_ADCN_NUM][4];
    Uint8 i_dac_p_b_byte[EEROM_I_DACP_NUM][4];
    Uint8 i_dac_n_b_byte[EEROM_I_DACN_NUM][4];

    Uint8 utctime[4];

    // 共 10个 [4]
    Uint8 sn_no[3][4];
    Uint8 reserve[7][4];

    Uint8 tail[2];
    Uint8 crc[2];
} ST_EEROM_COEF_STORED;

typedef struct
{
    Uint16 eerom_ok;

    float32 f32v_adc_p_k[EEROM_V_ADCP_NUM];
    float32 f32v_adc_n_k[EEROM_V_ADCN_NUM];
    float32 f32v_dac_p_k[EEROM_V_DACP_NUM];
    float32 f32v_dac_n_k[EEROM_V_DACN_NUM];

    float32 f32i_adc_p_k[EEROM_I_ADCP_NUM];
    float32 f32i_adc_n_k[EEROM_I_ADCN_NUM];
    float32 f32i_dac_p_k[EEROM_I_DACP_NUM];
    float32 f32i_dac_n_k[EEROM_I_DACN_NUM];

    float32 f32v_adc_p_b[EEROM_V_ADCP_NUM];
    float32 f32v_adc_n_b[EEROM_V_ADCN_NUM];
    float32 f32v_dac_p_b[EEROM_V_DACP_NUM];
    float32 f32v_dac_n_b[EEROM_V_DACN_NUM];

    float32 f32i_adc_p_b[EEROM_I_ADCP_NUM];
    float32 f32i_adc_n_b[EEROM_I_ADCN_NUM];
    float32 f32i_dac_p_b[EEROM_I_DACP_NUM];
    float32 f32i_dac_n_b[EEROM_I_DACN_NUM];

    Uint32 utctime;
    Uint32 sn_no[3];
} ST_EEROM_COEF;

// 后缀名需要和 .c 中定义的 NAME 一样！！！
extern ST_EEROM_COEF st_eerom_para_COEF;

// 函数声明
void InitTaskEeprom_COEF(void);
void TaskEeprom_COEF(void);

#endif /* SRC_TASK_EEPROM_COEF_H_ */
