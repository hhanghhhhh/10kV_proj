#include "task_eeprom_download.h"
//=========================================================================================
// 参数存放在eeprom中
//=========================================================================================

Uint16 glb_download_flag = 0;

typedef void (*pFunction)(void);
#define BOOT_START_ADDR       ((Uint32)0x33FFF6)
void BOOT_JumpToBoot(Uint32 Addr)
{
    //  asm(" LB  0x310000"); //最开始采用的是汇编跳转方式,只能跳转为固定的地址
    // (*((void(*)(void))(Addr)))();  //后续参考STM32的跳转方式
    //(*(pFunction)(Addr))();
    pFunction jump;
    jump = (pFunction)(Addr);
    jump();
}
void load_boot(void)
{
    DINT;
    DRTM;
    DELAY_US(10000);
    BOOT_JumpToBoot(BOOT_START_ADDR);
}


void InitEeromPara_Downloads(void)
{
    //读eerom，读到就用eerom的数据，没读到就用默认的
    RDE2ROM(&glb_download_flag, EEROM_DOWNLOAD_FLAG, 1);
    DELAY_US(10000);
}


void WriteEeromPara_Downloads(Uint16 download_flag)
{

    DisableDog();
    glb_download_flag = download_flag;
    WRE2ROM(&glb_download_flag, EEROM_DOWNLOAD_FLAG, 1);
    DELAY_US(10000);
//    EnableWDog();
}






//===========================================================================
// No more.
//===========================================================================
