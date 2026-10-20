/*
 * task_range.h
 *
 *  Created on: 2025Äê7ÔÂ2ÈÕ
 *      Author: m
 */

#ifndef SRC_TASK_RANGE_H_
#define SRC_TASK_RANGE_H_

#include "TYPEDEF.h"

#define OUTMODE_V 0
#define OUTMODE_I 1

#define RANGE_VOL_NUM 6

#define RANGE_VOL_400V_INDEX 0
#define RANGE_VOL_1000V_INDEX 1
#define RANGE_VOL_2000V_INDEX 2
#define RANGE_VOL_4000V_INDEX 3
#define RANGE_VOL_7000V_INDEX 4
#define RANGE_VOL_10000V_INDEX 5

/* Local range word, LSB first: VMEN,VMA2,VMA1,VMA0,VAEN,VA2,VA1,VA0. */
#define VOLRANGE_400V 0b00010001U
#define VOLRANGE_1000V 0b10011001U
#define VOLRANGE_2000V 0b01010101U
#define VOLRANGE_4000V 0b11011101U
#define VOLRANGE_7000V 0b00110011U
#define VOLRANGE_10000V 0b10111011U

typedef union VOLRANGE
{
    Uint16 word;
    struct VOLRANGE_BIT
    {
        Uint16 vmaen : 1;
        Uint16 vma2 : 1;
        Uint16 vma1 : 1;
        Uint16 vma0 : 1;
        Uint16 vaen : 1;
        Uint16 va2 : 1;
        Uint16 va1 : 1;
        Uint16 va0 : 1;
        Uint16 reserve : 8;
    } bit;
} UN_VOLRANGESET;

extern UN_VOLRANGESET un_volrangeset;

#define RANGE_CUR_NUM 9

#define RANGE_CUR_1nA_INDEX 0
#define RANGE_CUR_10nA_INDEX 1
#define RANGE_CUR_100nA_INDEX 2
#define RANGE_CUR_1uA_INDEX 3
#define RANGE_CUR_10uA_INDEX 4
#define RANGE_CUR_100uA_INDEX 5
#define RANGE_CUR_1mA_INDEX 6
#define RANGE_CUR_5mA_INDEX 7
#define RANGE_CUR_50mA_INDEX 8

#define CURRANGE_1nA 0b111101010
#define CURRANGE_10nA 0b111010101
#define CURRANGE_100nA 0b111001010
#define CURRANGE_1uA 0b110010101
#define CURRANGE_10uA 0b110001010
#define CURRANGE_100uA 0b100010101
#define CURRANGE_1mA 0b100001010
#define CURRANGE_5mA 0b000010101
#define CURRANGE_50mA 0b000001010

typedef union
{
    Uint16 word;
    struct
    {
        Uint16 imeas_gain1 : 1;
        Uint16 imeas_gain10 : 1;
        Uint16 ifb_gain1 : 1;
        Uint16 ifb_gain3 : 1;
        Uint16 idac_div3 : 1; // idac_amps
        Uint16 range_10n_100n : 1;
        Uint16 range_1u_10u : 1;
        Uint16 range_100u_1m : 1;
        Uint16 range_5m_50m : 1;
        Uint16 reserve : 7;
    } bit;
} UN_CURRANGESET;

extern UN_CURRANGESET un_currangeset;

typedef struct
{
    Uint16 range_index_PC;
    Uint16 range_index;
    float32 full_range;
    float32 dac_full;
    float32 adc_full;
} ST_RANGE_PARA;

extern ST_RANGE_PARA st_range_vol_use, st_range_cur_use;

extern float32 rangeVol_buff[];
extern float32 rangeCur_buff[];

void InitTaskRange(void);
void RangeVolSel(Uint16 range_index);
void RangeCurSel(Uint16 range_index);
void RangeLimit(void);
void CmdImplement(void);

#endif /* SRC_TASK_RANGE_H_ */
