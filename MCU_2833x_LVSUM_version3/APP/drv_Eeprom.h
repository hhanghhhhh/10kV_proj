/*****************************************************************************************
*   NAME:           EEPROM.h
*   DESCRIPTION:    EEPROM header file
*   VERSION:        1.0
*   AUTHOR:         LuXi
*   ACTION:         build
*   DATE:           2013-3-22
******************************************************************************************/
#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "drv_GlobalVar.h"
#if DSP
    #include "DSP2833x_Device.h"     // Header file Include File
    #include "DSP2833x_Examples.h"   // Examples Include File
#endif


//sizeof(Uint16) = 1  28335��Uint16Ϊһ��������λ
//EEROM_PARA_LEN = 118 = 0x1E6
#define EEROM_PARA_START           0x0000
#define EEROM_PARA_LEN_NOCRC      (sizeof(ST_EEROM_PARA_STORED) - 2)
#define EEROM_PARA_LEN             sizeof(ST_EEROM_PARA_STORED)


#define EEROM_PARA11_START         0x0500
#define EEROM_PARA11_LEN_NOCRC    (sizeof(ST_EEROM_PARA11_STORED) - 2)
#define EEROM_PARA11_LEN           sizeof(ST_EEROM_PARA11_STORED)


#define EEROM_HEAD    0x4848
#define EEROM_TAIL    0x6868

#define EEROM_DATAOK             0x1212
#define EEROM_DATAERR_OVERRANG   0x1001    //1001 - 1008




#define  EEROM_PARA_V_NUM    (EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM + EEROM_V_DACP_NUM + \
                              EEROM_V_DACN_NUM                                         \
                             )
#define  EEROM_PARA_I_NUM    (EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM +                    \
                              EEROM_I_DACP_NUM + EEROM_I_DACN_NUM                      \
                             )

#define  EEROM_PARA_NUM      (EEROM_V_ADCP_NUM + EEROM_V_ADCN_NUM + EEROM_V_DACP_NUM + \
                              EEROM_V_DACN_NUM + EEROM_I_ADCP_NUM + EEROM_I_ADCN_NUM + \
                              EEROM_I_DACP_NUM + EEROM_I_DACN_NUM                      \
                             )
//这里改变后，需要注意modbus那边地址的数组也要改变。否则会越界
#define  EEROM_V_ADCP_NUM    5
#define  EEROM_V_ADCN_NUM    5
#define  EEROM_V_DACP_NUM    5
#define  EEROM_V_DACN_NUM    5
#define  EEROM_I_ADCP_NUM    10
#define  EEROM_I_ADCN_NUM    10
#define  EEROM_I_DACP_NUM    10
#define  EEROM_I_DACN_NUM    10


typedef struct
{
    Uint16 head[2];

    Uint16 v_adc_p_k_byte[EEROM_V_ADCP_NUM][4];
    Uint16 v_adc_n_k_byte[EEROM_V_ADCN_NUM][4];
    Uint16 v_dac_p_k_byte[EEROM_V_DACP_NUM][4];
    Uint16 v_dac_n_k_byte[EEROM_V_DACN_NUM][4];

    Uint16 i_adc_p_k_byte[EEROM_I_ADCP_NUM][4];
    Uint16 i_adc_n_k_byte[EEROM_I_ADCN_NUM][4];
    Uint16 i_dac_p_k_byte[EEROM_I_DACP_NUM][4];
    Uint16 i_dac_n_k_byte[EEROM_I_DACN_NUM][4];

    Uint16 v_adc_p_b_byte[EEROM_V_ADCP_NUM][4];
    Uint16 v_adc_n_b_byte[EEROM_V_ADCN_NUM][4];
    Uint16 v_dac_p_b_byte[EEROM_V_DACP_NUM][4];
    Uint16 v_dac_n_b_byte[EEROM_V_DACN_NUM][4];

    Uint16 i_adc_p_b_byte[EEROM_I_ADCP_NUM][4];
    Uint16 i_adc_n_b_byte[EEROM_I_ADCN_NUM][4];
    Uint16 i_dac_p_b_byte[EEROM_I_DACP_NUM][4];
    Uint16 i_dac_n_b_byte[EEROM_I_DACN_NUM][4];

    Uint16 tail[2];
    Uint16 crc[2];
}ST_EEROM_PARA_STORED;
extern ST_EEROM_PARA_STORED st_eerom_para_write, st_eerom_para_read;



typedef struct
{
    Uint16 eerom_ok;

    float32 f32v_adc_p_k[EEROM_V_ADCP_NUM];
    float32 f32v_adc_n_k[EEROM_V_ADCN_NUM];
    float32 f32v_dac_p_k[EEROM_V_DACP_NUM];
    float32 f32v_dac_n_k[EEROM_V_DACN_NUM];

    float32 f32i_adc_p_k[EEROM_I_ADCP_NUM];
    float32 f32i_adc_n_k[EEROM_I_ADCN_NUM];
    float32 f32i_dac_p_k[EEROM_I_DACP_NUM];
    float32 f32i_dac_n_k[EEROM_I_DACN_NUM];

    float32 f32v_adc_p_b[EEROM_V_ADCP_NUM];
    float32 f32v_adc_n_b[EEROM_V_ADCN_NUM];
    float32 f32v_dac_p_b[EEROM_V_DACP_NUM];
    float32 f32v_dac_n_b[EEROM_V_DACN_NUM];

    float32 f32i_adc_p_b[EEROM_I_ADCP_NUM];
    float32 f32i_adc_n_b[EEROM_I_ADCN_NUM];
    float32 f32i_dac_p_b[EEROM_I_DACP_NUM];
    float32 f32i_dac_n_b[EEROM_I_DACN_NUM];

}ST_EEROM_PARA;
extern ST_EEROM_PARA st_eerom_para;



typedef struct
{
    Uint16 head[2];
    Uint16 ip[4];
    Uint16 tail[2];
    Uint16 crc[2];
}ST_EEROM_PARA11_STORED;
extern ST_EEROM_PARA11_STORED st_eerom_para11_read;


typedef struct
{
    Uint16 eerom_ok;
    Uint16 ip[4];
}ST_EEROM_PARA11;
extern ST_EEROM_PARA11 st_eerom_para11;




extern  Uint16  RDE2ROM(Uint16 *RamAddr, Uint16 RomAddress, Uint16 number);
extern  Uint16  WRE2ROM(Uint16 *Wdata, Uint16 RomAddress, Uint16 number);
extern  Uint16  I2C_xrdy(void);
extern  Uint16  I2C_rrdy(void);


extern  void    D16TO8(Uint16 Source, Uint16 *Des);
extern  void    D32TO8(Uint32 Source, Uint16 *Des);
extern  Uint16  D8TO16(Uint16 *Source);
extern  Uint32  D8TO32(Uint16 *Source);
extern  float   IEEETOF(Uint32 Source);
extern  Uint32  FTOIEEE(float Source);

extern void InitEeromPara(ST_EEROM_PARA* pPara);
extern void WriteEeromPara(ST_EEROM_PARA_STORED* pWrite, ST_EEROM_PARA* pPara);
extern void ReadEeromPara(ST_EEROM_PARA_STORED* pRead, ST_EEROM_PARA* pPara);
extern void RecvAndWriteEerom(ST_EEROM_PARA* pPara);

extern Uint16 WriteEeromPara11(ST_EEROM_PARA11* pPara);
extern void InitEeromPara11(ST_EEROM_PARA11* pPara);
extern void RecvAndWriteEerom11(ST_EEROM_PARA11* pPara);
#endif
/***************************************************************************
*           END, do not code behind this line!!                            *
****************************************************************************/
