#include "COMMON.h"
#include "drv_ModbusData.h"
#include <string.h>
#include <task_can_ctrlboard.h>
#include <task_uph_common.h>

void InitPara(void)
{
    ST_FROM_PC* pFrompc = &st_frompc;
//    pFrompc->group_num = 1;
//    pFrompc->para_group[0].item = ITEM_IDSS;
//    pFrompc->para_group[0].mode.bit.trigger_meter = 0;
//    pFrompc->para_group[0].setvol = 200;
//    pFrompc->para_group[0].setcur = 0.1;
//    pFrompc->para_group[0].plusewidth = 0.1;
//    pFrompc->para_group[0].nplc = 1;
//    pFrompc->para_group[0].trigger[0] = 1;
//    pFrompc->para_group[0].judge_vol_min = 0;
//    pFrompc->para_group[0].judge_vol_max = 300;
//    pFrompc->para_group[0].judge_cur_min = -0.1;
//    pFrompc->para_group[0].judge_cur_max = 0.1;
}

void SetPara(Uint16 index)
{
    ST_FROM_PC* pFrompc = &st_frompc;
    ST_SCI_RX_PAR* pMdbus = &mgmd_stSCIRx;

    //下发参数
    pMdbus->v_set.f32           = pFrompc->para_group[index].setvol;
    pMdbus->i_set.f32           = pFrompc->para_group[index].setcur;
    pMdbus->v_range.f32         = pFrompc->para_group[index].setvol;	// range 与 set 一致
    pMdbus->i_range.f32         = pFrompc->para_group[index].setcur;
    pMdbus->plusewidth.f32      = pFrompc->para_group[index].plusewidth;
    pMdbus->nplc.f32            = pFrompc->para_group[index].nplc;
    pMdbus->trigger_meter       = pFrompc->para_group[index].mode.bit.trigger_meter;
    pMdbus->item_num            = pFrompc->para_group[index].item;
    pMdbus->relay_on_always		= 1;	// 源表分离时输出继电器常开

    if(pMdbus->item_num == ITEM_VBR)    	 pMdbus->test_mode = MODE_VBR;
    else if(pMdbus->item_num == ITEM_IDSS)   pMdbus->test_mode = MODE_UPH;
    else									 pMdbus->test_mode = MODE_NORMAL;
}

Uint16 GetOverTimeMs(Uint16 index)
{
    ST_FROM_PC* pFrompc = &st_frompc;

    Uint16 overtime_ms = 0;
    if(pFrompc->para_group[index].plusewidth > 0)
    {
        overtime_ms = pFrompc->para_group[index].plusewidth * 1000 + 500;
    }
    else
    {
        overtime_ms = 1000;
    }

    return overtime_ms;
}

void TriggerFunc(Uint16 type)
{
    ST_SCI_RX_PAR* pMdbus = &mgmd_stSCIRx;

    if(type == TYPE_ON)
    {
        pMdbus->power_ctrl = 1;
    }
    else if(type == TYPE_OFF)
    {
        pMdbus->power_ctrl = 2;
    }
}

extern Uint16 pwon_flag;
//记录结果并给主控板返回结果判断ok标志
Uint16 CheckResult(Uint16 type, Uint16 group_index)
{
    ST_SCI_RX_PAR* pMdbus = &mgmd_stSCIRx;
    ST_FROM_PC* pFrompc = &st_frompc;
    ST_TO_PC*   pTopc = &st_topc;

    float32  set_vol = 0, vol_min = 0, vol_max = 0;

    if(type == TYPE_ON)
    {
        if(pMdbus->plusewidth.f32 > 0)
        {
            if(pMdbus->dataok == 1)
            {
                if(pwon_flag == 0)
                {
                    //记录反馈值
                    pTopc->fbk_data[group_index].vol = pMdbus->vsamp_avg.f32;
                    pTopc->fbk_data[group_index].cur = pMdbus->isamp_avg.f32;

                    //判断反馈值是否符合标准
                    if(pMdbus->vsamp_avg.f32 > pFrompc->para_group[group_index].judge_vol_max
                            || pMdbus->vsamp_avg.f32 < pFrompc->para_group[group_index].judge_vol_min)
                    {
                        return RST_VOL_OVERRANGE;
                    }
                    else if(pMdbus->isamp_avg.f32 > pFrompc->para_group[group_index].judge_cur_max
                            || pMdbus->isamp_avg.f32 < pFrompc->para_group[group_index].judge_cur_min)
                    {
                        return RST_CUR_OVERRANGE;
                    }
                    else
                    {
                        return RST_OK;
                    }
                }
            }
            else if(pMdbus->dataok != 0)
            {
                // 报错
                return RST_ERR;
            }
        }
        else
        {
            //仅上电或者有脉宽不采样，一般这种情况是设置电压，仅判断电压满足要求
            //判断实时值与设置值比较则ok
            set_vol = pMdbus->v_set.f32;
//            vol_min = MIN(0.95*set_vol, 1.05*set_vol);
//            vol_max = MAX(0.95*set_vol, 1.05*set_vol);
            vol_min = pFrompc->para_group[group_index].judge_vol_min;
            vol_max = pFrompc->para_group[group_index].judge_vol_max;
            //此处为一直判断，只有这个条件满足时才ok，否则时间到则返回超时
            if(pMdbus->vsamp.f32 > vol_min && pMdbus->vsamp.f32 < vol_max)
                return RST_OK;
        }
    }
    else if(type == TYPE_OFF)
    {
        //等完全下电ok，返回ok
        if(pMdbus->vsamp.f32 < 0.5 && pMdbus->vsamp.f32 > -0.5)
        {
            return RST_OK;
        }
    }
    else
    {
        //不存在该触发类型
        return RST_INVALID;
    }

    return RST_OVERTIME;
}


