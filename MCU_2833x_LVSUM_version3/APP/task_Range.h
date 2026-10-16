#ifndef _TASK_RANGE_H_
#define _TASK_RANGE_H_


#define OUTMODE_V     0
#define OUTMODE_I     1
#define OUTMODE_V_OFFVI     2



#define R_STATE_IDEL      0
//source V  range switch V
#define R_STATE_SVRV_1    1
#define R_STATE_SVRV_2    2
#define R_STATE_SVRV_3    3
#define R_STATE_SVRV_4    4
#define R_STATE_SVRV_5    5
//source V  range switch I
#define R_STATE_SVRI      6
#define R_STATE_SVRI_1    7
#define R_STATE_SVRI_2    8
//source I  range switch I
#define R_STATE_SIRI_1    9
#define R_STATE_SIRI_2    10
#define R_STATE_SIRI_3    11
#define R_STATE_SIRI_4    12
//source I  range switch V manual
#define R_STATE_SIRV      13

#define R_STATE_SVLIMIT   14
#define R_STATE_SILIMIT   15

#define R_STATE_UPDATEDAC   16




#define RANGE_VOL_NUM    5

#define RANGE_VOL_20mV   0.02f
#define RANGE_VOL_200mV  0.2f
#define RANGE_VOL_2V     2
#define RANGE_VOL_20V    40
#define RANGE_VOL_200V   200

#define RANGE_VOL_20mV_RANGE   0.02001f
#define RANGE_VOL_200mV_RANGE  0.2001f
#define RANGE_VOL_2V_RANGE     2.001
#define RANGE_VOL_20V_RANGE    40.001
#define RANGE_VOL_200V_RANGE   200.001

#define RANGE_VOL_20mV_INDEX   0
#define RANGE_VOL_200mV_INDEX  1
#define RANGE_VOL_2V_INDEX     2
#define RANGE_VOL_20V_INDEX    3
#define RANGE_VOL_200V_INDEX   4

#define VOLRANGE_20mV   0b0010001
#define VOLRANGE_200mV  0b0010010
#define VOLRANGE_2V     0b0000010
#define VOLRANGE_20V    0b0000110
#define VOLRANGE_200V   0b1101110



struct VOLRANGE_BIT
{
    Uint16  dac1:1;
    Uint16  dac01:1;
    Uint16  div10:1;
    Uint16  div200:1;
    Uint16  gain10:1;
    Uint16  en235P:1;
    Uint16  en235N:1;
    Uint16  reserve:9;
};

union VOLRANGE
{
    Uint16 word;
    struct VOLRANGE_BIT bit;
};


typedef struct
{
    union VOLRANGE range;
    Uint16 gainA1;
    Uint16 gainA0;
}ST_VOLRANGESET;


extern ST_VOLRANGESET st_volrangeset;


#define RANGE_CUR_NUM     12

#define RANGE_CUR_1nA     1e-9f
#define RANGE_CUR_10nA    1e-8f
#define RANGE_CUR_100nA   1e-7f
#define RANGE_CUR_1uA     1e-6f
#define RANGE_CUR_10uA    1e-5f
#define RANGE_CUR_100uA   1e-4f
#define RANGE_CUR_1mA     1e-3f
#define RANGE_CUR_10mA    1e-2f
#define RANGE_CUR_100mA   1e-1f
#define RANGE_CUR_2A      2.0
#define RANGE_CUR_10A     10.0
#define RANGE_CUR_200uA   2e-4f


#define RANGE_CUR_1nA_INDEX     0
#define RANGE_CUR_10nA_INDEX    1
#define RANGE_CUR_100nA_INDEX   2
#define RANGE_CUR_1uA_INDEX     3
#define RANGE_CUR_10uA_INDEX    4
#define RANGE_CUR_100uA_INDEX   5
#define RANGE_CUR_1mA_INDEX     6
#define RANGE_CUR_10mA_INDEX    7
#define RANGE_CUR_100mA_INDEX   8
#define RANGE_CUR_2A_INDEX      9
#define RANGE_CUR_10A_INDEX     10
#define RANGE_CUR_200uA_INDEX   11


#define CURRANGE_1nA     0b01000000001
#define CURRANGE_10nA    0b01000000010
#define CURRANGE_100nA   0b01000000110
#define CURRANGE_1uA     0b01000001010
#define CURRANGE_10uA    0b01000010010
#define CURRANGE_100uA   0b01000100010
#define CURRANGE_1mA     0b01001000010
#define CURRANGE_10mA    0b11010000010
#define CURRANGE_100mA   0b11100000010
#define CURRANGE_2A      0b10000000010
#define CURRANGE_10A     0b10000000010
#define CURRANGE_200uA   0b01000100010


struct CURRANGE_BIT
{
    Uint16  dac1:1;
    Uint16  dac01:1;
    Uint16  range100nA:1;
    Uint16  range1uA:1;
    Uint16  range10uA:1;
    Uint16  range100uA:1;
    Uint16  range1mA:1;
    Uint16  range10mA:1;
    Uint16  range100mA:1;
    Uint16  range1A:1;
    Uint16  lowIsense_n:1;
    Uint16  reserve:5;
};

union CURRANGE
{
    Uint16 word;
    struct CURRANGE_BIT bit;
};


typedef struct
{
    union CURRANGE range;
    Uint16 gainA1;
    Uint16 gainA0;
    Uint16 swP_1A;
    Uint16 swN_1A;
    Uint16 protect_1A;
    Uint16 sense_1A;
}ST_CURRANGESET;

extern ST_CURRANGESET st_currangeset;



typedef struct
{
    Uint16 set_vol;
    Uint16 now_vol;

    Uint16 set_cur;
    Uint16 now_cur;

    Uint16 limitflag;
    Uint16 autorange;
}ST_RANGESTATE;
extern ST_RANGESTATE st_rangestate;


extern Uint16 rangeVolword[RANGE_VOL_NUM];
extern Uint16 rangeCurword[RANGE_CUR_NUM];

extern float32 rangeVol_buff[RANGE_VOL_NUM];
extern float32 rangeCur_buff[RANGE_CUR_NUM];


extern void Init_RangeData();
extern void RangeVolSel(Uint16 range_index, ST_VOLRANGESET *p);
extern void RangeCurSel(Uint16 range_index, ST_CURRANGESET *p);
extern void OutModeSel(Uint16 mode, float32 setcur, float32 setvol);
extern void RangeSwitch(Uint16 mode, ST_RANGESTATE* prange, Uint16 power);
#endif
