/**************************************************************************************************
Project:
Filename:   drv_MgModbus.c
Description:
Complier:
***************************************************************************************************
Author:   WANG DAN 
Created on:  2022��3��3��
Revision: 
***************************************************************************************************/



/**************************************************************************************************
COMPILER DIRECTIVES                                                                
**************************************************************************************************/
#ifndef _MG_MODBUS_C_
   #define _MG_MODBUS_C_
#endif

/**************************************************************************************************
HEADER FILES INCLUDED                                                               
**************************************************************************************************/

#include "drv_MgModbus.h"
#include "drv_ModbusData.h"
#include "DSP2833x_Sci.h"
#include "DSP2833x_Gpio.h"
#include "DSP2833x_Device.h"
#include "TypeDefine.h"
#include "drv_GlobalVar.h"
#include <string.h>
#include "../W5500/drv_Spi.h"
#include "drv_Eeprom.h"
#include "test_hh.h"
#include "task_Range.h"
/**************************************************************************************************
CONSTANT DESCRIPTION                                                               
**************************************************************************************************/
Uint16 mgmd_u16FrameLength = MGMD_FRAME_LENGTH_MIN;
Uint16 mgmd_u16SlaveID = 1;
Uint16 mgmd_u16RxReady = 0;
Uint16 mgmd_u16Urt1BytesReceived = 0;
Uint16 mgmd_u16Urt1Bytes2Send = 0;
Uint16 mgmd_u16Urt1TxBuf[MGMD_MODBUS_RXTX_BUFFER_SIZE_MACRO];
Uint16 mgmd_u16Urt1RxBuf[MGMD_MODBUS_RXTX_BUFFER_SIZE_MACRO];

Uint16 mgmd_u16ModbusFunCode;
Uint16 mgmd_u16ModbusAddr;
Uint16 mgmd_u16ModbusCrc;
Uint16 mgmd_u16ModbusRegNum;

ST_SCI_RX_PAR    mgmd_stSCIRx;

/**************************************************************************************************
EXPORTED FUNCTIONS
**************************************************************************************************/
/**************************************************************************************************
FUNCTION: mgmd_vInit
DESCRIPTION: mgcc modbus init
**************************************************************************************************/
void mgmd_vInit(void)
{
   Uint16 i;

   //prepare data 
   md_vPrepareData();

   //Init ID
   mgmd_u16SlaveID = 0x01;
   //Init Flag
   mgmd_u16FrameLength = MGMD_FRAME_LENGTH_MIN;
   mgmd_u16RxReady = 0;
   mgmd_u16Urt1BytesReceived = 0;
   mgmd_u16Urt1Bytes2Send = 0;

   for(i=0;i<MGMD_MODBUS_RXTX_BUFFER_SIZE_MACRO;i++)
   {
      mgmd_u16Urt1TxBuf[i]=0;
	  mgmd_u16Urt1RxBuf[i]=0;
   }

   memset(&mgmd_stSCIRx,0,sizeof(ST_SCI_RX_PAR));



   mgmd_stSCIRx.ADC_mux = 1;
   mgmd_stSCIRx.CMD_ControlLoop_BW1_n = 1;
   mgmd_stSCIRx.CMD_ControlLoop_BW2_n = 1;
   mgmd_stSCIRx.CMD_ControlLoop_BW3_n = 1;
   mgmd_stSCIRx.CMD_ControlLoop_BW4_n = 0;  // 有电流环， 测igss需要大档位充的时候可置1，将电流环断开，不限流
   mgmd_stSCIRx.CMD_SNSCONTROL_n = 1;       // 2线
   mgmd_stSCIRx.CMD_AFBRESET = 1;
   mgmd_stSCIRx.CMD_Control_VEN = 1;
   mgmd_stSCIRx.CMD_Control_IEN = 1;
   mgmd_stSCIRx.CMD_GUARDEN = 0;            // 1 LO接PE，测量精度高，   0 LO不接PE
//   mgmd_stSCIRx.CMD_SW_F = 1;  //1uA以下开启
   mgmd_stSCIRx.CMD_VGAINA1 = 0;
   mgmd_stSCIRx.CMD_VGAINA0 = 1;

   mgmd_stSCIRx.SetRangeCurH = 0x3A83;  //1mA
   mgmd_stSCIRx.SetRangeCurL = 0x126F;
   mgmd_stSCIRx.SetRangeVolH = 0x41A0;  //20V
   mgmd_stSCIRx.SetRangeVolL = 0x0000;

   mgmd_stSCIRx.PluseWidthH = 0xBF80;   //-1, 默认常开，需手动下电
   mgmd_stSCIRx.PluseWidthL = 0x0000;

   mgmd_stSCIRx.AVMA_A_data = 65535;
   mgmd_stSCIRx.AVMA_B_data = 65535;

   mgmd_stSCIRx.nPLCH = 0x3F80;  //1
   mgmd_stSCIRx.nPLCL = 0x0000;

   mgmd_stSCIRx.Outmode = OUTMODE_V;  //输出电压

   mgmd_stSCIRx.PluseWidthH = 0xBF80;  //-1
   mgmd_stSCIRx.PluseWidthL = 0;

   mgmd_stSCIRx.vth_dbsmu.ds.delayms.f32 = 3;


   mgmd_stSCIRx.pw3_initial_cur.f32 = 1e-3;
   mgmd_stSCIRx.pw3_initial_delay_s.f32 = 10e-3;


   // 8 段参数初始化
   mgmd_stSCIRx.vth_8pre.pre_num = 8;
   mgmd_stSCIRx.CMD_SNSCONTROL_n = 1;   // 2 线
   for(i = 0; i < 8; i++)
   {
       mgmd_stSCIRx.vth_8pre.pre[i].vset.f32 = 2 + i;
       mgmd_stSCIRx.vth_8pre.pre[i].iset.f32 = 10e-3;
       mgmd_stSCIRx.vth_8pre.pre[i].plusewidth.f32 = 0.02;
       mgmd_stSCIRx.vth_8pre.pre[i].delay.f32 = 0.01;
   }

   mgmd_stSCIRx.vth_8pre.vth.vset.f32 = 20;
   mgmd_stSCIRx.vth_8pre.vth.iset.f32 = 10e-3;
   mgmd_stSCIRx.vth_8pre.vth.plusewidth.f32 = 0.05;
   mgmd_stSCIRx.vth_8pre.vth.nplc.f32 = 1;



   //所有的系数k = 1  b = 0
   for(i = 0; i < EEROM_PARA_NUM; i++)
   {
       mgmd_stSCIRx.v_adc_p_k_H[i] = 0x3F80;
       mgmd_stSCIRx.v_adc_p_k_L[i] = 0;
   }

   mgmd_stSCIRx.program_type = IN_APP;
   mgmd_stSCIRx.testflag = 1;//默认不开启
}




//网口发送
void eth_msg(Uint16* msg, Uint16 len)
{
    int j;
    //网口发送
//    for(j = 0; j < len; j++)
//    {
//        W5500_trans_buff[j] = msg[j];
//    }
    W5500_trans_flag = 1;
    W5500_trans_len = len;
}
/**************************************************************************************************
FUNCTION: mgmd_vRxData
DESCRIPTION: mgcc modbus poll
**************************************************************************************************/

/**************************************************************************************************
FUNCTION: mgmd_vHandleData
DESCRIPTION: mgcc modbus poll
**************************************************************************************************/
void mgmd_vHandleData(void)
{
   Uint16 i;
   Uint16 u16AddrOffset;
   Uint16 u16RegValue;
   Uint8  u8RegValueHi;
   Uint8  u8RegValueLo;
   switch(mgmd_u16ModbusFunCode)
   {
        case MGMD_MODBUS_FUN_READ_MACRO:
            mgmd_u16Urt1Bytes2Send = 6;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16SlaveID;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16ModbusFunCode;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16ModbusRegNum*2;


            if((mgmd_u16ModbusAddr >= MD_RD_GROUP1_START_ADDR)
             && (mgmd_u16ModbusAddr <= MD_RD_GROUP1_END_ADDR)
             && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RD_GROUP1_END_ADDR))//
			{
               for(i = 0; i < mgmd_u16ModbusRegNum; i++)
               {
                  u16AddrOffset = mgmd_u16ModbusAddr - MD_RD_GROUP1_START_ADDR + i;
                  u16RegValue = (Uint16)((*md_u16RDGroup1Buf[u16AddrOffset]) * 1);

                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = (u16RegValue >> 8) & 0x00FF;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = u16RegValue & 0x00FF;
               }
			}
            else if((mgmd_u16ModbusAddr >= MD_RD_GROUP2_START_ADDR)
             && (mgmd_u16ModbusAddr <= MD_RD_GROUP2_END_ADDR)
             && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RD_GROUP2_END_ADDR))
            {
               for(i = 0; i < mgmd_u16ModbusRegNum; i++)
               {
                  u16AddrOffset = mgmd_u16ModbusAddr - MD_RD_GROUP2_START_ADDR + i;
                  u16RegValue = (Uint16)((*md_u16RDGroup2Buf[u16AddrOffset]) * 1);

                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = (u16RegValue >> 8) & 0x00FF;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = u16RegValue & 0x00FF;
               }
            }
            else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP_START_ADDR)
             && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP_END_ADDR)
             && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RDWE_GROUP_END_ADDR))
            {

               for(i = 0; i < mgmd_u16ModbusRegNum; i++)
               {

                  u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP_START_ADDR + i;
                  u16RegValue = (Uint16)((*md_u16RDWEBuf[u16AddrOffset]) * 1);

                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = (u16RegValue >> 8) & 0x00FF;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = u16RegValue & 0x00FF;
               }
            }
            else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP2_START_ADDR)
             && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP2_END_ADDR)
             && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RDWE_GROUP2_END_ADDR))
            {
               for(i = 0; i < mgmd_u16ModbusRegNum; i++)
               {
                  u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP2_START_ADDR + i;
                  u16RegValue = (Uint16)((*md_u16RDWEBuf2[u16AddrOffset]) * 1);

                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = (u16RegValue >> 8) & 0x00FF;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = u16RegValue & 0x00FF;
               }
            }
            else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP4_START_ADDR)
             && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP4_END_ADDR)
             && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RDWE_GROUP4_END_ADDR))
            {
               for(i = 0; i < mgmd_u16ModbusRegNum; i++)
               {
                  u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP4_START_ADDR + i;
                  u16RegValue = (Uint16)((*md_u16RDWEBuf4[u16AddrOffset]) * 1);

                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = (u16RegValue >> 8) & 0x00FF;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = u16RegValue & 0x00FF;
               }
            }
            else if((mgmd_u16ModbusAddr >= MD_WE_GROUP1_START_ADDR)
             && (mgmd_u16ModbusAddr <= MD_WE_GROUP1_END_ADDR)
             && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_WE_GROUP1_END_ADDR))
            {
               for(i = 0; i < mgmd_u16ModbusRegNum; i++)
               {
                  u16AddrOffset = mgmd_u16ModbusAddr - MD_WE_GROUP1_START_ADDR + i;
                  u16RegValue = (Uint16)((*md_u16WEBuf[u16AddrOffset]) * 1);

                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = (u16RegValue >> 8) & 0x00FF;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = u16RegValue & 0x00FF;
               }
            }
            else //register addr error
            {
                  mgmd_u16Urt1Bytes2Send = 6;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16SlaveID;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16ModbusFunCode + 0x80;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = 0x02;
            }

            break;


        case MGMD_MODBUS_FUN_WRITE_SINGLE_MACRO:

            u16RegValue = (mgmd_u16Urt1RxBuf[10] << 8) | mgmd_u16Urt1RxBuf[11];

            //organize tx data
            mgmd_u16Urt1Bytes2Send = 6;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16SlaveID;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16ModbusFunCode;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = (mgmd_u16ModbusAddr >> 8) & 0x00FF;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16ModbusAddr & 0x00FF;
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16Urt1RxBuf[10];
            mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16Urt1RxBuf[11];

            //write data to register
			if((mgmd_u16ModbusAddr >= MD_WE_GROUP1_START_ADDR )
			   && (mgmd_u16ModbusAddr <= MD_WE_GROUP1_END_ADDR))
			{
			  u16AddrOffset = mgmd_u16ModbusAddr - MD_WE_GROUP1_START_ADDR;
			  *md_u16WEBuf[u16AddrOffset]	= u16RegValue;
			}
			else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP_START_ADDR )
               && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP_END_ADDR))
            {
              u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP_START_ADDR;
              *md_u16RDWEBuf[u16AddrOffset]   = u16RegValue;
            }
            else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP2_START_ADDR )
               && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP2_END_ADDR))
            {
              u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP2_START_ADDR;
              *md_u16RDWEBuf2[u16AddrOffset]   = u16RegValue;
            }
            else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP4_START_ADDR )
               && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP4_END_ADDR))
            {
              u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP4_START_ADDR;
              *md_u16RDWEBuf4[u16AddrOffset]   = u16RegValue;
            }
            else //register addr error
            {
                  mgmd_u16Urt1Bytes2Send = 6;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16SlaveID;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16ModbusFunCode + 0x80;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = 0x02;
            }
            break;

        case MGMD_MODBUS_FUN_WRITE_MULT_MACRO:

               mgmd_u16Urt1Bytes2Send = 6;
               mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16SlaveID;
               mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16Urt1RxBuf[7];	//funtion code
               mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16Urt1RxBuf[8];	//addr hi-byte
               mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16Urt1RxBuf[9];	//addr low-byte
               mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16Urt1RxBuf[10];	//num hi-byte
               mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16Urt1RxBuf[11];	//num low-byte

               if((mgmd_u16ModbusAddr >= MD_WE_GROUP1_START_ADDR )
                  && (mgmd_u16ModbusAddr <= MD_WE_GROUP1_END_ADDR)
                  && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_WE_GROUP1_END_ADDR))
               {
                  u16AddrOffset = mgmd_u16ModbusAddr - MD_WE_GROUP1_START_ADDR;
                  for(i = 0; i < mgmd_u16ModbusRegNum; i++)
                  {
                     u8RegValueHi = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2];
                     u8RegValueLo = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2 +1];
                     u16RegValue = (u8RegValueHi << 8) | u8RegValueLo;

                     *md_u16WEBuf[u16AddrOffset + i] = u16RegValue;
                  }
               }
               else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP_START_ADDR )
                       && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP_END_ADDR)
                       && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RDWE_GROUP_END_ADDR))
               {
                   u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP_START_ADDR;
                   for(i = 0; i < mgmd_u16ModbusRegNum; i++)
                   {
                      u8RegValueHi = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2];
                      u8RegValueLo = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2 +1];
                      u16RegValue = (u8RegValueHi << 8) | u8RegValueLo;

                      *md_u16RDWEBuf[u16AddrOffset + i] = u16RegValue;

                   }
               }
               else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP2_START_ADDR )
                       && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP2_END_ADDR)
                       && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RDWE_GROUP2_END_ADDR))
               {
                   u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP2_START_ADDR;
                   for(i = 0; i < mgmd_u16ModbusRegNum; i++)
                   {
                      u8RegValueHi = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2];
                      u8RegValueLo = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2 +1];
                      u16RegValue = (u8RegValueHi << 8) | u8RegValueLo;

                      *md_u16RDWEBuf2[u16AddrOffset + i] = u16RegValue;
                   }
               }
               else if((mgmd_u16ModbusAddr >= MD_RDWE_GROUP4_START_ADDR )
                       && (mgmd_u16ModbusAddr <= MD_RDWE_GROUP4_END_ADDR)
                       && ((mgmd_u16ModbusAddr  + mgmd_u16ModbusRegNum - 1) <= MD_RDWE_GROUP4_END_ADDR))
               {
                   u16AddrOffset = mgmd_u16ModbusAddr - MD_RDWE_GROUP4_START_ADDR;
                   for(i = 0; i < mgmd_u16ModbusRegNum; i++)
                   {
                      u8RegValueHi = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2];
                      u8RegValueLo = mgmd_u16Urt1RxBuf[MGMD_MODBUS_DATA_OFFSET_MACRO + i * 2 +1];
                      u16RegValue = (u8RegValueHi << 8) | u8RegValueLo;

                      *md_u16RDWEBuf4[u16AddrOffset + i] = u16RegValue;
                   }
               }
               else //register addr error
               {
                  mgmd_u16Urt1Bytes2Send = 0;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16SlaveID;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = mgmd_u16ModbusFunCode + 0x80;
                  mgmd_u16Urt1TxBuf[mgmd_u16Urt1Bytes2Send++] = 0x02;
               }
           break;
        default:
           break;
   }
}

/**************************************************************************************************
FUNCTION: mgmd_vPollTb1
DESCRIPTION: mgcc modbus poll
**************************************************************************************************/
//以下为modbusTCP格式
void mgmd_vPollTb1(void)
{
   int i;

   if(mgmd_u16RxReady == 1)
   {
      mgmd_u16RxReady = 0;

      mgmd_u16ModbusFunCode = mgmd_u16Urt1RxBuf[7];
      mgmd_u16ModbusAddr = mgmd_u16Urt1RxBuf[8] << 8;
      mgmd_u16ModbusAddr |= mgmd_u16Urt1RxBuf[9];
      mgmd_u16ModbusRegNum = mgmd_u16Urt1RxBuf[10] << 8;
      mgmd_u16ModbusRegNum |= mgmd_u16Urt1RxBuf[11];

      //LIMIT(mgmd_u16ModbusRegNum, 0, 122);

      mgmd_u16Urt1BytesReceived = 0;

      //最开始四个字节与接收的一致
      for(i = 0; i < 4; i++)
      {
          mgmd_u16Urt1TxBuf[i] = mgmd_u16Urt1RxBuf[i];
      }

      mgmd_vHandleData();

      //lenth
      mgmd_u16Urt1TxBuf[4] = ((mgmd_u16Urt1Bytes2Send-6) >> 8) & 0x00FF;
      mgmd_u16Urt1TxBuf[5] = ((mgmd_u16Urt1Bytes2Send-6) & 0x00FF);

      eth_msg(mgmd_u16Urt1TxBuf, mgmd_u16Urt1Bytes2Send);
   }
}


/**************************************************************************************************
COMPILER DIRECTIVES
**************************************************************************************************/
#ifdef _MG_MODBUS_C_
   #undef _MG_MODBUS_C_
#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
