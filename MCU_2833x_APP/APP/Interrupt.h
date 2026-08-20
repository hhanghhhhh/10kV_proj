

#ifndef __ECAT_INT_H_
#define __ECAT_INT_H_

/***********************************************************************
Declare external variables
***********************************************************************/

/***********************************************************************
 * Function header definition
 ***********************************************************************/

extern interrupt void INT6(void);
extern interrupt void Interrupt_CanA0Isr(void);
extern interrupt void Interrupt_Ad7982EPwm1Isr(void);
extern interrupt void Interrupt_Ad7982DmaCh2Isr(void);

#endif
/***************************************************************************
 *			END, do not code behind this line!!                            *
 ****************************************************************************/
