
#include "drv_GlobalVar.h"
#include "DSP2833x_Adc.h"
#include "drv_Adc.h"
#include "DSP2833x_Gpio.h"
#include <math.h>
#include "drv_fpga.h"
#include "drv_MgModbus.h"
#include "test_hh.h"
#include "task_Transfer.h"
#include "drv_Eeprom.h"
/*=============== ADC sample results () ============================*/
ST_ADC_BUFFER_VAL      adc_stBuf = {0};
ST_ADC_SAMPPLE_RESULT  adc_stResult = {0};

Uint16  adc_u16RmsPointCnt = 0;
float32 adc_fIaSquare = 0;
float32 adc_fIbSquare = 0;
float32 adc_fIcSquare = 0;
float32 adc_fImodSquare = 0;
float32 Zero_Curr_Err_Ia = 0;
float32 Zero_Curr_Err_Ib = 0;
float32 Zero_Curr_Err_Ic = 0;
extern float  MonParaArr[333];
extern int   UpdatePara_CNT;
/***********************************************************************
* Function Name : GetAdc
* Arguments     :
* Returns       :
* Description   : Read ADC value, include the calibrated offset part
* Updated ID    :
***********************************************************************/
void GetAdc(void)
{

	//读取DSP侧ADC寄存器数据
//    adc_stBuf.fAD1 = (AdcRegs.ADCRESULT0) >> 4 ;    //Vfbk
//    adc_stBuf.fAD2 = (AdcRegs.ADCRESULT1) >> 4 ;    //Vfbk
//    adc_stBuf.fAD3 = (AdcRegs.ADCRESULT2) >> 4 ;    //Vfbk

    //触发下一次采样
    AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;




}







/******************************************************************************
* == END OF MODULE/FILE (EOF) ==
******************************************************************************/
