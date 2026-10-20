/*
 * task_eeprom_ip.c
 *
 *  Created on: 2025年7月10日
 *      Author: m
 */

#include "COMMON.h"

#define	NAME				IP
#define ST_STORED			ST_EEROM_PARA_IP_STORED
#define ST_PARA				ST_EEROM_PARA_IP
#define PARA_LEN			EEROM_PARA_IP_LEN
#define PARA_LEN_NOCRC		EEROM_PARA_IP_LEN_NOCRC
#define START_ADDR 			EEROM_PARA_IP_START
#define WRITE_CMD1			EEROM_W_IP1
#define WRITE_CMD2			EEROM_W_IP2
#define EEPROM_INIT_FUNC	InitTaskEeprom_IP
#define EEPROM_RUN_FUNC		TaskEeprom_IP


ST_STORED EEPROM_ST_STORED_READ(IP), EEPROM_ST_STORED_WRITE(IP);
ST_PARA EEPROM_ST_PARA(IP);

static ST_STORED *pRead = &EEPROM_ST_STORED_READ(IP);
static ST_STORED *pWrite = &EEPROM_ST_STORED_WRITE(IP);
static ST_PARA *pPara = &EEPROM_ST_PARA(IP);


Uint8 local_lip[4] = {192, 168, 1, 66};
// 写入eeprom之前检查pPara的参数是否合理
static Uint16 CheckValid(ST_PARA* pPara)
{
    Uint16 i = 0;

    //---根据实际修改
    for(i = 0; i < 4; i++)
    {
        if(pPara->ip[i] > 255)
            return 1;
    }

    //ok
    return 0;
}

// pPara参数赋初值---根据实际修改
static void ParaInit(void)
{
    memset(pRead, 0, sizeof(ST_STORED));
    memset(pPara, 0, sizeof(ST_PARA));

    pPara->ip[0] = local_lip[0];
    pPara->ip[1] = local_lip[1];
    pPara->ip[2] = local_lip[2];
    pPara->ip[3] = local_lip[3];
}

// 用读到的系数，将 pRead 的数据赋值给 pPara ---根据实际修改
static void ReadToPara(void)
{
    pPara->ip[0] = pRead->ip[0];
    pPara->ip[1] = pRead->ip[1];
    pPara->ip[2] = pRead->ip[2];
    pPara->ip[3] = pRead->ip[3];
}

// 将正在使用的值赋值给上位机 ---根据实际修改
static void ParaToPC(void)
{
    mgmd_stSCIRx.ip.addr1 = pPara->ip[0];
    mgmd_stSCIRx.ip.addr2 = pPara->ip[1];
    mgmd_stSCIRx.ip.addr3 = pPara->ip[2];
    mgmd_stSCIRx.ip.addr4 = pPara->ip[3];
}

//将下发的参数赋值给pPara  ---根据实际修改
static void PCToPara(void)
{
	pPara->ip[0] = mgmd_stSCIRx.ip.addr1;
	pPara->ip[1] = mgmd_stSCIRx.ip.addr2;
	pPara->ip[2] = mgmd_stSCIRx.ip.addr3;
	pPara->ip[3] = mgmd_stSCIRx.ip.addr4;
}

// 将 pPara 的数据赋值给 pWrite，用于写入  ---根据实际修改
static void ParaToWrite(void)
{
    pWrite->ip[0] = pPara->ip[0];
    pWrite->ip[1] = pPara->ip[1];
    pWrite->ip[2] = pPara->ip[2];
    pWrite->ip[3] = pPara->ip[3];
}


//-------- 以下所有内容不能改变--------------------

void EEPROM_INIT_FUNC(void)
{
	// pPara参数赋初值
    ParaInit();

    //读eerom，读到就用eerom的数据，没读到就用默认的
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








