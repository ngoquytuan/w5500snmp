/**
 * @file	ioconfig.c
 * @brief	Cau hinh GPIO WIZ550web
 * @version 1.0
 * @date	03/Sep/2019
 * @par Revision
 *			03/Sep/2019 - 1.0 Release
 * @author	tuannq
 * \n\n @par 
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define _USE_XFUNC_OUT	1	/* 1: Use output functions */
#define	_CR_CRLF		1	/* 1: Convert \n ==> \r\n in the output char */

#define _USE_XFUNC_IN	1	/* 1: Use input function */
#define	_LINE_ECHO		1	/* 1: Echo back input chars in xgets function */
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void USART1_Init(void);
void USART1_SendStr_(char *str);
void USART2_Init(void);
void USART2_SendStr_(char *str);
void USER_UART_NVIC(void);
void USART1_SendChar_(char ch);
void USART2_SendChar_(char ch);
void u2Transmit(char *str,char length);


void printmcuclk(void);


#endif /* __UART_H */

