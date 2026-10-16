#include "Version.h"
#include <string.h>
#include "drv_fpga.h"
#include "task_ComFPGA.h"

ST_VERSION st_version;

void Init_Version(ST_VERSION *p)
{

    memset(p, 0, sizeof(ST_VERSION));

    p->DSPversion = 254;     // DSP 版本
    p->FPGAversion_std = 21; // 相匹配的FPGA版本
    p->device_num = DEVICE_LOWVOL2_0;
}

void Run_Version(ST_VERSION *p)
{
    p->FPGAversion = st_from_fpga.fpga_info.bit.version;
    p->version = (p->FPGAversion << 8) + p->DSPversion;
    p->FPGArunflag = (p->FPGAversion >= 20 && p->FPGAversion <= 30) ? 1 : 0;
}
