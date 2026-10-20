/*
 * task_eeprom_coef.c
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#include "COMMON.h"

#define ST_STORED ST_EEROM_COEF_STORED
#define ST_PARA ST_EEROM_COEF
#define PARA_LEN EEROM_COEF_LEN
#define PARA_LEN_NOCRC EEROM_COEF_LEN_NOCRC
#define START_ADDR EEROM_COEF_START
#define WRITE_CMD1 EEROM_W_COEF1
#define WRITE_CMD2 EEROM_W_COEF2
#define EEPROM_INIT_FUNC InitTaskEeprom_COEF
#define EEPROM_RUN_FUNC TaskEeprom_COEF

ST_STORED EEPROM_ST_STORED_READ(COEF), EEPROM_ST_STORED_WRITE(COEF);
ST_PARA EEPROM_ST_PARA(COEF);

static ST_STORED *pRead = &EEPROM_ST_STORED_READ(COEF);
static ST_STORED *pWrite = &EEPROM_ST_STORED_WRITE(COEF);
static ST_PARA *pPara = &EEPROM_ST_PARA(COEF);

// 写入eeprom之前检查pPara的参数是否合理
static Uint16 CheckValid(ST_PARA *pPara)
{
    Uint16 i = 0;

    //---根据实际修改
    // 判断系数是否符合规范
    // 所有的k都在0.9 - 1.1之间
    //    for(i = 0; i < EEROM_PARA_NUM; i++)
    //    {
    //        if((pPara->f32v_adc_p_k[i] < 0.9) || (pPara->f32v_adc_p_k[i] > 1.1))
    //        {
    //            return 1;
    //        }
    //    }
    // ok
    return 0;
}

// pPara参数赋初值---根据实际修改
static void ParaInit(void)
{
    Uint16 i = 0;
    memset(pRead, 0, sizeof(ST_STORED));
    memset(pPara, 0, sizeof(ST_PARA));

    for (i = 0; i < EEROM_PARA_NUM; i++)
    {
        pPara->f32v_adc_p_k[i] = 1;
        pPara->f32v_adc_p_b[i] = 0;
    }
}

// 用读到的系数，将 pRead 的数据赋值给 pPara ---根据实际修改
static void ReadToPara(void)
{
    Uint32 u32tmp = 0;
    Uint16 i = 0;

    for (i = 0; i < EEROM_PARA_NUM; i++)
    {
        u32tmp = U8ToU32(pRead->v_adc_p_k_byte[i]);
        pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
        u32tmp = U8ToU32(pRead->v_adc_p_b_byte[i]);
        pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
    }

    pPara->utctime = U8ToU32(pRead->utctime);
    pPara->sn_no[0] = U8ToU32(pRead->sn_no[0]);
    pPara->sn_no[1] = U8ToU32(pRead->sn_no[1]);
    pPara->sn_no[2] = U8ToU32(pRead->sn_no[2]);
}

// 将正在使用的值赋值给上位机 ---根据实际修改
static void ParaToPC(void)
{
    Uint16 i = 0;

    for (i = 0; i < EEROM_PARA_NUM; i++)
    {
        mgmd_stSCIRx.coef.f32v_adc_p_k[i].f32 = pPara->f32v_adc_p_k[i];
        mgmd_stSCIRx.coef.f32v_adc_p_b[i].f32 = pPara->f32v_adc_p_b[i];
    }
    mgmd_stSCIRx.coef.utctime.u32 = pPara->utctime;
    mgmd_stSCIRx.coef.sn_no[0].u32 = pPara->sn_no[0];
    mgmd_stSCIRx.coef.sn_no[1].u32 = pPara->sn_no[1];
    mgmd_stSCIRx.coef.sn_no[2].u32 = pPara->sn_no[2];
}

// 将下发的参数赋值给pPara  ---根据实际修改
static void PCToPara(void)
{
    Uint16 i = 0;

    for (i = 0; i < EEROM_PARA_NUM; i++)
    {
        pPara->f32v_adc_p_k[i] = mgmd_stSCIRx.coef.f32v_adc_p_k[i].f32;
        pPara->f32v_adc_p_b[i] = mgmd_stSCIRx.coef.f32v_adc_p_b[i].f32;
    }

    pPara->utctime = mgmd_stSCIRx.coef.utctime.u32;
    pPara->sn_no[0] = mgmd_stSCIRx.coef.sn_no[0].u32;
    pPara->sn_no[1] = mgmd_stSCIRx.coef.sn_no[1].u32;
    pPara->sn_no[2] = mgmd_stSCIRx.coef.sn_no[2].u32;
}

// 将 pPara 的数据赋值给 pWrite，用于写入  ---根据实际修改
static void ParaToWrite(void)
{
    Uint16 i = 0;
    Uint32 u32tmp = 0;

    for (i = 0; i < EEROM_PARA_NUM; i++)
    {
        u32tmp = FTOIEEE(pPara->f32v_adc_p_k[i]);
        U32ToU8(u32tmp, pWrite->v_adc_p_k_byte[i]);
        u32tmp = FTOIEEE(pPara->f32v_adc_p_b[i]);
        U32ToU8(u32tmp, pWrite->v_adc_p_b_byte[i]);
    }

    U32ToU8(pPara->utctime, pWrite->utctime);
    U32ToU8(pPara->sn_no[0], pWrite->sn_no[0]);
    U32ToU8(pPara->sn_no[1], pWrite->sn_no[1]);
    U32ToU8(pPara->sn_no[2], pWrite->sn_no[2]);
}

/* Old four-range voltage coefficients must not be applied to the new ranges.
 * Current calibration and the EEPROM structure/Modbus addresses stay intact.
 * The layout ID is saved only with an explicit coefficient write command.
 */
static void CheckVoltageRangeLayout(void)
{
    Uint16 i;
    if (U8ToU32(pRead->reserve[0]) == EEROM_V_RANGE_LAYOUT_ID)
        return;

    for (i = 0; i < EEROM_V_ADCP_NUM; i++)
    {
        pPara->f32v_adc_p_k[i] = 1.0f;
        pPara->f32v_adc_p_b[i] = 0.0f;
        pPara->f32v_adc_n_k[i] = 1.0f;
        pPara->f32v_adc_n_b[i] = 0.0f;
        pPara->f32v_dac_p_k[i] = 1.0f;
        pPara->f32v_dac_p_b[i] = 0.0f;
        pPara->f32v_dac_n_k[i] = 1.0f;
        pPara->f32v_dac_n_b[i] = 0.0f;
    }
}

//-------- 以下所有内容不能改变--------------------

void EEPROM_INIT_FUNC(void)
{
    // pPara参数赋初值
    ParaInit();

    // 读eerom, 读到就用eerom的数据，没读到就用默认的
    PS_IIC0_SeqRead(START_ADDR, (Uint8 *)pRead, PARA_LEN);
    pPara->eerom_ok = CheckReadCrc((Uint8 *)pRead, PARA_LEN_NOCRC);
    if (pPara->eerom_ok == EEROM_DATAOK)
    {
        // 用读到的系数，将 pRead 的数据赋值给 pPara
        ReadToPara();
    }

    // 将正在使用的值赋值给上位机
    CheckVoltageRangeLayout();
    ParaToPC();
}

static void WriteEeromPara(ST_PARA *pPara)
{

    Uint16 crc_tmp = 0;

    //    DisableDog();

    memset(pWrite, 0, sizeof(ST_STORED));
    U16ToU8(EEROM_HEAD, pWrite->head);
    U16ToU8(EEROM_TAIL, pWrite->tail);

    // 将 pPara 的数据赋值给 pWrite，用于写入
    ParaToWrite();
    U32ToU8(EEROM_V_RANGE_LAYOUT_ID, pWrite->reserve[0]);

    // 将 pWrite 写入
    crc_tmp = CalCrc((Uint8 *)pWrite, PARA_LEN_NOCRC);
    U16ToU8(crc_tmp, pWrite->crc);
    PS_IIC0_SeqWrite(START_ADDR, (Uint8 *)pWrite, PARA_LEN);

    //    EnableWDog();
}

// 上位机下发参数，写进eerom中
void EEPROM_RUN_FUNC(void)
{
    Uint16 rst = 0;

    if ((mgmd_stSCIRx.coef_mode1 == WRITE_CMD1) && (mgmd_stSCIRx.coef_mode2 == WRITE_CMD2))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        // 将下发的参数赋值给pPara
        PCToPara();

        // 检查下发参数是否ok
        rst = CheckValid(pPara);
        if (rst != 0)
            return;

        // ok则写全部系数
        WriteEeromPara(pPara);
    }
}
