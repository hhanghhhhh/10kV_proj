#ifndef __DRV_MCBSP_H_
#define __DRV_MCBSP_H_


#include "TypeDefine.h"







extern Uint16 McBSPa_Xmit16(Uint16 senddata);
extern Uint32 ReadFPGA32(Uint16 addr);
extern void WriteFPGA32(Uint16 addr, Uint32 data);
  


#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
