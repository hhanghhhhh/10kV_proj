#ifndef _TASK_UPH_COMMON_H_
#define _TASK_UPH_COMMON_H_

#include <task_uph.h>

#define FUNCOODE_W      0
#define FUNCOODE_R      1

#define HcSourceMeter 3
#define HvSourceMeter 1
#define LvSourceMeter 2
#define Meter         0


typedef struct
{
    Uint8  u8_1;
    Uint8  u8_2;
    Uint8  u8_3;
    Uint8  u8_4;
}CONV_U8;

typedef union
{
    Uint32      u32;
    struct
    {
        Uint32 LL:8;
        Uint32 LH:8;
        Uint32 HL:8;
        Uint32 HH:8;
    }u8;
}CONV_F32;


#define  SELF_PROTOCOL_HEAD     5
typedef struct
{
    Uint16  funcode;
    Uint16  startaddr;
    Uint16  datanum;
    Uint16  recv_flag;
    Uint16  recv_len;
    Uint16  send_len;
}ST_SELF_PROTOCOL;

extern ST_SELF_PROTOCOL  st_selfprotocol;


#define     TYPE_ON         1
#define     TYPE_OFF        2
#define     TYPE_CHARGE     3
#define     TYPE_PULSE      4
#define     TYPE_READ       5


#define     ITEM_RDSON                      0
#define     ITEM_VSD                        1
#define     ITEM_IDSS                       2
#define     ITEM_IGSS                       3
#define     ITEM_VGSTH_PRE_CONDITION        4
#define     ITEM_VGSTH                      5
#define     ITEM_VBR                        6
#define     ITEM_GFS                        7


#define     IDEL            0
#define     SET_PARA        1
#define     TRIGGER         2
#define     RESULT          3
#define     NEXT_PARA       4
#define     SEND_RST        5
#define     MULTI_END       6
#define     WAIT_START      7
#define     PROCESS_PARA    8


#define     R_REMAIN_NUM         0
#define     R_DEVICE_NO          1
#define     R_STATE              2

#define     W_START              0
#define     W_TRIGGER            1
#define     W_STOP               2


#define     RST_OVERTIME          0     // 超时
#define     RST_OK                1     // ok
#define     RST_INVALID           2     // 无效上电类型
#define     RST_VOL_OVERRANGE     3     // 电压超限
#define     RST_CUR_OVERRANGE     4     // 电流超限
#define     RST_ERR               5     // 源保护
#define     RST_ASK               13
#define     RST_STOP              14
#define     RST_START             15

typedef struct
{
    struct
    {
        Uint16      total;                  //总动作数量
        Uint16      remain;                 //剩余动作数量
        Uint16      group_per[GROUP_NUM];   //每组参数有多少个触发动作
    }trigger_num;

    struct
    {
        Uint32      recv_cnt;         //接收符合格式的报文数量
        Uint16      recv_num_tmp;     //记录上位机每次下发的数量，跟总数量相等则判断接收完成
        Uint16      total_para_num;   //本地最多接收多少个参数，多了就超限了
        Uint16      recv_ok_num;      //计算下发多少个参数算ok
        Uint16      num_per_group;    //每组几个参数
        Uint16      para_ok_cnt;      //设置了几次参数
    }com;

    Uint16          group_index;        //参数组index
    Uint16          trigger_index;      //每组参数的动作index
    Uint16          state;
    Uint16          result_cnt;         //等待结果超时计数
    Uint16          overtime_ms;        //等待结果超时时间
    Uint16          result;             //结果ok标志
    Uint16          device_no;          //本机设备号
    Uint16          recv_ok;            //上位机下发完所有参数
}ST_UPH;

extern  ST_UPH     st_uph;


//与主控板交互
typedef struct
{
    Uint16          trigger;
    Uint16          start;
    Uint16          stop;
}ST_COM_CRTLCARD;

extern  ST_COM_CRTLCARD     st_com_ctrl;

#define     DEVICE_HC       0
#define     DEVICE_HV       1
#define     DEVICE_LV       2


extern void SetPara(Uint16 index);
extern Uint16 GetOverTimeMs(Uint16 index);
extern void TriggerFunc(Uint16 type);
extern Uint16 CheckResult(Uint16 type, Uint16 group_index);

extern void MultiModeFSM(void);
extern void Init_taskUPH(void);
extern void WriteMeterStart();
extern void WriteMeterStop();
extern void WriteMeterTrig();
extern Uint16 ReadMeterStatus();
extern Uint16 ReadMeterItem();
extern void SendRstOnce(Uint16 rst);

#endif
