#include "task_ComFPGA.h"
#include <string.h>
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "drv_McBSP.h"
#include "drv_MgModbus.h"
#include "task_ADC.h"
#include "test_hh.h"
#include "task_Range.h"
#include "Version.h"

CMD_TO_FPGA_DATA1 un_cmd_data1;
CMD_TO_FPGA_DATA2 un_cmd_data2;
ST_FROM_FPGA st_from_fpga;
ST_DATA_TO_FPGA st_datatoFPGA;

void Init_ComFPGA(void)
{
    memset(&un_cmd_data1, 0, sizeof(CMD_TO_FPGA_DATA1));
    memset(&un_cmd_data2, 0, sizeof(CMD_TO_FPGA_DATA2));
    memset(&st_from_fpga, 0, sizeof(ST_FROM_FPGA));
    memset(&st_datatoFPGA, 0, sizeof(ST_DATA_TO_FPGA));
}


Uint16 hh_test_err = 0;
void Run_ComFPGA(void)
{

    static Uint16 firstflag = 0, com_cnt = 0;
    //超时纠错
    if(firstflag == 0)
    {
        firstflag = 1;
    }
    {
        if(st_from_fpga.fpga_info.bit.version > 30 ||
                st_from_fpga.fpga_info.bit.version < 20)
        {
            com_cnt++;

            if(com_cnt > Tms_CNT(10))
            {
                com_cnt = 0;
            }
            else
            {
                hh_test_err++;
                return;
            }
        }
    }


    //fpga - > dsp
    st_from_fpga.fpga_info.word = ReadFPGA32(DATAFROMFPGA_INFO);
    st_from_fpga.mdata1.word = ReadFPGA32(DATAFROMFPGA_MDATA1);
    st_from_fpga.mdata2.word = ReadFPGA32(DATAFROMFPGA_MDATA2);
    st_from_fpga.mdata3.word = ReadFPGA32(DATAFROMFPGA_MDATA3);
    st_from_fpga.adc24_V.word = ReadFPGA32(DATAFROMFPGA_VOL);
    st_from_fpga.adc24_I.word = ReadFPGA32(DATAFROMFPGA_CUR);



    //adc采样   有符号转化为float
    if(st_from_fpga.adc24_V.bit.sign == 1)
    {
        mgmd_stSCIRx.f32DataVolRealT_5v = -(float32)((~st_from_fpga.adc24_V.bit.data + 1)&0x7FFFFF) / 8388608 * 5;
    }
    else
    {
        mgmd_stSCIRx.f32DataVolRealT_5v = (float32)(st_from_fpga.adc24_V.bit.data) / 8388608 * 5;
    }

    if(st_from_fpga.adc24_I.bit.sign == 1)
    {
        mgmd_stSCIRx.f32DataCurRealT_5v = -(float32)((~st_from_fpga.adc24_I.bit.data + 1)&0x7FFFFF) / 8388608 * 5;
    }
    else
    {
        mgmd_stSCIRx.f32DataCurRealT_5v = (float32)(st_from_fpga.adc24_I.bit.data) / 8388608 * 5;
    }
	
	
    //dsp -> fpga
    un_cmd_data1.bit.range_1uA = st_currangeset.range.bit.range1uA;
    un_cmd_data1.bit.range_10uA = st_currangeset.range.bit.range10uA;
    un_cmd_data1.bit.range_100uA = st_currangeset.range.bit.range100uA;
    un_cmd_data1.bit.range_1mA = st_currangeset.range.bit.range1mA;
    un_cmd_data1.bit.range_10mA = st_currangeset.range.bit.range10mA;
    un_cmd_data1.bit.range_100mA = st_currangeset.range.bit.range100mA;
    un_cmd_data1.bit.range_1A = st_currangeset.range.bit.range1A;
    un_cmd_data1.bit.range_100nA = st_currangeset.range.bit.range100nA;
    un_cmd_data1.bit.swp1A = st_currangeset.swP_1A;
    un_cmd_data1.bit.swnp1A = st_currangeset.swN_1A;
    un_cmd_data1.bit.protect_1A = st_currangeset.protect_1A;
    un_cmd_data1.bit.sense_1A = st_currangeset.sense_1A;
    un_cmd_data1.bit.dac_Vset_1 = st_volrangeset.range.bit.dac1;
    un_cmd_data1.bit.dac_Vset_01 = st_volrangeset.range.bit.dac01;
    un_cmd_data1.bit.dac_Iset_1 = st_currangeset.range.bit.dac1;
    un_cmd_data1.bit.dac_Iset_01 = st_currangeset.range.bit.dac01;
    un_cmd_data1.bit.BW1 = mgmd_stSCIRx.CMD_ControlLoop_BW1_n;
    un_cmd_data1.bit.BW2 = mgmd_stSCIRx.CMD_ControlLoop_BW2_n;
    un_cmd_data1.bit.BW3 = mgmd_stSCIRx.CMD_ControlLoop_BW3_n;
    un_cmd_data1.bit.BW4 = mgmd_stSCIRx.CMD_ControlLoop_BW4_n;
    un_cmd_data1.bit.IgainA1 = st_currangeset.gainA1;
    un_cmd_data1.bit.IgainA0 = st_currangeset.gainA0;
    un_cmd_data1.bit.Vdiv200 = st_volrangeset.range.bit.div200;
    un_cmd_data1.bit.Vdiv10 = st_volrangeset.range.bit.div10;
    un_cmd_data1.bit.Vgain10 = st_volrangeset.range.bit.gain10;

    un_cmd_data1.bit.VgainA1 = st_volrangeset.gainA1;
    un_cmd_data1.bit.VgainA0 = st_volrangeset.gainA0;
//    //---test---
//    un_cmd_data1.bit.VgainA1 = mgmd_stSCIRx.CMD_VGAINA1;
//    un_cmd_data1.bit.VgainA0 = mgmd_stSCIRx.CMD_VGAINA0;


    un_cmd_data1.bit.guarden = mgmd_stSCIRx.CMD_GUARDEN;
    un_cmd_data1.bit.outen = mgmd_stSCIRx.CMD_OUT_EN1;
    un_cmd_data1.bit.swo = mgmd_stSCIRx.CMD_SW_O;
    un_cmd_data1.bit.swf = mgmd_stSCIRx.CMD_SW_F;
    un_cmd_data1.bit.snscontrol = mgmd_stSCIRx.CMD_SNSCONTROL_n;

    un_cmd_data2.bit.lowIsense = st_currangeset.range.bit.lowIsense_n;
    un_cmd_data2.bit.afbcontrol = mgmd_stSCIRx.CMD_AFBCONTROL;
    un_cmd_data2.bit.afbsample = mgmd_stSCIRx.CMD_AFB_SAMPLE_N;
    un_cmd_data2.bit.afbreset = mgmd_stSCIRx.CMD_AFBRESET;
    un_cmd_data2.bit.afbaddlb = mgmd_stSCIRx.CMD_AFB_ADDLB_N;
    un_cmd_data2.bit.control_Ven = mgmd_stSCIRx.CMD_Control_VEN;
    un_cmd_data2.bit.control_Ien = mgmd_stSCIRx.CMD_Control_IEN;
    un_cmd_data2.bit.priority = mgmd_stSCIRx.CMD_PRIORITY;
    un_cmd_data2.bit.polarity = mgmd_stSCIRx.CMD_POLARITY;
    //un_cmd_data2.bit.Vdac_update = ;
    //un_cmd_data2.bit.Idac_update = ;
    //un_cmd_data2.bit.adc_trigger =;
    //un_cmd_data2.bit.avmadac_update = ;


//    st_datatoFPGA.dac1dac2.bit.dac1
//    st_datatoFPGA.dac1dac2.bit.dac2

    st_datatoFPGA.sampinterval.bit.sample = st_ADCdata.sample_num;
    st_datatoFPGA.sampinterval.bit.interval = st_ADCdata.interval_cnt;

//    st_datatoFPGA.dacavmab.bit.avmA
//    st_datatoFPGA.dacavmab.bit.avmB




    WriteFPGA32(DATATOFPGA_DAC1DAC2, st_datatoFPGA.dac1dac2.word);
    WriteFPGA32(DATATOFPGA_SAMPINTERVAL, st_datatoFPGA.sampinterval.word);
    WriteFPGA32(DATATOFPGA_DACAVMAB, st_datatoFPGA.dacavmab.word);
    WriteFPGA32(DATATOFPGA_CMDDATA1, un_cmd_data1.word);
    WriteFPGA32(DATATOFPGA_CMDDATA2, un_cmd_data2.word);


    if(st_volrangeset.range.bit.en235P == 1)        EN235P_H;
    else if(st_volrangeset.range.bit.en235P == 0)   EN235P_L;

    if(st_volrangeset.range.bit.en235N == 1)        EN235N_H;
    else if(st_volrangeset.range.bit.en235N == 0)   EN235N_L;

}







