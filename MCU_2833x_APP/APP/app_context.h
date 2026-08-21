#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "drv_GlobalVar.h"
#include "drv_ad7982.h"
#include "drv_can_sample.h"

typedef struct
{
    Ad7982_Context_t ad7982;
    CanSample_Context_t can_sample;
    Uint16 can_sample_init_status;
} App_Context_t;

extern App_Context_t app_context;

#endif
