#include "drv_GlobalVar.h"
#include <string.h>
#include "drv_MgModbus.h"
#include "task_Transfer.h"
#include "task_ADC.h"
#include "test_hh.h"
#include "task_Range.h"




float32 MdU16ToF32(Uint16 dataH, Uint16 dataL)
{
    Uint32 u32_tmp = 0;
    float32 f32_tmp = 0;

    u32_tmp = ((Uint32)dataH << 16) | dataL;
    f32_tmp = IEEETOF(u32_tmp);

    return f32_tmp;
}

void MdF32ToU16(float32 fdata, Uint16* pdataH, Uint16* pdataL)
{
    Uint32 u32_tmp = 0;

    u32_tmp = FTOIEEE(fdata);
    *pdataH = (u32_tmp >> 16) & 0x0000FFFF;
    *pdataL = u32_tmp & 0x0000FFFF;
}



void ModbusParaTransfer(ST_SCI_RX_PAR  *p)
{
    p->f32SetVol = MdU16ToF32(p->u16SetVolH, p->u16SetVolL);
    p->f32SetCur = MdU16ToF32(p->u16SetCurH, p->u16SetCurL);
    p->f32SetRangeCur = MdU16ToF32(p->SetRangeCurH, p->SetRangeCurL);
    p->f32SetRangeVol = MdU16ToF32(p->SetRangeVolH, p->SetRangeVolL);
    p->f32PluseWidth = MdU16ToF32(p->PluseWidthH, p->PluseWidthL);
    p->f32nPLC = MdU16ToF32(p->nPLCH, p->nPLCL);


    MdF32ToU16(p->f32DataVol, &p->u16DataVolH, &p->u16DataVolL);
    MdF32ToU16(p->f32DataCur, &p->u16DataCurH, &p->u16DataCurL);
    MdF32ToU16(p->f32SampData, &p->SampDataH, &p->SampDataL);
    MdF32ToU16(p->f32DataCurRealT, &p->u16DataCurRealTH, &p->u16DataCurRealTL);
    MdF32ToU16(p->f32DataVolRealT, &p->u16DataVolRealTH, &p->u16DataVolRealTL);
}



void ModbusParaLimit(ST_SCI_RX_PAR  *p)
{
    if(p->f32nPLC < 0.0001)     p->f32nPLC = 0.0001;

    //电压档位
    if(p->f32SetRangeVol < RANGE_VOL_20mV_RANGE)         p->SetRangeVol_index = RANGE_VOL_20mV_INDEX;
    else if(p->f32SetRangeVol < RANGE_VOL_200mV_RANGE)   p->SetRangeVol_index = RANGE_VOL_200mV_INDEX;
    else if(p->f32SetRangeVol < RANGE_VOL_2V_RANGE)      p->SetRangeVol_index = RANGE_VOL_2V_INDEX;
    else if(p->f32SetRangeVol < RANGE_VOL_20V_RANGE)     p->SetRangeVol_index = RANGE_VOL_20V_INDEX;
    else                                                 p->SetRangeVol_index = RANGE_VOL_200V_INDEX;

    p->f32SetRangeVol = rangeVol_buff[p->SetRangeVol_index];


    //电流档位
    if(p->f32SetRangeCur < 1.001e-9)        p->SetRangeCur_index = RANGE_CUR_1nA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-8)   p->SetRangeCur_index = RANGE_CUR_10nA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-7)   p->SetRangeCur_index = RANGE_CUR_100nA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-6)   p->SetRangeCur_index = RANGE_CUR_1uA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-5)   p->SetRangeCur_index = RANGE_CUR_10uA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-4)   p->SetRangeCur_index = RANGE_CUR_100uA_INDEX;
    else if(p->f32SetRangeCur < 2.001e-4)   p->SetRangeCur_index = RANGE_CUR_200uA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-3)   p->SetRangeCur_index = RANGE_CUR_1mA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-2)   p->SetRangeCur_index = RANGE_CUR_10mA_INDEX;
    else if(p->f32SetRangeCur < 1.001e-1)   p->SetRangeCur_index = RANGE_CUR_100mA_INDEX;
    else if(p->f32SetRangeCur < 2.001   )   p->SetRangeCur_index = RANGE_CUR_2A_INDEX;
    else                                    p->SetRangeCur_index = RANGE_CUR_10A_INDEX;


    p->f32SetRangeCur = rangeCur_buff[p->SetRangeCur_index];


    //设置电压、电流限制
    //f32SetVol_use 是正值
    if(p->f32SetVol > p->f32SetRangeVol)             p->f32SetVol_use = p->f32SetRangeVol;
    else if(p->f32SetVol > 0)                        p->f32SetVol_use = p->f32SetVol;
    else if(p->f32SetVol > -p->f32SetRangeVol)       p->f32SetVol_use = -p->f32SetVol;
    else                                             p->f32SetVol_use = p->f32SetRangeVol;

    if(p->f32SetCur > p->f32SetRangeCur)             p->f32SetCur_use = p->f32SetRangeCur;
    else if(p->f32SetCur > 0)                        p->f32SetCur_use = p->f32SetCur;
    else if(p->f32SetCur > -p->f32SetRangeCur)       p->f32SetCur_use = -p->f32SetCur;
    else                                             p->f32SetCur_use = p->f32SetRangeCur;
}










