/*
 * PS_QSPI.h
 *
 *  Created on: 2025Äê8ÔÂ20ÈÕ
 *      Author: m
 */

#ifndef DRIVE_PS_QSPI_PS_QSPI_H_
#define DRIVE_PS_QSPI_PS_QSPI_H_

#include "TYPEDEF.h"

void PS_QSPI_Init(void);
Uint16 QspiUpdate(u32 total_bytes, const u8 *flash_data);



#endif /* DRIVE_PS_QSPI_PS_QSPI_H_ */
