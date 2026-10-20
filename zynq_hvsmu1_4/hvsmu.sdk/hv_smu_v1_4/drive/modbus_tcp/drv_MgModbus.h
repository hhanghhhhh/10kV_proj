#ifndef _DRV_MGMODBUS_H_
#define _DRV_MGMODBUS_H_

#include "TYPEDEF.h"

#define MD_DATA_OFFSET    (13)


#define READ_MULT    (0x03)
#define WRITE_SINGLE (0x06)
#define WRITE_MULT   (0x10)






void TaskModbusTCPInit(void);
Uint16 ModbusTCPParse(Uint8* prxdata, Uint8* ptxdata);


Uint16 JudgeModbusTCP(Uint8* buf, Uint16 len);
Uint16 ModbusTCPHandle(Uint8* recv_buf, Uint16 recv_len, Uint8* send_buf);



#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
