#include "drv_GlobalVar.h"
#include <string.h>
#include "task_Range.h"
#include "test_hh.h"
#include "drv_MgModbus.h"
#include "task_DAC.h"
#include "task_Transfer.h"
#include "drv_fpga.h"
#include "task_ComFPGA.h"

ST_RANGESTATE st_rangestate;


Uint16 rangeVolword[RANGE_VOL_NUM] = {VOLRANGE_20mV, VOLRANGE_200mV, VOLRANGE_2V, VOLRANGE_20V, VOLRANGE_200V};
Uint16 rangeCurword[RANGE_CUR_NUM] = {CURRANGE_1nA, CURRANGE_10nA, CURRANGE_100nA, CURRANGE_1uA, CURRANGE_10uA,
                                      CURRANGE_100uA, CURRANGE_1mA, CURRANGE_10mA, CURRANGE_100mA, CURRANGE_2A, CURRANGE_10A, CURRANGE_200uA};
float32 rangeVol_buff[RANGE_VOL_NUM] = {RANGE_VOL_20mV, RANGE_VOL_200mV, RANGE_VOL_2V, RANGE_VOL_20V, RANGE_VOL_200V};
float32 rangeCur_buff[RANGE_CUR_NUM] = {RANGE_CUR_1nA, RANGE_CUR_10nA, RANGE_CUR_100nA, RANGE_CUR_1uA, RANGE_CUR_10uA,
                                        RANGE_CUR_100uA, RANGE_CUR_1mA, RANGE_CUR_10mA, RANGE_CUR_100mA, RANGE_CUR_2A, RANGE_CUR_10A, RANGE_CUR_200uA};



void Init_RangeData()
{
    memset(&st_volrangeset, 0 ,sizeof(ST_VOLRANGESET));
    memset(&st_currangeset, 0 ,sizeof(ST_CURRANGESET));
    memset(&st_rangestate, 0, sizeof(ST_RANGESTATE));
}




void RangeVolSel(Uint16 range_index, ST_VOLRANGESET *p)
{

    if(range_index == RANGE_VOL_20V_INDEX)
    {
        p->gainA0 = 0;
        p->gainA1 = 0;
    }
    else
    {
        p->gainA0 = 1;
        p->gainA1 = 0;
    }

    p->range.word = rangeVolword[range_index];
}


void RangeCurSel(Uint16 range_index, ST_CURRANGESET *p)
{
    if(range_index == RANGE_CUR_200uA_INDEX)
    {
        p->gainA0 = 0;
        p->gainA1 = 0;
    }
    else if(range_index == RANGE_CUR_2A_INDEX)  //2A和10A这边有区别
    {
        p->gainA0 = 1;
        p->gainA1 = 1;
    }
    else
    {
        p->gainA0 = 1;
        p->gainA1 = 0;
    }

    if(range_index == RANGE_CUR_2A_INDEX || range_index == RANGE_CUR_10A_INDEX)
    {
        p->protect_1A = 1;
        p->sense_1A = 0;
        p->swP_1A = 1;
        p->swN_1A = 1;

    }
    else //except 1A range
    {
        p->protect_1A = 0;
        p->sense_1A = 1;
        p->swP_1A = 0;
        p->swN_1A = 0;
    }

    p->range.word = rangeCurword[range_index];
}





void OutModeSel(Uint16 mode, float32 setcur, float32 setvol)
{
    if(mode == OUTMODE_I)
    {
        mgmd_stSCIRx.CMD_PRIORITY = 0;
        if(setcur >= 0)   mgmd_stSCIRx.CMD_POLARITY = 0;
        else              mgmd_stSCIRx.CMD_POLARITY = 1;
    }
    else  // mode == OUTMODE_V || mode == OUTMODE_V_OFFVI
    {
        mgmd_stSCIRx.CMD_PRIORITY = 1;
        if(setvol >= 0)   mgmd_stSCIRx.CMD_POLARITY = 0;
        else              mgmd_stSCIRx.CMD_POLARITY = 1;
    }

}


// mode  souceVorI,  autorange measure的是否autorange， limitflag measure的是否limt 需要切档  range_vol 下发的初始rang vol档位   range_cur 下发的初始range cur档位
// prange 结构体， power 是否处于上电状态
//Uint16 state_range = 0;
void RangeSwitch(Uint16 mode, ST_RANGESTATE* prange, Uint16 power)
{
    if(power == 0)
    {
        //不上电时，可随意实时改变range
        prange->now_vol = prange->set_vol;  //prange->set_cur
        prange->now_cur = prange->set_cur;
    }

    RangeVolSel(prange->now_vol, &st_volrangeset);
    RangeCurSel(prange->now_cur, &st_currangeset);
}





