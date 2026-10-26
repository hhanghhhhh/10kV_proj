/*
 * task_onoff.c
 *
 *  Created on: 2025年7月2日
 *      Author: m
 */
#include "COMMON.h"
#include "drv_can_controller.h"

#define PWON_CAN_WAIT_TX (12U)

Uint16 pwoff_flag = 0;
Uint16 pwon_flag = 0;

union
{
    Uint16 u16;
    struct
    {
        Uint16 over_temp : 1;
        Uint16 reserve : 15;
    } bit;
} output_disable = {0};

Uint16 ReadRelay(void)
{
    return st_topl.cmd.bit.relay;
}
void VoRelayOn(void)
{
    st_topl.cmd.bit.relay = 1;
}

void VoRelayOff(void)
{
    st_topl.cmd.bit.relay = 0;
}

void DischargeOn(void)
{
    st_topl.cmd.bit.pramp_p = 1;
}
void DischargeOff(void)
{
    st_topl.cmd.bit.pramp_p = 0;
}
void DisOut(void)
{
    st_topl.cmd.bit.outoff = 0;
}
void EnOut(void)
{
    st_topl.cmd.bit.outoff = 1;
}

void InitDeviceState(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = 0;

    switch (state)
    {
    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case 0:
        if (st_version.anlu_runflag == 1)
        {
            state = 1;
            // delay 并且状态跳转
            FSM_DELAY_NEXT(st_delay, 100, 2);
        }
        break;

    case 2:
        // 初始化操作
        VoRelayOff();
        // 电压档位设置为400V档位,电流档位设置为1mA档位
        RangeVolSel(RANGE_VOL_400V_INDEX);
        RangeCurSel(RANGE_CUR_50mA_INDEX);
        SetVolRef(0);
        SetCurRef(0);
        //	delay 并且状态跳转
        FSM_DELAY_NEXT(st_delay, 100, 4);
        break;

    case 4:
        // 初始化ok
        st_version.initok = 1;
        break;

    default:
        state = 0;
        break;
    }
}

void PfcDisable(void)
{
    PS_GPIO_SetPort(IO_PL_PFC_DRIVE, 0);
}
void PfcEnable(void)
{
    PS_GPIO_SetPort(IO_PL_PFC_DRIVE, 1);
}
void LlcDisable(void)
{
    PS_GPIO_SetPort(IO_PL_PFC_EN, 1);
}
void LlcEnable(void)
{
    PS_GPIO_SetPort(IO_PL_PFC_EN, 0);
}
Uint16 PfcOk(void)
{
    return PS_GPIO_GetPort(IO_PL_PFC_OK);
}

Uint16 main_power_ok = 0;
Uint16 main_power_reset = 0;
Uint16 main_power_self_test_en = 0;
Uint16 main_power_self_test_ok = 1;
void MainPowerSelfTest(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = SELFT_IDEL;
    static Uint16 vol_index = RANGE_VOL_400V_INDEX;

    // 下电状态下允许自检
    if (pwon_flag == 1)
    {
        return;
    }

    if (main_power_self_test_en == 1)
    {
        main_power_self_test_en = 0;
        main_power_self_test_ok = 0;
        VoRelayOff();
        // 允许输出
        EnOut();
        vol_index = RANGE_VOL_400V_INDEX;
        FSM_DELAY_NEXT(st_delay, 10, SELFT_OUT);
    }

    switch (state)
    {
    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;
    case SELFT_IDEL:
        break;
    case SELFT_OUT:
        // 切换档位、设置电流、电压
        RangeVolSel(vol_index);
        RangeCurSel(RANGE_CUR_50mA_INDEX);
        SetCurRef(1e-3);
        SetVolRef(rangeVol_buff[vol_index]);
        FSM_DELAY_NEXT(st_delay, 100, SELFT_CHECK);
        break;

    case SELFT_CHECK:
        if (st_ADCdata.vol_1ms > rangeVol_buff[vol_index] * 0.9)
        {
            // 输出正常，切换电压
            if (vol_index < RANGE_VOL_10000V_INDEX)
            {
                vol_index++;
                state = SELFT_OUT;
            }
            else
            {
                // 自检ok，下电
                main_power_self_test_ok = 1;
                state = SELFT_OFF;
            }
        }
        else
        {
            // 输出异常，记录异常、下电
            main_power_self_test_ok = vol_index + 10;
            state = SELFT_OFF;
        }
        break;

    case SELFT_OFF:
        // 禁止输出
        DisOut();
        // 电压档位设置为400V档位,电流档位设置为1mA档位
        RangeVolSel(RANGE_VOL_400V_INDEX);
        RangeCurSel(RANGE_CUR_50mA_INDEX);
        SetVolRef(0);
        SetCurRef(0);
        state = SELFT_IDEL;
        break;

    default:
        state = SELFT_IDEL;
    }
}

void MainPowerCtrl(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    // 上电后先 reset 一下主功率
    static Uint16 state = MP_DISLLC;
    static Uint16 err_cnt = 0;
    static Uint32 pfcok_overcnt = 0;

    // 上位机检修，结束后主功率再重启一次
    if (mgmd_stSCIRx.dis_main_power == 1)
    {
        LlcDisable();
        PfcDisable();
        state = MP_DISLLC;
        return;
    }

    switch (state)
    {
    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;
    case MP_IDEL:
        break;
        // 主功率重启第一步
    case MP_DISLLC:
        main_power_ok = 0;
        LlcDisable();
        FSM_DELAY_NEXT(st_delay, 100, MP_DISPFC);
        break;
    case MP_DISPFC:
        PfcDisable();
        FSM_DELAY_NEXT(st_delay, 10000, MP_LLCPULSE1);
        break;
        // 重启的时候需要LLC_EN和Drive信号同时为低，LLC芯片才能退出锁死状态
    case MP_LLCPULSE1:
        LlcEnable();
        FSM_DELAY_NEXT(st_delay, 200, MP_LLCPULSE2);
        break;
    case MP_LLCPULSE2:
        LlcDisable();
        pfcok_overcnt = 0;
        FSM_DELAY_NEXT(st_delay, 100, MP_PFCEN);
        break;
    case MP_PFCEN:
        PfcEnable();
        pfcok_overcnt++;
        if (pfcok_overcnt > Tms_CNT(5000))
        {
            LlcDisable();
            LlcDisable();
            main_power_ok = 2;
            state = MP_CHECK;
        }
        if (PfcOk() == 1)
        {
            FSM_DELAY_NEXT(st_delay, 1000, MP_LLCEN);
        }
        break;
    case MP_LLCEN:
        LlcEnable();
        FSM_DELAY_NEXT(st_delay, 1000, MP_OK);
        break;
        // 主功率重启结束
    case MP_OK:
        main_power_ok = 1;
        state = MP_CHECK;
        break;

    case MP_CHECK:
        // 自检
        MainPowerSelfTest();
        // 自检不通过才允许重启主功率
        if (main_power_self_test_ok != 1 && main_power_self_test_ok != 0)
        {
            if (main_power_reset == 1)
            {
                main_power_reset = 0;
                state = MP_DISLLC;
            }
        }

        if (main_power_ok != 1)
        {
            err_cnt = 0;
            break;
        }
        // else if (main_power_ok == 1)
        // init_ok 并且下电后检测
        if (pwon_flag == 0 && st_version.initok == 1)
        {
            if (PfcOk() == 0)
            {
                err_cnt++;
                if (err_cnt > Tms_CNT(10))
                {
                    PfcDisable();
                    LlcDisable();
                    main_power_ok = 3;
                }
            }
            else
                err_cnt = 0;
        }
        break;
    default:
        state = MP_IDEL;
    }
}

// over_temp 低电平代表过温，高电平正常
void OverTemp(void)
{
    static Uint32 l_cnt = 0;
    static Uint32 h_cnt = 0;
    static Uint16 flag = 0;
    static Uint16 first_flag = 0;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    if (st_frompl.state.bit.over_temp == 0)
    {
        l_cnt++;
        h_cnt = 0;
        if (l_cnt > Tms_CNT(1))
            flag = 1;
    }
    else
    {
        l_cnt = 0;
        h_cnt++;
        if (h_cnt > Tms_CNT(1))
            flag = 0;
    }

    output_disable.bit.over_temp = flag;

    // 输出过程中出现禁止输出的情况，立马下电
    if (pwon_flag == 1)
    {
        if (output_disable.u16 != 0 && first_flag == 0)
        {
            first_flag = 1;
            pPC->dataok = 20 + output_disable.u16;
            pPC->isamp_avg.f32 = 0;
            pPC->vsamp_avg.f32 = 0;
            // 下电
            pPC->power_ctrl = POWER_OFF;
        }
    }
    else
    {
        first_flag = 0;
    }
}

void PowerOnNormal(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = PWON_IDEL;
    static int32 delay_ms = 0;
    static Uint32 waitok_cnt = 0;
    ST_RANGE_PARA *pRange_vol = &st_range_vol_use;
    ST_RANGE_PARA *pRange_cur = &st_range_cur_use;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    // 下电优先级最高
    if (pwoff_flag == 1)
    {
        state = PWON_IDEL;
        return;
    }

    if (pPC->power_ctrl == POWER_ON)
    {
        pPC->power_ctrl = POWER_IDEL;

        // 禁止输出，dataok 20 以上表示禁止输出的原因
        // if (output_disable.u16 != 0)
        // {
        //     pPC->dataok = 20 + output_disable.u16;
        //     pPC->isamp_avg.f32 = 0;
        //     pPC->vsamp_avg.f32 = 0;
        //     state = PWON_IDEL;
        //     return;
        // }

        pwon_flag = 1;
        // 允许输出
        EnOut();
        // 清零变量
        pPC->dataok = 0;
        pPC->isamp_avg.f32 = 0;
        pPC->vsamp_avg.f32 = 0;
        waitok_cnt = 0;
        // 打开输出继电器
        if (pPC->relay_on_always == 1 && ReadRelay() == 1)
        {
            state = PWON_VOL_RANGESW;
        }
        else
        {
            VoRelayOn();
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, 7, PWON_VOL_RANGESW);
        }
    }

    switch (state)
    {
    case PWON_IDEL:
        break;

    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case PWON_VOL_RANGESW:
        // 切换到上位机设置电压档位
        RangeVolSel(pRange_vol->range_index_PC);
        RangeCurSel(GetCurRangeIndex(pPC->mode0_init_range.f32));
        state = PWON_SET_REF;
        break;

    case PWON_SET_REF:
        // 开启保护
        EnableProt_All();
        // 设置电压值
        SetVolRef(pPC->v_set.f32);
        // 电流ref设置为50mA
        SetCurRef(pPC->mode0_init_range.f32);
        // 波形记录
        if (pPC->wave.enable == 1 && pPC->plusewidth.f32 > 0)
        {
            StartWaveRecord(pPC->plusewidth.f32 * 1000 + 20);
            WaveRangeSaveCur(GetCurRangeIndex(pPC->mode0_init_range.f32));
            WaveRangeSaveVol(pRange_vol->range_index_PC);
        }
        // 计算延时时间
        delay_ms = pPC->plusewidth.f32 * 1000 - pPC->sw_delay_ms - pPC->nplc.f32 * 20;
        if (delay_ms < 0)
            delay_ms = 0;
        // delay 并状态跳转
        FSM_DELAY_NEXT(st_delay, pPC->sw_delay_ms, PWON_CUR_RANGESW);
        break;

    case PWON_CUR_RANGESW:
        // 切换到上位机设置的电流档位
        RangeCurSel(pRange_cur->range_index_PC);
        // 电流ref设置为本档位最大值
        SetCurRef(st_range_cur_use.full_range);

        if (pPC->wave.enable == 1)
        {
            WaveRangeSwitchCur();
            WaveRangeSaveCur(pRange_cur->range_index_PC);
        }
        // 仅上电
        if (pPC->plusewidth.f32 < 0)
            state = PWON_UPDATE_TRIGGER;
        else
        {
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, delay_ms, PWON_SAMP_TRIGGER);
        }
        break;

    case PWON_SAMP_TRIGGER:
        // 采样
        TriggerAdcSamp();
        waitok_cnt = 0;
        state = PWON_WAIT_DATA;
        break;

    case PWON_WAIT_DATA:
        // 等待数据ok
        // 超时判断
        waitok_cnt++;
        if (waitok_cnt > Tms_CNT(pPC->nplc.f32 * 1000 + pPC->cfgvalue[EEROM_CONFIG_idss_overtime]))
        {
            pPC->dataok = ERR_IDSS_OVERTIME;
            // 下电
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        // 数据采样ok，下电
        if (AdcSampOk() == 1)
        {
            pPC->dataok = 1;
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            // 下电
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        break;

    case PWON_UPDATE_TRIGGER:
        TriggerAdcSamp();
        state = PWON_UPDATE_WAIT;
        break;

    case PWON_UPDATE_WAIT:
        if (AdcSampOk() == 1)
        {
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            state = PWON_UPDATE_TRIGGER;
        }
        break;

    default:
        state = PWON_IDEL;
        break;
    }
}

/* 原正常上电时序的CAN联动版本；正脉宽触发一次，持续输出仅本机采样。 */
void PowerOnCan(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = PWON_IDEL;
    static int32 delay_ms = 0;
    static Uint32 waitok_cnt = 0;
    static Uint16 can_enabled = 0U;
    ST_RANGE_PARA *pRange_vol = &st_range_vol_use;
    ST_RANGE_PARA *pRange_cur = &st_range_cur_use;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    // 下电优先级最高
    if (pwoff_flag == 1)
    {
        g_can_controller.request_pending = 0U;
        state = PWON_IDEL;
        return;
    }

    if (pPC->power_ctrl == POWER_ON)
    {
        pPC->power_ctrl = POWER_IDEL;
        pPC->pw_can_flag = 0U;
        can_enabled = (pPC->plusewidth.f32 > 0.0F) ? 1U : 0U;

        // 禁止输出，dataok 20 以上表示禁止输出的原因
        // if (output_disable.u16 != 0)
        // {
        //     pPC->dataok = 20 + output_disable.u16;
        //     pPC->isamp_avg.f32 = 0;
        //     pPC->vsamp_avg.f32 = 0;
        //     state = PWON_IDEL;
        //     return;
        // }

        pwon_flag = 1;
        // 允许输出
        EnOut();
        // 清零变量
        pPC->dataok = 0;
        pPC->isamp_avg.f32 = 0;
        pPC->vsamp_avg.f32 = 0;
        waitok_cnt = 0;
        // 打开输出继电器
        if (pPC->relay_on_always == 1 && ReadRelay() == 1)
        {
            state = PWON_VOL_RANGESW;
        }
        else
        {
            VoRelayOn();
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, 7, PWON_VOL_RANGESW);
        }
    }

    switch (state)
    {
    case PWON_IDEL:
        break;

    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case PWON_VOL_RANGESW:
        // 切换到上位机设置电压档位
        RangeVolSel(pRange_vol->range_index_PC);
        RangeCurSel(RANGE_CUR_50mA_INDEX);
        state = PWON_SET_REF;
        break;

    case PWON_SET_REF:
        // 开启保护
        EnableProt_All();
        // 设置电压值
        SetVolRef(pPC->v_set.f32);
        // 电流ref设置为50mA
        SetCurRef(50e-3);
        // 波形记录
        if (pPC->wave.enable == 1 && pPC->plusewidth.f32 > 0)
        {
            StartWaveRecord(pPC->plusewidth.f32 * 1000 + 20);
            WaveRangeSaveCur(RANGE_CUR_50mA_INDEX);
            WaveRangeSaveVol(pRange_vol->range_index_PC);
        }
        // 计算延时时间
        delay_ms = pPC->plusewidth.f32 * 1000 - pPC->sw_delay_ms - pPC->nplc.f32 * 20;
        if (delay_ms < 0)
            delay_ms = 0;
        // delay 并状态跳转
        FSM_DELAY_NEXT(st_delay, pPC->sw_delay_ms, PWON_CUR_RANGESW);
        break;

    case PWON_CUR_RANGESW:
        // 切换到上位机设置的电流档位
        RangeCurSel(pRange_cur->range_index_PC);
        // 电流ref设置为本档位最大值
        SetCurRef(st_range_cur_use.full_range);

        if (pPC->wave.enable == 1)
        {
            WaveRangeSwitchCur();
            WaveRangeSaveCur(pRange_cur->range_index_PC);
        }
        // 仅上电
        if (pPC->plusewidth.f32 < 0)
            state = PWON_UPDATE_TRIGGER;
        else
        {
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, delay_ms, PWON_SAMP_TRIGGER);
        }
        break;

    case PWON_SAMP_TRIGGER:
        TriggerAdcSamp();
        waitok_cnt = 0U;
        if (can_enabled != 0U)
        {
            CanController_RequestStart(CAN_CONTROLLER_DEFAULT_SAMPLE_ID);
            state = PWON_CAN_WAIT_TX;
        }
        else
        {
            state = PWON_WAIT_DATA;
        }
        break;

    case PWON_CAN_WAIT_TX:
        /* main发送期间定时任务继续运行，发送错误立即走原下电流程。 */
        waitok_cnt++;
        if (g_can_controller.tx_done != 0U)
        {
            if (g_can_controller.tx_status != CAN_CONTROLLER_STATUS_SUCCESS)
            {
                pPC->pw_can_flag = 2U;
                pPC->power_ctrl = POWER_OFF;
                state = PWON_IDEL;
            }
            else
            {
                waitok_cnt = 0U;
                state = PWON_WAIT_DATA;
            }
        }
        else if (waitok_cnt >= Tms_CNT(CAN_CONTROLLER_REPLY_TIMEOUT_MS))
        {
            g_can_controller.request_pending = 0U;
            pPC->pw_can_flag = 2U;
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        break;

    case PWON_WAIT_DATA:
        waitok_cnt++;
        if ((AdcSampOk() == 1U) &&
            ((can_enabled == 0U) || (g_can_controller.reply_received != 0U)))
        {
            pPC->dataok = ERR_OK;
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            if (can_enabled != 0U)
            {
                pPC->pw_can_flag = 1U;
            }
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        else if ((can_enabled != 0U) &&
                 (g_can_controller.reply_received == 0U) &&
                 (waitok_cnt >= Tms_CNT(CAN_CONTROLLER_REPLY_TIMEOUT_MS)))
        {
            pPC->pw_can_flag = 3U;
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        else if (waitok_cnt >
                 Tms_CNT(pPC->nplc.f32 * 1000 +
                         pPC->cfgvalue[EEROM_CONFIG_idss_overtime]))
        {
            pPC->dataok = ERR_IDSS_OVERTIME;
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        break;

    case PWON_UPDATE_TRIGGER:
        TriggerAdcSamp();
        state = PWON_UPDATE_WAIT;
        break;

    case PWON_UPDATE_WAIT:
        if (AdcSampOk() == 1)
        {
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            state = PWON_UPDATE_TRIGGER;
        }
        break;

    default:
        state = PWON_IDEL;
        break;
    }
}

void PowerOnUph_nometer(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = PWON_IDEL;
    static int32 delay_ms = 0;
    static Uint32 waitok_cnt = 0;
    ST_RANGE_PARA *pRange_vol = &st_range_vol_use;
    ST_RANGE_PARA *pRange_cur = &st_range_cur_use;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    // 下电优先级最高
    if (pwoff_flag == 1)
    {
        state = PWON_IDEL;
        return;
    }

    if (pPC->power_ctrl == POWER_ON)
    {
        pPC->power_ctrl = POWER_IDEL;

        // 禁止输出，dataok 20 以上表示禁止输出的原因
        if (output_disable.u16 != 0)
        {
            pPC->dataok = 20 + output_disable.u16;
            pPC->isamp_avg.f32 = 0;
            pPC->vsamp_avg.f32 = 0;
            state = PWON_IDEL;
            return;
        }

        pwon_flag = 1;
        // 允许输出
        EnOut();
        // 清零变量
        pPC->dataok = 0;
        pPC->isamp_avg.f32 = 0;
        pPC->vsamp_avg.f32 = 0;
        waitok_cnt = 0;
        // 打开输出继电器
        if (pPC->relay_on_always == 1 && ReadRelay() == 1)
        {
            state = PWON_VOL_RANGESW;
        }
        else
        {
            VoRelayOn();
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, 7, PWON_VOL_RANGESW);
        }
    }

    switch (state)
    {
    case PWON_IDEL:
        break;

    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case PWON_VOL_RANGESW:
        // 切换到上位机设置电压档位
        RangeVolSel(pRange_vol->range_index_PC);
        RangeCurSel(RANGE_CUR_50mA_INDEX);
        state = PWON_SET_REF;
        break;

    case PWON_SET_REF:
        // 开启保护
        EnableProt_All();
        // 设置电压值
        SetVolRef(pPC->v_set.f32);
        // 电流ref设置为50mA
        SetCurRef(50e-3);
        // 波形记录
        if (pPC->wave.enable == 1 && pPC->plusewidth.f32 > 0)
        {
            StartWaveRecord(pPC->plusewidth.f32 * 1000 + 20);
            WaveRangeSaveCur(RANGE_CUR_50mA_INDEX);
            WaveRangeSaveVol(pRange_vol->range_index_PC);
        }
        // 计算延时时间
        delay_ms = pPC->plusewidth.f32 * 1000 - pPC->sw_delay_ms - pPC->nplc.f32 * 20;
        if (delay_ms < 0)
            delay_ms = 0;
        // delay 并状态跳转
        FSM_DELAY_NEXT(st_delay, pPC->sw_delay_ms, PWON_CUR_RANGESW);
        break;

    case PWON_CUR_RANGESW:
        // 切换到上位机设置的电流档位
        RangeCurSel(pRange_cur->range_index_PC);
        // 电流ref设置为本档位最大值
        SetCurRef(st_range_cur_use.full_range);

        if (pPC->wave.enable == 1)
        {
            WaveRangeSwitchCur();
            WaveRangeSaveCur(pRange_cur->range_index_PC);
        }
        // 仅上电
        if (pPC->plusewidth.f32 < 0)
            state = PWON_UPDATE_TRIGGER;
        else
        {
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, delay_ms, PWON_SAMP_TRIGGER);
        }
        break;

    case PWON_SAMP_TRIGGER:
        // 采样
        TriggerAdcSamp();
        // 触发表采样
        if (mgmd_stSCIRx.trigger_meter == 1)
        {
            TriggerMeter();
        }
        waitok_cnt = 0;
        state = PWON_WAIT_DATA;
        break;

    case PWON_WAIT_DATA:
        // 等待数据ok
        // 超时判断
        waitok_cnt++;
        if (waitok_cnt > Tms_CNT(pPC->nplc.f32 * 1000 + pPC->cfgvalue[EEROM_CONFIG_idss_overtime]))
        {
            pPC->dataok = ERR_IDSS_OVERTIME;
            // 下电
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        // 数据采样ok，下电
        if (mgmd_stSCIRx.trigger_meter == 1)
        {
            if (revc_meter_flag == 1 && AdcSampOk() == 1)
            {
                // revc_meter_rst 结果
                pPC->dataok = 1;
                pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
                pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
                // 下电
                pPC->power_ctrl = POWER_OFF;
                state = PWON_IDEL;
            }
        }
        else
        {
            if (AdcSampOk() == 1)
            {
                pPC->dataok = 1;
                pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
                pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
                // 下电
                pPC->power_ctrl = POWER_OFF;
                state = PWON_IDEL;
            }
        }

        break;

    case PWON_UPDATE_TRIGGER:
        TriggerAdcSamp();
        state = PWON_UPDATE_WAIT;
        break;

    case PWON_UPDATE_WAIT:
        if (AdcSampOk() == 1)
        {
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            state = PWON_UPDATE_TRIGGER;
        }
        break;

    default:
        state = PWON_IDEL;
        break;
    }
}

void PowerOnUph(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = PWON_IDEL;
    static int32 delay_ms = 0;
    static Uint32 waitok_cnt = 0;
    ST_RANGE_PARA *pRange_vol = &st_range_vol_use;
    ST_RANGE_PARA *pRange_cur = &st_range_cur_use;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    // 下电优先级最高
    if (pwoff_flag == 1)
    {
        state = PWON_IDEL;
        return;
    }

    if (pPC->power_ctrl == POWER_ON)
    {
        pPC->power_ctrl = POWER_IDEL;

        // 禁止输出，dataok 20 以上表示禁止输出的原因
        if (output_disable.u16 != 0)
        {
            pPC->dataok = 20 + output_disable.u16;
            pPC->isamp_avg.f32 = 0;
            pPC->vsamp_avg.f32 = 0;
            state = PWON_IDEL;
            return;
        }

        pwon_flag = 1;
        // 允许输出
        EnOut();
        // 清零变量
        pPC->dataok = 0;
        pPC->isamp_avg.f32 = 0;
        pPC->vsamp_avg.f32 = 0;
        waitok_cnt = 0;
        // 打开输出继电器
        if (pPC->relay_on_always == 1 && ReadRelay() == 1)
        {
            state = PWON_VOL_RANGESW;
        }
        else
        {
            VoRelayOn();
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, 7, PWON_VOL_RANGESW);
        }
    }

    switch (state)
    {
    case PWON_IDEL:
        break;

    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case PWON_VOL_RANGESW:
        // 切换到上位机设置电压档位
        // 电流默认用  50mA 档位
        RangeVolSel(pRange_vol->range_index_PC);
        RangeCurSel(RANGE_CUR_50mA_INDEX);
        state = PWON_SET_REF;
        break;

    case PWON_SET_REF:
        // 开启保护
        EnableProt_All();
        // 设置电压值
        SetVolRef(pPC->v_set.f32);
        // 电流ref设置为50mA
        SetCurRef(50e-3);
        // 波形记录
        if (pPC->wave.enable == 1 && pPC->plusewidth.f32 > 0)
        {
            StartWaveRecord(pPC->plusewidth.f32 * 1000 + 20);
            WaveRangeSaveCur(RANGE_CUR_50mA_INDEX);
            WaveRangeSaveVol(pRange_vol->range_index_PC);
        }
        // 计算延时时间
        delay_ms = pPC->plusewidth.f32 * 1000 - pPC->nplc.f32 * 20;
        if (delay_ms < 0)
            delay_ms = 0;

        // 仅上电
        if (pPC->plusewidth.f32 < 0)
            state = PWON_UPDATE_TRIGGER;
        else
        {
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, delay_ms, PWON_SAMP_TRIGGER);
        }
        break;

    case PWON_SAMP_TRIGGER:
        // 采样
        TriggerAdcSamp();
        // 触发表采样
        if (mgmd_stSCIRx.trigger_meter == 1)
        {
            TriggerMeter();
        }
        waitok_cnt = 0;
        state = PWON_WAIT_DATA;
        break;

    case PWON_WAIT_DATA:
        // 等待数据ok
        // 超时判断
        waitok_cnt++;
        if (waitok_cnt > Tms_CNT(pPC->nplc.f32 * 1000 + pPC->cfgvalue[EEROM_CONFIG_idss_overtime]))
        {
            pPC->dataok = ERR_IDSS_OVERTIME;
            // 下电
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        // 数据采样ok，下电
        if (mgmd_stSCIRx.trigger_meter == 1)
        {
            if (revc_meter_flag == 1 && AdcSampOk() == 1)
            {
                // revc_meter_rst 结果
                pPC->dataok = 1;
                pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
                pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
                // 下电
                pPC->power_ctrl = POWER_OFF;
                state = PWON_IDEL;
            }
        }
        else
        {
            if (AdcSampOk() == 1)
            {
                pPC->dataok = 1;
                pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
                pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
                // 下电
                pPC->power_ctrl = POWER_OFF;
                state = PWON_IDEL;
            }
        }
        break;

    case PWON_UPDATE_TRIGGER:
        TriggerAdcSamp();
        state = PWON_UPDATE_WAIT;
        break;

    case PWON_UPDATE_WAIT:
        if (AdcSampOk() == 1)
        {
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            state = PWON_UPDATE_TRIGGER;
        }
        break;

    default:
        state = PWON_IDEL;
        break;
    }
}

void PowerOff(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = PWOFF_IDEL;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;
    static Uint32 discharge_cnt = 0;

    if (pPC->power_ctrl == POWER_OFF)
    {
        pPC->power_ctrl = POWER_IDEL;
        // 禁止输出
        DisOut();
        // 终止所有上电过程
        pwoff_flag = 1;
        // 关闭保护
        DisableProt_All();
        // 清零变量
        discharge_cnt = 0;
        state = PWOFF_SET_REF;
    }

    switch (state)
    {
    case PWOFF_IDEL:
        pwoff_flag = 0;
        break;

    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case PWOFF_SET_REF:
        SetVolRef(0);
        SetCurRef(0);
        // delay 并状态跳转
        FSM_DELAY_NEXT(st_delay, pPC->delay_ref, PWOFF_DISCHARGE);
        break;

    case PWOFF_DISCHARGE:
        DischargeOn();
        if (pPC->auto_discharge == 1)
        {
            // 最大放电时间
            if (discharge_cnt++ > Tms_CNT(pPC->delay_discharge))
                state = PWOFF_RANGESW;

            // 电压小于 2V
            if (st_ADCdata.vol < 2)
                FSM_DELAY_NEXT(st_delay, pPC->auto_discharge_delay, PWOFF_RANGESW);
        }
        else
            // 固定放电时间，delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, pPC->delay_discharge, PWOFF_RANGESW);
        break;

    case PWOFF_RANGESW:
        DischargeOff();
        // 切换档位为默认值
        RangeVolSel(RANGE_VOL_400V_INDEX);
        RangeCurSel(RANGE_CUR_50mA_INDEX);
        // 关闭继电器
        if (pPC->relay_on_always == 0)
            VoRelayOff();
        // 若发生了打火保护，复位一次dac
        if (pPC->dataok == ERR_PROT_FIRE)
        {
            //            ResetDac();
        }
        pwon_flag = 0;
        state = PWOFF_IDEL;
        break;

    default:
        state = PWOFF_IDEL;
        break;
    }
}

void PowerOnVbr(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = PWONVBR_IDEL;
    static int32 delay_ms = 0;
    static float32 last_vol1ms = 0;
    static float32 last_vol10ms = 0;
    static Uint16 vbr_pwon_cnt = 0;
    static Uint16 cnt_1ms = 0;
    static Uint16 cnt_10ms = 0;
    static Uint16 vbr_1ms_cnt = 0;
    static Uint16 vbr_10ms_cnt = 0;
    ST_RANGE_PARA *pRange_vol = &st_range_vol_use;
    ST_RANGE_PARA *pRange_cur = &st_range_cur_use;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    // 下电优先级最高
    if (pwoff_flag == 1)
    {
        state = PWON_IDEL;
        return;
    }

    if (pPC->power_ctrl == POWER_ON)
    {
        pPC->power_ctrl = POWER_IDEL;
        // 禁止输出，dataok 20 以上表示禁止输出的原因
        // if (output_disable.u16 != 0)
        // {
        //     pPC->dataok = 20 + output_disable.u16;
        //     pPC->isamp_avg.f32 = 0;
        //     pPC->vsamp_avg.f32 = 0;
        //     state = PWON_IDEL;
        //     return;
        // }

        pwon_flag = 1;
        // 允许输出
        EnOut();
        // 清零变量
        pPC->dataok = 0;
        pPC->isamp_avg.f32 = 0;
        pPC->vsamp_avg.f32 = 0;
        cnt_1ms = 0;
        cnt_10ms = 0;
        vbr_1ms_cnt = 0;
        vbr_10ms_cnt = 0;
        vbr_pwon_cnt = 0;
        last_vol10ms = 0;
        last_vol1ms = 0;
        // 打开输出继电器
        if (pPC->relay_on_always == 1 && ReadRelay() == 1)
        {
            state = PWONVBR_RANGESW;
        }
        else
        {
            VoRelayOn();
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, 7, PWONVBR_RANGESW);
        }
    }

    switch (state)
    {
    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case PWONVBR_IDEL:
        break;

    case PWONVBR_RANGESW:
        // 切换档位为上位机设置值
        RangeVolSel(pRange_vol->range_index_PC);
        RangeCurSel(pRange_cur->range_index_PC);
        state = PWONVBR_SETCUR;
        break;

    case PWONVBR_SETCUR:
        // 开启保护
        EnableProt_All();
        // 设置dac
        SetCurRef(pPC->i_set.f32);
        SetVolRef(pPC->v_set.f32);
        // 波形记录
        if (pPC->wave.enable == 1)
        {
            StartWaveRecord(pPC->wave.time_ms);
            WaveRangeSaveCur(pRange_cur->range_index_PC);
            WaveRangeSaveVol(pRange_vol->range_index_PC);
        }
        state = PWONVBR_DETECT_VBR;
        break;

    case PWONVBR_DETECT_VBR:
        // 开始检测是否达到Vbr
        // 超时下电1s
        vbr_pwon_cnt++;
        if (vbr_pwon_cnt > Tms_CNT(pPC->cfgvalue[EEROM_CONFIG_vbr_overtime]))
        {
            vbr_pwon_cnt = 0;
            pPC->dataok = ERR_VBR_OVERTIME;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_1ms;
            pPC->isamp_avg.f32 = st_ADCdata.cur_1ms;
            // 下电
            pPC->power_ctrl = POWER_OFF;
            state = PWONVBR_IDEL;
        }

        // 判断电压是否平稳
        if (st_ADCdata.vol > 50)
        {
            if (++cnt_1ms > Tms_CNT(1))
            {
                cnt_1ms = 0;

                if (st_ADCdata.vol_1ms - last_vol1ms < 0.01 * pPC->cfgvalue[EEROM_CONFIG_vbr_thx100] &&
                    last_vol1ms - st_ADCdata.vol_1ms < 0.01 * pPC->cfgvalue[EEROM_CONFIG_vbr_thx100])
                    vbr_1ms_cnt++;
                else
                    vbr_1ms_cnt = 0;

                last_vol1ms = st_ADCdata.vol_1ms;
            }

            if (++cnt_10ms > Tms_CNT(10))
            {
                cnt_10ms = 0;

                if (st_ADCdata.vol_1ms - last_vol10ms < 10 * 0.01 * pPC->cfgvalue[EEROM_CONFIG_vbr_thx100] &&
                    last_vol10ms - st_ADCdata.vol_1ms < 10 * 0.01 * pPC->cfgvalue[EEROM_CONFIG_vbr_thx100])
                    vbr_10ms_cnt++;
                else
                    vbr_10ms_cnt = 0;

                last_vol10ms = st_ADCdata.vol_1ms;
            }

            if (vbr_1ms_cnt >= pPC->cfgvalue[EEROM_CONFIG_vbr_fast_cnt] ||
                vbr_10ms_cnt >= pPC->cfgvalue[EEROM_CONFIG_vbr_slow_cnt])
            {
                delay_ms = pPC->plusewidth.f32 * 1000 - 10;
                if (delay_ms < 0)
                    delay_ms = 0;

                // delay 并且状态跳转
                FSM_DELAY_NEXT(st_delay, delay_ms, PWONVBR_DELAY_PLUSEWIDTH);
            }
        }
        else
        {
            cnt_1ms = 0;
            cnt_10ms = 0;
            vbr_1ms_cnt = 0;
            vbr_10ms_cnt = 0;
        }
        break;

    case PWONVBR_DELAY_PLUSEWIDTH:
        // dataok放在延时后  记录此时的电压、电流值，以及反馈ok标志
        pPC->dataok = 1;
        pPC->vsamp_avg.f32 = st_ADCdata.vol_1ms;
        pPC->isamp_avg.f32 = st_ADCdata.cur_1ms;
        // 下电
        pPC->power_ctrl = POWER_OFF;
        state = PWONVBR_IDEL;
        break;

    default:
        state = PWONVBR_IDEL;
        break;
    }
}

void PowerOnCurTest(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = PWONVBR_IDEL;
    static int32 delay_ms = 0;
    ST_RANGE_PARA *pRange_vol = &st_range_vol_use;
    ST_RANGE_PARA *pRange_cur = &st_range_cur_use;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    // 下电优先级最高
    if (pwoff_flag == 1)
    {
        state = PWON_IDEL;
        return;
    }

    if (pPC->power_ctrl == POWER_ON)
    {
        pPC->power_ctrl = POWER_IDEL;
        // // 禁止输出，dataok 20 以上表示禁止输出的原因
        // if (output_disable.u16 != 0)
        // {
        //     pPC->dataok = 20 + output_disable.u16;
        //     pPC->isamp_avg.f32 = 0;
        //     pPC->vsamp_avg.f32 = 0;
        //     state = PWON_IDEL;
        //     return;
        // }

        pwon_flag = 1;
        // 允许输出
        EnOut();
        // 清零变量
        pPC->dataok = 0;
        pPC->isamp_avg.f32 = 0;
        pPC->vsamp_avg.f32 = 0;
        // 打开输出继电器
        if (pPC->relay_on_always == 1 && ReadRelay() == 1)
        {
            state = PWONVBR_RANGESW;
        }
        else
        {
            VoRelayOn();
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, 7, PWONVBR_RANGESW);
        }
    }

    switch (state)
    {
    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;

    case PWONVBR_IDEL:
        break;

    case PWONVBR_RANGESW:
        // 切换档位为上位机设置值
        RangeVolSel(pRange_vol->range_index_PC);
        RangeCurSel(pRange_cur->range_index_PC);
        state = PWONVBR_SETCUR;
        break;

    case PWONVBR_SETCUR:
        // 开启保护
        //        EnableProt_All();
        // 设置dac
        SetCurRef(pPC->i_set.f32);
        SetVolRef(pPC->v_set.f32);
        // 波形记录
        if (pPC->wave.enable == 1 && pPC->plusewidth.f32 > 0)
        {
            StartWaveRecord(pPC->plusewidth.f32 * 1000 + 20);
            WaveRangeSaveCur(pRange_cur->range_index_PC);
            WaveRangeSaveVol(pRange_vol->range_index_PC);
        }
        // 仅上电
        if (pPC->plusewidth.f32 < 0)
            state = PWON_UPDATE_TRIGGER;
        else
        {
            // 计算延时时间
            delay_ms = pPC->plusewidth.f32 * 1000 - pPC->nplc.f32 * 20;
            if (delay_ms < 0)
                delay_ms = 0;
            // delay 并状态跳转
            FSM_DELAY_NEXT(st_delay, delay_ms, PWON_SAMP_TRIGGER);
        }
        break;

    case PWON_SAMP_TRIGGER:
        // 采样
        TriggerAdcSamp();
        state = PWON_WAIT_DATA;
        break;

    case PWON_WAIT_DATA:
        // 等待数据ok
        // 数据采样ok，下电
        if (AdcSampOk() == 1)
        {
            pPC->dataok = 1;
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            // 下电
            pPC->power_ctrl = POWER_OFF;
            state = PWON_IDEL;
        }
        break;

    case PWON_UPDATE_TRIGGER:
        TriggerAdcSamp();
        state = PWON_UPDATE_WAIT;
        break;

    case PWON_UPDATE_WAIT:
        if (AdcSampOk() == 1)
        {
            pPC->isamp_avg.f32 = st_ADCdata.cur_avg;
            pPC->vsamp_avg.f32 = st_ADCdata.vol_avg;
            state = PWON_UPDATE_TRIGGER;
        }
        break;
    default:
        state = PWON_IDEL;
        break;
    }
}

// 自动校零功能
void AutoZeroFunc(void)
{
    static ST_FSM_DELAY st_delay = {0, 0, 0, 0};
    static Uint16 state = 0;
    static Uint16 start_range_cur = 0, start_range_vol = 0;
    static Uint32 delay_ms = 0;
    ST_SCI_RX_PAR *pPC = &mgmd_stSCIRx;

    // 下电优先级最高
    if (pwoff_flag == 1)
    {
        state = 0;
        return;
    }

    if (pPC->cmd_autozero == 1)
    {
        pPC->cmd_autozero = 0;
        pPC->autozero_okflag = 0;
        // 允许输出
        EnOut();
        // 断开继电器
        VoRelayOff();
        // 初值
        pPC->nplc.f32 = 10;
        start_range_cur = RANGE_CUR_1nA_INDEX;
        start_range_vol = RANGE_VOL_400V_INDEX;
        // delay 并且状态跳转
        FSM_DELAY_NEXT(st_delay, 10, 2);
    }

    switch (state) // state
    {
    case S_FSM_DELAY:
        FSM_DELAY_FUNC(st_delay);
        break;
    case 0:
        break;

    case 2:
        // 电压档位切
        RangeVolSel(start_range_vol);
        // 电流切换档位
        RangeCurSel(start_range_cur);
        // 输出200V电压，电流ref设置为本档位最大值
        SetVolRef(200);
        SetCurRef(st_range_cur_use.full_range);
        // delay
        if (start_range_cur == RANGE_CUR_1nA_INDEX ||
            start_range_cur == RANGE_CUR_10nA_INDEX ||
            start_range_cur == RANGE_CUR_100nA_INDEX)
            delay_ms = 10000;
        else
            delay_ms = 2000;
        // delay 并且状态跳转
        FSM_DELAY_NEXT(st_delay, delay_ms, 3);
        break;

    case 3:
        // sample
        TriggerAdcSamp();
        state = 4;
        break;

    case 4:
        if (AdcSampOk() == 1)
        {
            // 将本档位使用的b减去当前采样值
            pPC->coef.f32v_adc_p_b[EEROM_PARA_V_NUM + start_range_cur].f32 -= st_ADCdata.cur_avg;
            st_eerom_para_COEF.f32v_adc_p_b[EEROM_PARA_V_NUM + start_range_cur] -= st_ADCdata.cur_avg;

            // 档位切换
            if (start_range_cur < RANGE_CUR_50mA_INDEX)
            {
                start_range_cur++;
                state = 2;
            }
            else
            {
                // 校准完毕
                // 系数写入
                //  pPC->coef_mode1 = EEROM_W_COEF1;
                //  pPC->coef_mode2 = EEROM_W_COEF2;
                // 标志位置1
                pPC->autozero_okflag = 1;
                pPC->nplc.f32 = 1;
                // 下电恢复默认值
                pPC->power_ctrl = POWER_OFF;
                state = 0;
            }
        }
        break;

    default:
        state = 0;
        break;
    }
}
