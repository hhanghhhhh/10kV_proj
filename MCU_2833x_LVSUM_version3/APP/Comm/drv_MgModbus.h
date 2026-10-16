/**************************************************************************************************
Project:  MVPS
Filename:   drv_MgModbus.h
Description:
Complier: CCS V3.3, TI.
***************************************************************************************************
Author:   WANG DAN
Created on:  2016��5��30��
Revision:
***************************************************************************************************
**************************************************************************************************/

/**************************************************************************************************
COMPILER DIRECTIVES
**************************************************************************************************/
#ifdef _MG_MODBUS_C_
#define _MG_MODBUS_EXT
#else
#define _MG_MODBUS_EXT extern
#endif

#include "TypeDefine.h"

#include "drv_Eeprom.h"

// 选择使用modbusTCP或者modbusRTU格式
#define MODBUSTCP 1

/**************************************************************************************************
CONSTANTS AND MACROS
**************************************************************************************************/

#define MGMD_FRAME_LENGTH_MIN (8)

#if MODBUSTCP
#define MGMD_MODBUS_DATA_OFFSET_MACRO (13)
#else
#define MGMD_MODBUS_DATA_OFFSET_MACRO (7)
#endif

#define MGMD_MODBUS_FUN_READ_MACRO (0x03)
// #define MGMD_MODBUS_FUN_READ_ONLY_MACRO (0x04)
#define MGMD_MODBUS_FUN_WRITE_SINGLE_MACRO (0x06)
#define MGMD_MODBUS_FUN_WRITE_MULT_MACRO (0x10)

#define MGMD_MODBUS_RXTX_BUFFER_SIZE_MACRO (300)

/**************************************************************************************************
TYPEDEFS
**************************************************************************************************/
// #define con_MAX_COMMAND_LENGTH 	35
// #define con_CHAR_ENTER 			13
/**************************************************************************************************
STRUCTURE TYPEDEFS
**************************************************************************************************/
#define SCIB_TX_DIS (ScibRegs.SCICTL1.bit.TXENA = 0)
#define SCIB_RX_DIS (ScibRegs.SCICTL1.bit.RXENA = 0)

#define SCIB_TX_ENABLE (ScibRegs.SCICTL1.bit.TXENA = 1)
#define SCIB_RX_ENABLE (ScibRegs.SCICTL1.bit.RXENA = 1)

#define SCIB_TX_READY ScibRegs.SCICTL2.bit.TXRDY   // 1=READY
#define SCIB_TX_EMPTY ScibRegs.SCICTL2.bit.TXEMPTY // 1=EMPTY

#define SCIB_RX_READY ScibRegs.SCIRXST.bit.RXRDY // 1 ready

#define SCIB_RECEIVE_DATA_BUF (ScibRegs.SCIRXBUF.bit.RXDT)
#define SCIB_SEED_DATA_BUF ScibRegs.SCITXBUF // bit 0~7
/**************************************************************************************************
EXPORTED VARIABLES
**************************************************************************************************/
#define MODE_NORMAL 0    // 0正常     正常自动挡位从大往小切不到500nA挡位， 若初始挡位是500nA挡位，则仅作为筛管使用，一开始就把挡位切好
#define MODE_VBR 1       // 击穿电压模式
#define MODE_PRECISION 2 // 精确模式

#define MODE_TEST 123 // 测试模式

#define IN_APP 1
#define IN_BOOT 2
#define JUMP_TO_APP 1
#define JUMP_TO_BOOT 2
typedef union
{
    float32 f32;
    struct
    {
        Uint16 L;
        Uint16 H;
    } u16;
} MODBUS_DATA;

typedef struct
{
    Uint16 test_state;
    Uint16 err_code;

    Uint16 gs_or_ds;
    Uint16 up_or_down;
    Uint16 poweron;
    Uint16 ok;
    struct
    {
        Uint16 four_or_two;
        struct
        {
            MODBUS_DATA vset;
            MODBUS_DATA iset;
            MODBUS_DATA plusewidth;
            MODBUS_DATA vsamp;
            MODBUS_DATA isamp;
        } pre;
        struct
        {
            MODBUS_DATA vmax;
            MODBUS_DATA vmin;
            MODBUS_DATA iset;
            MODBUS_DATA delta_v;
            MODBUS_DATA delta_t;
            MODBUS_DATA vsamp;
            MODBUS_DATA isamp;
        } step;
    } gs;
    struct
    {
        Uint16 four_or_two;
        MODBUS_DATA delayms;
        MODBUS_DATA vset;
        MODBUS_DATA iset;
        MODBUS_DATA vth;
        MODBUS_DATA vsamp;
        MODBUS_DATA isamp;
    } ds;
} VTH_DOUBLESMU;

typedef struct
{
    Uint16 state;
    Uint16 ok;
    Uint16 pre_num;
    //    Uint16 four_or_two;
    Uint16 poweron;
    struct
    {
        MODBUS_DATA vset;
        MODBUS_DATA iset;
        MODBUS_DATA plusewidth;
        MODBUS_DATA delay;
        MODBUS_DATA vsamp;
        MODBUS_DATA isamp;
    } pre[8];
    struct
    {
        MODBUS_DATA vset;
        MODBUS_DATA iset;
        MODBUS_DATA plusewidth;
        MODBUS_DATA nplc;
        MODBUS_DATA vsamp;
        MODBUS_DATA isamp;
    } vth;
} VTH_8PRE;

typedef struct
{
    // write
    // 命令

    // 参数
    Uint16 SampleNum_I; // 1PLC = 20ms  1000个点
    Uint16 SampleNum_V; // 1PLC = 400个点

    Uint16 Mode; // 0正常     正常自动挡位从大往小切不到500nA挡位， 若初始挡位是500nA挡位，则仅作为筛管使用，一开始就把挡位切好
                 // 1雪崩模式
                 // 2精确模式 --500nA挡位精确测量模式，需要等1.5s  支持自动挡从大切到500nA挡位
                 // 123调试模式，不保护，也可切到500nA精确模式
                 //

    // set para
    Uint16 u16SetVolH;
    Uint16 u16SetVolL;
    Uint16 u16SetCurH;
    Uint16 u16SetCurL;

    float32 f32SetVol;
    float32 f32SetCur;
    float32 f32SetVol_use;
    float32 f32SetCur_use;

    Uint16 autorange; // measure 的是否autorange
    Uint16 Outmode;   // 1:v 2:I
    Uint16 highCmode;
    Uint16 Testmode; //  1:漏电流，先用大电流挡位充电一段时间，再切换小挡位测试电流
    Uint16 testflag; //  1: 不进行一个负电流/10的操作

    Uint16 SetRangeVolH; // 档位参数必须要放在设置电流电压值的前边
    Uint16 SetRangeVolL;
    Uint16 SetRangeCurH;
    Uint16 SetRangeCurL;
    float32 f32SetRangeVol;
    float32 f32SetRangeCur;
    Uint16 SetRangeVol_index;
    Uint16 SetRangeCur_index;

    Uint16 Autosample;
    Uint16 PluseWidthH;
    Uint16 PluseWidthL;
    float32 f32PluseWidth;

    Uint16 nPLCH;
    Uint16 nPLCL;
    float32 f32nPLC;

    MODBUS_DATA pw3_initial_cur;
    MODBUS_DATA pw3_initial_delay_s;

    // Vgsth特殊需求，需要连续测两次，第一次precondition，第二次vgsth
    // 第一次precondition的参数
    Uint16 vth_outmode;
    Uint16 vth_four_two;
    Uint16 vth_poweron;
    Uint16 vth_ok;
    MODBUS_DATA vth_setvol1;
    MODBUS_DATA vth_setcur1;
    MODBUS_DATA vth_volrange1;
    MODBUS_DATA vth_currange1;
    MODBUS_DATA vth_plusewidth1;
    MODBUS_DATA vth_nplc1;
    MODBUS_DATA vth_setvol2;
    MODBUS_DATA vth_setcur2;
    MODBUS_DATA vth_volrange2;
    MODBUS_DATA vth_currange2;
    MODBUS_DATA vth_plusewidth2;
    MODBUS_DATA vth_nplc2;
    MODBUS_DATA vth_forward_samp;
    MODBUS_DATA vth_volsamp1;
    MODBUS_DATA vth_volsamp2;
    MODBUS_DATA vth_cursamp1;
    MODBUS_DATA vth_cursamp2;

    VTH_DOUBLESMU vth_dbsmu;
    VTH_8PRE vth_8pre;

    // read
    // 状态
    Uint16 DataOKFlag;  // 数据ok
    Uint16 pw_can_flag; /* 0执行中，1成功，2发送失败，3回复超时 */

    // 数据
    Uint16 SampDataH; // 采样值转化为电压值（满码值对应5V）
    Uint16 SampDataL; // 采样值转化为电压值（满码值对应5V）
    float32 f32SampData;

    // 采样平均值
    Uint16 u16DataVolH;
    Uint16 u16DataVolL;
    Uint16 u16DataCurH;
    Uint16 u16DataCurL;

    float32 f32DataVol;
    float32 f32DataCur;

    // 采样实时值
    Uint16 u16DataVolRealTH;
    Uint16 u16DataVolRealTL;
    Uint16 u16DataCurRealTH;
    Uint16 u16DataCurRealTL;

    float32 f32DataVolRealT;
    float32 f32DataCurRealT;

    float32 f32DataVolRealT_5v;
    float32 f32DataCurRealT_5v;

    Uint16 u16setVol5VH;
    Uint16 u16setVol5VL;
    Uint16 u16setCur5VH;
    Uint16 u16setCur5VL;

    float32 f32setVol5V;
    float32 f32setCur5V;

    // 错误码
    Uint16 u16FPGA_state;       // FPGA state
    Uint16 u16FPGA_err_flag;    // FPGA err flag
    Uint16 u16FPGA_err_code_hi; // FPGA err code
    Uint16 u16FPGA_err_code_lo; // FPGA err code

    // CMD
    Uint16 CMD_RANGE_1uA;
    Uint16 CMD_RANGE_10uA;
    Uint16 CMD_RANGE_100uA;
    Uint16 CMD_RANGE_1mA;
    Uint16 CMD_RANGE_10mA;
    Uint16 CMD_RANGE_100mA;
    Uint16 CMD_RANGE_1A;
    Uint16 CMD_RANGE_100nA;
    Uint16 CMD_Current_SW_P1A;
    Uint16 CMD_Current_SW_N1A;
    Uint16 CMD_Current_Protect_1A;
    Uint16 CMD_Current_Sense_1A;
    Uint16 CMD_EN_235P;
    Uint16 CMD_EN_235N;
    Uint16 CMD_DAC_Vset_1_n;
    Uint16 CMD_DAC_Vset_01_n;
    Uint16 CMD_DAC_Iset_1_n;
    Uint16 CMD_DAC_Iset_01_n;
    Uint16 CMD_IGAINA1;
    Uint16 CMD_IGAINA0;
    Uint16 CMD_VDIV200;
    Uint16 CMD_VDIV10;
    Uint16 CMD_V_GAIN10;
    Uint16 CMD_VGAINA1;
    Uint16 CMD_VGAINA0;
    Uint16 CMD_OUT_EN1;
    Uint16 CMD_LOWISENSE_N;
    Uint16 CMD_PRIORITY;
    Uint16 CMD_POLARITY;
    Uint16 CMD_SW_O;
    Uint16 CMD_SW_F;
    Uint16 CMD_ControlLoop_BW1_n;
    Uint16 CMD_ControlLoop_BW2_n;
    Uint16 CMD_ControlLoop_BW3_n;
    Uint16 CMD_ControlLoop_BW4_n;
    Uint16 CMD_GUARDEN;
    Uint16 CMD_SNSCONTROL_n;
    Uint16 CMD_AFBCONTROL;
    Uint16 CMD_AFB_SAMPLE_N;
    Uint16 CMD_AFBRESET;
    Uint16 CMD_AFB_ADDLB_N;
    Uint16 CMD_Control_VEN;
    Uint16 CMD_Control_IEN;
    Uint16 POWER_CTRL;
    Uint16 adjustflag; // 0  正常使用系数        1  DAC系数为1   2 ADC系数为1    3 两个系数都为1
    Uint16 adjust_adc_v;
    Uint16 adjust_adc_i;
    Uint16 adjust_dac_v;
    Uint16 adjust_dac_i;
    // ADC
    Uint16 ADC_mux;     // 上位机下发1-16   dsp向FPGA下发0-15
    Uint16 ADC_trigger; // ADC采样一次

    // 验证 trigger 信号是否正确
    Uint16 test_trigger_mode;
    Uint16 test_set_out;
    Uint16 test_read_in;

    // DAC
    Uint16 CMD_VDAC_UPDATE;
    Uint16 CMD_IDAC_UPDATE;
    Uint16 CMD_AVMADAC_UPDATE;
    Uint16 AVMA_A_data;
    Uint16 AVMA_B_data;
    // 版本号
    Uint16 com_Version;

    // Ip addr
    Uint16 ip_addr1;
    Uint16 ip_addr2;
    Uint16 ip_addr3;
    Uint16 ip_addr4;

    Uint16 IPwriteOK;

    Uint16 program_type;
    Uint16 jump_cmd; //  1--boot->app  2--app->boot

    // 系数
    Uint16 write_ok;
    Uint16 coef_mode1;
    Uint16 coef_mode2;
    // 首地址
    Uint16 v_adc_p_k_H[EEROM_PARA_NUM];
    Uint16 v_adc_p_k_L[EEROM_PARA_NUM];

    Uint16 v_adc_p_b_H[EEROM_PARA_NUM];
    Uint16 v_adc_p_b_L[EEROM_PARA_NUM];

    float32 f32v_adc_p_k[EEROM_PARA_NUM];
    float32 f32v_adc_p_b[EEROM_PARA_NUM];

    Uint16 reserve;

} ST_SCI_RX_PAR;
extern ST_SCI_RX_PAR mgmd_stSCIRx;

extern Uint16 mgmd_u16ModbusFunCode;
extern Uint16 mgmd_u16ModbusAddr;
extern Uint16 mgmd_u16ModbusCrc;
extern Uint16 mgmd_u16ModbusRegNum;

extern Uint16 mgmd_u16SlaveID;
extern Uint16 mgmd_u16RxReady;
extern Uint16 mgmd_u16Urt1BytesReceived;
extern Uint16 mgmd_u16Urt1TxBuf[];
extern Uint16 mgmd_u16Urt1RxBuf[];

extern Uint32 modbusTCP_rcv_cnt;
// extern Bool   Mgmd_bOscilloscopeTxEnable;

/**************************************************************************************************
INPUT BUFFER ALLOCATION
**************************************************************************************************/

/**************************************************************************************************
FLAG DEFINITIONS
**************************************************************************************************/

/**************************************************************************************************
INPUT BUFFER ALLOCATION
**************************************************************************************************/

/**************************************************************************************************
EXPORTED FUNCTIONS
**************************************************************************************************/
extern void mgmd_vInit(void);
extern void mgmd_vPollTb1(void);
/**************************************************************************************************
LOCAL FUNCTIONS
**************************************************************************************************/
#ifdef _MG_MODBUS_C_

#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
