

#ifndef __DRV_ADC_H_
#define __DRV_ADC_H_
/******************************************************************************
* INCLUDE FILES                                                                
******************************************************************************/

#include "TypeDefine.h"
/******************************************************************************
* == DEFINES ==                                                          
******************************************************************************/

/******************************************************************************
* == STRUCTURES ==                                                       
******************************************************************************/
extern Uint16  adc_u16RmsPointCnt;
extern float32 adc_fIaSquare;
extern float32 adc_fIbSquare;
extern float32 adc_fIcSquare;

typedef struct
{
    Uint16        u16AD1;
    Uint16        u16AD2;
    Uint16        u16AD3;
    Uint16        u16AD4;
    Uint16        u16AD5;
    Uint16        u16AD6;
    Uint16        u16AD7;
    Uint16        u16AD8;
    Uint16        u16AD9;
    Uint16        u16AD10;

    float32       fAD1;
    float32       fAD2;
    float32       fAD3;
    float32       fAD4;
    float32       fAD5;
    float32       fAD6;
    float32       fAD7;
    float32       fAD8;
    float32       fAD9;
    float32       fAD10;
    float32       fAD11;
    float32       fAD12;
    float32       fAD13;
    float32       fAD14;
    float32       fAD15;
    float32       fAD16;
}ST_ADC_BUFFER_VAL;
extern  ST_ADC_BUFFER_VAL   adc_stBuf;



typedef struct
{
    Uint16     u16Vol1ratio_code;
    Uint16     u16Vol2ratio_code;
    Uint16     u16Curratio_code;


    Uint32     u32Vol1Total;
    Uint32     u32Vol2Total;
    Uint32     u32CurTotal;

    float32    f32_vol1_mean;
    float32    f32_vol2_mean;
    float32    f32_cur_mean;

    float32    fUfbk;
	float32    fUdc;
	float32    fIfbk;
	float32    fTemperture;
	float32    fUctrl;
	float32    fT_oil;
	
	float64    fVol1;
	float64    fVol2;
	float64    fCur;
	float64    fRdson1;
	float64    fRdson2;
	float64    fVol1Ratio;
	float64    fVol1Offset;
	float64    fVol2Ratio;
	float64    fVol2Offset;
	float64    fCurRatio;
	float64    fCurOffset;


}ST_ADC_SAMPPLE_RESULT;
extern ST_ADC_SAMPPLE_RESULT  adc_stResult;
/*=============== ADC FBK() =====================================*/

/******************************************************************************
* == FUNCTIONS DECLARATION ==                                                          
******************************************************************************/

/******************************************************************************
* == FUNCTIONS DEFINITION ==                                                        
******************************************************************************/
extern void GetAdc(void);
/******************************************************************************
* == END OF MODULE/FILE (EOF) ==
******************************************************************************/
#endif
