

#ifndef __ECAT_INT_H_
#define __ECAT_INT_H_

#include "drv_GlobalVar.h"
#if DSP
    #include "DSP2833x_Device.h"     // Header file Include File
    #include "DSP2833x_Examples.h"   // Examples Include File
#endif
/***********************************************************************
Declare external variables
***********************************************************************/

extern Uint16   slINTcnt;
extern Uint16   slINT1ms;
extern Uint16   slINT10ms;
extern Uint16   slINT100ms;
extern Uint16   slINT1s;


#if	DSP
extern float	test1;
extern float    test2;
extern float    test3;
extern float    test4;
extern float    test5;
extern float    test6;
#endif

/***********************************************************************
* Function header definition
***********************************************************************/

extern void IntTimeBase(void);

#if DSP
    extern interrupt    void    INT6(void);
    extern interrupt    void    Interrupt_CanControllerRxIsr(void);
#else
    extern              void    INT6(void);
#endif

#endif
/***************************************************************************
*			END, do not code behind this line!!                            *
****************************************************************************/

