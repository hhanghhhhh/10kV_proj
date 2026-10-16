#include "drv_GlobalVar.h"
#include "drv_MgModbus.h"
#include "task_ADC.h"
#include "test_hh.h"
#include "task_DAC.h"
#include "Version.h"
#include "task_ComFPGA.h"
#include "task_PowerOnOff.h"
#include "task_Range.h"
#include "task_wave.h"

#define ABS(x) (((x) > 0) ? (x) : (-x))

/* 从CAN发送完成起计时，允许采样板完成统计后回复。 */
#define POWER_ON_CAN_REPLY_TIMEOUT_MS (1000UL)

extern void MdF32ToU16(float32 fdata, Uint16 *pdataH, Uint16 *pdataL);
Uint16 poweronflag = 0;
Uint16 hh_test_on[2] = {0, 0};

Uint16 last_vol_range = 0;
Uint16 last_cur_range = 0;
float32 vth_forward_samp = 0;

float32 AutoRangeVol(float32 vol)
{
    float32 range = 0;

    if (ABS(vol) < 0.02001)
        range = RANGE_VOL_20mV;
    else if (ABS(vol) < 0.2001)
        range = RANGE_VOL_200mV;
    else if (ABS(vol) < 2.001)
        range = RANGE_VOL_2V;
    else if (ABS(vol) < 35.001)
        range = RANGE_VOL_20V;
    else
        range = RANGE_VOL_200V;

    return range;
}

float32 AutoRangeCur(float32 cur)
{
    float32 range = 0;

    if (ABS(cur) < 1.001e-9)
        range = RANGE_CUR_1nA;
    else if (ABS(cur) < 1.001e-8)
        range = RANGE_CUR_10nA;
    else if (ABS(cur) < 1.001e-7)
        range = RANGE_CUR_100nA;
    else if (ABS(cur) < 1.001e-6)
        range = RANGE_CUR_1uA;
    else if (ABS(cur) < 1.001e-5)
        range = RANGE_CUR_10uA;
    else if (ABS(cur) < 1.001e-4)
        range = RANGE_CUR_100uA;
    else if (ABS(cur) < 1.001e-3)
        range = RANGE_CUR_1mA;
    else if (ABS(cur) < 1.001e-2)
        range = RANGE_CUR_10mA;
    else if (ABS(cur) < 1.001e-1)
        range = RANGE_CUR_100mA;
    else
        range = RANGE_CUR_2A;

    return range;
}

void VthAutoRange(void)
{
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    pPC->vth_volrange1.f32 = AutoRangeVol(pPC->vth_setvol1.f32);
    pPC->vth_currange1.f32 = AutoRangeCur(pPC->vth_setcur1.f32);
    pPC->vth_volrange2.f32 = AutoRangeVol(pPC->vth_setvol2.f32);
    pPC->vth_currange2.f32 = AutoRangeCur(pPC->vth_setcur2.f32);
}

void PowerOn_Vth(void)
{
    static Uint16 state = 0;
    static Uint32 overtime_cnt = 0;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    if (pPC->vth_poweron == 1)
    {
        pPC->vth_poweron = 0;

        // 清除变量
        TO_FPGA_TRIGGER_OUT_L;
        pPC->vth_ok = 0;
        pPC->vth_volsamp1.f32 = 0;
        pPC->vth_cursamp1.f32 = 0;
        pPC->vth_volsamp2.f32 = 0;
        pPC->vth_cursamp2.f32 = 0;
        vth_forward_samp = 0;
        overtime_cnt = 0;
        // 状态跳转
        state = 1;
    }

    switch (state)
    {
    case 0:
        break;

    case 1:
        // precondition, 四线，采样0.2ms
        pPC->vth_nplc1.f32 = 0.01; // 0.2ms
        VthAutoRange();

        pPC->Outmode = OUTMODE_V;
        // 已经放在寄存器那边实时赋值了
        //        pPC->CMD_SNSCONTROL_n   = pPC->vth_four_two;    //四线
        pPC->Autosample = 1;
        pPC->highCmode = 0;
        pPC->u16SetVolH = pPC->vth_setvol1.u16.H;
        pPC->u16SetVolL = pPC->vth_setvol1.u16.L;
        pPC->u16SetCurH = pPC->vth_setcur1.u16.H;
        pPC->u16SetCurL = pPC->vth_setcur1.u16.L;
        pPC->SetRangeVolH = pPC->vth_volrange1.u16.H;
        pPC->SetRangeVolL = pPC->vth_volrange1.u16.L;
        pPC->SetRangeCurH = pPC->vth_currange1.u16.H;
        pPC->SetRangeCurL = pPC->vth_currange1.u16.L;
        pPC->PluseWidthH = pPC->vth_plusewidth1.u16.H;
        pPC->PluseWidthL = pPC->vth_plusewidth1.u16.L;
        pPC->nPLCH = pPC->vth_nplc1.u16.H;
        pPC->nPLCL = pPC->vth_nplc1.u16.L;
        vth_forward_samp = 0;
        pPC->POWER_CTRL = 1; // 上电

        // 清除变量
        st_ADCdata.dataOK = 0;
        // 状态跳转
        state = 2;
        break;

    case 2:
        overtime_cnt++;
        if (overtime_cnt > Tms_CNT(1000))
        {
            // 清除变量、记录错误码
            TO_FPGA_TRIGGER_OUT_L;
            pPC->vth_ok = 2;
            overtime_cnt = 0;
            state = 0;
        }

        // 等待precondition下电
        if (st_ADCdata.dataOK == 1)
        {
            // 数据记录
            pPC->vth_volsamp1.f32 = pPC->f32DataVol;
            pPC->vth_cursamp1.f32 = pPC->f32DataCur;
            // 发trigger out
            TO_FPGA_TRIGGER_OUT_H;
            overtime_cnt = 0;
            // 状态跳转
            state = 3;
        }
        break;

    case 3:
        overtime_cnt++;
        if (overtime_cnt > Tms_CNT(300))
        {
            // 清除变量、记录错误码
            TO_FPGA_TRIGGER_OUT_L;
            pPC->vth_ok = 3;
            overtime_cnt = 0;
            state = 0;
        }

        // 等待trigger in且下电完成
        if (FROM_FPGA_TRIGGER_IN == 1 && pPC->POWER_CTRL == 0)
        {
            // 拉低trigger out
            TO_FPGA_TRIGGER_OUT_L;
            // vgsth测试, 四线，采样0.2ms
            VthAutoRange();

            pPC->Outmode = OUTMODE_V;
            // 已经放在寄存器那边实时赋值了
            //            pPC->CMD_SNSCONTROL_n   = pPC->vth_four_two;    //四线
            pPC->Autosample = 1;
            pPC->highCmode = 0;
            pPC->u16SetVolH = pPC->vth_setvol2.u16.H;
            pPC->u16SetVolL = pPC->vth_setvol2.u16.L;
            pPC->u16SetCurH = pPC->vth_setcur2.u16.H;
            pPC->u16SetCurL = pPC->vth_setcur2.u16.L;
            pPC->SetRangeVolH = pPC->vth_volrange2.u16.H;
            pPC->SetRangeVolL = pPC->vth_volrange2.u16.L;
            pPC->SetRangeCurH = pPC->vth_currange2.u16.H;
            pPC->SetRangeCurL = pPC->vth_currange2.u16.L;
            pPC->PluseWidthH = pPC->vth_plusewidth2.u16.H;
            pPC->PluseWidthL = pPC->vth_plusewidth2.u16.L;
            pPC->nPLCH = pPC->vth_nplc2.u16.H;
            pPC->nPLCL = pPC->vth_nplc2.u16.L;
            vth_forward_samp = pPC->vth_forward_samp.f32; // 测vth时采样时刻前推x s
            pPC->POWER_CTRL = 1;                          // 上电

            // 清除变量
            st_ADCdata.dataOK = 0;
            // 状态跳转
            state = 4;
        }
        break;

    case 4:
        // 等待vgsth测试ok
        if (st_ADCdata.dataOK == 1)
        {
            // 数据记录
            pPC->vth_volsamp2.f32 = pPC->f32DataVol;
            pPC->vth_cursamp2.f32 = pPC->f32DataCur;
            pPC->vth_ok = 1;

            vth_forward_samp = 0; // 清零变量
            // 状态跳转
            state = 0;
        }
        break;

    default:
        state = 0;
    }
}

#define VTH_UP 0
#define VTH_DOWN 1
#define VTH_GS 0
#define VTH_DS 1

void PowerOn_Vth_DoubleSmu_Ds(void)
{
    static MODBUS_DATA tmp_nplc;
    static MODBUS_DATA tmp_plusewidth;
    static MODBUS_DATA tmp_volrange;
    static MODBUS_DATA tmp_currange;
    static Uint16 state = 0;
    static Uint32 overtime_cnt = 0;
    static Uint16 firstflag = 0;
    static Uint32 nowtime = 0;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
    VTH_DOUBLESMU *pVth = &mgmd_stSCIRx.vth_dbsmu;

    pVth->test_state = state;

    // 下电
    if (pVth->poweron == 2)
    {
        pVth->poweron = 0;
        // 下电
        pPC->POWER_CTRL = 2;
        // 清除变量
        TO_FPGA_TRIGGER_OUT_L;
        state = 0;
    }

    if (pVth->poweron == 1)
    {
        pVth->poweron = 0;

        // 清除变量
        TO_FPGA_TRIGGER_OUT_L;
        pVth->ok = 0;
        pVth->ds.vsamp.f32 = 0;
        pVth->ds.isamp.f32 = 0;
        vth_forward_samp = 0;
        overtime_cnt = 0;
        // 状态跳转
        state = 1;
    }

    switch (state)
    {
    case 0:
        break;

    case 1:
        // 等待开始上电
        if (FROM_FPGA_TRIGGER_IN == 1)
        {
            tmp_nplc.f32 = 0.01; // 0.2ms
            tmp_plusewidth.f32 = -1;
            tmp_volrange.f32 = AutoRangeVol(pVth->ds.vset.f32);
            tmp_currange.f32 = AutoRangeCur(pVth->ds.iset.f32);

            pPC->Outmode = OUTMODE_V;
            // 已经放在寄存器那边实时赋值了
            //            pPC->CMD_SNSCONTROL_n   = pVth->ds.four_or_two;    //四线
            pPC->Autosample = 0;
            pPC->highCmode = 0;
            pPC->u16SetVolH = pVth->ds.vset.u16.H;
            pPC->u16SetVolL = pVth->ds.vset.u16.L;
            pPC->u16SetCurH = pVth->ds.iset.u16.H;
            pPC->u16SetCurL = pVth->ds.iset.u16.L;
            pPC->SetRangeVolH = tmp_volrange.u16.H;
            pPC->SetRangeVolL = tmp_volrange.u16.L;
            pPC->SetRangeCurH = tmp_currange.u16.H;
            pPC->SetRangeCurL = tmp_currange.u16.L;
            pPC->PluseWidthH = tmp_plusewidth.u16.H;
            pPC->PluseWidthL = tmp_plusewidth.u16.L;
            pPC->nPLCH = tmp_nplc.u16.H;
            pPC->nPLCL = tmp_nplc.u16.L;
            pPC->POWER_CTRL = 1; // 上电

            // 清除变量
            st_ADCdata.dataOK = 0;
            // 状态跳转
            state = 2;
        }
        break;

    case 2:
        // 等待 delta t
        if (firstflag == 0)
        {
            firstflag = 1;
            nowtime = task_run_cnt;
        }
        if ((task_run_cnt - nowtime) > Tms_CNT(pVth->ds.delayms.f32))
        {
            firstflag = 0;
            state = 3;
        }
        break;

    case 3:
        overtime_cnt++;
        if (overtime_cnt > Tms_CNT(10000))
        {
            // 清除变量、记录错误码
            TO_FPGA_TRIGGER_OUT_L;
            pVth->ok = VTH_RST_OVER_TIME;
            // 下电
            pPC->POWER_CTRL = 2;
            state = 0;
        }

        if (pVth->up_or_down == VTH_DOWN)
        {
            // ds 从 0 变到最大值，等待达到阈值
            if (ABS(pPC->f32DataVolRealT) > ABS(pVth->ds.vth.f32))
            {
                // 给主控板发信号
                TO_FPGA_TRIGGER_OUT_H;
                // 记录数据
                pVth->ds.vsamp.f32 = pPC->f32DataVolRealT;
                pVth->ds.isamp.f32 = pPC->f32DataCurRealT;
                pVth->ok = VTH_RST_OK;
                // 下电
                pPC->POWER_CTRL = 2;
                state = 4;
            }
        }
        else
        {
            // ds 从最大值变到 0，等待达到阈值
            if (ABS(pPC->f32DataVolRealT) < ABS(pVth->ds.vth.f32))
            {
                // 给主控板发信号
                TO_FPGA_TRIGGER_OUT_H;
                // 记录数据
                pVth->ds.vsamp.f32 = pPC->f32DataVolRealT;
                pVth->ds.isamp.f32 = pPC->f32DataCurRealT;
                pVth->ok = VTH_RST_OK;
                // 下电
                pPC->POWER_CTRL = 2;
                state = 4;
            }
        }
        break;

    case 4:
        if (FROM_FPGA_TRIGGER_IN == 0)
        {
            // 清除信号
            TO_FPGA_TRIGGER_OUT_L;
            state = 0;
        }
        break;
    }
}
// 双 smu 测试方法
void PowerOn_Vth_DoubleSmu_Gs(void)
{
    static MODBUS_DATA tmp_nplc;
    static MODBUS_DATA tmp_plusewidth;
    static MODBUS_DATA tmp_volrange;
    static MODBUS_DATA tmp_currange;
    static MODBUS_DATA tmp_setvol;
    static Uint16 state = VTH_IDEL;
    static Uint32 overtime_cnt = 0;
    static Uint16 firstflag = 0;
    static Uint32 nowtime = 0;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
    VTH_DOUBLESMU *pVth = &mgmd_stSCIRx.vth_dbsmu;

    pVth->test_state = state;

    // 下电
    if (pVth->poweron == 2)
    {
        pVth->poweron = 0;
        // 下电
        pPC->POWER_CTRL = 2;
        // 清除变量
        TO_FPGA_TRIGGER_OUT_L;
        state = 0;
    }

    if (pVth->poweron == 1)
    {
        pVth->poweron = 0;

        // 清除变量
        TO_FPGA_TRIGGER_OUT_L;
        pVth->ok = 0;
        pVth->gs.pre.vsamp.f32 = 0;
        pVth->gs.pre.isamp.f32 = 0;
        pVth->gs.step.vsamp.f32 = 0;
        pVth->gs.step.isamp.f32 = 0;
        vth_forward_samp = 0;
        overtime_cnt = 0;
        firstflag = 0;
        // 状态跳转
        state = VTH_PRE_PARA;
    }

    switch (state)
    {
    case VTH_IDEL:
        break;

    case VTH_PRE_PARA:
        // precondition，采样0.2ms
        tmp_nplc.f32 = 0.01; // 0.2ms
        tmp_volrange.f32 = AutoRangeVol(pVth->gs.pre.vset.f32);
        tmp_currange.f32 = AutoRangeCur(pVth->gs.pre.iset.f32);

        pPC->Outmode = OUTMODE_V;
        // 已经放在寄存器那边实时赋值了
        //        pPC->CMD_SNSCONTROL_n   = pVth->gs.four_or_two;    //四线
        pPC->Autosample = 1;
        pPC->highCmode = 0;
        pPC->u16SetVolH = pVth->gs.pre.vset.u16.H;
        pPC->u16SetVolL = pVth->gs.pre.vset.u16.L;
        pPC->u16SetCurH = pVth->gs.pre.iset.u16.H;
        pPC->u16SetCurL = pVth->gs.pre.iset.u16.L;
        pPC->SetRangeVolH = tmp_volrange.u16.H;
        pPC->SetRangeVolL = tmp_volrange.u16.L;
        pPC->SetRangeCurH = tmp_currange.u16.H;
        pPC->SetRangeCurL = tmp_currange.u16.L;
        pPC->PluseWidthH = pVth->gs.pre.plusewidth.u16.H;
        pPC->PluseWidthL = pVth->gs.pre.plusewidth.u16.L;
        pPC->nPLCH = tmp_nplc.u16.H;
        pPC->nPLCL = tmp_nplc.u16.L;
        pPC->POWER_CTRL = 1; // 上电

        // 清除变量
        st_ADCdata.dataOK = 0;
        // 状态跳转
        state = VTH_PRE_OK;
        break;

    case VTH_PRE_OK:
        // 等待precondition下电
        if (st_ADCdata.dataOK == 1)
        {
            // 数据记录
            pVth->gs.pre.vsamp.f32 = pPC->f32DataVol;
            pVth->gs.pre.isamp.f32 = pPC->f32DataCur;
            // 发trigger out
            TO_FPGA_TRIGGER_OUT_H;
            // 状态跳转
            overtime_cnt = 0;
            state = VTH_WAIT_STEP;
        }
        break;

    case VTH_WAIT_STEP:
        // floating time
        overtime_cnt++;
        if (overtime_cnt > Tms_CNT(1000))
        {
            // 清除变量、记录错误码
            TO_FPGA_TRIGGER_OUT_L;
            pVth->ok = VTH_RST_OVER_TIME;
            state = VTH_IDEL;
        }

        // 等待trigger in且下电完成
        if (FROM_FPGA_TRIGGER_IN == 1 && pPC->POWER_CTRL == 0)
        {
            // 拉低trigger out
            TO_FPGA_TRIGGER_OUT_L;
            // vgsth测试，脉宽-1
            tmp_volrange.f32 = AutoRangeVol(pVth->gs.step.vmax.f32);
            tmp_currange.f32 = AutoRangeCur(pVth->gs.step.iset.f32);
            tmp_plusewidth.f32 = -1;
            if (pVth->up_or_down == VTH_DOWN)
            {
                tmp_setvol.f32 = pVth->gs.step.vmax.f32;
            }
            else
            {
                tmp_setvol.f32 = pVth->gs.step.vmin.f32;
            }

            pPC->Outmode = OUTMODE_V;
            // 已经放在寄存器那边实时赋值了
            //            pPC->CMD_SNSCONTROL_n   = pVth->gs.four_or_two;
            pPC->Autosample = 0;
            pPC->highCmode = 0;
            pPC->u16SetVolH = tmp_setvol.u16.H;
            pPC->u16SetVolL = tmp_setvol.u16.L;
            pPC->u16SetCurH = pVth->gs.step.iset.u16.H;
            pPC->u16SetCurL = pVth->gs.step.iset.u16.L;
            pPC->SetRangeVolH = tmp_volrange.u16.H;
            pPC->SetRangeVolL = tmp_volrange.u16.L;
            pPC->SetRangeCurH = tmp_currange.u16.H;
            pPC->SetRangeCurL = tmp_currange.u16.L;
            pPC->PluseWidthH = tmp_plusewidth.u16.H;
            pPC->PluseWidthL = tmp_plusewidth.u16.L;
            pPC->nPLCH = tmp_nplc.u16.H;
            pPC->nPLCL = tmp_nplc.u16.L;
            pPC->POWER_CTRL = 1; // 上电

            // 清除变量
            st_ADCdata.dataOK = 0;
            // 状态跳转
            state = VTH_STEP_DELAY;
        }
        break;

    case VTH_STEP_DELAY:
        // 等待 delta t
        if (firstflag == 0)
        {
            firstflag = 1;
            nowtime = task_run_cnt;
        }
        if ((task_run_cnt - nowtime) > Tms_CNT(pVth->gs.step.delta_t.f32 * 1000))
        {
            firstflag = 0;
            state = VTH_STEP_CHECK;
        }
        break;

    case VTH_STEP_CHECK:
        // 判断是否  且上电完成
        if (mgmd_stSCIRx.POWER_CTRL == 0)
        {
            if (FROM_FPGA_TRIGGER_IN == 1)
            {
                // 达到稳定
                // 记录当前实时值
                pVth->gs.step.vsamp.f32 = pPC->f32DataVolRealT;
                pVth->gs.step.isamp.f32 = pPC->f32DataCurRealT;
                pVth->ok = VTH_RST_OK;
                // 下电
                pPC->POWER_CTRL = 2;
                // 告诉主控板
                TO_FPGA_TRIGGER_OUT_H;

                state = VTH_STEP_OK;
            }
            else
            {
                // 电压继续步进改变
                if (pVth->up_or_down == VTH_DOWN)
                {
                    if (tmp_setvol.f32 > pVth->gs.step.vmin.f32)
                    {
                        tmp_setvol.f32 -= pVth->gs.step.delta_v.f32;
                        // 改变设置值
                        st_VDACpara.set = ABS(tmp_setvol.f32);
                        DACCalculate(&st_VDACpara, &st_IDACpara);
                        UPDATE_VREF(st_VDACpara.AD)
                        state = VTH_STEP_DELAY;
                    }
                    else
                    {
                        // 达到最小值还没触发，报错、下电
                        pVth->ok = VTH_RST_OVER_MIN;
                        pPC->POWER_CTRL = 2;
                        state = VTH_IDEL;
                    }
                }
                else
                {
                    if (tmp_setvol.f32 < pVth->gs.step.vmax.f32)
                    {
                        tmp_setvol.f32 += pVth->gs.step.delta_v.f32;
                        // 改变设置值
                        st_VDACpara.set = ABS(tmp_setvol.f32);
                        DACCalculate(&st_VDACpara, &st_IDACpara);
                        UPDATE_VREF(st_VDACpara.AD)
                        state = VTH_STEP_DELAY;
                    }
                    else
                    {
                        // 达到最大值还没触发，报错
                        pVth->ok = VTH_RST_OVER_MAX;
                        pPC->POWER_CTRL = 2;
                        state = VTH_IDEL;
                    }
                }
            }
        }
        break;

    case VTH_STEP_OK:
        // 等待主控板收到
        if (FROM_FPGA_TRIGGER_IN == 0)
        {
            TO_FPGA_TRIGGER_OUT_L;
            state = VTH_IDEL;
        }
        break;

    default:
        state = VTH_IDEL;
    }
}

void PowerOn_Vth_DoubleSmu(void)
{
    VTH_DOUBLESMU *pVth = &mgmd_stSCIRx.vth_dbsmu;

    if (pVth->gs_or_ds == VTH_GS)
    {
        PowerOn_Vth_DoubleSmu_Gs();
    }
    else
    {
        PowerOn_Vth_DoubleSmu_Ds();
    }
}

// 八段precondition测试方法
void PowerOn_Vth_8pre(void)
{
    VTH_8PRE *pVth = &mgmd_stSCIRx.vth_8pre;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = 0;
    static Uint16 pre_num = 0;
    static MODBUS_DATA tmp_nplc;
    static MODBUS_DATA tmp_volrange;
    static MODBUS_DATA tmp_currange;
    static Uint32 overtime_cnt = 0;
    Uint16 i = 0;

    pVth->state = state;

    //    // ------ test-------
    //    if(pPC->vth_poweron == 1)
    //    {
    //        pPC->vth_poweron = 0;
    //        pPC->vth_ok = 0;
    //        pVth->poweron = 1;
    //    }
    //    pPC->vth_volsamp2.f32 = pVth->vth.vsamp.f32;
    //    pPC->vth_cursamp2.f32 = pVth->vth.isamp.f32;
    //    pPC->vth_ok = pVth->ok;
    //    // ------ test-------

    // 下电
    if (pVth->poweron == 2)
    {
        pVth->poweron = 0;
        // 下电
        pPC->POWER_CTRL = 2;
        state = 0;
    }

    if (pVth->poweron == 1)
    {
        pVth->poweron = 0;
        // 清除变量
        pVth->ok = 0;
        for (i = 0; i < 8; i++)
        {
            pVth->pre[i].vsamp.f32 = 0;
            pVth->pre[i].isamp.f32 = 0;
        }
        pVth->vth.vsamp.f32 = 0;
        pVth->vth.isamp.f32 = 0;
        pre_num = 0;
        overtime_cnt = 0;
        TO_FPGA_TRIGGER_OUT_L;
        // 状态跳转
        state = 1;
    }

    switch (state)
    {
    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case 0:
        break;

    case 1:
        if (pre_num < pVth->pre_num)
        {
            // precondition，采样0.2ms
            tmp_nplc.f32 = 0.01; // 0.2ms
            tmp_volrange.f32 = AutoRangeVol(pVth->pre[pre_num].vset.f32);
            tmp_currange.f32 = AutoRangeCur(pVth->pre[pre_num].iset.f32);

            pPC->Outmode = OUTMODE_V;
            // 已经放在寄存器那边实时赋值了
            //        pPC->CMD_SNSCONTROL_n   = pVth->gs.four_or_two;    //四线
            pPC->Autosample = 1;
            pPC->highCmode = 0;
            pPC->u16SetVolH = pVth->pre[pre_num].vset.u16.H;
            pPC->u16SetVolL = pVth->pre[pre_num].vset.u16.L;
            pPC->u16SetCurH = pVth->pre[pre_num].iset.u16.H;
            pPC->u16SetCurL = pVth->pre[pre_num].iset.u16.L;
            pPC->SetRangeVolH = tmp_volrange.u16.H;
            pPC->SetRangeVolL = tmp_volrange.u16.L;
            pPC->SetRangeCurH = tmp_currange.u16.H;
            pPC->SetRangeCurL = tmp_currange.u16.L;
            pPC->PluseWidthH = pVth->pre[pre_num].plusewidth.u16.H;
            pPC->PluseWidthL = pVth->pre[pre_num].plusewidth.u16.L;
            pPC->nPLCH = tmp_nplc.u16.H;
            pPC->nPLCL = tmp_nplc.u16.L;
            pPC->POWER_CTRL = 1; // 上电

            // 清除变量
            st_ADCdata.dataOK = 0;
            // 状态跳转，判断
            state = 2;
        }
        else
        {
            // 发trigger out
            TO_FPGA_TRIGGER_OUT_H;
            overtime_cnt = 0;
            // 状态跳转, vth
            state = 3;
        }
        break;

    case 2:
        // 等待precondition下电
        if (st_ADCdata.dataOK == 1)
        {
            // 数据记录
            pVth->pre[pre_num].vsamp.f32 = pPC->f32DataVol;
            pVth->pre[pre_num].isamp.f32 = pPC->f32DataCur;
            pre_num++;
            // 状态跳转，delay再判断
            FSM_DELAY_NEXT(st_delay, pVth->pre[pre_num - 1].delay.f32 * 1000, 1);
        }
        break;

    case 3:
        // floating time
        overtime_cnt++;
        if (overtime_cnt > Tms_CNT(1000))
        {
            // 清除变量、记录错误码
            TO_FPGA_TRIGGER_OUT_L;
            pVth->ok = 2;
            overtime_cnt = 0;
            state = 0;
        }

        // 等待trigger in且下电完成
        if (FROM_FPGA_TRIGGER_IN == 1 && pPC->POWER_CTRL == 0)
        {
            // 拉低trigger out
            TO_FPGA_TRIGGER_OUT_L;
            // vth
            tmp_volrange.f32 = AutoRangeVol(pVth->vth.vset.f32);
            tmp_currange.f32 = AutoRangeCur(pVth->vth.iset.f32);

            pPC->Outmode = OUTMODE_V;
            // 已经放在寄存器那边实时赋值了
            //        pPC->CMD_SNSCONTROL_n   = pVth->gs.four_or_two;    //四线
            pPC->Autosample = 1;
            pPC->highCmode = 0;
            pPC->u16SetVolH = pVth->vth.vset.u16.H;
            pPC->u16SetVolL = pVth->vth.vset.u16.L;
            pPC->u16SetCurH = pVth->vth.iset.u16.H;
            pPC->u16SetCurL = pVth->vth.iset.u16.L;
            pPC->SetRangeVolH = tmp_volrange.u16.H;
            pPC->SetRangeVolL = tmp_volrange.u16.L;
            pPC->SetRangeCurH = tmp_currange.u16.H;
            pPC->SetRangeCurL = tmp_currange.u16.L;
            pPC->PluseWidthH = pVth->vth.plusewidth.u16.H;
            pPC->PluseWidthL = pVth->vth.plusewidth.u16.L;
            pPC->nPLCH = pVth->vth.nplc.u16.H;
            pPC->nPLCL = pVth->vth.nplc.u16.L;
            pPC->POWER_CTRL = 1; // 上电

            // 清除变量
            st_ADCdata.dataOK = 0;
            // 状态跳转
            state = 4;
        }
        break;

    case 4:
        // 等待vth下电
        if (st_ADCdata.dataOK == 1)
        {
            // 数据记录
            pVth->vth.vsamp.f32 = pPC->f32DataVol;
            pVth->vth.isamp.f32 = pPC->f32DataCur;
            pVth->ok = 1;
            // 状态跳转
            state = 0;
        }
        break;
    }
}

void PowerON(void)
{
    static Uint16 state = 0;
    static Uint16 firstflag = 0;
    static Uint32 nowtime = 0;
    static int32 plusewidth = 0;

    //    if(poweronflag == 0)
    //    {
    //        state = PWON_IDEL;
    //    }

    if (mgmd_stSCIRx.POWER_CTRL == 1)
    {
        switch (state)
        {
        case PWON_IDEL:
            // 上电时先把dataok标志清零
            st_ADCdata.dataOK = 0;
            mgmd_stSCIRx.DataOKFlag = 0;
            poweronflag = 1;

            // 清零变量
            firstflag = 0;

            hh_test_on[0]++;
            state = PWON_RANGESW;
            break;

        case PWON_RANGESW:
            if (last_vol_range != mgmd_stSCIRx.SetRangeVol_index && (last_vol_range == RANGE_VOL_200V_INDEX || mgmd_stSCIRx.SetRangeVol_index == RANGE_VOL_200V_INDEX))
            {
                state = PWON_200DELAY;
            }
            else if (mgmd_stSCIRx.Outmode == OUTMODE_V_OFFVI) // 测 Rdson
            {
                state = PWON_200DELAY;
            }
            else
            {
                state = PWON_START;
            }
            break;

        case PWON_200DELAY:
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(10))
            {
                firstflag = 0;
                state = PWON_START;
            }
            break;

        case PWON_START:
            // 设置DAC
            st_VDACpara.set = mgmd_stSCIRx.f32SetVol_use;
            st_IDACpara.set = mgmd_stSCIRx.f32SetCur_use;
            st_VDACpara.range_index = mgmd_stSCIRx.SetRangeVol_index;
            st_IDACpara.range_index = mgmd_stSCIRx.SetRangeCur_index;
            DACCalculate(&st_VDACpara, &st_IDACpara);
            UPDATE_VREF(st_VDACpara.AD)
            UPDATE_IREF(st_IDACpara.AD)
            UPDATE_AVMAREF(65534, 65534)

            if (mgmd_stSCIRx.f32PluseWidth > 0)
            {
                if (mgmd_stSCIRx.Autosample == 1)
                {
                    // 需要采样时延时要减采样所需时间
                    plusewidth = (mgmd_stSCIRx.f32PluseWidth - mgmd_stSCIRx.f32nPLC * 0.02 - vth_forward_samp) * 1000 - 2;

                    // fpga 版本大于 21 才支持波形上传功能
                    if (st_version.FPGAversion >= 21)
                        EnableWave();
                }
                else
                {
                    plusewidth = mgmd_stSCIRx.f32PluseWidth * 1000 - 3;
                }

                if (plusewidth < 0)
                    plusewidth = 0;

                state = PWON_DELAY;
            }
            else
            {
                // 直接上电ok  等待
                state = PWON_ONOK;
            }

            break;

        case PWON_DELAY: // 延时
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(plusewidth))
            {
                firstflag = 0;
                state = PWON_SAMPLE;
            }
            break;

        case PWON_SAMPLE:
            if (mgmd_stSCIRx.Autosample == 1)
            {
                // 开启一次采样
                st_ADCdata.trigger = 1;
                st_ADCdata.dataOK = 0;
                mgmd_stSCIRx.DataOKFlag = 0;

                state = PWON_WAITOK;
            }
            else
            {
                // 不采样，延时后下电
                state = PWON_OFF;
            }
            break;

        case PWON_WAITOK:
            // 采样ok后下电
            if (st_ADCdata.dataOK == 1)
            {
                state = PWON_AFTER_SAMP_DLY;
            }
            break;

        case PWON_AFTER_SAMP_DLY:
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(vth_forward_samp * 1000))
            {
                firstflag = 0;
                state = PWON_OFF;
            }
            break;

        case PWON_OFF:
            // 下电
            mgmd_stSCIRx.POWER_CTRL = 2;
            break;

        case PWON_ONOK:
            // 上电ok
            mgmd_stSCIRx.POWER_CTRL = 0;
            break;

        default:
            state = PWON_IDEL;
        }
    }
    else
    {
        state = PWON_IDEL;
    }
}

void PowerON_RangeSW(void)
{
    static Uint16 state = 0;
    static Uint16 firstflag = 0;
    static Uint32 nowtime = 0;
    static int32 plusewidth = 0;
    static Uint16 record_setcurH = 0, record_setcurL = 0, record_rangecurH = 0, record_rangecurL = 0;

    //    if(poweronflag == 0)
    //    {
    //        state = PWON_IDEL;
    //    }

    if (mgmd_stSCIRx.POWER_CTRL == 3)
    {
        switch (state)
        {
        case PWON_IDEL:
            // 上电时先把dataok标志清零
            st_ADCdata.dataOK = 0;
            mgmd_stSCIRx.DataOKFlag = 0;
            poweronflag = 1;
            hh_test_on[1]++;
            state = PWON_RANGESW;
            break;

        case PWON_RANGESW:
            if (last_vol_range != mgmd_stSCIRx.SetRangeVol_index && (last_vol_range == RANGE_VOL_200V_INDEX || mgmd_stSCIRx.SetRangeVol_index == RANGE_VOL_200V_INDEX))
            {
                state = PWON_200DELAY;
            }
            else
            {
                state = PWON_START;
            }
            break;

        case PWON_200DELAY:
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(10))
            {
                firstflag = 0;
                state = PWON_START;
            }
            break;

        case PWON_START:
            // 记录上位机下发的档位值
            record_setcurH = mgmd_stSCIRx.u16SetCurH;
            record_setcurL = mgmd_stSCIRx.u16SetCurL;
            record_rangecurH = mgmd_stSCIRx.SetRangeCurH;
            record_rangecurL = mgmd_stSCIRx.SetRangeCurL;

            // 将电流、电流档位设置为1mA
            MdF32ToU16(mgmd_stSCIRx.pw3_initial_cur.f32, &mgmd_stSCIRx.u16SetCurH, &mgmd_stSCIRx.u16SetCurL);
            MdF32ToU16(mgmd_stSCIRx.pw3_initial_cur.f32, &mgmd_stSCIRx.SetRangeCurH, &mgmd_stSCIRx.SetRangeCurL);

            state = PWON_SETDAC;
            break;

        case PWON_SETDAC:
            // 设置DAC
            st_VDACpara.set = mgmd_stSCIRx.f32SetVol_use;
            st_IDACpara.set = mgmd_stSCIRx.f32SetCur_use;
            st_VDACpara.range_index = mgmd_stSCIRx.SetRangeVol_index;
            st_IDACpara.range_index = mgmd_stSCIRx.SetRangeCur_index;
            DACCalculate(&st_VDACpara, &st_IDACpara);
            UPDATE_VREF(st_VDACpara.AD)
            UPDATE_IREF(st_IDACpara.AD)
            UPDATE_AVMAREF(65534, 65534)

            // fpga 版本大于 21 才支持波形上传功能
            if (st_version.FPGAversion >= 21 && mgmd_stSCIRx.f32PluseWidth > 0)
                EnableWave();

            state = PWON_DELAY_CHARGE;
            break;

        case PWON_DELAY_CHARGE:
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(mgmd_stSCIRx.pw3_initial_delay_s.f32 * 1000))
            {
                firstflag = 0;
                state = PWON_SW_RANGE;
            }
            break;

        case PWON_SW_RANGE:
            // 将电流、电流档位设置为上位机下发值
            mgmd_stSCIRx.u16SetCurH = record_setcurH;
            mgmd_stSCIRx.u16SetCurL = record_setcurL;
            mgmd_stSCIRx.SetRangeCurH = record_rangecurH;
            mgmd_stSCIRx.SetRangeCurL = record_rangecurL;

            state = PWON_RE_SETDAC;
            break;

        case PWON_RE_SETDAC:
            // 再次设置DAC
            st_IDACpara.set = mgmd_stSCIRx.f32SetCur_use;
            st_IDACpara.range_index = mgmd_stSCIRx.SetRangeCur_index;
            DACCalculate(&st_VDACpara, &st_IDACpara);
            UPDATE_IREF(st_IDACpara.AD)

            // 计算delay时间
            if (mgmd_stSCIRx.f32PluseWidth > 0 && mgmd_stSCIRx.Autosample == 1)
            {
                plusewidth = (mgmd_stSCIRx.f32PluseWidth - mgmd_stSCIRx.f32nPLC * 0.02) * 1000 - 10;
            }
            else
            {
                plusewidth = 0;
            }

            if (plusewidth < 0)
                plusewidth = 0;

            state = PWON_DELAY_SW;
            break;

        case PWON_DELAY_SW:
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(plusewidth))
            {
                firstflag = 0;
                state = PWON_SAMPLE;
            }
            break;

        case PWON_SAMPLE:
            if (mgmd_stSCIRx.Autosample == 1)
            {
                // 开启一次采样
                st_ADCdata.trigger = 1;
                st_ADCdata.dataOK = 0;
                mgmd_stSCIRx.DataOKFlag = 0;

                state = PWON_WAITOK;
            }
            else
            {
                // 不采样，延时后下电
                state = PWON_OFF;
            }

            break;

        case PWON_WAITOK:

            // 采样ok后下电
            if (st_ADCdata.dataOK == 1)
            {
                // 将电流、电流档位设置为1mA
                MdF32ToU16(1e-3, &mgmd_stSCIRx.u16SetCurH, &mgmd_stSCIRx.u16SetCurL);
                MdF32ToU16(1e-3, &mgmd_stSCIRx.SetRangeCurH, &mgmd_stSCIRx.SetRangeCurL);

                state = PWON_RE_RE_SETDAC;
            }

            break;

        case PWON_RE_RE_SETDAC:
            // 再再次设置DAC
            st_IDACpara.set = mgmd_stSCIRx.f32SetCur_use;
            st_IDACpara.range_index = mgmd_stSCIRx.SetRangeCur_index;
            DACCalculate(&st_VDACpara, &st_IDACpara);
            UPDATE_IREF(st_IDACpara.AD)

            state = PWON_DELAY_OFF;
            break;

        case PWON_DELAY_OFF:
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(1))
            {
                firstflag = 0;
                state = PWON_OFF;
            }
            break;

        case PWON_OFF:

            // 下电
            mgmd_stSCIRx.POWER_CTRL = 2;

            break;

        case PWON_ONOK:

            // 上电ok
            mgmd_stSCIRx.POWER_CTRL = 0;

            break;

        default:
            state = PWON_IDEL;
        }
    }
    else
    {
        state = PWON_IDEL;
    }
}

/* 普通上电的CAN联动版本，POWER_CTRL=4。 */
void PowerON_Can(void)
{
    Uint16 can_reply_status;
    Uint16 can_send_status;
    Uint16 can_reply_received;
    static Uint16 state = 0;
    static Uint16 firstflag = 0;
    static Uint32 nowtime = 0;
    static int32 plusewidth = 0;

    //    if(poweronflag == 0)
    //    {
    //        state = PWON_IDEL;
    //    }

    if (mgmd_stSCIRx.POWER_CTRL == POWER_CTRL_ON_CAN)
    {
        switch (state)
        {
        case PWON_IDEL:
            mgmd_stSCIRx.pw_can_flag = 0U;
            st_ADCdata.dataOK = 0;
            mgmd_stSCIRx.DataOKFlag = 0;
            poweronflag = 1;
            firstflag = 0;
            state = PWON_START;
            break;

        case PWON_START:
            // 设置DAC
            st_VDACpara.set = mgmd_stSCIRx.f32SetVol_use;
            st_IDACpara.set = mgmd_stSCIRx.f32SetCur_use;
            st_VDACpara.range_index = mgmd_stSCIRx.SetRangeVol_index;
            st_IDACpara.range_index = mgmd_stSCIRx.SetRangeCur_index;
            DACCalculate(&st_VDACpara, &st_IDACpara);
            UPDATE_VREF(st_VDACpara.AD)
            UPDATE_IREF(st_IDACpara.AD)
            UPDATE_AVMAREF(65534, 65534)

            if (mgmd_stSCIRx.f32PluseWidth > 0)
            {
                // 需要采样时延时要减采样所需时间
                plusewidth = (mgmd_stSCIRx.f32PluseWidth - mgmd_stSCIRx.f32nPLC * 0.02) * 1000 - 2;

                // fpga 版本大于 21 才支持波形上传功能
                if (st_version.FPGAversion >= 21)
                    EnableWave();

                if (plusewidth < 0)
                    plusewidth = 0;

                state = PWON_DELAY;
            }
            else
            {
                // 直接上电ok  等待
                state = PWON_ONOK;
            }
            break;

        case PWON_DELAY: // 延时
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(plusewidth))
            {
                firstflag = 0;
                state = PWON_SAMPLE;
            }
            break;

        case PWON_SAMPLE:
            /* 本机与采样板各触发一次，并清除上次完成标志。 */
            st_ADCdata.dataOK = 0;
            mgmd_stSCIRx.DataOKFlag = 0;
            g_can_controller.reply_received = 0U;
            st_ADCdata.trigger = 1;
            can_send_status = CanController_StartUnicast(
                &g_can_controller, CAN_CONTROLLER_DEFAULT_SAMPLE_ID);
            if (can_send_status != CAN_CONTROLLER_STATUS_SUCCESS)
            {
                mgmd_stSCIRx.pw_can_flag = 2U;
                mgmd_stSCIRx.POWER_CTRL = 2U;
                state = PWON_IDEL;
                break;
            }

            nowtime = task_run_cnt;
            state = PWON_WAITOK;
            break;

        case PWON_WAITOK:
            /* CAN回复独立计时，本机完成且收到回复后才判定成功。 */
            can_reply_received = CanController_GetReply(&g_can_controller,
                                                        &can_reply_status);
            if ((st_ADCdata.dataOK == 1U) && (can_reply_received != 0U))
            {
                mgmd_stSCIRx.pw_can_flag = 1U;
                state = PWON_OFF;
            }
            else if ((can_reply_received == 0U) &&
                     ((task_run_cnt - nowtime) >=
                      Tms_CNT(POWER_ON_CAN_REPLY_TIMEOUT_MS)))
            {
                mgmd_stSCIRx.pw_can_flag = 3U;
                mgmd_stSCIRx.POWER_CTRL = 2U;
                state = PWON_IDEL;
            }
            break;

        case PWON_OFF:
            // 下电
            mgmd_stSCIRx.POWER_CTRL = 2;
            break;

        case PWON_ONOK:
            // 上电ok
            mgmd_stSCIRx.POWER_CTRL = 0;
            break;

        default:
            state = PWON_IDEL;
        }
    }
    else
    {
        state = PWON_IDEL;
    }
}

extern Uint16 wave_coef_record;
void PowerOff(void)
{
    static Uint16 state = 0;
    static Uint16 firstflag = 0;
    static Uint32 nowtime = 0;

    //    if(poweronflag == 1)
    //    {
    //        state = PWOFF_IDEL;
    //    }

    if (mgmd_stSCIRx.POWER_CTRL == 2)
    {
        switch (state)
        {
        case PWOFF_IDEL:
            // 清除变量
            poweronflag = 0;
            //            mgmd_stSCIRx.Testmode = 0;

            // fpga 版本大于 21 才支持波形上传功能
            if (st_version.FPGAversion >= 21)
                DisableWave();

            // 下电时确认存储波形的计算挡位
            wave_coef_record = 1;

            // 刷新 AVM dac 的值
            UPDATE_AVMAREF(65533, 65533);

            // 放电模式的下电方法
            // 设置DAC，电压源先关Vdac，电流源先关Idac
            if (mgmd_stSCIRx.Outmode == OUTMODE_V_OFFVI)
            {
                st_VDACpara.set = 0;
                st_IDACpara.set = 100e-3;
                DACCalculate(&st_VDACpara, &st_IDACpara);
                UPDATE_VREF(st_VDACpara.AD)
                UPDATE_IREF(st_IDACpara.AD)
                mgmd_stSCIRx.CMD_SNSCONTROL_n = 1; // 切到两线下电
            }
            else if (mgmd_stSCIRx.Outmode == OUTMODE_I)
            {
                st_IDACpara.set = 0;
                DACCalculate(&st_VDACpara, &st_IDACpara);
                UPDATE_IREF(st_IDACpara.AD)
            }
            else // mgmd_stSCIRx.Outmode == OUTMODE_V
            {
                st_VDACpara.set = 0;
                DACCalculate(&st_VDACpara, &st_IDACpara);
                UPDATE_VREF(st_VDACpara.AD)
            }

            // 若当前200V档位，下电后切换到35V档位
            if (mgmd_stSCIRx.SetRangeVol_index == RANGE_VOL_200V_INDEX)
            {
                state = PWOFF_DELAY;
            }
            else
            {
                state = PWOFF_OK;
            }
            break;

        case PWOFF_DELAY: // 延时
            if (firstflag == 0)
            {
                firstflag = 1;
                nowtime = task_run_cnt;
            }
            if ((task_run_cnt - nowtime) > Tms_CNT(5))
            {
                firstflag = 0;
                state = PWOFF_RANGESW;
            }
            break;

        case PWOFF_RANGESW:
            // 将电压、电压档位设置为35V
            MdF32ToU16(35, &mgmd_stSCIRx.u16SetVolH, &mgmd_stSCIRx.u16SetVolL);
            MdF32ToU16(35, &mgmd_stSCIRx.SetRangeVolH, &mgmd_stSCIRx.SetRangeVolL);
            state = PWOFF_OK;
            break;

        case PWOFF_OK:

            last_vol_range = mgmd_stSCIRx.SetRangeVol_index;
            last_cur_range = mgmd_stSCIRx.SetRangeCur_index;
            // 下电ok
            mgmd_stSCIRx.POWER_CTRL = 0;
            break;

        default:
            state = PWOFF_IDEL;
        }
    }
    else
    {
        state = PWOFF_IDEL;
    }
}

// 在FPGA开机后初始化的一些事情
void InitFpga()
{
    static Uint16 firstflag = 0;

    if (st_version.FPGArunflag == 1)
    {
        if (firstflag == 0)
        {
            firstflag = 1;
            //-----------------------------------------初始化操作，仅上电后执行一次--------------------------------------//
            // 将DAC的值写一遍，第一次写好像会应用不上
            UPDATE_VREF(0)
            UPDATE_IREF(0)
            UPDATE_AVMAREF(65535, 65535)

            // 执行一次采样
            st_ADCdata.trigger = 1;

            // 设置输出继电器为1
            mgmd_stSCIRx.CMD_OUT_EN1 = 1;

            //-----------------------------------------初始化操作，仅上电后执行一次--------------------------------------//
        }
    }
    else
    {
        firstflag = 0;
    }
}
