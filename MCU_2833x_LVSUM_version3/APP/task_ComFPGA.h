#ifndef __TASK_COMFPGA_H_
#define __TASK_COMFPGA_H_


#include "TypeDefine.h"



#define  DATATOFPGA_DAC1DAC2         0
#define  DATATOFPGA_SAMPINTERVAL     1
#define  DATATOFPGA_DACAVMAB         2
#define  DATATOFPGA_CMDDATA1         3
#define  DATATOFPGA_CMDDATA2         4


#define  DATAFROMFPGA_INFO           0
#define  DATAFROMFPGA_MDATA1         1
#define  DATAFROMFPGA_MDATA2         2
#define  DATAFROMFPGA_MDATA3         3
#define  DATAFROMFPGA_VOL            4
#define  DATAFROMFPGA_CUR            5


typedef union
{
    Uint32 word;
    struct
    {
        Uint32 range_1uA:1;
        Uint32 range_10uA:1;
        Uint32 range_100uA:1;
        Uint32 range_1mA:1;
        Uint32 range_10mA:1;
        Uint32 range_100mA:1;
        Uint32 range_1A:1;
        Uint32 range_100nA:1;
        Uint32 swp1A:1;
        Uint32 swnp1A:1;
        Uint32 protect_1A:1;
        Uint32 sense_1A:1;
        Uint32 dac_Vset_1:1;
        Uint32 dac_Vset_01:1;
        Uint32 dac_Iset_1:1;
        Uint32 dac_Iset_01:1;
        Uint32 BW1:1;
        Uint32 BW2:1;
        Uint32 BW3:1;
        Uint32 BW4:1;
        Uint32 IgainA1:1;
        Uint32 IgainA0:1;
        Uint32 Vdiv200:1;
        Uint32 Vdiv10:1;
        Uint32 Vgain10:1;
        Uint32 VgainA1:1;
        Uint32 VgainA0:1;
        Uint32 guarden:1;
        Uint32 outen:1;
        Uint32 swo:1;
        Uint32 swf:1;
        Uint32 snscontrol:1;
    }bit;
}CMD_TO_FPGA_DATA1;

extern CMD_TO_FPGA_DATA1 un_cmd_data1;



typedef union
{
    Uint32 word;
    struct
    {
        Uint32 lowIsense:1;
        Uint32 afbcontrol:1;
        Uint32 afbsample:1;
        Uint32 afbreset:1;
        Uint32 afbaddlb:1;

        Uint32 control_Ven:1;
        Uint32 control_Ien:1;
        Uint32 priority:1;
        Uint32 polarity:1;
        Uint32 adc_trigger:1;

        Uint32 fastSW:3;


        Uint32 reserve:19;
    }bit;
}CMD_TO_FPGA_DATA2;

extern CMD_TO_FPGA_DATA2 un_cmd_data2;



typedef struct
{
    union
    {
        Uint32 word;
        struct
        {
            Uint32 dac2:16;
            Uint32 dac1:16;
        }bit;
    }dac1dac2;


    union
    {
        Uint32 word;
        struct
        {
            Uint32 interval:16;
            Uint32 sample:16;
        }bit;
    }sampinterval;

    union
    {
        Uint32 word;
        struct
        {
            Uint32 avmB:16;
            Uint32 avmA:16;
        }bit;
    }dacavmab;


}ST_DATA_TO_FPGA;

extern ST_DATA_TO_FPGA st_datatoFPGA;






typedef struct
{
    union
    {
        Uint32 word;
        struct
        {
            Uint32 version:8;
            Uint32 adcok:1;
            Uint32 reserve:23;
        }bit;
    }fpga_info;

    union
    {
        Uint32 word;
        struct
        {
            Uint32 cur_M:16;
            Uint32 cur_H:16;
        }bit;
    }mdata1;

    union
    {
        Uint32 word;
        struct
        {
            Uint32 vol_H:16;
            Uint32 cur_L:16;
        }bit;
    }mdata2;

    union
    {
        Uint32 word;
        struct
        {
            Uint32 vol_L:16;
            Uint32 vol_M:16;
        }bit;
    }mdata3;


    union
    {
        Uint32 word;
        struct
        {
            Uint32 data:23;
            Uint32 sign:1;
            Uint32 reserve:8;
        }bit;
    }adc24_V;

    union
    {
        Uint32 word;
        struct
        {
            Uint32 data:23;
            Uint32 sign:1;
            Uint32 reserve:8;
        }bit;
    }adc24_I;
}ST_FROM_FPGA;
extern ST_FROM_FPGA st_from_fpga;





extern void Init_ComFPGA(void);
extern void Run_ComFPGA(void);





#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
