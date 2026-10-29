/*
 * user_gpio.c
 *
 *  Created on: 2025年6月15日
 *      Author: m
 */

#include "COMMON.h"


// gpio 引脚初始化
void UserGpioInit(void)
{
	PS_GPIO_SetMode(IO_PS_PHY_RESET, OUTPUT, 0);
	PS_GPIO_SetMode(IO_PL_LED1, OUTPUT, 0);
	PS_GPIO_SetMode(IO_PL_LED2, OUTPUT, 0);
	PS_GPIO_SetMode(IO_PL_TRIGGER_IN, INPUT, 0);
	PS_GPIO_SetMode(IO_PL_TRIGGER_OUT, OUTPUT, 0);
	PS_GPIO_SetMode(IO_PL_PFC_EN, OUTPUT, 0);
	PS_GPIO_SetMode(IO_PL_PFC_DRIVE, OUTPUT, 0);
	PS_GPIO_SetMode(IO_PL_PFC_OK, INPUT, 0);
	PS_GPIO_SetMode(IO_PL_PFC_RVD, INPUT, 0);

	// 100 ms 后再把 phy_reset 拉高，手册 10ms
	usleep(100000);
	PS_GPIO_SetPort(IO_PS_PHY_RESET, 1);
	usleep(200000);
}
