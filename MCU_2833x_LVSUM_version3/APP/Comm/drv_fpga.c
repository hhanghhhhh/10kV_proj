
#include "drv_GlobalVar.h"
#include "drv_fpga.h"
#include "DSP2833x_Gpio.h"
#include "drv_MgModbus.h"
#include "drv_Adc.h"
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File







Uint32          *ExRamStart = (Uint32 *)0x180000;
/***********************************************************************
* Function Name : Data write
* Arguments     :
* Returns       :
* Description   : Write Data to FPGA
* Updated ID    :
***********************************************************************/

//void DataW(Uint16 Addr, Uint16 Data)
//{
//    Uint32 Temp;
//
//    Temp = (Uint32)(Data & 0xFFFF);
//    Temp = ((Temp<<16) & 0xFFFF0000);
//
//    *(ExRamStart + Addr) = Temp;
//
//}
/***********************************************************************
* Function Name : Data read
* Arguments     :
* Returns       :
* Description   : Read Data From FPGA
* Updated ID    :
***********************************************************************/
//Uint16 DataR(Uint16 ADDR)
//{
//    Uint16 Data;
//
//    SPI_R_L;
//
//    Data = (*(ExRamStart + ADDR)) & 0x0000FFFF;
//
//    SPI_R_H;
//
//    return Data;
//
//}




/******************************************************************************
* == END OF MODULE/FILE (EOF) ==
******************************************************************************/
