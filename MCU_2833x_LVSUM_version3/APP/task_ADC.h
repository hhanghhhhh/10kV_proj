#ifndef _TASK_ADC_H_
#define _TASK_ADC_H_

#include "task_ComFPGA.h"
#include "drv_Eeprom.h"



#define  ADCTRIGGER_H()                      \
        un_cmd_data2.bit.adc_trigger = 1;


#define  ADCTRIGGER_L()                      \
        un_cmd_data2.bit.adc_trigger = 0;


#define  CUR_INTERVAL_2US   50

#define  CUR_2US_1   (0.5f)

//20mV
#define ADC_VOL_P_RATIO0     -1.031743048
#define ADC_VOL_P_OFFSET0    1.4319242E-03
//200mV
#define ADC_VOL_P_RATIO1     -1.033385509
#define ADC_VOL_P_OFFSET1    1.4070772E-03
//2V
#define ADC_VOL_P_RATIO2     -1.000332628
#define ADC_VOL_P_OFFSET2    1.3419566E-02
//20V
#define ADC_VOL_P_RATIO3     -1.007954467
#define ADC_VOL_P_OFFSET3    1.2511812E-01
//200V
#define ADC_VOL_P_RATIO4     -1.000555121
#define ADC_VOL_P_OFFSET4    1.2371212E+00



//-20mV
#define ADC_VOL_N_RATIO0     -1.031488043
#define ADC_VOL_N_OFFSET0    1.3917234E-03
//-200mV
#define ADC_VOL_N_RATIO1     -1.033606237
#define ADC_VOL_N_OFFSET1    1.4149923E-03
//-2V
#define ADC_VOL_N_RATIO2     -1.000358499
#define ADC_VOL_N_OFFSET2    1.3305697E-02
//-20V
#define ADC_VOL_N_RATIO3     -1.007955631
#define ADC_VOL_N_OFFSET3    1.2474075E-01
//-200V
#define ADC_VOL_N_RATIO4     -1.001167741
#define ADC_VOL_N_OFFSET4    1.3361157E+00



//楼下那台
//1nA
//#define ADC_CUR_P_RATIO0     -1.010010297
//#define ADC_CUR_P_OFFSET0    -3.1350662E-12
////10nA
//#define ADC_CUR_P_RATIO1     -1.010010297
//#define ADC_CUR_P_OFFSET1    -3.1350662E-11
//上海新校准的
//1nA
#define ADC_CUR_P_RATIO0      0.970125
#define ADC_CUR_P_OFFSET0     4.06313E-10
//10nA
#define ADC_CUR_P_RATIO1      0.966608
#define ADC_CUR_P_OFFSET1     4.34376E-10
//100nA
#define ADC_CUR_P_RATIO2      1.010010297
#define ADC_CUR_P_OFFSET2    -3.1350662E-10
//1uA
#define ADC_CUR_P_RATIO3      1.010010297
#define ADC_CUR_P_OFFSET3    -3.1350662E-09
//10uA
#define ADC_CUR_P_RATIO4      1.000657892
#define ADC_CUR_P_OFFSET4     6.1145854E-08
//100uA
#define ADC_CUR_P_RATIO5      1.001799001
#define ADC_CUR_P_OFFSET5     6.6633613E-07
//1mA
#define ADC_CUR_P_RATIO6      0.989421074
#define ADC_CUR_P_OFFSET6     6.58983E-06
#define ADC_CUR_P_RATIO6_2    0.937032926  //0-500nA
#define ADC_CUR_P_OFFSET6_2   6.19145E-06
//10mA
#define ADC_CUR_P_RATIO7      0.999872215
#define ADC_CUR_P_OFFSET7     6.5488261E-05
//100mA
#define ADC_CUR_P_RATIO8      0.999947189
#define ADC_CUR_P_OFFSET8     6.5815074E-04
//1A
#define ADC_CUR_P_RATIO9      0.991631027
#define ADC_CUR_P_OFFSET9     6.5537737E-02




//-1nA
#define ADC_CUR_N_RATIO0     1
#define ADC_CUR_N_OFFSET0    0
//-10nA
#define ADC_CUR_N_RATIO1     1.025477699
#define ADC_CUR_N_OFFSET1    3.20157E-10
//-100nA
#define ADC_CUR_N_RATIO2     1.119557084
#define ADC_CUR_N_OFFSET2    3.34906E-10
//-1uA
#define ADC_CUR_N_RATIO3     1.030979653
#define ADC_CUR_N_OFFSET3    1.08353E-09
//-10uA
#define ADC_CUR_N_RATIO4     1.020370211
#define ADC_CUR_N_OFFSET4    5.16347E-09
//-100uA
#define ADC_CUR_N_RATIO5     1.017408353
#define ADC_CUR_N_OFFSET5    6.80609E-08
//-1mA
#define ADC_CUR_N_RATIO6     1.010593675
#define ADC_CUR_N_OFFSET6    6.20362E-07
//-10mA
#define ADC_CUR_N_RATIO7     1.020574125
#define ADC_CUR_N_OFFSET7    6.18442E-06
//-100mA
#define ADC_CUR_N_RATIO8     1.018285159
#define ADC_CUR_N_OFFSET8    6.29662E-05
//-1A
#define ADC_CUR_N_RATIO9     1.000328907
#define ADC_CUR_N_OFFSET9    0.00839886






typedef struct
{
    Uint16 trigger;
    Uint16 dataH;
    Uint16 dataM;
    Uint16 dataL;
    Uint16 dataOK;

    float32 data_cur_avg;   //AD值的平均值
    float32 data_cur_V;     //将AD值化为5V对应的电压值
    float32 data_vol_avg;   //AD值的平均值
    float32 data_vol_V;     //将AD值化为5V对应的电压值

    Uint16 sample_num;   //采样点数
    Uint16 interval_cnt;   //采样间隔，50个cnt为500k(2us)，以此为倍数

    float32 vol_gain;
    float32 vol_offset;
    float32 cur_gain;
    float32 cur_offset;

    Uint32 nowtime;
    Uint32 sampletime;
}ST_ADC;

extern ST_ADC  st_ADCdata;

extern float32 ADC_Vol_P_ratio_buf[];
extern float32 ADC_Vol_P_offset_buf[];
extern float32 ADC_Vol_N_ratio_buf[];
extern float32 ADC_Vol_N_offset_buf[];
extern float32 ADC_Cur_P_ratio_buf[];
extern float32 ADC_Cur_P_offset_buf[];
extern float32 ADC_Cur_N_ratio_buf[];
extern float32 ADC_Cur_N_offset_buf[];


extern void Init_ADCdata(ST_ADC* p);
extern void ADCSample(ST_ADC* p);

#endif
