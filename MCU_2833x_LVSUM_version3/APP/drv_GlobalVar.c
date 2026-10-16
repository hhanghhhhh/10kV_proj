
#include "Interrupt.h"
#include "Main.h"
#include "drv_GlobalVar.h"


#if DSP
    #include    "drv_eeprom.h"
#endif
#include "drv_Adc.h"
#include <string.h>

CanController_Context_t g_can_controller;
Uint16 g_can_controller_init_status = CAN_CONTROLLER_STATUS_TIMEOUT;




/***********************************************************************
* Function Name : Variable definition
***********************************************************************/






/***********************************************************************
* Function Name : glb_VarInit
* Arguments     :
* Returns       :
* Description   : Initial the control parameter
* Updated ID    :
***********************************************************************/
void glb_VarInit(void)
{
/*================ GUI =============================================*/

/*================ Fault =============================================*/

/*================  =============================================*/
    slMain100ms = 0;
    slMain10ms = 0;
    slMain1ms = 0;
    slMain1s = 0;



/*=============== ADC sample ============================*/
    memset(&adc_stBuf,0,sizeof(ST_ADC_BUFFER_VAL));
    memset(&adc_stResult,0,sizeof(ST_ADC_SAMPPLE_RESULT));
/*=============== INV Control() =======================================*/

/*=============== TimeBase INT () =====================================*/
    slINTcnt = 0;
    slINT1ms = 0;
    slINT10ms = 0;
    slINT100ms = 0;
    slINT1s = 0;

/*=============== CAN communicate variable ()=========================*/

}
/******************************************************************************
* == END OF MODULE/FILE (EOF) ==
******************************************************************************/
