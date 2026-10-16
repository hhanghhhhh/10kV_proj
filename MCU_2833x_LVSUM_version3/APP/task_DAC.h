#ifndef _TASK_DAC_H_
#define _TASK_DAC_H_


typedef struct
{
    Uint16 AD;
    Uint16 range_index;
    float32 set;
    float32 gain;
    float32 offset;
}ST_DACPARA;

extern ST_DACPARA st_VDACpara, st_IDACpara;


#define  UPDATE_VREF(data)                       \
        st_datatoFPGA.dac1dac2.bit.dac1 = data;


#define  UPDATE_IREF(data)                       \
        st_datatoFPGA.dac1dac2.bit.dac2 = data;


#define  UPDATE_AVMAREF(dacA, dacB)              \
        st_datatoFPGA.dacavmab.bit.avmA = dacA;  \
        st_datatoFPGA.dacavmab.bit.avmB = dacB;


extern void Init_DACData();
extern void DACCalculate(ST_DACPARA* vol, ST_DACPARA* cur);







#endif
