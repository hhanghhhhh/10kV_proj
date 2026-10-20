/*
 * task_eeprom_config.c
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#include "COMMON.h"

#define ST_STORED			ST_EEROM_CONFIG_STORED
#define ST_PARA				ST_EEROM_CONFIG
#define PARA_LEN			EEROM_CONFIG_LEN
#define PARA_LEN_NOCRC		EEROM_CONFIG_LEN_NOCRC
#define START_ADDR 			EEROM_CONFIG_START
#define WRITE_CMD1			EEROM_W_CONFIG1
#define WRITE_CMD2			EEROM_W_CONFIG2
#define EEPROM_INIT_FUNC	InitTaskEeprom_CONFIG
#define EEPROM_RUN_FUNC		TaskEeprom_CONFIG

ST_STORED EEPROM_ST_STORED_READ(CONFIG), EEPROM_ST_STORED_WRITE(CONFIG);
ST_PARA EEPROM_ST_PARA(CONFIG);

static ST_STORED *pRead = &EEPROM_ST_STORED_READ(CONFIG);
static ST_STORED *pWrite = &EEPROM_ST_STORED_WRITE(CONFIG);
static ST_PARA *pPara = &EEPROM_ST_PARA(CONFIG);




// 写入eeprom之前检查pPara的参数是否合理
static Uint16 CheckValid(ST_PARA* pPara)
{

    //---根据实际修改
    // 不 ok 则 return 非 0

    //ok
    return 0;
}

// pPara参数赋初值---根据实际修改
static void ParaInit(void)
{
    memset(pRead, 0, sizeof(ST_STORED));
    memset(pPara, 0, sizeof(ST_PARA));

    pPara->data[EEROM_CONFIG_vbr_thx100] = 100;				// 1V
    pPara->data[EEROM_CONFIG_ovp_th] = 200;
    pPara->data[EEROM_CONFIG_fire_th] = 500;
    pPara->data[EEROM_CONFIG_fire_drop_ratio_100] = 50;
    pPara->data[EEROM_CONFIG_fire_drop_value] = 100;
    pPara->data[EEROM_CONFIG_scp_th] = 20;
    pPara->data[EEROM_CONFIG_scp_time_0p1ms] = 10;			//1ms
    pPara->data[EEROM_CONFIG_idss_overtime] = 100;
    pPara->data[EEROM_CONFIG_vbr_overtime] = 1000;
    pPara->data[EEROM_CONFIG_vbr_shorttime_0p1ms] = 1500;	//150ms
    pPara->data[EEROM_CONFIG_vbr_fast_cnt] = 2;				// 快速判断 1 次
    pPara->data[EEROM_CONFIG_vbr_slow_cnt] = 2;				// 慢速判断 2 次
}

// 用读到的系数，将 pRead 的数据赋值给 pPara ---根据实际修改
static void ReadToPara(void)
{
    Uint16 i = 0;

    for(i = 0; i < EEROM_CONFIG_NUM; i++)
    {
    	pPara->data[i] = U8ToU16(pRead->data[i]);
    }
}

// 将正在使用的值赋值给上位机 ---根据实际修改
static void ParaToPC(void)
{
	Uint16 i = 0;

    for(i = 0; i < EEROM_CONFIG_NUM; i++)
    {
    	mgmd_stSCIRx.cfgvalue[i] = pPara->data[i];
    }
}

//将下发的参数赋值给pPara  ---根据实际修改
static void PCToPara(void)
{
	Uint16 i = 0;

	for(i = 0; i < EEROM_CONFIG_NUM; i++)
	{
		pPara->data[i] = mgmd_stSCIRx.cfgvalue[i];
	}
}

// 将 pPara 的数据赋值给 pWrite，用于写入  ---根据实际修改
static void ParaToWrite(void)
{
	Uint16 i = 0;

    for(i = 0; i < EEROM_CONFIG_NUM; i++)
    {
        U16ToU8(pPara->data[i], pWrite->data[i]);
    }
}



//-------- 以下所有内容不能改变--------------------

void EEPROM_INIT_FUNC(void)
{
	// pPara参数赋初值
	ParaInit();

    //读eerom, 读到就用eerom的数据，没读到就用默认的
    PS_IIC0_SeqRead(START_ADDR, (Uint8*)pRead, PARA_LEN);
    pPara->eerom_ok = CheckReadCrc((Uint8*)pRead, PARA_LEN_NOCRC);
    if(pPara->eerom_ok == EEROM_DATAOK)
    {
    	// 用读到的系数，将 pRead 的数据赋值给 pPara
    	ReadToPara();
    }

    //将正在使用的值赋值给上位机
    ParaToPC();
}



static void WriteEeromPara(ST_PARA* pPara)
{

    Uint16 crc_tmp = 0;

//    DisableDog();

    memset(pWrite, 0, sizeof(ST_STORED));
    U16ToU8(EEROM_HEAD, pWrite->head);
    U16ToU8(EEROM_TAIL, pWrite->tail);

    // 将 pPara 的数据赋值给 pWrite，用于写入
    ParaToWrite();

    // 将 pWrite 写入
    crc_tmp = CalCrc((Uint8*)pWrite, PARA_LEN_NOCRC);
    U16ToU8(crc_tmp, pWrite->crc);
    PS_IIC0_SeqWrite(START_ADDR, (Uint8*)pWrite, PARA_LEN);

//    EnableWDog();
}

//上位机下发参数，写进eerom中
void EEPROM_RUN_FUNC(void)
{
    Uint16 rst = 0;

    if((mgmd_stSCIRx.coef_mode1 == WRITE_CMD1) && (mgmd_stSCIRx.coef_mode2 == WRITE_CMD2))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //将下发的参数赋值给pPara
        PCToPara();

        //检查下发参数是否ok
        rst = CheckValid(pPara);
        if(rst != 0)
            return;

        //ok则写全部系数
        WriteEeromPara(pPara);
    }
}







