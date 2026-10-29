#ifndef _TASK_UPH_H_
#define _TASK_UPH_H_

#include "TYPEDEF.h"

#define  DEVICE_ID     HvSourceMeter

//#define     MAX(x, y)   ((x) > (y)) ? (x) : (y)
//#define     MIN(x, y)   ((x) > (y)) ? (y) : (x)


//参数组别
#define     GROUP_NUM       50
//每组参数的动作数量
#define     TRIGGER_NUM     2
//参数组的地址，也就是其余参数的数量
#define     GROUP_ADDR      1

typedef struct
{
    //接收上位机数据内存空间
    float32              group_num;
    struct
    {
        //-------正常参数---------//
        union
        {
            Uint32      u32;
            struct
            {
                Uint32      trigger_meter:1;
                Uint32      reserve2:1;
                Uint32      reserve3:1;
                Uint32      reserve4:1;

                Uint32      reserve:28;
            }bit;
        }mode;
        float32          setvol;
        float32          setcur;
        float32          plusewidth;
        float32          nplc;
        //-------额外参数---------//
        float32          judge_vol_min;
        float32          judge_vol_max;
        float32          judge_cur_min;
        float32          judge_cur_max;
        float32          trigger[TRIGGER_NUM];   //0代表空的  不执行
        float32          item;   //测试项
    }para_group[GROUP_NUM];
}ST_FROM_PC;

extern  ST_FROM_PC     st_frompc;

typedef struct
{
    // 地址1
    union
    {
        Uint32      u32;
        struct
        {
            Uint32      group_num:8;
            Uint32      reserve:24;
        }bit;
    }info;

    //地址2往后
    struct
    {
        float32          vol;
        float32          cur;
    }fbk_data[GROUP_NUM];
}ST_TO_PC;

extern  ST_TO_PC     st_topc;


//与主控板交互结果
//从3开始，最大到12

extern void InitPara(void);


#endif
