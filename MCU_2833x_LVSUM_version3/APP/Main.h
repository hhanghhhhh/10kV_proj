


#ifndef __ECAT_MAIN_H_
#define __ECAT_MAIN_H_

#include "drv_GlobalVar.h"
#if DSP
    #include "DSP2833x_Device.h"     // Header file Include File
    #include "DSP2833x_Examples.h"   // Examples Include File
#endif
/***********************************************************************
Declare external variables
***********************************************************************/
extern Uint16           slMain100ms;
extern Uint16           slMain10ms;
extern Uint16           slMain1ms;
extern Uint16           slMain1s;



extern void MainTimeBase(void);
#if DSP
#else
	extern void MainFunc(void);
#endif
	



/***************************************************************************
*			END, do not code behind this line!!                            *
****************************************************************************/
#endif









