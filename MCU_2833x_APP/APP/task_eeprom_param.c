#include "task_eeprom_param.h"
#include "app_eeprom_config.h"
#include "app_boot_eeprom.h"
#include "drv_ModbusData.h"
#include "drv_GlobalVar.h"
#include "task_sampling.h"
#include "DSP2833x_Examples.h"

#define I2C_SUCCESS 0x0000
#define EEPROM_CMD_COEF_1 1
#define EEPROM_CMD_COEF_2 2
#define EEPROM_CMD_COMM_IP_1 100
#define EEPROM_CMD_COMM_IP_2 101

static void EepromConfig_SyncToRuntime(void)
{
    Uint16 i = 0;
    APP_EEPROM_USER_DATA *user = &g_app_eeprom_config.payload.user;

    mgmd_stSCIRx.ip.addr1 = g_app_boot_eeprom_param.ip[0];
    mgmd_stSCIRx.ip.addr2 = g_app_boot_eeprom_param.ip[1];
    mgmd_stSCIRx.ip.addr3 = g_app_boot_eeprom_param.ip[2];
    mgmd_stSCIRx.ip.addr4 = g_app_boot_eeprom_param.ip[3];

    for (i = 0; i < APP_EEPROM_CURRENT_CALIBRATION_NUM; i++)
    {
        mgmd_stSCIRx.coef.current_k[i] = user->current_calibration[i].k;
        mgmd_stSCIRx.coef.current_b[i] = user->current_calibration[i].b;
    }
}

static void EepromConfig_SyncFromRuntime(void)
{
    Uint16 i = 0;
    APP_EEPROM_USER_DATA *user = &g_app_eeprom_config.payload.user;

    for (i = 0; i < APP_EEPROM_CURRENT_CALIBRATION_NUM; i++)
    {
        user->current_calibration[i].k = mgmd_stSCIRx.coef.current_k[i];
        user->current_calibration[i].b = mgmd_stSCIRx.coef.current_b[i];
    }
}

static Uint16 EepromConfig_CheckCoef(void)
{
    // Uint16 i = 0;
    // float32 k;
    // float32 b;

    // for (i = 0; i < APP_EEPROM_CURRENT_CALIBRATION_NUM; i++)
    // {
    //     k = mgmd_stSCIRx.coef.current_k[i];
    //     b = mgmd_stSCIRx.coef.current_b[i];
    //     if (!((k >= 0.9F) && (k <= 1.1F)) || (b != b))
    //     {
    //         return EEROM_DATAERR_OVERRANG;
    //     }
    // }
    return 0;
}

/* 运行期写 boot 共享块可能耗时较长，写入期间暂停看门狗。 */
static Uint16 EepromParam_SaveBoot(void)
{
    Uint16 rst = 0;

    DisableDog();
    rst = AppBootEeprom_Save();
    EnableWDog();
    return rst;
}

/* 统一配置块包含多页数据，整个写入过程都需要暂停看门狗。 */
static Uint16 EepromParam_SaveConfig(void)
{
    Uint16 rst = 0;

    DisableDog();
    rst = AppEepromConfig_Save();
    EnableWDog();
    return rst;
}

void EepromParam_Init(void)
{
    AppBootEeprom_Init();
    AppEepromConfig_Init();
    EepromConfig_SyncToRuntime();
}

void EepromParam_Process(void)
{
    Uint16 rst = 0;

    if ((mgmd_stSCIRx.coef_mode1 == EEPROM_CMD_COMM_IP_1) &&
        (mgmd_stSCIRx.coef_mode2 == EEPROM_CMD_COMM_IP_2))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;
        g_app_boot_eeprom_param.ip[0] = mgmd_stSCIRx.ip.addr1;
        g_app_boot_eeprom_param.ip[1] = mgmd_stSCIRx.ip.addr2;
        g_app_boot_eeprom_param.ip[2] = mgmd_stSCIRx.ip.addr3;
        g_app_boot_eeprom_param.ip[3] = mgmd_stSCIRx.ip.addr4;
        rst = EepromParam_SaveBoot();
        mgmd_stSCIRx.ip.writeOk = (rst == I2C_SUCCESS) ? 1 : 0;
        return;
    }

    if ((mgmd_stSCIRx.coef_mode1 == EEPROM_CMD_COEF_1) &&
        (mgmd_stSCIRx.coef_mode2 == EEPROM_CMD_COEF_2))
    {
        mgmd_stSCIRx.coef_mode1 = 0;
        mgmd_stSCIRx.coef_mode2 = 0;
        if (EepromConfig_CheckCoef() != 0)
        {
            g_app_eeprom_status = EEROM_DATAERR_OVERRANG;
            return;
        }
        EepromConfig_SyncFromRuntime();
        rst = EepromParam_SaveConfig();
        EepromConfig_SyncToRuntime();
        if (rst == I2C_SUCCESS)
        {
            SamplingTask_RefreshCurrentCalibration();
        }
    }
}
