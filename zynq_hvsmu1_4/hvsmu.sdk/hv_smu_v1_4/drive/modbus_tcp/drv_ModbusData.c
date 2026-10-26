#include "COMMON.h"

ST_SCI_RX_PAR mgmd_stSCIRx;
Uint16 *md_r1_buf[MD_R1_SIZE];
Uint16 *md_r2_buf[MD_R2_SIZE];
Uint16 *md_rw1_buf[MD_RW1_SIZE];
Uint16 *md_rw2_buf[MD_RW2_SIZE];
Uint16 *md_rw3_buf[MD_RW3_SIZE];
Uint16 *md_rw4_buf[MD_RW4_SIZE];
Uint16 *md_w1_buf[MD_W1_SIZE];

void InitModbusData(void)
{
    ST_SCI_RX_PAR *p = &mgmd_stSCIRx;

    memset(&mgmd_stSCIRx, 0, sizeof(ST_SCI_RX_PAR));

    p->nplc.f32 = 1;
    p->v_set.f32 = 20;
    p->i_set.f32 = 1e-3;
    p->autorangei = 1;
    p->autorangev = 1;
    p->reserved_200A = 0; /* Former low-voltage range disable: no longer used. */
    p->dis100nA = 1;
    p->delay_ref = 2;
    p->delay_discharge = 2000;
    p->auto_discharge_delay = 2;
    p->mode0_init_range.f32 = 50e-3;
    p->sw_delay_ms = 10;
    p->relay_on_always = 0;
    p->auto_discharge = 1;
    p->wave.enable = 1;
    p->wave.time_ms = 100;
    p->wave.cur_coef_en = 0;
    p->wave.vol_coef_en = 1;
    p->wave.filter_en = 0;
    p->cfg.disable_all_prot = 0;
    p->fire_drop_50us = 300; /* 临时参数，暂不放入 eeprom 中*/
    p->coef.enable.u16 = 0xFFFF; // enable all coefficient
}

extern Uint16 main_power_ok, main_power_reset;
extern Uint16 main_power_self_test_ok, main_power_self_test_en;
extern Uint16 eth_speed;
extern Uint16 eth_nego_cnt;
void ModbusDataRegister(void)
{
    Uint16 i = 0;

    // 0x1000
    // info
    md_r1_buf[0x00] = &mgmd_stSCIRx.dataok;
    md_r1_buf[0x01] = &main_power_ok;
    md_r1_buf[0x02] = &main_power_self_test_ok;
    md_r1_buf[0x03] = &mgmd_stSCIRx.autozero_okflag;
    md_r1_buf[0x04] = &mgmd_stSCIRx.pw_can_flag;
    md_r1_buf[0x05] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x06] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x07] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x08] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x09] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x0A] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x0B] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x0C] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x0D] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x0E] = &eth_speed;
    md_r1_buf[0x0F] = &eth_nego_cnt;
    md_r1_buf[0x10] = &st_version.anlu_version;
    md_r1_buf[0x11] = &st_version.zynq_version;
    md_r1_buf[0x12] = &st_version.utc_time.u16.h;
    md_r1_buf[0x13] = &st_version.utc_time.u16.l;
    md_r1_buf[0x14] = &st_version.initok;
    md_r1_buf[0x15] = &st_version.device_num;
    md_r1_buf[0x16] = &mgmd_stSCIRx.coef.utctime.u16.H;
    md_r1_buf[0x17] = &mgmd_stSCIRx.coef.utctime.u16.L;
    md_r1_buf[0x18] = &mgmd_stSCIRx.coef.sn_no[0].u16.H;
    md_r1_buf[0x19] = &mgmd_stSCIRx.coef.sn_no[0].u16.L;
    md_r1_buf[0x1A] = &mgmd_stSCIRx.coef.sn_no[1].u16.H;
    md_r1_buf[0x1B] = &mgmd_stSCIRx.coef.sn_no[1].u16.L;
    md_r1_buf[0x1C] = &mgmd_stSCIRx.coef.sn_no[2].u16.H;
    md_r1_buf[0x1D] = &mgmd_stSCIRx.coef.sn_no[2].u16.L;
    md_r1_buf[0x1E] = &mgmd_stSCIRx.reserve;
    md_r1_buf[0x1F] = &mgmd_stSCIRx.reserve;

    // 0x1100
    // data
    md_r2_buf[0x00] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x01] = &mgmd_stSCIRx.vsamp_avg.u16.H;
    md_r2_buf[0x02] = &mgmd_stSCIRx.vsamp_avg.u16.L;
    md_r2_buf[0x03] = &mgmd_stSCIRx.isamp_avg.u16.H;
    md_r2_buf[0x04] = &mgmd_stSCIRx.isamp_avg.u16.L;
    md_r2_buf[0x05] = &mgmd_stSCIRx.vsamp.u16.H;
    md_r2_buf[0x06] = &mgmd_stSCIRx.vsamp.u16.L;
    md_r2_buf[0x07] = &mgmd_stSCIRx.isamp.u16.H;
    md_r2_buf[0x08] = &mgmd_stSCIRx.isamp.u16.L;
    md_r2_buf[0x09] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x0A] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x0B] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x0C] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x0D] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x0E] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x0F] = &mgmd_stSCIRx.adc_vol_avg.u16.H;
    md_r2_buf[0x10] = &mgmd_stSCIRx.adc_vol_avg.u16.L;
    md_r2_buf[0x11] = &mgmd_stSCIRx.adc_cur_avg.u16.H;
    md_r2_buf[0x12] = &mgmd_stSCIRx.adc_cur_avg.u16.L;
    md_r2_buf[0x13] = &mgmd_stSCIRx.adc_vol_full.u16.H;
    md_r2_buf[0x14] = &mgmd_stSCIRx.adc_vol_full.u16.L;
    md_r2_buf[0x15] = &mgmd_stSCIRx.adc_cur_full.u16.H;
    md_r2_buf[0x16] = &mgmd_stSCIRx.adc_cur_full.u16.L;
    md_r2_buf[0x17] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x18] = &mgmd_stSCIRx.rx_err_cnt.u16.H;
    md_r2_buf[0x19] = &mgmd_stSCIRx.rx_err_cnt.u16.L;
    md_r2_buf[0x1A] = &mgmd_stSCIRx.dac_vol_ad;
    md_r2_buf[0x1B] = &mgmd_stSCIRx.dac_cur_ad;
    md_r2_buf[0x1C] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x1D] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x1E] = &mgmd_stSCIRx.reserve;
    md_r2_buf[0x1F] = &mgmd_stSCIRx.reserve;

    // 0x2000
    md_w1_buf[0x00] = &mgmd_stSCIRx.ip.addr1;
    md_w1_buf[0x01] = &mgmd_stSCIRx.ip.addr2;
    md_w1_buf[0x02] = &mgmd_stSCIRx.ip.addr3;
    md_w1_buf[0x03] = &mgmd_stSCIRx.ip.addr4;
    md_w1_buf[0x04] = &mgmd_stSCIRx.coef_mode1;
    md_w1_buf[0x05] = &mgmd_stSCIRx.coef_mode2;
    md_w1_buf[0x06] = &mgmd_stSCIRx.coef.enable.u16;
    md_w1_buf[0x07] = &mgmd_stSCIRx.autorangev;
    md_w1_buf[0x08] = &st_ADCdata.trigger;
    md_w1_buf[0x09] = &mgmd_stSCIRx.cfg.disable_all_prot;
    md_w1_buf[0x0A] = &mgmd_stSCIRx.reserved_200A;
    md_w1_buf[0x0B] = &mgmd_stSCIRx.highCmode;
    md_w1_buf[0x0C] = &mgmd_stSCIRx.autorangei;
    md_w1_buf[0x0D] = &mgmd_stSCIRx.delay_ref;
    md_w1_buf[0x0E] = &mgmd_stSCIRx.delay_discharge;
    md_w1_buf[0x0F] = &main_power_reset;
    md_w1_buf[0x10] = &main_power_self_test_en;
    md_w1_buf[0x11] = &mgmd_stSCIRx.sw_delay_ms;
    md_w1_buf[0x12] = &mgmd_stSCIRx.cmd_autozero;
    md_w1_buf[0x13] = &mgmd_stSCIRx.test_idac_en;
    md_w1_buf[0x14] = &mgmd_stSCIRx.test_idac_a0;
    md_w1_buf[0x15] = &mgmd_stSCIRx.test_idac_a1;
    md_w1_buf[0x16] = &mgmd_stSCIRx.test_vdac_en;
    md_w1_buf[0x17] = &mgmd_stSCIRx.test_vdac_a0;
    md_w1_buf[0x18] = &mgmd_stSCIRx.test_vdac_a1;
    md_w1_buf[0x19] = &mgmd_stSCIRx.dis_main_power;
    md_w1_buf[0x1A] = &mgmd_stSCIRx.dis100nA;
    md_w1_buf[0x1B] = &mgmd_stSCIRx.relay_on_always;
    md_w1_buf[0x1C] = &mgmd_stSCIRx.auto_discharge;
    md_w1_buf[0x1D] = &mgmd_stSCIRx.auto_discharge_delay;
    md_w1_buf[0x1E] = &mgmd_stSCIRx.mode0_init_range.u16.H;
    md_w1_buf[0x1F] = &mgmd_stSCIRx.mode0_init_range.u16.L;
    //...
    md_w1_buf[0x27] = &mgmd_stSCIRx.wave.enable;
    md_w1_buf[0x28] = &mgmd_stSCIRx.wave.trigger;
    md_w1_buf[0x29] = &mgmd_stSCIRx.wave.time_ms;
    md_w1_buf[0x2A] = &mgmd_stSCIRx.coef.utctime.u16.H;
    md_w1_buf[0x2B] = &mgmd_stSCIRx.coef.utctime.u16.L;
    md_w1_buf[0x2C] = &mgmd_stSCIRx.coef.sn_no[0].u16.H;
    md_w1_buf[0x2D] = &mgmd_stSCIRx.coef.sn_no[0].u16.L;
    md_w1_buf[0x2E] = &mgmd_stSCIRx.coef.sn_no[1].u16.H;
    md_w1_buf[0x2F] = &mgmd_stSCIRx.coef.sn_no[1].u16.L;
    md_w1_buf[0x30] = &mgmd_stSCIRx.coef.sn_no[2].u16.H;
    md_w1_buf[0x31] = &mgmd_stSCIRx.coef.sn_no[2].u16.L;

    // 0x3000
    // parameter
    md_rw1_buf[0x00] = &mgmd_stSCIRx.test_mode;
    md_rw1_buf[0x01] = &mgmd_stSCIRx.reserve; // reserve, corresponds to the output_mode of 2.0
    md_rw1_buf[0x02] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x03] = &mgmd_stSCIRx.v_range.u16.H; // vol_range
    md_rw1_buf[0x04] = &mgmd_stSCIRx.v_range.u16.L;
    md_rw1_buf[0x05] = &mgmd_stSCIRx.i_range.u16.H; // corresponds to the force_cur_range of 2.0
    md_rw1_buf[0x06] = &mgmd_stSCIRx.i_range.u16.L;
    md_rw1_buf[0x07] = &mgmd_stSCIRx.reserve; // reserve, corresponds to the measure_cur_range of 2.0
    md_rw1_buf[0x08] = &mgmd_stSCIRx.reserve; //
    md_rw1_buf[0x09] = &mgmd_stSCIRx.v_set.u16.H;
    md_rw1_buf[0x0A] = &mgmd_stSCIRx.v_set.u16.L;
    md_rw1_buf[0x0B] = &mgmd_stSCIRx.i_set.u16.H; // set cur
    md_rw1_buf[0x0C] = &mgmd_stSCIRx.i_set.u16.L;
    md_rw1_buf[0x0D] = &mgmd_stSCIRx.plusewidth.u16.H;
    md_rw1_buf[0x0E] = &mgmd_stSCIRx.plusewidth.u16.L;
    md_rw1_buf[0x0F] = &mgmd_stSCIRx.nplc.u16.H;
    md_rw1_buf[0x10] = &mgmd_stSCIRx.nplc.u16.L;
    md_rw1_buf[0x11] = &mgmd_stSCIRx.power_ctrl;
    md_rw1_buf[0x12] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x13] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x14] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x15] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x16] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x17] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x18] = &mgmd_stSCIRx.reserve;
    md_rw1_buf[0x19] = &mgmd_stSCIRx.reserve;

    // 0x5000
    // calibration coefficient
    for (i = 0; i < EEROM_PARA_NUM; i++)
    {
        md_rw2_buf[0x00 + 4 * i] = &mgmd_stSCIRx.coef.f32v_adc_p_k[i].u16.H;
        md_rw2_buf[0x01 + 4 * i] = &mgmd_stSCIRx.coef.f32v_adc_p_k[i].u16.L;
        md_rw2_buf[0x02 + 4 * i] = &mgmd_stSCIRx.coef.f32v_adc_p_b[i].u16.H;
        md_rw2_buf[0x03 + 4 * i] = &mgmd_stSCIRx.coef.f32v_adc_p_b[i].u16.L;
    }

    // 0x5300
    md_rw3_buf[0x00] = &mgmd_stSCIRx.cfg.disable_all_prot;
    md_rw3_buf[0x01] = &mgmd_stSCIRx.fire_drop_50us; // 0x5301：每 50us 的电压跌落阈值，单位 V
    md_rw3_buf[0x02] = &mgmd_stSCIRx.reserve;
    // 0x5310
    for (i = 0; i < 40; i++)
    {
        md_rw3_buf[0x10 + i] = &mgmd_stSCIRx.cfgvalue[i];
    }

    // 0x7000
    // 120 个
    for (i = 0; i < 0x78; i++)
    {
        md_rw4_buf[0x00 + i] = &mgmd_stSCIRx.download.data[i];
    }
    md_rw4_buf[0x78] = &mgmd_stSCIRx.download.frame_type;
    md_rw4_buf[0x79] = &mgmd_stSCIRx.download.crc;
    md_rw4_buf[0x7A] = &mgmd_stSCIRx.download.ok;
    md_rw4_buf[0x7B] = &mgmd_stSCIRx.download.byte_num.u16.h;
    md_rw4_buf[0x7C] = &mgmd_stSCIRx.download.byte_num.u16.l;
    md_rw4_buf[0x7D] = &mgmd_stSCIRx.download.write_flash;
    md_rw4_buf[0x7E] = &mgmd_stSCIRx.download.reset;
}

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
