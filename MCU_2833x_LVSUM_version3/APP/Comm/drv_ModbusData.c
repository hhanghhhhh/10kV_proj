/**************************************************************************************************
Project:  MVPS
Filename:   drv_ModbusData.c
Description:
Complier: CCS V3.3, TI.
***************************************************************************************************
Author:   WANG DAN
Created on:  2016��5��30��
Revision:
***************************************************************************************************
**************************************************************************************************/

/**************************************************************************************************
COMPILER DIRECTIVES
**************************************************************************************************/
#ifndef _MODBUS_DATA_C_
#define _MODBUS_DATA_C_
#endif

/**************************************************************************************************
HEADER FILES INCLUDED
**************************************************************************************************/
#include "drv_ModbusData.h"
#include "drv_MgModbus.h"
#include "drv_GlobalVar.h"
#include "drv_Adc.h"
#include "Version.h"
/**************************************************************************************************
CONSTANT DESCRIPTION
**************************************************************************************************/

/**************************************************************************************************
EXPORTED FUNCTIONS
**************************************************************************************************/
/**************************************************************************************************
FUNCTION: md_vPrepareData
DESCRIPTION: Prepare data
**************************************************************************************************/
Uint16 *md_u16RDGroup1Buf[RD_GROUP1_BUF_SIZE_MACRO];
Uint16 *md_u16RDGroup2Buf[RD_GROUP2_BUF_SIZE_MACRO];
Uint16 *md_u16RDWEBuf[RD_WE_BUF_SIZE_MACRO];
Uint16 *md_u16RDWEBuf2[RD_WE2_BUF_SIZE_MACRO];
Uint16 *md_u16RDWEBuf4[RD_WE4_BUF_SIZE_MACRO];
Uint16 *md_u16WEBuf[WE_GROUP_BUF_SIZE];
extern Uint16 hh_test_state;
void md_vPrepareData(void)
{
    int i;

    // 0x1000
    // 状态
    md_u16RDGroup1Buf[0x00] = &mgmd_stSCIRx.DataOKFlag;
    md_u16RDGroup1Buf[0x01] = &st_version.device_num;
    md_u16RDGroup1Buf[0x02] = &st_version.DSPversion;
    md_u16RDGroup1Buf[0x03] = &st_version.FPGAversion;
    md_u16RDGroup1Buf[0x04] = &mgmd_stSCIRx.IPwriteOK;
    md_u16RDGroup1Buf[0x05] = &mgmd_stSCIRx.vth_ok;
    md_u16RDGroup1Buf[0x06] = &mgmd_stSCIRx.vth_dbsmu.ok;
    md_u16RDGroup1Buf[0x07] = &mgmd_stSCIRx.pw_can_flag;
    md_u16RDGroup1Buf[0x08] = &mgmd_stSCIRx.reserve;
    md_u16RDGroup1Buf[0x09] = &mgmd_stSCIRx.test_read_in;

    // 0x1100
    // 数据
    md_u16RDGroup2Buf[0x00] = &mgmd_stSCIRx.reserve;
    md_u16RDGroup2Buf[0x01] = &mgmd_stSCIRx.u16DataVolH;
    md_u16RDGroup2Buf[0x02] = &mgmd_stSCIRx.u16DataVolL;
    md_u16RDGroup2Buf[0x03] = &mgmd_stSCIRx.u16DataCurH;
    md_u16RDGroup2Buf[0x04] = &mgmd_stSCIRx.u16DataCurL;
    md_u16RDGroup2Buf[0x05] = &mgmd_stSCIRx.u16DataVolRealTH;
    md_u16RDGroup2Buf[0x06] = &mgmd_stSCIRx.u16DataVolRealTL;
    md_u16RDGroup2Buf[0x07] = &mgmd_stSCIRx.u16DataCurRealTH;
    md_u16RDGroup2Buf[0x08] = &mgmd_stSCIRx.u16DataCurRealTL;
    md_u16RDGroup2Buf[0x09] = &mgmd_stSCIRx.vth_volsamp1.u16.H;
    md_u16RDGroup2Buf[0x0A] = &mgmd_stSCIRx.vth_volsamp1.u16.L;
    md_u16RDGroup2Buf[0x0B] = &mgmd_stSCIRx.vth_cursamp1.u16.H;
    md_u16RDGroup2Buf[0x0C] = &mgmd_stSCIRx.vth_cursamp1.u16.L;
    md_u16RDGroup2Buf[0x0D] = &mgmd_stSCIRx.vth_volsamp2.u16.H;
    md_u16RDGroup2Buf[0x0E] = &mgmd_stSCIRx.vth_volsamp2.u16.L;
    md_u16RDGroup2Buf[0x0F] = &mgmd_stSCIRx.vth_cursamp2.u16.H;
    md_u16RDGroup2Buf[0x10] = &mgmd_stSCIRx.vth_cursamp2.u16.L;
    md_u16RDGroup2Buf[0x11] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.vsamp.u16.H;
    md_u16RDGroup2Buf[0x12] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.vsamp.u16.L;
    md_u16RDGroup2Buf[0x13] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.isamp.u16.H;
    md_u16RDGroup2Buf[0x14] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.isamp.u16.L;
    md_u16RDGroup2Buf[0x15] = &mgmd_stSCIRx.vth_dbsmu.gs.step.vsamp.u16.H;
    md_u16RDGroup2Buf[0x16] = &mgmd_stSCIRx.vth_dbsmu.gs.step.vsamp.u16.L;
    md_u16RDGroup2Buf[0x17] = &mgmd_stSCIRx.vth_dbsmu.gs.step.isamp.u16.H;
    md_u16RDGroup2Buf[0x18] = &mgmd_stSCIRx.vth_dbsmu.gs.step.isamp.u16.L;
    md_u16RDGroup2Buf[0x19] = &mgmd_stSCIRx.vth_dbsmu.ds.vsamp.u16.H;
    md_u16RDGroup2Buf[0x1A] = &mgmd_stSCIRx.vth_dbsmu.ds.vsamp.u16.L;
    md_u16RDGroup2Buf[0x1B] = &mgmd_stSCIRx.vth_dbsmu.ds.isamp.u16.H;
    md_u16RDGroup2Buf[0x1C] = &mgmd_stSCIRx.vth_dbsmu.ds.isamp.u16.L;
    md_u16RDGroup2Buf[0x1D] = &mgmd_stSCIRx.vth_dbsmu.test_state;

    md_u16RDGroup2Buf[0x1E] = &mgmd_stSCIRx.vth_8pre.state;
    md_u16RDGroup2Buf[0x1F] = &mgmd_stSCIRx.vth_8pre.ok;
    for (i = 0; i < 8; i++)
    {
        md_u16RDGroup2Buf[0x20 + 0 + i * 4] = &mgmd_stSCIRx.vth_8pre.pre[i].vsamp.u16.H;
        md_u16RDGroup2Buf[0x20 + 1 + i * 4] = &mgmd_stSCIRx.vth_8pre.pre[i].vsamp.u16.L;
        md_u16RDGroup2Buf[0x20 + 2 + i * 4] = &mgmd_stSCIRx.vth_8pre.pre[i].isamp.u16.H;
        md_u16RDGroup2Buf[0x20 + 3 + i * 4] = &mgmd_stSCIRx.vth_8pre.pre[i].isamp.u16.L;
    }
    md_u16RDGroup2Buf[0x40] = &mgmd_stSCIRx.vth_8pre.vth.vsamp.u16.H;
    md_u16RDGroup2Buf[0x41] = &mgmd_stSCIRx.vth_8pre.vth.vsamp.u16.L;
    md_u16RDGroup2Buf[0x42] = &mgmd_stSCIRx.vth_8pre.vth.isamp.u16.H;
    md_u16RDGroup2Buf[0x43] = &mgmd_stSCIRx.vth_8pre.vth.isamp.u16.L;

    // 0x2000
    // 写命令
    md_u16WEBuf[0x00] = &mgmd_stSCIRx.ip_addr1;
    md_u16WEBuf[0x01] = &mgmd_stSCIRx.ip_addr2;
    md_u16WEBuf[0x02] = &mgmd_stSCIRx.ip_addr3;
    md_u16WEBuf[0x03] = &mgmd_stSCIRx.ip_addr4;
    md_u16WEBuf[0x04] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x05] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x06] = &mgmd_stSCIRx.vth_forward_samp.u16.H;
    md_u16WEBuf[0x07] = &mgmd_stSCIRx.vth_forward_samp.u16.L;
    md_u16WEBuf[0x08] = &mgmd_stSCIRx.vth_outmode;
    md_u16WEBuf[0x09] = &mgmd_stSCIRx.CMD_SNSCONTROL_n;
    md_u16WEBuf[0x0A] = &mgmd_stSCIRx.vth_setvol1.u16.H;
    md_u16WEBuf[0x0B] = &mgmd_stSCIRx.vth_setvol1.u16.L;
    md_u16WEBuf[0x0C] = &mgmd_stSCIRx.vth_setcur1.u16.H;
    md_u16WEBuf[0x0D] = &mgmd_stSCIRx.vth_setcur1.u16.L;
    md_u16WEBuf[0x0E] = &mgmd_stSCIRx.vth_plusewidth1.u16.H;
    md_u16WEBuf[0x0F] = &mgmd_stSCIRx.vth_plusewidth1.u16.L;
    md_u16WEBuf[0x10] = &mgmd_stSCIRx.vth_setvol2.u16.H;
    md_u16WEBuf[0x11] = &mgmd_stSCIRx.vth_setvol2.u16.L;
    md_u16WEBuf[0x12] = &mgmd_stSCIRx.vth_setcur2.u16.H;
    md_u16WEBuf[0x13] = &mgmd_stSCIRx.vth_setcur2.u16.L;
    md_u16WEBuf[0x14] = &mgmd_stSCIRx.vth_plusewidth2.u16.H;
    md_u16WEBuf[0x15] = &mgmd_stSCIRx.vth_plusewidth2.u16.L;
    md_u16WEBuf[0x16] = &mgmd_stSCIRx.vth_nplc2.u16.H;
    md_u16WEBuf[0x17] = &mgmd_stSCIRx.vth_nplc2.u16.L;
    md_u16WEBuf[0x18] = &mgmd_stSCIRx.vth_poweron;
    md_u16WEBuf[0x19] = &mgmd_stSCIRx.CMD_OUT_EN1;
    md_u16WEBuf[0x1A] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x1B] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x1C] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x1D] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x1E] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x1F] = &mgmd_stSCIRx.CMD_SW_O;
    md_u16WEBuf[0x20] = &mgmd_stSCIRx.CMD_SW_F;
    md_u16WEBuf[0x21] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x22] = &mgmd_stSCIRx.ADC_trigger;
    md_u16WEBuf[0x23] = &mgmd_stSCIRx.CMD_ControlLoop_BW1_n;
    md_u16WEBuf[0x24] = &mgmd_stSCIRx.CMD_ControlLoop_BW2_n;
    md_u16WEBuf[0x25] = &mgmd_stSCIRx.CMD_ControlLoop_BW3_n;
    md_u16WEBuf[0x26] = &mgmd_stSCIRx.CMD_ControlLoop_BW4_n;
    md_u16WEBuf[0x27] = &mgmd_stSCIRx.CMD_GUARDEN;
    md_u16WEBuf[0x28] = &mgmd_stSCIRx.CMD_SNSCONTROL_n; //
    md_u16WEBuf[0x29] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x2A] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x2B] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x2C] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x2D] = &mgmd_stSCIRx.CMD_Control_VEN;
    md_u16WEBuf[0x2E] = &mgmd_stSCIRx.CMD_Control_IEN;
    md_u16WEBuf[0x2F] = &mgmd_stSCIRx.CMD_AVMADAC_UPDATE;
    md_u16WEBuf[0x30] = &mgmd_stSCIRx.POWER_CTRL; // 1 ON  2 OFF
    md_u16WEBuf[0x31] = &mgmd_stSCIRx.adjustflag;
    md_u16WEBuf[0x32] = &mgmd_stSCIRx.testflag;
    md_u16WEBuf[0x33] = &mgmd_stSCIRx.Testmode;
    md_u16WEBuf[0x34] = &mgmd_stSCIRx.adjust_adc_v;
    md_u16WEBuf[0x35] = &mgmd_stSCIRx.adjust_adc_i;
    md_u16WEBuf[0x36] = &mgmd_stSCIRx.adjust_dac_v;
    md_u16WEBuf[0x37] = &mgmd_stSCIRx.adjust_dac_i;
    md_u16WEBuf[0x38] = &mgmd_stSCIRx.reserve;
    md_u16WEBuf[0x39] = &mgmd_stSCIRx.coef_mode1;
    md_u16WEBuf[0x3A] = &mgmd_stSCIRx.coef_mode2;
    md_u16WEBuf[0x3B] = &mgmd_stSCIRx.test_set_out;
    md_u16WEBuf[0x3C] = &mgmd_stSCIRx.test_trigger_mode;

    // 0x3000
    // 写参数
    md_u16RDWEBuf[0x00] = &mgmd_stSCIRx.Outmode;          // 0 电压   1电流
    md_u16RDWEBuf[0x01] = &mgmd_stSCIRx.CMD_SNSCONTROL_n; // 1 两线 / 0 四线
    md_u16RDWEBuf[0x02] = &mgmd_stSCIRx.Autosample;       // 1采  0不采
    md_u16RDWEBuf[0x03] = &mgmd_stSCIRx.highCmode;        // 1 开  0关
    md_u16RDWEBuf[0x04] = &mgmd_stSCIRx.u16SetVolH;
    md_u16RDWEBuf[0x05] = &mgmd_stSCIRx.u16SetVolL;
    md_u16RDWEBuf[0x06] = &mgmd_stSCIRx.u16SetCurH;
    md_u16RDWEBuf[0x07] = &mgmd_stSCIRx.u16SetCurL;
    md_u16RDWEBuf[0x08] = &mgmd_stSCIRx.SetRangeVolH;
    md_u16RDWEBuf[0x09] = &mgmd_stSCIRx.SetRangeVolL;
    md_u16RDWEBuf[0x0A] = &mgmd_stSCIRx.SetRangeCurH;
    md_u16RDWEBuf[0x0B] = &mgmd_stSCIRx.SetRangeCurL;
    md_u16RDWEBuf[0x0C] = &mgmd_stSCIRx.PluseWidthH;
    md_u16RDWEBuf[0x0D] = &mgmd_stSCIRx.PluseWidthL;
    md_u16RDWEBuf[0x0E] = &mgmd_stSCIRx.nPLCH;
    md_u16RDWEBuf[0x0F] = &mgmd_stSCIRx.nPLCL;
    md_u16RDWEBuf[0x10] = &mgmd_stSCIRx.POWER_CTRL;
    md_u16RDWEBuf[0x11] = &mgmd_stSCIRx.reserve;
    md_u16RDWEBuf[0x12] = &mgmd_stSCIRx.reserve;
    md_u16RDWEBuf[0x13] = &mgmd_stSCIRx.reserve;
    md_u16RDWEBuf[0x14] = &mgmd_stSCIRx.reserve;
    md_u16RDWEBuf[0x15] = &mgmd_stSCIRx.vth_dbsmu.gs_or_ds;
    md_u16RDWEBuf[0x16] = &mgmd_stSCIRx.vth_dbsmu.up_or_down;
    md_u16RDWEBuf[0x17] = &mgmd_stSCIRx.CMD_SNSCONTROL_n; // vth_dbsmu.gs.four_or_two
    md_u16RDWEBuf[0x18] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.vset.u16.H;
    md_u16RDWEBuf[0x19] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.vset.u16.L;
    md_u16RDWEBuf[0x1A] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.iset.u16.H;
    md_u16RDWEBuf[0x1B] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.iset.u16.L;
    md_u16RDWEBuf[0x1C] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.plusewidth.u16.H;
    md_u16RDWEBuf[0x1D] = &mgmd_stSCIRx.vth_dbsmu.gs.pre.plusewidth.u16.L;
    md_u16RDWEBuf[0x1E] = &mgmd_stSCIRx.vth_dbsmu.gs.step.vmax.u16.H;
    md_u16RDWEBuf[0x1F] = &mgmd_stSCIRx.vth_dbsmu.gs.step.vmax.u16.L;
    md_u16RDWEBuf[0x20] = &mgmd_stSCIRx.vth_dbsmu.gs.step.vmin.u16.H;
    md_u16RDWEBuf[0x21] = &mgmd_stSCIRx.vth_dbsmu.gs.step.vmin.u16.L;
    md_u16RDWEBuf[0x22] = &mgmd_stSCIRx.vth_dbsmu.gs.step.iset.u16.H;
    md_u16RDWEBuf[0x23] = &mgmd_stSCIRx.vth_dbsmu.gs.step.iset.u16.L;
    md_u16RDWEBuf[0x24] = &mgmd_stSCIRx.vth_dbsmu.gs.step.delta_v.u16.H;
    md_u16RDWEBuf[0x25] = &mgmd_stSCIRx.vth_dbsmu.gs.step.delta_v.u16.L;
    md_u16RDWEBuf[0x26] = &mgmd_stSCIRx.vth_dbsmu.gs.step.delta_t.u16.H;
    md_u16RDWEBuf[0x27] = &mgmd_stSCIRx.vth_dbsmu.gs.step.delta_t.u16.L;
    md_u16RDWEBuf[0x28] = &mgmd_stSCIRx.CMD_SNSCONTROL_n; // vth_dbsmu.ds.four_or_two
    md_u16RDWEBuf[0x29] = &mgmd_stSCIRx.vth_dbsmu.ds.vset.u16.H;
    md_u16RDWEBuf[0x2A] = &mgmd_stSCIRx.vth_dbsmu.ds.vset.u16.L;
    md_u16RDWEBuf[0x2B] = &mgmd_stSCIRx.vth_dbsmu.ds.iset.u16.H;
    md_u16RDWEBuf[0x2C] = &mgmd_stSCIRx.vth_dbsmu.ds.iset.u16.L;
    md_u16RDWEBuf[0x2D] = &mgmd_stSCIRx.vth_dbsmu.ds.vth.u16.H;
    md_u16RDWEBuf[0x2E] = &mgmd_stSCIRx.vth_dbsmu.ds.vth.u16.L;
    md_u16RDWEBuf[0x2F] = &mgmd_stSCIRx.vth_dbsmu.poweron;
    md_u16RDWEBuf[0x30] = &mgmd_stSCIRx.vth_dbsmu.ds.delayms.u16.H;
    md_u16RDWEBuf[0x31] = &mgmd_stSCIRx.vth_dbsmu.ds.delayms.u16.L;

    md_u16RDWEBuf[0x32] = &mgmd_stSCIRx.vth_8pre.pre_num;
    md_u16RDWEBuf[0x33] = &mgmd_stSCIRx.CMD_SNSCONTROL_n;
    for (i = 0; i < 8; i++)
    {
        md_u16RDWEBuf[0x34 + 0 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].vset.u16.H;
        md_u16RDWEBuf[0x34 + 1 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].vset.u16.L;
        md_u16RDWEBuf[0x34 + 2 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].iset.u16.H;
        md_u16RDWEBuf[0x34 + 3 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].iset.u16.L;
        md_u16RDWEBuf[0x34 + 4 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].plusewidth.u16.H;
        md_u16RDWEBuf[0x34 + 5 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].plusewidth.u16.L;
        md_u16RDWEBuf[0x34 + 6 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].delay.u16.H;
        md_u16RDWEBuf[0x34 + 7 + i * 8] = &mgmd_stSCIRx.vth_8pre.pre[i].delay.u16.L;
    }
    md_u16RDWEBuf[0x74] = &mgmd_stSCIRx.vth_8pre.vth.vset.u16.H;
    md_u16RDWEBuf[0x75] = &mgmd_stSCIRx.vth_8pre.vth.vset.u16.L;
    md_u16RDWEBuf[0x76] = &mgmd_stSCIRx.vth_8pre.vth.iset.u16.H;
    md_u16RDWEBuf[0x77] = &mgmd_stSCIRx.vth_8pre.vth.iset.u16.L;
    md_u16RDWEBuf[0x78] = &mgmd_stSCIRx.vth_8pre.vth.plusewidth.u16.H;
    md_u16RDWEBuf[0x79] = &mgmd_stSCIRx.vth_8pre.vth.plusewidth.u16.L;
    md_u16RDWEBuf[0x7A] = &mgmd_stSCIRx.vth_8pre.vth.nplc.u16.H;
    md_u16RDWEBuf[0x7B] = &mgmd_stSCIRx.vth_8pre.vth.nplc.u16.L;
    md_u16RDWEBuf[0x7C] = &mgmd_stSCIRx.vth_8pre.poweron;
    md_u16RDWEBuf[0x7D] = &mgmd_stSCIRx.pw3_initial_cur.u16.H;
    md_u16RDWEBuf[0x7E] = &mgmd_stSCIRx.pw3_initial_cur.u16.L;
    md_u16RDWEBuf[0x7F] = &mgmd_stSCIRx.pw3_initial_delay_s.u16.H;
    md_u16RDWEBuf[0x80] = &mgmd_stSCIRx.pw3_initial_delay_s.u16.L;

    // 0x5000
    // 写系数
    for (i = 0; i < EEROM_PARA_NUM; i++)
    {
        md_u16RDWEBuf2[0x00 + 4 * i] = &mgmd_stSCIRx.v_adc_p_k_H[i];
        md_u16RDWEBuf2[0x01 + 4 * i] = &mgmd_stSCIRx.v_adc_p_k_L[i];
        md_u16RDWEBuf2[0x02 + 4 * i] = &mgmd_stSCIRx.v_adc_p_b_H[i];
        md_u16RDWEBuf2[0x03 + 4 * i] = &mgmd_stSCIRx.v_adc_p_b_L[i];
    }

    // 0X7000
    md_u16RDWEBuf4[0x00] = &mgmd_stSCIRx.program_type;
    md_u16RDWEBuf4[0x01] = &mgmd_stSCIRx.jump_cmd;
}

/**************************************************************************************************
COMPILER DIRECTIVES
**************************************************************************************************/
#ifdef _MODBUS_DATA_C_
#undef _MODBUS_DATA_C_
#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
