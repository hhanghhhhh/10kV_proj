/**************************************************************************************************
Project:
Filename:   drv_ModbusData.h
Description:
Complier: CCS V3.3, TI.
***************************************************************************************************
Author:   WANG DAN 
Created on:
Revision: 
***************************************************************************************************/



/**************************************************************************************************
COMPILER DIRECTIVES                                                                
**************************************************************************************************/
#ifdef _MODBUS_DATA_C_
   #define _MODBUS_DATA_EXT 
#else
   #define _MODBUS_DATA_EXT extern
#endif


#include "TypeDefine.h"
/**************************************************************************************************
CONSTANTS AND MACROS
**************************************************************************************************/
#define RD_GROUP1_BUF_SIZE_MACRO        (10)
#define RD_GROUP2_BUF_SIZE_MACRO        (70)
#define WE_GROUP_BUF_SIZE               (64)
#define RD_WE_BUF_SIZE_MACRO            (180)
#define RD_WE2_BUF_SIZE_MACRO           (300)   //EEROM_PARA_NUM * 4
#define RD_WE4_BUF_SIZE_MACRO           (10)

#define MD_RD_GROUP1_START_ADDR  (0x1000)
#define MD_RD_GROUP1_END_ADDR    (0x1009)

#define MD_RD_GROUP2_START_ADDR  (0x1100)
#define MD_RD_GROUP2_END_ADDR    (0x1145)

#define MD_WE_GROUP1_START_ADDR  (0x2000)
#define MD_WE_GROUP1_END_ADDR    (0x203C)

#define MD_RDWE_GROUP_START_ADDR  (0x3000)
#define MD_RDWE_GROUP_END_ADDR    (0x3083)

#define MD_RDWE_GROUP2_START_ADDR  (0x5000)
#define MD_RDWE_GROUP2_END_ADDR    (0x512B)

#define MD_RDWE_GROUP4_START_ADDR  (0x7000)
#define MD_RDWE_GROUP4_END_ADDR    (0x7009)

/**************************************************************************************************
TYPEDEFS
**************************************************************************************************/

/**************************************************************************************************
EXPORTED VARIABLES
**************************************************************************************************/

extern Uint16* md_u16RDGroup1Buf[RD_GROUP1_BUF_SIZE_MACRO];
extern Uint16* md_u16RDGroup2Buf[RD_GROUP2_BUF_SIZE_MACRO];
extern Uint16* md_u16RDWEBuf[RD_WE_BUF_SIZE_MACRO];
extern Uint16* md_u16RDWEBuf2[RD_WE2_BUF_SIZE_MACRO];
extern Uint16* md_u16RDWEBuf4[RD_WE4_BUF_SIZE_MACRO];
extern Uint16* md_u16WEBuf[WE_GROUP_BUF_SIZE];

/**************************************************************************************************
INPUT BUFFER ALLOCATION
**************************************************************************************************/


/**************************************************************************************************
FLAG DEFINITIONS
**************************************************************************************************/

/**************************************************************************************************
INPUT BUFFER ALLOCATION
**************************************************************************************************/

/**************************************************************************************************
EXPORTED FUNCTIONS
**************************************************************************************************/
extern void md_vPrepareData(void);
extern Uint16 crc_u16CRC16Check(Uint16 *pu8Msg, Uint16 u16DataLen);
extern Uint16 crc_u16CRC8Check(Uint8 *tpbBuffer,Uint32 wLength);
/**************************************************************************************************
LOCAL FUNCTIONS
**************************************************************************************************/
#ifdef _MODBUS_DATA_C_
  
#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
