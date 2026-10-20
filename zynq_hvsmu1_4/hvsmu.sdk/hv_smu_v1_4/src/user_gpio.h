/*
 * user_gpio.h
 *
 *  Created on: 2025年6月15日
 *      Author: m
 */

#ifndef SRC_USER_GPIO_H_
#define SRC_USER_GPIO_H_


#define	IO_PS_PHY_RESET		49
// EMIOn对应的GPIO编号为54+n，因为MIO总数为54，EMIO是从54开始算的
// 此处 0 - 3 分别代表哪个引脚是看 PL 的 top 文件中是如何分配的，和实际引脚如何对应
#define IO_PL_LED1 			(54 + 0) //EMIO_LED1为EMIO0，对应的GPIO编号为54+0=54
#define IO_PL_LED2 			(54 + 1) //EMIO_LED2为EMIO1，对应的GPIO编号为54+1=55
#define IO_PL_TRIGGER_IN 	(54 + 2)
#define IO_PL_TRIGGER_OUT 	(54 + 3)
#define IO_PL_PFC_EN 		(54 + 4)
#define IO_PL_PFC_DRIVE 	(54 + 5)
#define IO_PL_PFC_OK 		(54 + 6)
#define IO_PL_PFC_RVD 		(54 + 7)


void UserGpioInit(void);

#endif /* SRC_USER_GPIO_H_ */
