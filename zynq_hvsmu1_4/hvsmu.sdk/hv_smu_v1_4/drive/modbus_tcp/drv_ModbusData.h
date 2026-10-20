#ifndef _DRV_MODBUSDATA_H_
#define _DRV_MODBUSDATA_H_

#include "TYPEDEF.h"

/**************************************************************************************************
CONSTANTS AND MACROS
**************************************************************************************************/
#define MD_R1_START_ADDR (0x1000)
#define MD_R1_END_ADDR (0x101F)
#define MD_R1_SIZE (MD_R1_END_ADDR - MD_R1_START_ADDR + 1)

#define MD_R2_START_ADDR (0x1100)
#define MD_R2_END_ADDR (0x113F)
#define MD_R2_SIZE (MD_R2_END_ADDR - MD_R2_START_ADDR + 1)

#define MD_W1_START_ADDR (0x2000)
#define MD_W1_END_ADDR (0x203F)
#define MD_W1_SIZE (MD_W1_END_ADDR - MD_W1_START_ADDR + 1)

#define MD_RW1_START_ADDR (0x3000)
#define MD_RW1_END_ADDR (0x3019)
#define MD_RW1_SIZE (MD_RW1_END_ADDR - MD_RW1_START_ADDR + 1)

#define MD_RW2_START_ADDR (0x5000)
#define MD_RW2_END_ADDR (0x5200)
#define MD_RW2_SIZE (MD_RW2_END_ADDR - MD_RW2_START_ADDR + 1)

#define MD_RW3_START_ADDR (0x5300)
#define MD_RW3_END_ADDR (0x5350)
#define MD_RW3_SIZE (MD_RW3_END_ADDR - MD_RW3_START_ADDR + 1)

#define MD_RW4_START_ADDR (0x7000)
#define MD_RW4_END_ADDR (0x7100)
#define MD_RW4_SIZE (MD_RW4_END_ADDR - MD_RW4_START_ADDR + 1)
/**************************************************************************************************
TYPEDEFS
**************************************************************************************************/
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

    // read
    Uint16 dataok;
    Uint16 pw_can_flag; /* 0执行中，1成功，2发送失败，3回复超时 */

    Uint16 highCmode;
    Uint16 power_ctrl;
    Uint16 test_mode;
    MODBUS_DATA v_range;
    MODBUS_DATA i_range;
    MODBUS_DATA v_set;
    MODBUS_DATA i_set;
    MODBUS_DATA nplc;
    MODBUS_DATA plusewidth;
    struct
    {
        Uint16 enable;
        Uint16 trigger;
        Uint16 ok;
        Uint16 time_ms;
        Uint16 filter_en;
        Uint16 cur_coef_en;
        Uint16 vol_coef_en;
    } wave;

    // read
    MODBUS_DATA isamp;
    MODBUS_DATA vsamp;
    MODBUS_DATA isamp_avg;
    MODBUS_DATA vsamp_avg;
    MODBUS_DATA adc_vol_avg;
    MODBUS_DATA adc_cur_avg;
    MODBUS_DATA adc_vol_full;
    MODBUS_DATA adc_cur_full;
    Uint16 dac_vol_ad;
    Uint16 dac_cur_ad;
    Uint16 autorangei;
    Uint16 autorangev;
    Uint16 reserved_200A;
    Uint16 dis100nA;
    Uint16 cmd_autozero;
    Uint16 autozero_okflag;

    Uint16 sw_delay_ms;
    Uint16 delay_ref;
    Uint16 delay_discharge;
    Uint16 relay_on_always;
    Uint16 auto_discharge;
    Uint16 auto_discharge_delay; // 到 2V 后 delay

    Uint16 test_idac_en;
    Uint16 test_idac_a0;
    Uint16 test_idac_a1;
    Uint16 test_vdac_en;
    Uint16 test_vdac_a0;
    Uint16 test_vdac_a1;

    Uint16 dis_main_power;
    // 源表分离
    Uint16 trigger_meter;
    Uint16 item_num;

    union
    {
        Uint32 u32;
        struct
        {
            Uint16 L;
            Uint16 H;
        } u16;
    } rx_err_cnt;

    struct
    {
        Uint16 disable_all_prot;
    } cfg;
    Uint16 cfgvalue[40];

    Uint16 coef_mode1;
    Uint16 coef_mode2;
    // Ip addr
    struct
    {
        Uint16 addr1;
        Uint16 addr2;
        Uint16 addr3;
        Uint16 addr4;
        Uint16 writeOk;
    } ip;

    struct
    {
        union
        {
            Uint16 u16;
            struct
            {
                Uint16 vadc : 1;
                Uint16 vdac : 1;
                Uint16 iadc : 1;
                Uint16 idac : 1;
                Uint16 reserve : 12;
            } bit;
        } enable;
        MODBUS_DATA f32v_adc_p_k[100];
        MODBUS_DATA f32v_adc_p_b[100];

        union
        {
            Uint32 u32;
            struct
            {
                Uint16 L;
                Uint16 H;
            } u16;
        } utctime, sn_no[3];
    } coef;

    struct
    {
        Uint16 data[120];
        Uint16 frame_type;
        Uint16 crc;
        Uint16 ok;
        union
        {
            Uint32 u32;
            struct
            {
                Uint16 l;
                Uint16 h;
            } u16;
        } byte_num;
        Uint16 write_flash;
        Uint16 reset;
    } download;

    Uint16 reserve;
} ST_SCI_RX_PAR;
extern ST_SCI_RX_PAR mgmd_stSCIRx;

extern Uint16 *md_r1_buf[];
extern Uint16 *md_r2_buf[];
extern Uint16 *md_rw1_buf[];
extern Uint16 *md_rw2_buf[];
extern Uint16 *md_rw3_buf[];
extern Uint16 *md_rw4_buf[];
extern Uint16 *md_w1_buf[];

void ModbusDataRegister(void);

#endif

/**************************************************************************************************
 END OF FILE (EOF)
**************************************************************************************************/
