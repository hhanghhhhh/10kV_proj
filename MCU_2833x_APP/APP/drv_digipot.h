#ifndef DRV_DIGIPOT_H
#define DRV_DIGIPOT_H

#include "TypeDefine.h"

#define DIGIPOT_STATUS_SUCCESS (0U)
#define DIGIPOT_STATUS_TIMEOUT (1U)

void Digipot_Init(void);
Uint16 Digipot_WriteTpl0501(Uint16 value);
Uint16 Digipot_WriteAd5290(Uint16 value);

#endif
