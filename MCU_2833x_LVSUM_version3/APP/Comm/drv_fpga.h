

#ifndef DRV_FPGA_H_
#define DRV_FPGA_H_


#include "drv_GlobalVar.h"



extern Uint32          *ExRamStart;




extern void IORead(void);
extern Uint16 DataR(Uint16 ADDR);
extern void DataW(Uint16 Addr, Uint16 Data);

/******************************************************************************
* == END OF MODULE/FILE (EOF) ==
******************************************************************************/
#endif
