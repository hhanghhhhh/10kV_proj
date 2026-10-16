#ifndef _TASK_TRANSFER_H_
#define _TASK_TRANSFER_H_





#define  IEEETOF(x)     (*(float32*)&(x))
#define  FTOIEEE(x)     (*(Uint32*)&(x))


extern void ModbusParaTransfer(ST_SCI_RX_PAR  *p);
extern void ModbusParaLimit(ST_SCI_RX_PAR  *p);














#endif
