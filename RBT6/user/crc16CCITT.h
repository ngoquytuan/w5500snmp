/**
 * @file	
 * @brief	
 * @version 
 * @date	16/Jan/2020
 * @par Revision
 *			
 * @author	tuannq
 * \n\n @par 
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CRC16_H
#define __CRC16_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
unsigned short CRC16_CCITT_calc(uint8_t input[], int length, unsigned short crc /*=0x0000*/) ;
#define bittest(D,i) ((D>>i) & 0x01 )


#endif /* __UART_H */

