
#include "drv_GlobalVar.h"

#if DSP
/*========================= Head file in DSP============================*/
#include "DSP2833x_Device.h"   // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h" // DSP2833x Examples Include File
/*======================================================================*/
#endif
#include <math.h>
#include "drv_Adc.h"
#include "drv_Eeprom.h"
#include "drv_fpga.h"
#include "drv_MgModbus.h"
#include "task_ADC.h"
#include "test_hh.h"
#include "task_Transfer.h"
#include <string.h>
#include "task_DAC.h"
#include "task_Range.h"
#include "drv_McBSP.h"
#include "Version.h"
#include "task_ComFPGA.h"
#include "task_PowerOnOff.h"
#include "task_wave.h"

extern Uint16 slINT100ms;
extern Uint16 slINT1s;
ST_VOLRANGESET st_volrangeset;
ST_CURRANGESET st_currangeset;

extern void PowerOn_Vth(void);

void HH_test_main()
{
}

void HH_test_INT(void)
{

    // 判断FPGA是否运行，并对其进行一些初始化操作
    InitFpga();

    // float类型转化
    ModbusParaTransfer(&mgmd_stSCIRx);

    // 下发参数限值
    ModbusParaLimit(&mgmd_stSCIRx);

    // 输出模式选择
    OutModeSel(mgmd_stSCIRx.Outmode, mgmd_stSCIRx.f32SetCur, mgmd_stSCIRx.f32SetVol);

    // st_rangestate.limitflag = ;   //根据采样值来判断
    st_rangestate.autorange = mgmd_stSCIRx.autorange;
    st_rangestate.set_vol = mgmd_stSCIRx.SetRangeVol_index;
    st_rangestate.set_cur = mgmd_stSCIRx.SetRangeCur_index;

    RangeSwitch(mgmd_stSCIRx.Outmode, &st_rangestate, 0);

    if (mgmd_stSCIRx.highCmode == 1)
    {
        // 打开high C mode  最后一个 0是打开
        mgmd_stSCIRx.CMD_ControlLoop_BW1_n = 0;
        //        mgmd_stSCIRx.CMD_SW_F = 1;
    }
    else
    {
        // 关闭high C mode  最后一个 1是关闭
        mgmd_stSCIRx.CMD_ControlLoop_BW1_n = 1;
        //        mgmd_stSCIRx.CMD_SW_F = 0;
    }

    // fpga 版本大于 21 才支持波形上传功能
    if (st_version.FPGAversion >= 21 && mgmd_stSCIRx.f32PluseWidth > 0)
        TaskWaveUpload((mgmd_stSCIRx.f32PluseWidth + 0.002) * 1000);

    // 放在切档后边
    PowerON();
    PowerON_RangeSW();
    PowerON_Can();
    PowerOn_Vth();
    PowerOn_Vth_DoubleSmu();
    PowerOn_Vth_8pre();
    PowerOff();

    if (mgmd_stSCIRx.test_trigger_mode == 1)
    {
        if (mgmd_stSCIRx.test_set_out == 1)
            TO_FPGA_TRIGGER_OUT_H;
        else
            TO_FPGA_TRIGGER_OUT_L;
    }
    mgmd_stSCIRx.test_read_in = FROM_FPGA_TRIGGER_IN;
}
