/***************************************************************************
*   NAME:           EEPROM.c
*   DESCRIPTION:    EEPROM Function file
*   VERSION:        1.0
*   AUTHOR:         LuXi
*   ACTION:         build
*   DATE:           2013-3-21
***************************************************************************/
#include "TypeDefine.h"
#include "drv_Eeprom.h"
#include <string.h>
#include "drv_MgModbus.h"
#include "drv_ModbusData.h"
#include "drv_Adc.h"
#include "test_hh.h"
#include "task_ADC.h"
#include "task_Transfer.h"


ST_EEROM_PARA_STORED st_eerom_para_write, st_eerom_para_read;
ST_EEROM_PARA st_eerom_para;

ST_EEROM_PARA11_STORED st_eerom_para11_read;
ST_EEROM_PARA11 st_eerom_para11;


extern Uint8 local_lip[4];

static void CheckValid(ST_EEROM_PARA* pPara)
{
//    Uint16 i = 0;
    //判断系数是否符合规范


    //所有的k都在0.9 - 1.1之间
//    for(i = 0; i < EEROM_PARA_NUM; i++)
//    {
//        if((pPara->f32v_adc_p_k[i] < 0.9) || (pPara->f32v_adc_p_k[i] > 1.1))
//        {
//            pPara->eerom_ok = EEROM_DATAERR_OVERRANG;
//            return;
//        }
//    }


}

#pragma CODE_SECTION(InitEeromPara,"runflashfuncs");
void InitEeromPara(ST_EEROM_PARA* pPara)
{
    memset(pPara, 0, sizeof(ST_EEROM_PARA));

    Uint16 i = 0;
    //参数赋初值
    for(i = 0; i < EEROM_PARA_NUM; i++)
    {
        pPara->f32v_adc_p_k[i] = 1;
        pPara->f32v_adc_p_b[i] = 0;
    }

    //读eerom，读到就用eerom的数据，没读到就用初始化的
    ReadEeromPara(&st_eerom_para_read, pPara);

    if(pPara->eerom_ok == EEROM_DATAOK)
    {
        CheckValid(pPara);
    }
}


//上位机下发参数，写进eerom中
#pragma CODE_SECTION(RecvAndWriteEerom,"runflashfuncs");
void RecvAndWriteEerom(ST_EEROM_PARA* pPara)
{
    Uint32 u32tmp = 0;
    Uint16 i = 0;


    //将参数写入eerom中
    //写全部系数
    if((mgmd_stSCIRx.coef_mode1 == 1) && (mgmd_stSCIRx.coef_mode2 == 2))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //写全部系数
        for(i = 0; i < EEROM_PARA_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4]) << 16 | *md_u16RDWEBuf2[i*4+1];
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4+2]) << 16 | *md_u16RDWEBuf2[i*4+3];
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 3) && (mgmd_stSCIRx.coef_mode2 == 4))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电压采样和电压输出系数
        for(i = 0; i < EEROM_PARA_V_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4]) << 16 | *md_u16RDWEBuf2[i*4+1];
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4+2]) << 16 | *md_u16RDWEBuf2[i*4+3];
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 5) && (mgmd_stSCIRx.coef_mode2 == 6))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电流采样和电压输出系数
        for(i = EEROM_PARA_V_NUM; i < EEROM_PARA_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4]) << 16 | *md_u16RDWEBuf2[i*4+1];
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4+2]) << 16 | *md_u16RDWEBuf2[i*4+3];
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }

    else if((mgmd_stSCIRx.coef_mode1 == 7) && (mgmd_stSCIRx.coef_mode2 == 8))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电压采样k
        for(i = 0; i < EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4]) << 16 | *md_u16RDWEBuf2[i*4+1];
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 9) && (mgmd_stSCIRx.coef_mode2 == 10))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电压采样b
        for(i = 0; i < EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4+2]) << 16 | *md_u16RDWEBuf2[i*4+3];
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 11) && (mgmd_stSCIRx.coef_mode2 == 12))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电压输出k
        for(i = EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM; i < EEROM_PARA_V_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4]) << 16 | *md_u16RDWEBuf2[i*4+1];
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 13) && (mgmd_stSCIRx.coef_mode2 == 14))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电压输出b
        for(i = EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM; i < EEROM_PARA_V_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4+2]) << 16 | *md_u16RDWEBuf2[i*4+3];
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }

    else if((mgmd_stSCIRx.coef_mode1 == 15) && (mgmd_stSCIRx.coef_mode2 == 16))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电流采样k
        for(i = EEROM_PARA_V_NUM; i < EEROM_PARA_V_NUM + EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4]) << 16 | *md_u16RDWEBuf2[i*4+1];
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 17) && (mgmd_stSCIRx.coef_mode2 == 18))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电流采样b
        for(i = EEROM_PARA_V_NUM; i < EEROM_PARA_V_NUM + EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4+2]) << 16 | *md_u16RDWEBuf2[i*4+3];
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 19) && (mgmd_stSCIRx.coef_mode2 == 20))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电流输出k
        for(i = EEROM_PARA_V_NUM + EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM; i < EEROM_PARA_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4]) << 16 | *md_u16RDWEBuf2[i*4+1];
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }
    else if((mgmd_stSCIRx.coef_mode1 == 21) && (mgmd_stSCIRx.coef_mode2 == 22))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //仅写电流输出b
        for(i = EEROM_PARA_V_NUM + EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM; i < EEROM_PARA_NUM; i++)
        {
            u32tmp = (Uint32)(*md_u16RDWEBuf2[i*4+2]) << 16 | *md_u16RDWEBuf2[i*4+3];
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);
        }
        WriteEeromPara(&st_eerom_para_write, pPara);
    }

    //将全部系数写为校准用的系数
    else if((mgmd_stSCIRx.coef_mode1 == 202) && (mgmd_stSCIRx.coef_mode2 == 203))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;

        //校准用的系数
        for(i = 0; i < EEROM_PARA_NUM; i++)
        {
            pPara->f32v_adc_p_k[i] = 1;
            pPara->f32v_adc_p_b[i] = 0;
        }

        WriteEeromPara(&st_eerom_para_write, pPara);
    }

}




//num为字节数，每两个字节组成一个Uint16，求所有的累加和
Uint16 CalCrc(Uint16* data, Uint16 num)
{
    Uint16 u16tmp = 0, i = 0;
    Uint16 crc = 0;

    //新增，原来计算crc时需要num为偶数，因为下边i+=2;
    //为了兼容原来，且修改错误，单独对num为奇数的情况进行-1将其处理为偶数
    if(num & 0x01 == 1)    num -= 1;

    for(i = 0; i < num; i+=2)
    {
        u16tmp = D8TO16(&data[i]);
        crc += u16tmp;
    }
    return crc;
}


Uint16 CheckEeromPara(ST_EEROM_PARA_STORED* pRead)
{
    Uint16 u16tmp = 0, crctmp = 0;
    u16tmp = D8TO16(pRead->head);
    if(u16tmp != EEROM_HEAD)
    {
        return 1;
    }
    u16tmp = D8TO16(pRead->tail);
    if(u16tmp != EEROM_TAIL)
    {
        return 2;
    }
    u16tmp = D8TO16(pRead->crc);
    crctmp = CalCrc((Uint16*)pRead, EEROM_PARA_LEN_NOCRC);
    if(u16tmp != crctmp)
    {
        return 3;
    }

    return EEROM_DATAOK;
}



void WriteEeromPara(ST_EEROM_PARA_STORED* pWrite, ST_EEROM_PARA* pPara)
{
    Uint32 u32tmp = 0;
    Uint16 crc_tmp = 0;
    Uint16 i = 0;
    Uint16 hh_test_len1 = 0, hh_test_start1 = 0;
    Uint16* hh_test_pointer1;

    memset(pWrite, 0, sizeof(ST_EEROM_PARA_STORED));
    D16TO8(EEROM_HEAD, pWrite->head);
    D16TO8(EEROM_TAIL, pWrite->tail);


    for(i = 0; i < EEROM_PARA_NUM; i++)
    {
        u32tmp = FTOIEEE(pPara->f32v_adc_p_k[i]);
        D32TO8(u32tmp, pWrite->v_adc_p_k_byte[i]);
        u32tmp = FTOIEEE(pPara->f32v_adc_p_b[i]);
        D32TO8(u32tmp, pWrite->v_adc_p_b_byte[i]);
    }


    crc_tmp = CalCrc((Uint16*)pWrite, EEROM_PARA_LEN_NOCRC);
    D16TO8(crc_tmp, pWrite->crc);

    hh_test_len1 = EEROM_PARA_LEN;
    hh_test_start1 = EEROM_PARA_START;
    hh_test_pointer1 = (Uint16*)pWrite;

    while(hh_test_len1 > 128)
    {
        WRE2ROM(hh_test_pointer1, hh_test_start1, 128);
        hh_test_len1 -= 128;
        hh_test_start1 += 128;
        hh_test_pointer1 += 128;
        DELAY_US(10000);   //两次写之间需要加delay
    }

    WRE2ROM(hh_test_pointer1, hh_test_start1, hh_test_len1);


    CheckValid(pPara);    //不符合范围则eerom_ok记录错误码，此时也会亮红灯

    if(pPara->eerom_ok == EEROM_DATAERR_OVERRANG)
    {
        //写的系数不符合范围
        mgmd_stSCIRx.write_ok = EEROM_DATAERR_OVERRANG;
    }
    else
    {
        //写的系数ok
        mgmd_stSCIRx.write_ok = 1;
    }
}


void ReadEeromPara(ST_EEROM_PARA_STORED* pRead, ST_EEROM_PARA* pPara)
{
    Uint32 u32tmp = 0;
    Uint16 i = 0;
    Uint16 hh_test_len222 = 0, hh_test_start222 = 0;
    Uint16* hh_test_pointer222;
    memset(pRead, 0, sizeof(ST_EEROM_PARA_STORED));

    hh_test_len222 = EEROM_PARA_LEN;
    hh_test_start222 = EEROM_PARA_START;
    hh_test_pointer222 = (Uint16*)pRead;

    while(hh_test_len222 > 128)
    {
        RDE2ROM(hh_test_pointer222, hh_test_start222, 128);
        hh_test_len222 -= 128;
        hh_test_start222 += 128;
        hh_test_pointer222 += 128;
        DELAY_US(10000);   //两次读之间需要加delay
    }
    RDE2ROM(hh_test_pointer222, hh_test_start222, hh_test_len222);
    DELAY_US(10000);

    pPara->eerom_ok = CheckEeromPara(pRead);

    if(pPara->eerom_ok == EEROM_DATAOK)
    {
        //用读到的系数
        for(i = 0; i < EEROM_PARA_NUM; i++)
        {
            u32tmp = D8TO32(pRead->v_adc_p_k_byte[i]);
            pPara->f32v_adc_p_k[i] = IEEETOF(u32tmp);
            u32tmp = D8TO32(pRead->v_adc_p_b_byte[i]);
            pPara->f32v_adc_p_b[i] = IEEETOF(u32tmp);


            //给modbus寄存器赋值，上位机可读取到当前用的系数
            mgmd_stSCIRx.v_adc_p_k_H[i] = (FTOIEEE(pPara->f32v_adc_p_k[i]) >> 16) & 0x0000FFFF;
            mgmd_stSCIRx.v_adc_p_k_L[i] = FTOIEEE(pPara->f32v_adc_p_k[i]) & 0x0000FFFF;
            mgmd_stSCIRx.v_adc_p_b_H[i] = (FTOIEEE(pPara->f32v_adc_p_b[i]) >> 16) & 0x0000FFFF;
            mgmd_stSCIRx.v_adc_p_b_L[i] = FTOIEEE(pPara->f32v_adc_p_b[i]) & 0x0000FFFF;
        }
    }

}




//=========================================================================================
// IP 地址等参数存放在eeprom中
//=========================================================================================
//ok返回0
static Uint16 CheckValid11(ST_EEROM_PARA11* pPara)
{
    if(pPara->ip[3] > 255)
        return 1;

    //判断系数是否符合规范
//    if(   (pPara->ip[0] == 192 && pPara->ip[1] == 168 && pPara->ip[2] == 1)
//       || (pPara->ip[0] == 10 && pPara->ip[1] == 10 && pPara->ip[2] == 100))
//        return 0;   //ok

    return 0;
}

Uint16 CheckEeromPara11(ST_EEROM_PARA11_STORED* pRead)
{
    Uint16 u16tmp = 0, crctmp = 0;
    u16tmp = D8TO16(pRead->head);
    if(u16tmp != EEROM_HEAD)
    {
        return 1;
    }
    u16tmp = D8TO16(pRead->tail);
    if(u16tmp != EEROM_TAIL)
    {
        return 2;
    }
    u16tmp = D8TO16(pRead->crc);
    crctmp = CalCrc((Uint16*)pRead, EEROM_PARA11_LEN_NOCRC);
    if(u16tmp != crctmp)
    {
        return 3;
    }

    return EEROM_DATAOK;
}
#pragma CODE_SECTION(InitEeromPara11,"runflashfuncs");
void InitEeromPara11(ST_EEROM_PARA11* pPara)
{
    ST_EEROM_PARA11_STORED *pRead = &st_eerom_para11_read;

    memset(pRead, 0, sizeof(ST_EEROM_PARA11_STORED));
    memset(pPara, 0, sizeof(ST_EEROM_PARA11));

    //读eerom，读到就用eerom的数据，没读到就用默认的
    RDE2ROM((Uint16*)pRead, EEROM_PARA11_START, EEROM_PARA11_LEN);
    DELAY_US(10000);

    pPara->eerom_ok = CheckEeromPara11(pRead);
    if(pPara->eerom_ok != EEROM_DATAOK)
    {
        //用默认的系数
        pPara->ip[0] = local_lip[0];
        pPara->ip[1] = local_lip[1];
        pPara->ip[2] = local_lip[2];
        pPara->ip[3] = local_lip[3];
    }
    else
    {
        //用读到的系数
        pPara->ip[0] = pRead->ip[0];
        pPara->ip[1] = pRead->ip[1];
        pPara->ip[2] = pRead->ip[2];
        pPara->ip[3] = pRead->ip[3];
    }

}



Uint16 WriteEeromPara11(ST_EEROM_PARA11* pPara)
{
    Uint16 crc_tmp = 0;
    ST_EEROM_PARA11_STORED tmp;
    ST_EEROM_PARA11_STORED* pWrite;

    pWrite = &tmp;

    //要写的系数不符合要求则不写
    if(CheckValid11(pPara) != 0)
    {
        return 1;
    }

    memset(pWrite, 0, sizeof(ST_EEROM_PARA11_STORED));
    D16TO8(EEROM_HEAD, pWrite->head);
    D16TO8(EEROM_TAIL, pWrite->tail);

    pWrite->ip[0] = pPara->ip[0];
    pWrite->ip[1] = pPara->ip[1];
    pWrite->ip[2] = pPara->ip[2];
    pWrite->ip[3] = pPara->ip[3];

    crc_tmp = CalCrc((Uint16*)pWrite, EEROM_PARA11_LEN_NOCRC);
    D16TO8(crc_tmp, pWrite->crc);

    WRE2ROM((Uint16*)pWrite, EEROM_PARA11_START, EEROM_PARA11_LEN);

    return 0;
}



//上位机下发参数，写进eerom中
void RecvAndWriteEerom11(ST_EEROM_PARA11* pPara)
{
    Uint16 rst_write = 0;


    //将参数写入eerom中
    //写IP地址
    if((mgmd_stSCIRx.coef_mode1 == 100) && (mgmd_stSCIRx.coef_mode2 == 101))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;
        //将下发的参数赋值给para
        pPara->ip[0] = mgmd_stSCIRx.ip_addr1;
        pPara->ip[1] = mgmd_stSCIRx.ip_addr2;
        pPara->ip[2] = mgmd_stSCIRx.ip_addr3;
        pPara->ip[3] = mgmd_stSCIRx.ip_addr4;

        rst_write = WriteEeromPara11(pPara);

        if(rst_write != 0)
        {
            mgmd_stSCIRx.IPwriteOK = 0;
        }
        else
        {
            mgmd_stSCIRx.IPwriteOK = 1;
        }
    }
}
















Uint16 WRE2ROM(Uint16   *Wdata, Uint16  RomAddress, Uint16  number)
{
   Uint16 i;
   if (I2caRegs.I2CSTR.bit.BB == 1)
   {
      return I2C_BUS_BUSY_ERROR;
   }
   while(!I2C_xrdy());
   I2caRegs.I2CSAR = 0x50;
   I2caRegs.I2CCNT = number + 2;
   I2caRegs.I2CDXR = (RomAddress >> 8);
//   I2caRegs.I2CCNT = number + 1;
   DELAY_US(1);
   I2caRegs.I2CMDR.all = 0x6E20;
   while(!I2C_xrdy());
   I2caRegs.I2CDXR = RomAddress;
   for (i=0; i<number; i++)
   {
      while(!I2C_xrdy());
      I2caRegs.I2CDXR = *Wdata;
      Wdata++;
      if (I2caRegs.I2CSTR.bit.NACK == 1)
          return    I2C_BUS_BUSY_ERROR;
   }
//   I2caRegs.I2CMDR.all = 0x6E20;
//   I2caRegs.I2CMDR.bit.STP = 1;
   return I2C_SUCCESS;
}

Uint16 RDE2ROM(Uint16  *RamAddr, Uint16 RomAddress, Uint16 number)
{
   Uint16  i,Temp;

   if (I2caRegs.I2CSTR.bit.BB == 1)
   {
       return I2C_BUS_BUSY_ERROR;
   }
   while(!I2C_xrdy());
   I2caRegs.I2CSAR = 0x50;
   I2caRegs.I2CCNT = 2;
   I2caRegs.I2CDXR = (RomAddress >> 8);
//   I2caRegs.I2CCNT = 1;

   DELAY_US(1);
   I2caRegs.I2CMDR.all = 0x6620;
   while(!I2C_xrdy());
   I2caRegs.I2CDXR = (RomAddress);
   if (I2caRegs.I2CSTR.bit.NACK == 1)
        return  I2C_BUS_BUSY_ERROR;
   DELAY_US(50);
   while(!I2C_xrdy());
   I2caRegs.I2CSAR = 0x50;
   I2caRegs.I2CCNT = number;
   I2caRegs.I2CMDR.all = 0x6C20;
   if (I2caRegs.I2CSTR.bit.NACK == 1)
        return  I2C_BUS_BUSY_ERROR;
   for(i=0;i<number;i++)
   {
      while(!I2C_rrdy());
      Temp = I2caRegs.I2CDRR;
      if (I2caRegs.I2CSTR.bit.NACK == 1)
          return    I2C_BUS_BUSY_ERROR;
      *RamAddr = Temp;
      RamAddr++;
   }
//   I2caRegs.I2CMDR.all = 0x6E20;
   return I2C_SUCCESS;
}



Uint16  I2C_xrdy()
{
    Uint16  t;
    t = I2caRegs.I2CSTR.bit.XRDY;
    return t;
}

Uint16  I2C_rrdy()
{
    Uint16  t;
    t = I2caRegs.I2CSTR.bit.RRDY;
    return t;
}


void D16TO8(Uint16 Source, Uint16 *Des)
{
    Uint16 temp;
/*========= Get Byte 1 ===============*/
    temp = Source;
    temp = temp & 0x00FF;
    Des[0] = temp;

/*========= Get Byte 2 ===============*/
    temp = Source;
    temp = temp >>8;
    temp = temp & 0x00FF;
    Des[1] = temp;
}


void D32TO8(Uint32 Source, Uint16 *Des)
{
    Uint32 temp;
/*========= Get Byte 1 ===============*/
    temp = Source;
    temp = temp & 0x00FF;
    Des[0] = temp;

/*========= Get Byte 2 ===============*/
    temp = Source;
    temp = temp >>8;
    temp = temp & 0x00FF;
    Des[1] = temp;

/*========= Get Byte 3 ===============*/
    temp = Source;
    temp = temp >>16;
    temp = temp & 0x00FF;
    Des[2] = temp;

/*========= Get Byte 4 ===============*/
    temp = Source;
    temp = temp >>24;
    temp = temp & 0x00FF;
    Des[3] = temp;
}


Uint16 D8TO16(Uint16 *Source)
{
    Uint16 temp;
    Uint16 Des;
    Des = 0;
    temp = 0;
/*========= Get Byte 2 ===============*/
    temp = Source[1];
    temp = temp <<8;
    Des = Des + temp;
    temp = 0;
/*========= Get Byte 1 ===============*/
    Des = Des + Source[0];
    return Des;
}

Uint32 D8TO32(Uint16 *Source)
{
    Uint32 temp;
    Uint32 Des;
    Des = 0;
    temp = 0;
/*========= Get Byte 4 ===============*/
    temp = Source[3];
    temp = temp <<24;
    Des = Des + temp;
    temp = 0;

/*========= Get Byte 3 ===============*/
    temp = Source[2];
    temp = temp <<16;
    Des = Des + temp;
    temp = 0;

/*========= Get Byte 2 ===============*/
    temp = Source[1];
    temp = temp <<8;
    Des = Des + temp;
    temp = 0;

/*========= Get Byte 1 ===============*/
    Des = Des + Source[0];
    return Des;

}


//===========================================================================
// No more.
//===========================================================================
